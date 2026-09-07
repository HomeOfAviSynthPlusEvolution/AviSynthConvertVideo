// SPDX-License-Identifier: GPL-2.0-or-later
#include "matrix/execute.h"
#include "matrix/highway.h"
#include "matrix/transform.h"
#include "video_convert/matrix.h"
#include <memory>
#include <future>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>
namespace {
using namespace vc::matrix;
// Independent scalar expressions follow the original B/G/R and Y/U/V formulas.
// Integer expectations use exact double products and floor instead of shifts.
template <class T>
std::array<T, 3> Expected(const Config& c, const Coefficients& m, T a, T b, T d) {
  std::array<T, 3> result{};
  if constexpr (std::is_same_v<T, float>) {
    float p = a, q = b, r = d;
    if (c.direction == Direction::RgbToYuv) {
      if (m.offset_rgb_f != 0) {
        p += m.offset_rgb_f;
        q += m.offset_rgb_f;
        r += m.offset_rgb_f;
      }
      result = {m.offset_y_f + (m.y_b_f * p + m.y_g_f * q + m.y_r_f * r),
                0.0f + (m.u_b_f * p + m.u_g_f * q + m.u_r_f * r), 0.0f + (m.v_b_f * p + m.v_g_f * q + m.v_r_f * r)};
      result[0] = std::clamp(result[0], 0.f, 1.f);
      result[1] = std::clamp(result[1], -.5f, .5f);
      result[2] = std::clamp(result[2], -.5f, .5f);
    } else {
      p += m.offset_y_f;
      q -= 0.0f;
      r -= 0.0f;
      result = {m.y_b_f * p + m.u_b_f * q + m.v_b_f * r + m.offset_rgb_f,
                m.y_g_f * p + m.u_g_f * q + m.v_g_f * r + m.offset_rgb_f,
                m.y_r_f * p + m.u_r_f * q + m.v_r_f * r + m.offset_rgb_f};
      for (auto& value : result)
        value = std::clamp(value, 0.f, 1.f);
    }
  } else {
    const double half = double(1 << (c.bits_per_sample - 1));
    const double scale = double(uint64_t{1} << c.precision);
    const double rounding = c.precision ? scale / 2 : 0;
    double p = a, q = b, r = d;
    std::array<double, 3> sums, offsets;
    if (c.direction == Direction::RgbToYuv) {
      p += m.offset_rgb;
      q += m.offset_rgb;
      r += m.offset_rgb;
      sums = {m.y_b * p + m.y_g * q + m.y_r * r, m.u_b * p + m.u_g * q + m.u_r * r, m.v_b * p + m.v_g * q + m.v_r * r};
      offsets = {double(m.offset_y), half, half};
    } else {
      p += m.offset_y;
      q -= half;
      r -= half;
      sums = {m.y_b * p + m.u_b * q + m.v_b * r, m.y_g * p + m.u_g * q + m.v_g * r, m.y_r * p + m.u_r * q + m.v_r * r};
      offsets.fill(m.offset_rgb);
    }
    for (int i = 0; i < 3; ++i)
      result[i] = static_cast<T>(
          std::clamp(std::floor((sums[i] + rounding) / scale) + offsets[i], 0.0, double((1 << c.bits_per_sample) - 1)));
  }
  return result;
}
template <class T>
void CheckRows(Config config) {
  constexpr int width = 133, height = 5, pitch = 139;
  constexpr T guard = static_cast<T>(77);
  for (const auto& weights : std::array<std::array<double, 2>, 3>{{{.299, .114}, {.2627, .0593}, {0, 0}}}) {
    config.kr = weights[0];
    config.kb = weights[1];
    const auto m = BuildCoefficients(config);
    std::array<std::vector<T>, 3> input, output, whole;
    std::array<vc_const_plane, 3> source;
    std::array<vc_plane, 3> destination, full;
    for (int c = 0; c < 3; ++c) {
      input[c].resize(pitch * height);
      output[c].assign(pitch * height, guard);
      whole[c] = output[c];
      for (int y = 0; y < height; ++y)
        for (int x = 0; x < pitch; ++x) {
          if constexpr (std::is_same_v<T, float>)
            input[c][y * pitch + x] = float((x * 37 + y * 53 + c * 101) % 383 - 128) / 128.f;
          else
            input[c][y * pitch + x] =
                static_cast<T>((x * 7919 + y * 3571 + c * 1031) & ((1 << config.bits_per_sample) - 1));
        }
      source[c] = {input[c].data() + (c == 1 ? pitch * (height - 1) : 0),
                   ptrdiff_t(c == 1 ? -pitch : pitch) * ptrdiff_t(sizeof(T))};
      destination[c] = {output[c].data() + (c == 2 ? pitch * (height - 1) : 0),
                        ptrdiff_t(c == 2 ? -pitch : pitch) * ptrdiff_t(sizeof(T))};
      full[c] = {whole[c].data() + (c == 2 ? pitch * (height - 1) : 0), destination[c].stride};
    }
    ASSERT_EQ(ExecuteC(config, m, source, destination, {width, height, 1, 3}), VC_OK);
    for (int y = 0; y < height; ++y)
      for (int x = 0; x < pitch; ++x) {
        const auto expected = Expected(config, m, input[0][y * pitch + x], input[1][(height - 1 - y) * pitch + x],
                                       input[2][y * pitch + x]);
        for (int c = 0; c < 3; ++c) {
          const T actual = output[c][(c == 2 ? height - 1 - y : y) * pitch + x];
          EXPECT_EQ(actual, (y >= 1 && y < 4 && x < width) ? expected[c] : guard)
              << "channel " << c << " x " << x << " y " << y;
        }
      }
    ASSERT_EQ(ExecuteC(config, m, source, full, {width, height, 0, height}), VC_OK);
    ASSERT_EQ(ExecuteC(config, m, source, destination, {width, height, 4, 1}), VC_OK);
    ASSERT_EQ(ExecuteC(config, m, source, destination, {width, height, 0, 1}), VC_OK);
    EXPECT_EQ(output, whole);
    const vc_matrix_config api_config{config.kr,
                                      config.kb,
                                      config.bits_per_sample,
                                      config.precision,
                                      config.source_full,
                                      config.destination_full,
                                      config.direction == Direction::RgbToYuv ? VC_RGB_TO_YUV : VC_YUV_TO_RGB};
    for (const int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE}) {
      vc_matrix_plan* raw_plan = nullptr;
      ASSERT_EQ(vc_matrix_create_for_target(&api_config, target, &raw_plan), VC_OK);
      const std::unique_ptr<vc_matrix_plan, decltype(&vc_matrix_destroy)> plan(raw_plan, vc_matrix_destroy);
      for (auto& channel : output)
        std::fill(channel.begin(), channel.end(), guard);
      // Reuse all live descriptors; named RGB fields must map back to B/G/R.
      if (config.direction == Direction::RgbToYuv) {
        ASSERT_EQ(vc_matrix_rgb_to_yuv(plan.get(), {source[2], source[1], source[0], {}},
                                       {destination[0], destination[1], destination[2]}, {width, height, 0, height}),
                  VC_OK);
        EXPECT_EQ(vc_matrix_yuv_to_rgb(plan.get(), {}, {}, {width, height, 0, 0}), VC_INVALID_ARGUMENT);
      } else {
        ASSERT_EQ(vc_matrix_yuv_to_rgb(plan.get(), {source[0], source[1], source[2]},
                                       {destination[2], destination[1], destination[0], {}},
                                       {width, height, 0, height}),
                  VC_OK);
        EXPECT_EQ(vc_matrix_rgb_to_yuv(plan.get(), {}, {}, {width, height, 0, 0}), VC_INVALID_ARGUMENT);
      }
      EXPECT_EQ(output, whole);
      for (auto& channel : output)
        std::fill(channel.begin(), channel.end(), guard);
      const auto run_band = [&](int first, int count) {
        if (config.direction == Direction::RgbToYuv)
          return vc_matrix_rgb_to_yuv(plan.get(), {source[2], source[1], source[0], {}},
                                      {destination[0], destination[1], destination[2]}, {width, height, first, count});
        return vc_matrix_yuv_to_rgb(plan.get(), {source[0], source[1], source[2]},
                                    {destination[2], destination[1], destination[0], {}},
                                    {width, height, first, count});
      };
      auto worker = std::async(std::launch::async, run_band, 2, 3);
      ASSERT_EQ(run_band(0, 2), VC_OK);
      ASSERT_EQ(worker.get(), VC_OK);
      EXPECT_EQ(output, whole);
      auto invalid = destination;
      invalid[2].data = nullptr;
      const int status =
          config.direction == Direction::RgbToYuv
              ? vc_matrix_rgb_to_yuv(plan.get(), {source[2], source[1], source[0], {}},
                                     {invalid[0], invalid[1], invalid[2]}, {width, height, 0, height})
              : vc_matrix_yuv_to_rgb(plan.get(), {source[0], source[1], source[2]},
                                     {invalid[2], invalid[1], invalid[0], {}}, {width, height, 0, height});
      EXPECT_EQ(status, VC_INVALID_ARGUMENT);
      EXPECT_EQ(output, whole);
    }
    const auto before = output;
    auto bad = destination;
    bad[2].data = nullptr;
    EXPECT_EQ(ExecuteC(config, m, source, bad, {width, height, 0, height}), VC_INVALID_ARGUMENT);
    EXPECT_EQ(output, before);
  }
}
class MatrixRows : public ::testing::TestWithParam<int> {};
TEST_P(MatrixRows, ArithmeticBandsStridesAndGuards) {
  int index = GetParam();
  const auto direction = index % 2 ? Direction::YuvToRgb : Direction::RgbToYuv;
  index /= 2;
  const bool sf = index % 2 != 0;
  index /= 2;
  const bool df = index % 2 != 0;
  index /= 2;
  constexpr int depths[] = {8, 9, 10, 12, 14, 16, 32};
  Config c{.299, .114, depths[index], direction == Direction::RgbToYuv ? 15 : 13, sf, df, direction};
  if (c.bits_per_sample == 32)
    CheckRows<float>(c);
  else if (c.bits_per_sample == 8)
    CheckRows<uint8_t>(c);
  else
    CheckRows<uint16_t>(c);
}
INSTANTIATE_TEST_SUITE_P(StorageAndRanges, MatrixRows, ::testing::Range(0, 56));
TEST(MatrixRowsValidation, EmptyAndInvalidGeometry) {
  const Config c{.299, .114, 8, 15, true, false, Direction::RgbToYuv};
  const auto m = BuildCoefficients(c);
  EXPECT_EQ(ExecuteC(c, m, {}, {}, {1, 1, 1, 0}), VC_OK);
  for (const auto rows :
       std::array<vc_rows, 5>{{{0, 1, 0, 0}, {1, 0, 0, 0}, {1, 1, -1, 1}, {1, 1, 0, 2}, {1, 1, 2, 0}}})
    EXPECT_EQ(ExecuteC(c, m, {}, {}, rows), VC_INVALID_ARGUMENT);
}
TEST(MatrixRowsValidation, PrecisionBounds) {
  for (const int precision : {0, 1, 13, 15, 20})
    for (const auto direction : {Direction::RgbToYuv, Direction::YuvToRgb})
      CheckRows<uint16_t>({.299, .114, 16, precision, false, true, direction});
}
template <class T>
void ExactAllocation(int depth) {
  for (const int width : {1, 2, 3, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65, 95, 96, 97, 127, 128, 129}) {
    for (const auto direction : {Direction::RgbToYuv, Direction::YuvToRgb}) {
      const Config c{.2126, .0722, depth, direction == Direction::RgbToYuv ? 15 : 13, true, false, direction};
      const auto m = BuildCoefficients(c);
      std::array<std::vector<T>, 3> input, output;
      std::array<vc_const_plane, 3> source;
      std::array<vc_plane, 3> destination;
      for (int channel = 0; channel < 3; ++channel) {
        input[channel].resize(width);
        output[channel].resize(width);
        for (int x = 0; x < width; ++x) {
          if constexpr (std::is_same_v<T, float>)
            input[channel][x] = float(x + channel * 7) / 32.f;
          else
            input[channel][x] = static_cast<T>((x * 7717 + channel * 107) & ((1 << depth) - 1));
        }
        source[channel] = {input[channel].data(), ptrdiff_t(width) * ptrdiff_t(sizeof(T))};
        destination[channel] = {output[channel].data(), source[channel].stride};
      }
      ASSERT_EQ(Execute(c, m, source, destination, {width, 1, 0, 1}, GetMatrixKernel(VC_TARGET_NATIVE, c, m)), VC_OK);
      for (int x = 0; x < width; ++x) {
        const auto expected = Expected(c, m, input[0][x], input[1][x], input[2][x]);
        for (int channel = 0; channel < 3; ++channel)
          EXPECT_EQ(output[channel][x], expected[channel]);
      }
    }
  }
}
TEST(MatrixRowsValidation, ExactAllocationsAroundVectorBoundaries) {
  ExactAllocation<uint8_t>(8);
  ExactAllocation<uint16_t>(16);
  ExactAllocation<float>(32);
}
TEST(MatrixPlanValidation, InvalidCreationAndNullPlans) {
  const vc_matrix_config base{.299, .114, 8, 15, 1, 0, VC_RGB_TO_YUV};
  EXPECT_EQ(vc_matrix_create(&base, nullptr), VC_INVALID_ARGUMENT);
  vc_matrix_plan* output = nullptr;
  EXPECT_EQ(vc_matrix_create(nullptr, &output), VC_INVALID_ARGUMENT);
  EXPECT_EQ(output, nullptr);
  for (int field = 0; field < 6; ++field) {
    auto config = base;
    switch (field) {
      case 0:
        config.source_full = 2;
        break;
      case 1:
        config.destination_full = -1;
        break;
      case 2:
        config.direction = 77;
        break;
      case 3:
        config.bits_per_sample = 7;
        break;
      case 4:
        config.precision = 21;
        break;
      case 5:
        config.kr = std::numeric_limits<double>::quiet_NaN();
        break;
    }
    output = reinterpret_cast<vc_matrix_plan*>(uintptr_t{1});
    EXPECT_EQ(vc_matrix_create(&config, &output), VC_INVALID_ARGUMENT);
    EXPECT_EQ(output, nullptr);
  }
  EXPECT_EQ(vc_matrix_rgb_to_yuv(nullptr, {}, {}, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_matrix_yuv_to_rgb(nullptr, {}, {}, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  vc_matrix_destroy(nullptr);
}
TEST(MatrixTargets, SelectionValidationAndWideAccumulator) {
  const Config c{.299, .114, 16, 20, false, true, Direction::RgbToYuv};
  const auto m = BuildCoefficients(c);
  EXPECT_FALSE(FitsInt32(MakeIntegerTransform(c, m), c.precision));
  auto small = c;
  small.bits_per_sample = 8;
  small.precision = 15;
  EXPECT_TRUE(FitsInt32(MakeIntegerTransform(small, BuildCoefficients(small)), small.precision));
  const int64_t supported = vc_matrix_supported_targets();
  EXPECT_EQ(GetMatrixKernel(VC_TARGET_NATIVE, c, m) != nullptr, supported != 0);
  const vc_matrix_config config{c.kr, c.kb, c.bits_per_sample, c.precision, 0, 1, VC_RGB_TO_YUV};
  for (int64_t target : {int64_t(-2), std::numeric_limits<int64_t>::max(), int64_t{1} << 61}) {
    vc_matrix_plan* plan = reinterpret_cast<vc_matrix_plan*>(uintptr_t{1});
    EXPECT_EQ(vc_matrix_create_for_target(&config, target, &plan), VC_INVALID_ARGUMENT);
    EXPECT_EQ(plan, nullptr);
  }
  // Explicit supported bits can be bound without mutating process-global dispatch.
  for (int64_t remaining = supported; remaining; remaining &= remaining - 1) {
    vc_matrix_plan* plan = nullptr;
    ASSERT_EQ(vc_matrix_create_for_target(&config, remaining & -remaining, &plan), VC_OK);
    vc_matrix_destroy(plan);
  }
  EXPECT_EQ(vc_matrix_supported_targets(), supported);
}
TEST(MatrixTargets, NegativeBiasMagnitudeParticipatesInOverflowProof) {
  IntegerTransform t{};
  t.limit = 255;
  t.input_offsets = {-128, -128, -128};
  t.weights[0][0] = 32767;
  t.biases[0] = -int64_t(std::numeric_limits<int32_t>::max());
  EXPECT_FALSE(FitsInt32(t, 15));
  t.biases[0] = -16;
  EXPECT_TRUE(FitsInt32(t, 15));
}
TEST(MatrixRowsValidation, LowDepthOffsetsAcrossPrecisionAndRange) {
  for (int precision : {0, 1, 13, 15, 20})
    for (int range = 0; range < 4; ++range)
      for (auto direction : {Direction::RgbToYuv, Direction::YuvToRgb}) {
        const Config c{.2126, .0722, 8, precision, (range & 1) != 0, (range & 2) != 0, direction};
        CheckRows<uint8_t>(c);
      }
}
TEST(MatrixRowsValidation, CustomCoefficientsNearWeightLimits) {
  // Small Kg generates large reverse chroma gains; never truncate them to i16.
  for (const auto direction : {Direction::RgbToYuv, Direction::YuvToRgb}) {
    const Config c{.499, .499, 16, 20, false, true, direction};
    const auto m = BuildCoefficients(c);
    std::array<uint16_t, 65> a{}, b{}, d{}, o0{}, o1{}, o2{};
    for (size_t x = 0; x < a.size(); ++x) {
      a[x] = uint16_t(x % 2 ? 65535 : 0);
      b[x] = uint16_t(x % 3 ? 65535 : 0);
      d[x] = uint16_t(x % 5 ? 65535 : 0);
    }
    constexpr ptrdiff_t stride = 65 * sizeof(uint16_t);
    ASSERT_EQ(Execute(c, m, {{{a.data(), stride}, {b.data(), stride}, {d.data(), stride}}},
                      {{{o0.data(), stride}, {o1.data(), stride}, {o2.data(), stride}}}, {65, 1, 0, 1},
                      GetMatrixKernel(VC_TARGET_NATIVE, c, m)),
              VC_OK);
    for (size_t x = 0; x < a.size(); ++x) {
      const auto expected = Expected(c, m, a[x], b[x], d[x]);
      EXPECT_EQ(o0[x], expected[0]);
      EXPECT_EQ(o1[x], expected[1]);
      EXPECT_EQ(o2[x], expected[2]);
    }
  }
}

template <class T>
void CheckLuma(int depth) {
  for (int range = 0; range < 4; ++range)
    for (int precision : {15, 20})
      for (int width : {1, 7, 16, 17, 33}) {
        const Config config{.2126, .0722, depth, precision, (range & 1) != 0, (range & 2) != 0, Direction::RgbToY};
        const auto m = BuildCoefficients(config);
        auto reference_config = config;
        reference_config.direction = Direction::RgbToYuv;
        constexpr int height = 3;
        std::array<std::vector<T>, 3> input;
        for (int c = 0; c < 3; ++c) {
          input[c].resize(size_t(width) * height);
          for (size_t x = 0; x < input[c].size(); ++x) {
            if constexpr (std::is_same_v<T, float>)
              input[c][x] = float(int(x % 9) - 3) + c * .125f;
            else
              input[c][x] = T((x * 5717 + c * 9973) & ((1 << depth) - 1));
          }
        }
        const auto original = input;
        const ptrdiff_t stride = ptrdiff_t(width) * sizeof(T);
        const vc_const_rgb_planes source{
            {input[2].data(), stride}, {input[1].data() + 2 * width, -stride}, {input[0].data(), stride}, {}};
        const vc_matrix_config public_config{
            config.kr, config.kb, depth, precision, config.source_full, config.destination_full, VC_RGB_TO_Y};
        for (int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE}) {
          vc_matrix_plan* raw = nullptr;
          ASSERT_EQ(vc_matrix_create_for_target(&public_config, target, &raw), VC_OK);
          const std::unique_ptr<vc_matrix_plan, decltype(&vc_matrix_destroy)> plan(raw, vc_matrix_destroy);
          std::vector<T> output(size_t(width) * height, T(77));
          const vc_plane destination{output.data() + 2 * width, -stride};
          EXPECT_EQ(vc_matrix_rgb_to_yuv(plan.get(), source, {}, {width, height, 0, height}), VC_INVALID_ARGUMENT);
          EXPECT_EQ(vc_matrix_rgb_to_y(plan.get(), {}, destination, {width, height, 0, height}), VC_INVALID_ARGUMENT);
          EXPECT_EQ(output, std::vector<T>(size_t(width) * height, T(77)));
          for (int y : {2, 0, 1}) {
            ASSERT_EQ(vc_matrix_rgb_to_y(plan.get(), source, destination, {width, height, y, 1}), VC_OK);
            for (int x = 0; x < width; ++x) {
              const T b = input[0][y * width + x], g = input[1][(2 - y) * width + x], r = input[2][y * width + x];
              T expected;
              if constexpr (std::is_same_v<T, float>)
                expected = m.offset_y_f + (m.y_b_f * (b + m.offset_rgb_f) + m.y_g_f * (g + m.offset_rgb_f) +
                                           m.y_r_f * (r + m.offset_rgb_f));
              else
                expected = Expected(reference_config, m, b, g, r)[0];
              EXPECT_EQ(output[(2 - y) * width + x], expected);
            }
          }
          EXPECT_EQ(input, original);
          EXPECT_EQ(vc_matrix_rgb_to_y(plan.get(), {}, {}, {width, height, height, 0}), VC_OK);
        }
      }
}
TEST(MatrixLuma, SharedIntegerArithmeticAndUnclippedFloat) {
  CheckLuma<uint8_t>(8);
  for (int depth : {10, 12, 14, 16})
    CheckLuma<uint16_t>(depth);
  CheckLuma<float>(32);
}
} // namespace
