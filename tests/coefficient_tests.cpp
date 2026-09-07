#include "coefficient_golden.h"
#include "resample_test_helpers.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace vc_test {
namespace {
using namespace vc::resample;

std::unique_ptr<ResamplingFunction> DefaultFunction(FilterKind kind) {
  for (const auto& golden : kMathGolden)
    if (golden.kind == kind)
      return MakeFunction(golden);
  return nullptr;
}

class CoefficientPlans : public testing::TestWithParam<CoefficientGolden> {};
TEST_P(CoefficientPlans, MatchesOriginalOffsetsSizesAndWeights) {
  const auto& golden = GetParam();
  const auto function = DefaultFunction(golden.kind);
  ASSERT_NE(function, nullptr);
  const auto plan = BuildCoefficients(*function, golden.request);
  EXPECT_EQ(plan.filter_size, golden.filter_size);
  EXPECT_EQ(plan.filter_size_real, golden.filter_size_real);
  ASSERT_EQ(plan.offsets, golden.offsets);
  ASSERT_EQ(plan.sizes, golden.sizes);
  size_t active = 0;
  for (int i = 0; i < plan.target_size; ++i) {
    ASSERT_GE(plan.offsets[i], 0);
    ASSERT_GE(plan.sizes[i], 1);
    ASSERT_LE(plan.offsets[i] + plan.sizes[i], plan.source_size);
    for (int k = 0; k < plan.filter_size; ++k) {
      const auto index = size_t(i) * plan.filter_size + k;
      const double expected = k < plan.sizes[i] ? golden.coefficients.at(active++) : 0;
      if (plan.bits_per_sample == 32)
        EXPECT_NEAR(plan.floats[index], expected,
                    4 * std::numeric_limits<float>::epsilon() * std::max(1.0, std::abs(expected)));
      else
        EXPECT_EQ(plan.integers[index], expected);
    }
  }
  EXPECT_EQ(active, golden.coefficients.size());
}

TEST_P(CoefficientPlans, RepeatedPreparationHasNoMutableFunctionState) {
  const auto& golden = GetParam();
  const auto function = DefaultFunction(golden.kind);
  ASSERT_NE(function, nullptr);
  const auto first = BuildCoefficients(*function, golden.request);
  (void)function->f(1.25);
  const auto second = BuildCoefficients(*function, golden.request);
  EXPECT_EQ(first.offsets, second.offsets);
  EXPECT_EQ(first.sizes, second.sizes);
  EXPECT_EQ(first.integers, second.integers);
  EXPECT_EQ(first.floats, second.floats);
}

INSTANTIATE_TEST_SUITE_P(OriginalPlans, CoefficientPlans, testing::ValuesIn(kCoefficientGolden),
                         [](const testing::TestParamInfo<CoefficientGolden>& info) { return info.param.name; });

TEST(CoefficientContract, PointIgnoresPixelCenterShift) {
  const PointFilter point;
  auto request = CoefficientRequest{8, 4, 0, 8, 8, 0.5, 0.5};
  const auto centered = BuildCoefficients(point, request);
  request.source_center = -12.5;
  request.destination_center = 27.5;
  const auto shifted = BuildCoefficients(point, request);
  EXPECT_EQ(centered.offsets, (std::vector<int>{0, 2, 4, 6}));
  EXPECT_EQ(centered.offsets, shifted.offsets);
  EXPECT_EQ(centered.integers, (std::vector<int16_t>{16384, 16384, 16384, 16384}));
}

TEST(CoefficientContract, DifferentialQuantizationKeepsConstantIntegerSamples) {
  for (int bits : {8, 9, 10, 12, 14, 16}) {
    const LanczosFilter lanczos(6);
    const auto plan = BuildCoefficients(lanczos, {23, 31, -0.5, 24, bits});
    for (int i = 0; i < plan.target_size; ++i) {
      int sum = 0;
      for (int k = 0; k < plan.sizes[i]; ++k)
        sum += plan.integers[size_t(i) * plan.filter_size + k];
      EXPECT_EQ(sum, bits == 8 ? 16384 : 8192);
    }
  }
}

class ZeroFunction : public ResamplingFunction {
public:
  double f(double) const override { return 0; }
  double support() const override { return 1; }
};
TEST(CoefficientContract, ZeroTotalPreservesOriginalFallbackRules) {
  const ZeroFunction zero;
  // A valid in-image window retains zero coefficients. An entirely out-of-image
  // window has no entries and receives the original single unit-weight fallback.
  const auto inside = BuildCoefficients(zero, {3, 3, 0, 3, 8});
  EXPECT_TRUE(std::all_of(inside.integers.begin(), inside.integers.end(), [](int16_t v) { return v == 0; }));
  const auto outside = BuildCoefficients(zero, {3, 1, 10, 1, 8});
  EXPECT_EQ(outside.offsets[0], 2);
  EXPECT_EQ(outside.sizes[0], 1);
  EXPECT_EQ(outside.integers[0], 16384);
}

TEST(CoefficientValidation, RejectsInvalidGeometryAndNonfiniteInputs) {
  const TriangleFilter triangle;
  const CoefficientRequest valid{4, 4, 0, 4, 8};
  for (int value : {0, -1}) {
    auto request = valid;
    request.source_size = value;
    EXPECT_THROW(BuildCoefficients(triangle, request), std::invalid_argument);
    request = valid;
    request.target_size = value;
    EXPECT_THROW(BuildCoefficients(triangle, request), std::invalid_argument);
    request = valid;
    request.crop_size = value;
    EXPECT_THROW(BuildCoefficients(triangle, request), std::invalid_argument);
  }
  for (int bits : {0, 7, 17, 31, 33}) {
    auto request = valid;
    request.bits_per_sample = bits;
    EXPECT_THROW(BuildCoefficients(triangle, request), std::invalid_argument);
  }
  for (double value : {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity()}) {
    auto request = valid;
    request.crop_start = value;
    EXPECT_THROW(BuildCoefficients(triangle, request), std::invalid_argument);
    request = valid;
    request.crop_size = value;
    EXPECT_THROW(BuildCoefficients(triangle, request), std::invalid_argument);
    request = valid;
    request.source_center = value;
    EXPECT_THROW(BuildCoefficients(triangle, request), std::invalid_argument);
    request = valid;
    request.destination_center = value;
    EXPECT_THROW(BuildCoefficients(triangle, request), std::invalid_argument);
  }
  auto huge = valid;
  huge.crop_start = 1e100;
  EXPECT_THROW(BuildCoefficients(triangle, huge), std::invalid_argument);
}

class InvalidFunction : public ResamplingFunction {
public:
  InvalidFunction(double value, double radius) : value_(value), radius_(radius) {}
  double f(double) const override { return value_; }
  double support() const override { return radius_; }

private:
  double value_, radius_;
};
TEST(CoefficientValidation, RejectsInvalidSupportAndValuesBeforeUnsafeConversions) {
  const CoefficientRequest valid{4, 4, 0, 4, 8};
  EXPECT_THROW(BuildCoefficients(InvalidFunction(1, -1), valid), std::invalid_argument);
  EXPECT_THROW(BuildCoefficients(InvalidFunction(1, std::numeric_limits<double>::quiet_NaN()), valid),
               std::invalid_argument);
  EXPECT_THROW(BuildCoefficients(InvalidFunction(1, 1e100), valid), std::length_error);
  EXPECT_THROW(BuildCoefficients(InvalidFunction(std::numeric_limits<double>::infinity(), 1), valid),
               std::invalid_argument);
  // Both counts fit int, but their float table exceeds a representable allocation.
  auto huge = valid;
  huge.target_size = std::numeric_limits<int>::max();
  huge.bits_per_sample = 32;
  EXPECT_THROW(BuildCoefficients(InvalidFunction(1, 1e9), huge), std::length_error);
}
} // namespace
} // namespace vc_test
