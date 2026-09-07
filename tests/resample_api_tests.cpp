#include "video_convert/resample.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <cmath>
#include <future>
#include <limits>
#include <memory>
#include <tuple>
#include <vector>
namespace {
const std::array<vc_filter_spec, 13> filters = {{{VC_POINT, {0, 0, 0}},
                                                 {VC_TRIANGLE, {0, 0, 0}},
                                                 {VC_BICUBIC, {1. / 3, 1. / 3, 0}},
                                                 {VC_LANCZOS, {3, 0, 0}},
                                                 {VC_BLACKMAN, {4, 0, 0}},
                                                 {VC_SPLINE16, {0, 0, 0}},
                                                 {VC_SPLINE36, {0, 0, 0}},
                                                 {VC_SPLINE64, {0, 0, 0}},
                                                 {VC_GAUSSIAN, {30, 2, 4}},
                                                 {VC_SINC, {4, 0, 0}},
                                                 {VC_SINPOWER, {2.5, 0, 0}},
                                                 {VC_SINCLIN2, {15, 0, 0}},
                                                 {VC_USER_DEFINED2, {121, 19, 2.3}}}};
using Plan = std::unique_ptr<vc_resample_plan, decltype(&vc_resample_destroy)>;
class ResampleAPI : public testing::TestWithParam<std::tuple<int, int, int>> {};
template <class T>
void CheckBands(int kind, int bits, int axis, int64_t target = VC_TARGET_C) {
  constexpr int sw = 129, target_size = 65;
  const vc_resample_config config = {
      axis, sw, 9, target_size, bits, -0.25, axis == VC_HORIZONTAL ? double(sw) : 9., 0.5, 0.5, filters[kind]};
  vc_resample_plan* raw = nullptr;
  ASSERT_EQ(vc_resample_create(&config, &raw), VC_OK);
  Plan baseline(raw, vc_resample_destroy);
  const int dw = axis == VC_HORIZONTAL ? target_size : sw, dh = axis == VC_HORIZONTAL ? 9 : target_size;
  std::vector<T> input(sw * 9), expected(size_t(dw) * dh), assembled(expected);
  for (size_t i = 0; i < input.size(); ++i)
    input[i] = T(i % 211);
  ASSERT_EQ(vc_resample_execute(raw, {{input.data(), sw * sizeof(T)}, {0, 9}},
                                {{expected.data(), ptrdiff_t(dw * sizeof(T))}, {0, dh}}),
            VC_OK);
  ASSERT_EQ(vc_resample_create_for_target(&config, target, &raw), VC_OK);
  Plan plan(raw, vc_resample_destroy);
  std::vector<T> full(expected.size());
  ASSERT_EQ(vc_resample_execute(raw, {{input.data(), sw * sizeof(T)}, {0, 9}},
                                {{full.data(), ptrdiff_t(dw * sizeof(T))}, {0, dh}}),
            VC_OK);
  EXPECT_EQ(expected, full);
  for (int y = dh - 1; y >= 0; --y) {
    vc_row_range needed{-1, -1};
    ASSERT_EQ(vc_resample_required_rows(raw, {y, 1}, &needed), VC_OK);
    ASSERT_GE(needed.first_row, 0);
    ASSERT_GT(needed.row_count, 0);
    ASSERT_LE(needed.first_row + needed.row_count, 9);
    // Exact-sized source band, physically reversed. The destination holds just
    // one row: neither descriptor is based on an allocation of the whole image.
    std::vector<T> band(size_t(needed.row_count) * sw), row(dw);
    for (int r = 0; r < needed.row_count; ++r)
      std::copy_n(input.data() + size_t(needed.first_row + r) * sw, sw,
                  band.data() + size_t(needed.row_count - 1 - r) * sw);
    const vc_const_row_band src = {{band.data() + size_t(needed.row_count - 1) * sw, -ptrdiff_t(sw * sizeof(T))},
                                   needed};
    ASSERT_EQ(vc_resample_execute(raw, src, {{row.data(), ptrdiff_t(dw * sizeof(T))}, {y, 1}}), VC_OK);
    std::copy(row.begin(), row.end(), assembled.begin() + size_t(y) * dw);
    if (needed.row_count > 1) {
      auto short_source = src;
      --short_source.rows.row_count;
      std::fill(row.begin(), row.end(), T(42));
      EXPECT_EQ(vc_resample_execute(raw, short_source, {{row.data(), ptrdiff_t(dw * sizeof(T))}, {y, 1}}),
                VC_INVALID_ARGUMENT);
      for (T value : row)
        EXPECT_EQ(value, T(42));
    }
  }
  EXPECT_EQ(expected, assembled);
  vc_row_range empty{-1, -1};
  EXPECT_EQ(vc_resample_required_rows(raw, {dh, 0}, &empty), VC_OK);
  EXPECT_EQ(empty.first_row, 0);
  EXPECT_EQ(empty.row_count, 0);
  EXPECT_EQ(vc_resample_execute(raw, {{nullptr, 0}, {0, 0}}, {{nullptr, 0}, {dh, 0}}), VC_OK);
}
TEST_P(ResampleAPI, RequiredBandsAreSufficientAndIndependent) {
  const auto [kind, bits, axis] = GetParam();
  if (bits == 8)
    CheckBands<uint8_t>(kind, bits, axis);
  else if (bits == 32)
    CheckBands<float>(kind, bits, axis);
  else
    CheckBands<uint16_t>(kind, bits, axis);
}
TEST_P(ResampleAPI, NativeBandsMatchOrdinaryC) {
  const auto [kind, bits, axis] = GetParam();
  if (bits == 8)
    CheckBands<uint8_t>(kind, bits, axis, VC_TARGET_NATIVE);
  else if (bits == 32)
    CheckBands<float>(kind, bits, axis, VC_TARGET_NATIVE);
  else
    CheckBands<uint16_t>(kind, bits, axis, VC_TARGET_NATIVE);
}
TEST(ResampleAPIProperties, SupportMatchesFilterGeometry) {
  const std::array<double, 13> expected{{0, 1, 2, 3, 4, 2, 3, 4, 4, 4, 2, 15, 2.3}};
  for (size_t i = 0; i < filters.size(); ++i) {
    double support = -1;
    ASSERT_EQ(vc_resample_filter_support(&filters[i], &support), VC_OK);
    EXPECT_DOUBLE_EQ(support, expected[i]);
  }
  double normalized = 0;
  const vc_filter_spec gaussian{VC_GAUSSIAN, {10, 2, 0}};
  ASSERT_EQ(vc_resample_filter_support(&gaussian, &normalized), VC_OK);
  EXPECT_DOUBLE_EQ(normalized, std::sqrt(4.6 / std::log(2.0)));
  const vc_filter_spec wide{VC_USER_DEFINED2, {121, 19, 100}};
  ASSERT_EQ(vc_resample_filter_support(&wide, &normalized), VC_OK);
  EXPECT_EQ(normalized, 15);
  double support = 42;
  const vc_filter_spec bad{-1, {}};
  EXPECT_EQ(vc_resample_filter_support(&bad, &support), VC_INVALID_ARGUMENT);
  EXPECT_EQ(support, 42);
  EXPECT_EQ(vc_resample_filter_support(nullptr, &support), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_resample_filter_support(&filters[0], nullptr), VC_INVALID_ARGUMENT);
}
TEST(ResampleAPITargets, ExplicitSelectionAndRejection) {
  vc_resample_config config = {VC_HORIZONTAL, 129, 9, 65, 8, 0, 129, 0.5, 0.5, filters[1]};
  const int64_t supported = vc_resample_supported_targets();
  EXPECT_GE(supported, 0);
  vc_resample_plan* raw = nullptr;
  for (int64_t target : {int64_t(VC_TARGET_C), int64_t(VC_TARGET_NATIVE), supported & -supported}) {
    ASSERT_EQ(vc_resample_create_for_target(&config, target, &raw), VC_OK);
    vc_resample_destroy(raw);
  }
  for (int64_t target : {int64_t(3), int64_t(-2)}) {
    raw = reinterpret_cast<vc_resample_plan*>(uintptr_t(1));
    EXPECT_EQ(vc_resample_create_for_target(&config, target, &raw), VC_INVALID_ARGUMENT);
    EXPECT_EQ(raw, nullptr);
  }
  EXPECT_EQ(vc_resample_create_for_target(&config, VC_TARGET_NATIVE, nullptr), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_resample_create_for_target(nullptr, VC_TARGET_NATIVE, &raw), VC_INVALID_ARGUMENT);
  EXPECT_EQ(raw, nullptr);
}
TEST(ResampleAPITargets, SharedNativePlanRunsConcurrentDisjointBands) {
  const vc_resample_config config = {VC_VERTICAL, 129, 33, 65, 16, 0, 33, 0.5, 0.5, filters[3]};
  vc_resample_plan* raw = nullptr;
  ASSERT_EQ(vc_resample_create(&config, &raw), VC_OK);
  Plan baseline(raw, vc_resample_destroy);
  ASSERT_EQ(vc_resample_create_for_target(&config, VC_TARGET_NATIVE, &raw), VC_OK);
  Plan native(raw, vc_resample_destroy);
  std::vector<uint16_t> input(129 * 33), expected(129 * 65), actual(expected.size());
  for (size_t i = 0; i < input.size(); ++i)
    input[i] = uint16_t(i * 191);
  const vc_const_row_band source = {{input.data(), 258}, {0, 33}};
  ASSERT_EQ(vc_resample_execute(baseline.get(), source, {{expected.data(), 258}, {0, 65}}), VC_OK);
  auto first = std::async(std::launch::async,
                          [&] { return vc_resample_execute(native.get(), source, {{actual.data(), 258}, {0, 32}}); });
  EXPECT_EQ(vc_resample_execute(native.get(), source, {{actual.data() + 129 * 32, 258}, {32, 33}}), VC_OK);
  EXPECT_EQ(first.get(), VC_OK);
  EXPECT_EQ(expected, actual);
}
INSTANTIATE_TEST_SUITE_P(AllFunctions, ResampleAPI,
                         testing::Combine(testing::Range(0, 13), testing::Values(8, 16, 32),
                                          testing::Values(VC_HORIZONTAL, VC_VERTICAL)));
TEST(ResampleAPIErrors, RejectInvalidPlansAndTranslateExceptions) {
  vc_resample_config config = {VC_HORIZONTAL, 11, 9, 17, 8, 0, 11, 0.5, 0.5, filters[0]};
  vc_resample_plan* result = nullptr;
  EXPECT_EQ(vc_resample_create(nullptr, &result), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_resample_create(&config, nullptr), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_resample_execute(nullptr, {}, {}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_resample_required_rows(nullptr, {0, 1}, nullptr), VC_INVALID_ARGUMENT);
  vc_resample_destroy(nullptr);
  for (int fault = 0; fault < 7; ++fault) {
    auto bad = config;
    switch (fault) {
      case 0:
        bad.axis = 42;
        break;
      case 1:
        bad.bits_per_sample = 7;
        break;
      case 2:
        bad.source_height = 0;
        break;
      case 3:
        bad.filter.kind = 42;
        break;
      case 4:
        bad.filter = {VC_LANCZOS, {2.5, 0, 0}};
        break;
      case 5:
        bad.crop_start = std::numeric_limits<double>::infinity();
        break;
      case 6:
        bad.filter.parameters[1] = std::numeric_limits<double>::quiet_NaN();
        break;
    }
    EXPECT_EQ(vc_resample_create(&bad, &result), VC_INVALID_ARGUMENT);
    EXPECT_EQ(result, nullptr);
  }
}
} // namespace
