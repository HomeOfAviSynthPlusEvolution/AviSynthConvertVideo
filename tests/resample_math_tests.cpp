#include "resample/functions.h"
#include "resample_test_helpers.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

namespace vc_test {
namespace {
using namespace vc::resample;

// Capture binary64 decimal values, but allow libm/compiler rounding differences
// across platforms. This tolerance does not define future pixel-output parity.
double MathTolerance(double expected) {
  return 32 * std::numeric_limits<double>::epsilon() * (1.0 + std::abs(expected));
}

class ResampleMath : public testing::TestWithParam<MathGolden> {};

TEST_P(ResampleMath, MatchesOriginalFormulaSamples) {
  const auto& golden = GetParam();
  const auto function = MakeFunction(golden);
  ASSERT_NE(function, nullptr);
  for (size_t i = 0; i < kPositions.size(); ++i) {
    SCOPED_TRACE(kPositions[i]);
    EXPECT_NEAR(function->f(kPositions[i]), golden.values[i], MathTolerance(golden.values[i]));
    EXPECT_NEAR(function->f(-kPositions[i]), golden.values[i], MathTolerance(golden.values[i]));
  }
}

TEST_P(ResampleMath, MatchesOriginalSupport) {
  const auto& golden = GetParam();
  const auto function = MakeFunction(golden);
  ASSERT_NE(function, nullptr);
  EXPECT_NEAR(function->support(), golden.support, MathTolerance(golden.support));
}

TEST_P(ResampleMath, FiniteSymmetricAndStableAcrossEvaluationOrder) {
  const auto function = MakeFunction(GetParam());
  ASSERT_NE(function, nullptr);
  const double radius = std::max(1.0, function->support());
  const double support_before = function->support();
  for (int i = 0; i <= 256; ++i) {
    const double x = radius * i / 128;
    const double value = function->f(x);
    EXPECT_TRUE(std::isfinite(value));
    EXPECT_DOUBLE_EQ(value, function->f(-x));
    (void)function->f(radius - x);
    EXPECT_DOUBLE_EQ(value, function->f(x));
  }
  EXPECT_DOUBLE_EQ(function->support(), support_before);
}

INSTANTIATE_TEST_SUITE_P(OriginalBaselines, ResampleMath, testing::ValuesIn(kMathGolden),
                         [](const testing::TestParamInfo<MathGolden>& info) { return info.param.name; });

TEST(ResampleMathContract, PointHasZeroSupportWithoutBecomingAnEmptyKernel) {
  const PointFilter point;
  EXPECT_DOUBLE_EQ(point.support(), 0);
  for (double x : {-10.0, -0.5, 0.0, 0.5, 10.0})
    EXPECT_DOUBLE_EQ(point.f(x), 1);
}

TEST(ResampleMathContract, SupportDoesNotUniversallyClipTheMathematicalFunction) {
  const SincFilter sinc(1);
  const GaussianFilter gaussian(30, 2, 0.1);
  const SinPowerFilter sinpower(10);
  const SincLin2Filter sinclin(1);
  const UserDefined2Filter user(121, 19, 1.5);
  EXPECT_NE(sinc.f(1.5), 0);
  EXPECT_GT(gaussian.f(0.5), 0);
  EXPECT_GT(sinpower.f(2.5), 0);
  EXPECT_NE(sinclin.f(1.5), 0);
  EXPECT_NE(user.f(2.5), 0);
}

TEST(ResampleMathContract, PiecewiseFunctionsClipAtTheirOwnBoundary) {
  const TriangleFilter triangle;
  const MitchellNetravaliFilter bicubic;
  const LanczosFilter lanczos;
  const BlackmanFilter blackman;
  const Spline16Filter spline16;
  const Spline36Filter spline36;
  const Spline64Filter spline64;
  const std::array<const ResamplingFunction*, 7> functions{&triangle, &bicubic,  &lanczos, &blackman,
                                                           &spline16, &spline36, &spline64};
  for (const ResamplingFunction* function : functions) {
    EXPECT_DOUBLE_EQ(function->f(function->support()), 0);
    EXPECT_DOUBLE_EQ(function->f(-function->support()), 0);
  }
}

TEST(ResampleMathContract, GaussianAutoSupportAndParameterClamps) {
  const GaussianFilter gaussian(30, 2, 0);
  EXPECT_NEAR(gaussian.support(), std::sqrt(4.6 / (3 * std::log(2.0))), 1e-14);
  EXPECT_DOUBLE_EQ(LanczosFilter(-1).support(), 1);
  EXPECT_DOUBLE_EQ(LanczosFilter(101).support(), 100);
  EXPECT_DOUBLE_EQ(SincFilter(151).support(), 150);
  EXPECT_DOUBLE_EQ(SincLin2Filter(31).support(), 30);
  EXPECT_DOUBLE_EQ(UserDefined2Filter(121, 19, -1).support(), 1.5);
  EXPECT_DOUBLE_EQ(UserDefined2Filter(121, 19, 16).support(), 15);
}

TEST(ResampleMathContract, UserDefinedAnchorsRetainLimitedRangeParameterMapping) {
  const UserDefined2Filter user(121, 19, 2.3);
  EXPECT_NEAR(user.f(0), 1, 1e-14);
  EXPECT_NEAR(user.f(1), (121.0 - 16) / 219, 1e-14);
  EXPECT_NEAR(user.f(2), (19.0 - 16) / 219, 1e-14);
}

TEST(ResampleMathContract, DefaultConstructorsRetainOriginalParameters) {
  const MitchellNetravaliFilter bicubic;
  const LanczosFilter lanczos;
  const BlackmanFilter blackman;
  const GaussianFilter gaussian;
  const SincFilter sinc;
  const SinPowerFilter sinpower;
  const SincLin2Filter sinclin;
  EXPECT_DOUBLE_EQ(bicubic.f(0.5), MitchellNetravaliFilter(1.0 / 3, 1.0 / 3).f(0.5));
  EXPECT_DOUBLE_EQ(lanczos.f(0.5), LanczosFilter(3).f(0.5));
  EXPECT_DOUBLE_EQ(blackman.f(0.5), BlackmanFilter(4).f(0.5));
  EXPECT_DOUBLE_EQ(gaussian.f(0.5), GaussianFilter(30, 2, 4).f(0.5));
  EXPECT_DOUBLE_EQ(sinc.support(), SincFilter(4).support());
  EXPECT_DOUBLE_EQ(sinpower.f(0.5), SinPowerFilter(2.5).f(0.5));
  EXPECT_DOUBLE_EQ(sinclin.f(0.5), SincLin2Filter(15).f(0.5));
}
} // namespace
} // namespace vc_test
