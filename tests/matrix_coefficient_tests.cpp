// SPDX-License-Identifier: GPL-2.0-or-later
#include "matrix/coefficients.h"
#include "fixtures/matrix_coefficients_active_golden.h"
#include <gtest/gtest.h>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
namespace {
using vc::matrix::Config;
using vc::matrix::Direction;
using vc::matrix::BuildCoefficients;
// Fixture order: weights, depth, precision, source range, destination range,
// direction (RGB->YUV first). Hash logical words, never struct padding.
Config MatrixConfig(int index) {
  constexpr double weights[][2] = {{.299, .114}, {.2126, .0722}, {1.0 / 3, 1.0 / 3}, {.2627, .0593}, {.3, .11},
                                   {.212, .087}, {0, 0}};
  const auto direction = index % 2 == 0 ? Direction::RgbToYuv : Direction::YuvToRgb;
  index /= 2;
  const bool destination_full = index % 2 != 0;
  index /= 2;
  const bool source_full = index % 2 != 0;
  index /= 2;
  constexpr int precisions[] = {13, 15, 16};
  const int precision = precisions[index % 3];
  index /= 3;
  constexpr int depths[] = {8, 10, 16, 32};
  const int depth = depths[index % 4];
  index /= 4;
  return {weights[index][0], weights[index][1], depth, precision, source_full, destination_full, direction};
}
template <class M>
uint64_t Hash(const M& m) {
  uint64_t hash = 14695981039346656037ull;
  auto word = [&](uint32_t value) {
    for (int i = 0; i < 4; ++i) {
      hash ^= (value >> (8 * i)) & 255;
      hash *= 1099511628211ull;
    }
  };
  for (int value : {m.y_r, m.y_g, m.y_b, m.u_r, m.u_g, m.u_b, m.v_r, m.v_g, m.v_b, m.offset_y, m.offset_rgb})
    word(uint32_t(value));
  for (float value : {m.y_r_f, m.y_g_f, m.y_b_f, m.u_r_f, m.u_g_f, m.u_b_f, m.v_r_f, m.v_g_f, m.v_b_f, m.offset_y_f,
                      m.offset_rgb_f}) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    word(bits);
  }
  return hash;
}

class MatrixCoefficientGolden : public ::testing::TestWithParam<int> {};
TEST_P(MatrixCoefficientGolden, MatchesReviewedRangeCorrections) {
  const auto config = MatrixConfig(GetParam());
  EXPECT_EQ(Hash(BuildCoefficients(config)), matrix_active_golden[GetParam()]);
}
INSTANTIATE_TEST_SUITE_P(StandardProfiles, MatrixCoefficientGolden, ::testing::Range(0, 672));
TEST(MatrixCoefficientValidation, RejectsInvalidConfigurations) {
  const Config base{.299, .114, 8, 15, true, false, Direction::RgbToYuv};
  for (const int depth : {7, 17, 31, 33}) {
    auto c = base;
    c.bits_per_sample = depth;
    EXPECT_THROW(BuildCoefficients(c), std::invalid_argument);
  }
  for (const int precision : {-1, 21}) {
    auto c = base;
    c.precision = precision;
    EXPECT_THROW(BuildCoefficients(c), std::invalid_argument);
  }
  for (const double weight :
       {-0.1, 1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
    auto c = base;
    c.kr = weight;
    EXPECT_THROW(BuildCoefficients(c), std::invalid_argument);
    c = base;
    c.kb = weight;
    EXPECT_THROW(BuildCoefficients(c), std::invalid_argument);
  }
  auto c = base;
  c.direction = static_cast<Direction>(99);
  EXPECT_THROW(BuildCoefficients(c), std::invalid_argument);
  c = base;
  c.kr = .5;
  c.kb = .5;
  EXPECT_THROW(BuildCoefficients(c), std::invalid_argument);
}
TEST(MatrixCoefficientValidation, RejectsUnrepresentableFixedPointCoefficient) {
  const Config c{.5, .5 - 1e-12, 16, 20, true, true, Direction::YuvToRgb};
  EXPECT_THROW(BuildCoefficients(c), std::invalid_argument);
}
TEST(MatrixCoefficientRange, NominalIntegerSpansAndLumaGain) {
  for (int depth : {8, 10, 12, 14, 16})
    for (int precision : {13, 14, 15})
      for (const auto weights : std::array<std::array<double, 2>, 3>{{{.299, .114}, {.2126, .0722}, {.2627, .0593}}}) {
        const double span = 219 << (depth - 8), maximum = (1 << depth) - 1, scale = 1 << precision;
        const auto forward =
            BuildCoefficients({weights[0], weights[1], depth, precision, false, true, Direction::RgbToYuv});
        EXPECT_NEAR(span * (forward.y_b + forward.y_g + forward.y_r) / scale, maximum, span / (2 * scale) + 1e-8);
        EXPECT_NEAR(span * (double(forward.y_b_f) + forward.y_g_f + forward.y_r_f), maximum, maximum * 2e-7);
        const auto reverse =
            BuildCoefficients({weights[0], weights[1], depth, precision, true, false, Direction::YuvToRgb});
        EXPECT_NEAR(maximum * reverse.y_b / scale, span, maximum / (2 * scale));
        EXPECT_NEAR(maximum * reverse.y_b_f, span, span * 2e-7);
      }
}
} // namespace
