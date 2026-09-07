#include "coefficient_golden.h"
#include "resample_test_helpers.h"
#include "resample/execute.h"
#include "resample/highway.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <future>
#include <limits>
#include <new>
#include <vector>
namespace vc_test {
namespace {
using namespace vc::resample;
std::unique_ptr<ResamplingFunction> DefaultFunction(FilterKind kind) {
  for (const auto& item : kMathGolden)
    if (item.kind == kind)
      return MakeFunction(item);
  return nullptr;
}
class ResampleExecution : public testing::TestWithParam<CoefficientGolden> {};
template <class T>
void Check(const CoefficientGolden& golden, Axis axis, bool reverse) {
  const auto function = DefaultFunction(golden.kind);
  ASSERT_NE(function, nullptr);
  const auto plan = BuildCoefficients(*function, golden.request);
  const bool horizontal = axis == Axis::Horizontal;
  const int sw = horizontal ? plan.source_size : 7;
  const int sh = horizontal ? 7 : plan.source_size;
  const int dw = horizontal ? plan.target_size : 7;
  const int dh = horizontal ? 7 : plan.target_size;
  const int sp = sw + 3, dp = dw + 5;
  const T guard = T(37);
  std::vector<T> input(size_t(sp) * sh, guard), output(size_t(dp) * dh, guard), bands(output);
  for (int y = 0; y < sh; ++y)
    for (int x = 0; x < sw; ++x) {
      if constexpr (std::is_same_v<T, float>)
        input[size_t(y) * sp + x] = float((x * 53 + y * 97) % 257 - 128) / 32;
      else
        input[size_t(y) * sp + x] = T((x * 12983 + y * 2789) & ((1 << plan.bits_per_sample) - 1));
    }
  const auto source = vc_const_plane{input.data() + (reverse ? size_t(sh - 1) * sp : 0),
                                     ptrdiff_t(reverse ? -sp : sp) * ptrdiff_t(sizeof(T))};
  auto destination = [&](std::vector<T>& data) {
    return vc_plane{data.data() + (reverse ? size_t(dh - 1) * dp : 0),
                    ptrdiff_t(reverse ? -dp : dp) * ptrdiff_t(sizeof(T))};
  };
  ASSERT_EQ(ExecuteC(plan, axis, source, destination(output), {dw, dh, 0, dh}), VC_OK);
  // Exercise a shared immutable plan concurrently, with disjoint output bands.
  const int split = dh / 2;
  auto first = std::async(std::launch::async,
                          [&] { return ExecuteC(plan, axis, source, destination(bands), {dw, dh, 0, split}); });
  EXPECT_EQ(ExecuteC(plan, axis, source, destination(bands), {dw, dh, split, dh - split}), VC_OK);
  EXPECT_EQ(first.get(), VC_OK);
  EXPECT_EQ(output, bands);
  std::vector<size_t> bases(plan.target_size);
  for (int i = 1; i < plan.target_size; ++i)
    bases[i] = bases[i - 1] + golden.sizes[i - 1];
  for (int y = 0; y < dh; ++y) {
    for (int x = 0; x < dw; ++x) {
      const int p = horizontal ? x : y;
      long double sum = 0;
      for (int k = 0; k < golden.sizes[p]; ++k) {
        const int sx = horizontal ? golden.offsets[p] + k : x;
        const int sy = horizontal ? y : golden.offsets[p] + k;
        const auto value = input[size_t(reverse ? sh - 1 - sy : sy) * sp + sx];
        sum += (static_cast<long double>(value) - (plan.bits_per_sample == 16 ? 32768 : 0)) *
               golden.coefficients[bases[p] + k];
      }
      const auto actual = output[size_t(reverse ? dh - 1 - y : y) * dp + x];
      if constexpr (std::is_same_v<T, float>) {
        EXPECT_NEAR(actual, double(sum), 2e-5 * std::max(1.0, std::abs(double(sum))));
      } else {
        const int scale = plan.bits_per_sample == 8 ? 16384 : 8192;
        sum += static_cast<long double>(plan.bits_per_sample == 16 ? 32768 : 0) * scale;
        const auto expected = std::clamp(std::floor((sum + scale / 2) / scale), 0.0L,
                                         static_cast<long double>((1 << plan.bits_per_sample) - 1));
        EXPECT_EQ(actual, T(expected));
      }
    }
    for (int x = dw; x < dp; ++x)
      EXPECT_EQ(output[size_t(y) * dp + x], guard);
  }
}
TEST_P(ResampleExecution, BothAxesSignedStridesAndConcurrentBands) {
  const auto& g = GetParam();
  for (auto axis : {Axis::Horizontal, Axis::Vertical})
    for (bool reverse : {false, true}) {
      if (g.request.bits_per_sample == 8)
        Check<uint8_t>(g, axis, reverse);
      else if (g.request.bits_per_sample == 32)
        Check<float>(g, axis, reverse);
      else
        Check<uint16_t>(g, axis, reverse);
    }
}
template <class T>
void CheckVerticalSIMD(const CoefficientGolden& golden) {
  const auto* table = GetResampleKernels(VC_TARGET_NATIVE);
  const auto kernel = table ? table->vertical : nullptr;
  if (!kernel)
    GTEST_SKIP() << "No native SIMD target in this build";
  const auto function = DefaultFunction(golden.kind);
  const auto plan = BuildCoefficients(*function, golden.request);
  for (int width : {1, 7, 17, 65, 129}) {
    std::vector<T> source(size_t(width) * plan.source_size), expected(size_t(width) * plan.target_size),
        actual(expected);
    for (size_t i = 0; i < source.size(); ++i) {
      if constexpr (std::is_same_v<T, float>)
        source[i] = float(int(i % 509) - 254) / 64;
      else
        source[i] = T((i * 7937) & ((1 << plan.bits_per_sample) - 1));
    }
    vc_const_plane src{source.data() + size_t(width) * (plan.source_size - 1), -ptrdiff_t(width * sizeof(T))};
    vc_plane dst{expected.data(), ptrdiff_t(width * sizeof(T))};
    vc_rows rows{width, plan.target_size, 0, plan.target_size};
    ASSERT_EQ(ExecuteC(plan, Axis::Vertical, src, dst, rows), VC_OK);
    kernel(plan, src, {actual.data(), dst.stride}, rows, 0, 0);
    EXPECT_EQ(actual, expected);
  }
}
TEST_P(ResampleExecution, NativeVerticalMatchesCWithoutPadding) {
  const auto& g = GetParam();
  if (g.request.bits_per_sample == 8)
    CheckVerticalSIMD<uint8_t>(g);
  else if (g.request.bits_per_sample == 32)
    CheckVerticalSIMD<float>(g);
  else
    CheckVerticalSIMD<uint16_t>(g);
}
template <class T>
void CheckHorizontalInteger(int bits, const ResampleKernels* table = GetResampleKernels(VC_TARGET_NATIVE)) {
  const auto kernel = table ? table->horizontal_integer : nullptr;
  if (!kernel)
    GTEST_SKIP();
  for (int kind = 0; kind < 13; ++kind) {
    const auto function = DefaultFunction(FilterKind(kind));
    ASSERT_NE(function, nullptr);
    for (int sw : {1, 2, 3, 31, 67, 257})
      for (int target : {7, 17, 65, 129}) {
        auto plan = BuildCoefficients(*function, {sw, target, -0.25, double(sw) + 0.5, bits});
        std::vector<T> source(size_t(sw) * 3), expected(size_t(target) * 3), actual(expected);
        for (size_t i = 0; i < source.size(); ++i)
          source[i] = T((i * 9829) & ((1 << bits) - 1));
        const vc_const_plane src{source.data() + size_t(sw) * 2, -ptrdiff_t(sw * sizeof(T))};
        const vc_rows rows{target, 3, 0, 3};
        ASSERT_EQ(ExecuteC(plan, Axis::Horizontal, src, {expected.data(), ptrdiff_t(target * sizeof(T))}, rows), VC_OK);
        kernel(plan, src, {actual.data(), ptrdiff_t(target * sizeof(T))}, rows, 0, 0);
        EXPECT_EQ(actual, expected);
        PrepareHorizontal(plan, table->lanes);
        kernel(plan, src, {actual.data(), ptrdiff_t(target * sizeof(T))}, rows, 0, 0);
        EXPECT_EQ(actual, expected) << "filter=" << kind << " source=" << sw << " target=" << target;
      }
  }
}
TEST(ResampleHighwayContract, HorizontalIntegerExactSizedGatherWindows) {
  CheckHorizontalInteger<uint8_t>(8);
  CheckHorizontalInteger<uint16_t>(10);
  CheckHorizontalInteger<uint16_t>(16);
}
const ResampleKernels* EightLaneTable() {
  // Select one available eight-lane implementation to exercise the distinct
  // dot-product algorithm even when native has a wider vector. This is not a
  // sweep of equivalent generated Highway targets.
  int64_t targets = ResampleSupportedTargets();
  while (targets) {
    const int64_t target = targets & -targets;
    targets &= ~target;
    const auto* candidate = GetResampleKernels(target);
    if (candidate && candidate->lanes == 8) {
      return candidate;
    }
  }
  return nullptr;
}
TEST(ResampleHighwayContract, HorizontalEightLaneDotAndBoundedTail) {
  const auto* table = EightLaneTable();
  if (!table)
    GTEST_SKIP() << "No eight-lane resampler target on this host";
  auto plan = BuildCoefficients(Spline36Filter(), {31, 17, -0.25, 31.5, 8});
  PrepareHorizontal(plan, table->lanes);
  ASSERT_GT(plan.horizontal.dot_outputs, 0);
  ASSERT_LT(plan.horizontal.dot_outputs, plan.target_size);
  CheckHorizontalInteger<uint8_t>(8, table);
  CheckHorizontalInteger<uint16_t>(10, table);
  CheckHorizontalInteger<uint16_t>(16, table);
}
TEST(ResampleHighwayContract, HorizontalFloatSlidingWindowsAndNonIntegralRatios) {
  const auto* table = EightLaneTable();
  if (!table)
    GTEST_SKIP();
  bool regular = false, general = false;
  for (int kind = 0; kind < 13; ++kind) {
    const auto function = DefaultFunction(FilterKind(kind));
    ASSERT_NE(function, nullptr);
    for (int sw : {256, 259})
      for (int width : {65, 97, 128, 129, 171})
        for (double start : {-0.25, 0.0, 0.5}) {
          auto plan = BuildCoefficients(*function, {sw, width, start, double(sw), 32});
          PrepareHorizontal(plan, table->lanes);
          for (const auto& block : plan.horizontal.blocks) {
            regular = regular || block.stride_two;
            general = general || (block.sliding_window && !block.stride_two);
          }
          std::vector<float> source(size_t(sw) * 3), expected(size_t(width) * 3), actual(expected);
          for (size_t i = 0; i < source.size(); ++i)
            source[i] = float(int((i * 997) % 4096) - 2048) / 128;
          const vc_const_plane src{source.data() + size_t(sw) * 2, -sw * 4};
          const vc_plane ref{expected.data() + size_t(width) * 2, -width * 4};
          const vc_plane dst{actual.data() + size_t(width) * 2, -width * 4};
          const vc_rows rows{width, 3, 0, 3};
          ASSERT_EQ(ExecuteC(plan, Axis::Horizontal, src, ref, rows), VC_OK);
          table->horizontal_float(plan, src, dst, rows, 0, 0);
          ASSERT_EQ(actual, expected) << "filter=" << kind << " source=" << sw << " target=" << width
                                      << " crop=" << start;
        }
  }
  EXPECT_TRUE(regular);
  EXPECT_TRUE(general);
}
TEST(ResampleHighwayContract, HorizontalFloatAllFunctionsAndPartialBands) {
  const auto* table = GetResampleKernels(VC_TARGET_NATIVE);
  const auto kernel = table ? table->horizontal_float : nullptr;
  if (!kernel)
    GTEST_SKIP();
  for (const auto& fixture : kMathGolden) {
    const auto function = MakeFunction(fixture);
    for (int target : {1, 7, 17, 65, 129}) {
      auto plan = BuildCoefficients(*function, {31, target, -0.5, 32, 32});
      std::vector<float> source(31 * 3), expected(size_t(target) * 3), actual(expected);
      for (size_t i = 0; i < source.size(); ++i)
        source[i] = float(int(i % 97) - 48) / 16;
      const vc_const_plane src{source.data() + 62, -31 * 4};
      const vc_rows rows{target, 3, 0, 3};
      ASSERT_EQ(ExecuteC(plan, Axis::Horizontal, src, {expected.data(), target * 4}, rows), VC_OK);
      for (int y = 2; y >= 0; --y)
        kernel(plan, {source.data() + size_t(2 - y) * 31, 31 * 4}, {actual.data() + size_t(y) * target, target * 4},
               {target, 3, y, 1}, y, y);
      EXPECT_EQ(actual, expected);
      PrepareHorizontal(plan, table->lanes);
      for (int y = 2; y >= 0; --y)
        kernel(plan, {source.data() + size_t(2 - y) * 31, 31 * 4}, {actual.data() + size_t(y) * target, target * 4},
               {target, 3, y, 1}, y, y);
      EXPECT_EQ(actual, expected) << fixture.name << " target=" << target;
    }
  }
}
template <class T>
void CheckLargeVerticalStore(int bits, bool pair = false,
                             const ResampleKernels* table = GetResampleKernels(VC_TARGET_NATIVE)) {
  if (!table)
    GTEST_SKIP();
  const int width = 4097, height = 129;
  const int pitch = ((width * int(sizeof(T)) + 63) & ~63) / int(sizeof(T));
  const auto plan = pair ? BuildCoefficients(TriangleFilter(), {3, height, 0, 3, bits})
                         : BuildCoefficients(LanczosFilter(3), {3, height, 0, 3, bits});
  std::vector<T> source(size_t(pitch) * 3), expected(size_t(pitch) * height, T(37));
  auto destroy = [](T* data) {
    ::operator delete[](data, std::align_val_t(64));
  };
  std::unique_ptr<T[], decltype(destroy)> actual(new (std::align_val_t(64)) T[size_t(pitch) * height], destroy);
  std::fill_n(actual.get(), size_t(pitch) * height, T(37));
  for (size_t i = 0; i < source.size(); ++i)
    source[i] = T(i % 127);
  const vc_const_plane src{source.data(), ptrdiff_t(pitch * sizeof(T))};
  const vc_plane ref{expected.data() + size_t(height - 1) * pitch, -ptrdiff_t(pitch * sizeof(T))};
  const vc_plane dst{actual.get() + size_t(height - 1) * pitch, ref.stride};
  const vc_rows rows{width, height, 0, height};
  ASSERT_EQ(ExecuteC(plan, Axis::Vertical, src, ref, rows), VC_OK);
  table->vertical(plan, src, dst, rows, 0, 0);
  for (size_t i = 0; i < expected.size(); ++i)
    ASSERT_EQ(actual[i], expected[i]) << "offset=" << i;
}
TEST(ResampleHighwayContract, LargeAlignedNegativeStrideOutputAndTail) {
  CheckLargeVerticalStore<uint8_t>(8);
  CheckLargeVerticalStore<uint16_t>(16);
  CheckLargeVerticalStore<float>(32);
}
TEST(ResampleHighwayContract, TwoTapU8AllSamplePairsRoundingAndSaturation) {
  const auto* table = GetResampleKernels(VC_TARGET_NATIVE);
  if (!table)
    GTEST_SKIP();
  constexpr int width = 256 * 256;
  std::vector<uint8_t> source(width * 2), expected(width), actual(width);
  for (int x = 0; x < width; ++x) {
    source[x] = uint8_t(x / 256);
    source[width + x] = uint8_t(x % 256);
  }
  for (int w : {-16383, -8192, -1, 0, 1, 8191, 8192, 8193, 16383, 16384, 16385, 24576, 32767}) {
    const Coefficients plan{2, 1, 8, 2, 2, {0}, {2}, {int16_t(16384 - w), int16_t(w)}, {}};
    const vc_const_plane src{source.data(), width};
    const vc_rows rows{width, 1, 0, 1};
    ASSERT_EQ(ExecuteC(plan, Axis::Vertical, src, {expected.data(), width}, rows), VC_OK);
    table->vertical(plan, src, {actual.data(), width}, rows, 0, 0);
    ASSERT_EQ(actual, expected) << "second coefficient=" << w;
  }
  CheckLargeVerticalStore<uint8_t>(8, true);
}
TEST(ResampleHighwayContract, TwoTapU16AllDifferencesAndCoefficientExtremes) {
  const auto* table = EightLaneTable();
  if (!table)
    GTEST_SKIP();
  for (int bits : {9, 10, 12, 14, 15, 16}) {
    const int count = 1 << bits, limit = count - 1, width = count * 3;
    std::vector<uint16_t> source(size_t(width) * 2), expected(width), actual(width);
    for (int x = 0; x < width; ++x) {
      source[x] = uint16_t(x < count ? 0 : x < 2 * count ? limit : limit / 2);
      source[width + x] = uint16_t(x % count);
    }
    for (int w : {-24575, -8192, -1, 0, 1, 4095, 4096, 4097, 8191, 8192, 8193, 16384, 32767}) {
      const Coefficients plan{2, 1, bits, 2, 2, {0}, {2}, {int16_t(8192 - w), int16_t(w)}, {}};
      const vc_const_plane src{source.data(), width * 2};
      const vc_rows rows{width, 1, 0, 1};
      ASSERT_EQ(ExecuteC(plan, Axis::Vertical, src, {expected.data(), width * 2}, rows), VC_OK);
      table->vertical(plan, src, {actual.data(), width * 2}, rows, 0, 0);
      ASSERT_EQ(actual, expected) << "bits=" << bits << " second coefficient=" << w;
    }
  }
  CheckLargeVerticalStore<uint16_t>(16, true, table);
}
TEST(ResampleHighwayContract, LargeHorizontalFloatStreamAndPartialNegativeStrideBand) {
  const auto* table = GetResampleKernels(VC_TARGET_NATIVE);
  if (!table)
    GTEST_SKIP();
  constexpr int width = 4097, source_width = 3073, height = 129;
  constexpr int pitch = (width + 15) & ~15;
  auto plan = BuildCoefficients(Spline36Filter(), {source_width, width, -0.5, source_width + 1.0, 32});
  PrepareHorizontal(plan, table->lanes);
  std::vector<float> source(size_t(source_width) * height), expected(size_t(pitch) * height, 37.0f);
  for (size_t i = 0; i < source.size(); ++i)
    source[i] = float(int((i * 997) % 4096) - 2048) / 128;
  auto destroy = [](float* data) {
    ::operator delete[](data, std::align_val_t(64));
  };
  std::unique_ptr<float[], decltype(destroy)> actual(new (std::align_val_t(64)) float[size_t(pitch) * height], destroy);
  std::fill_n(actual.get(), expected.size(), 37.0f);
  const vc_const_plane src{source.data() + size_t(source_width) * (height - 1), -source_width * 4};
  const vc_plane ref{expected.data() + size_t(pitch) * (height - 1), -pitch * 4};
  const vc_plane dst{actual.get() + size_t(pitch) * (height - 1), -pitch * 4};
  const vc_rows rows{width, height, 7, 117};
  ASSERT_EQ(ExecuteC(plan, Axis::Horizontal, src, ref, rows), VC_OK);
  table->horizontal_float(plan, src, dst, rows, 0, 0);
  for (size_t i = 0; i < expected.size(); ++i)
    ASSERT_EQ(actual[i], expected[i]) << "offset=" << i;
}
template <class T>
void CheckLongHorizontal(int bits, const ResampleKernels* table = GetResampleKernels(VC_TARGET_NATIVE)) {
  if (!table)
    GTEST_SKIP();
  for (int taps : {4, 5, 8, 9, 15, 30}) {
    for (int source_width : {63, 129, 257}) {
      for (int width : {17, 65, 131}) {
        auto plan = BuildCoefficients(SincLin2Filter(taps), {source_width, width, -.25, double(source_width), bits});
        PrepareHorizontal(plan, table->lanes);
        constexpr int height = 3;
        // Exact allocations exercise the last source row under ASan. Negative
        // strides and separate bands also exercise source/destination offsets.
        std::vector<T> input(source_width * height), expected(width * height), actual(width * height);
        for (size_t i = 0; i < input.size(); ++i) {
          if constexpr (std::is_same_v<T, float>)
            input[i] = float(int((i * 7919) % 65536) - 32768) / 16384;
          else
            input[i] = T((i * 7919) & ((1 << bits) - 1));
        }
        const vc_const_plane src{input.data() + source_width * (height - 1), -ptrdiff_t(source_width * sizeof(T))};
        const vc_plane ref{expected.data() + width * (height - 1), -ptrdiff_t(width * sizeof(T))};
        const vc_plane dst{actual.data() + width * (height - 1), -ptrdiff_t(width * sizeof(T))};
        ASSERT_EQ(ExecuteC(plan, Axis::Horizontal, src, ref, {width, height, 0, height}), VC_OK);
        const auto kernel = std::is_same_v<T, float> ? table->horizontal_float : table->horizontal_integer;
        kernel(plan, src, dst, {width, height, 0, 1}, 0, 0);
        kernel(plan, src, dst, {width, height, 1, 2}, 0, 0);
        ASSERT_EQ(actual, expected) << "bits=" << bits << " taps=" << taps << " source=" << source_width
                                    << " width=" << width;
      }
    }
  }
}
TEST(ResampleHighwayContract, LongHorizontalSupportExactBuffers) {
  CheckLongHorizontal<uint8_t>(8);
  CheckLongHorizontal<uint16_t>(10);
  CheckLongHorizontal<uint16_t>(16);
  CheckLongHorizontal<float>(32);
}
TEST(ResampleHighwayContract, LongHorizontalEightLaneExactBuffers) {
  const auto* table = EightLaneTable();
  CheckLongHorizontal<uint8_t>(8, table);
  CheckLongHorizontal<uint16_t>(16, table);
  CheckLongHorizontal<float>(32, table);
}

template <class T>
void CheckSaturatingNarrowing(int bits) {
  const auto* table = GetResampleKernels(VC_TARGET_NATIVE);
  if (!table)
    GTEST_SKIP();
  const int limit = (1 << bits) - 1;
  for (int taps : {3, 4}) {
    std::vector<int16_t> coefficients =
        bits == 8 ? std::vector<int16_t>{-16384, 32767, 1} : std::vector<int16_t>{-8192, 24576, -8192};
    coefficients.resize(taps);
    for (auto axis : {Axis::Horizontal, Axis::Vertical}) {
      const bool horizontal = axis == Axis::Horizontal;
      const int width = 129, target = horizontal ? width : 1, source_size = horizontal ? taps + 2 : taps;
      Coefficients plan{source_size, target, bits, taps, taps, std::vector<int>(target), std::vector<int>(target, taps),
                        {},          {}};
      plan.max_abs_sum = bits == 8 ? 49152 : 40960;
      for (int i = 0; i < target; ++i) {
        plan.offsets[i] = horizontal ? i % 3 : 0;
        plan.integers.insert(plan.integers.end(), coefficients.begin(), coefficients.end());
      }
      std::vector<T> source(horizontal ? source_size : size_t(width) * taps), expected(width), actual(width);
      for (size_t i = 0; i < source.size(); ++i)
        source[i] = T(i % 2 ? limit : 0);
      const vc_const_plane src{source.data(), ptrdiff_t((horizontal ? source_size : width) * sizeof(T))};
      const vc_rows rows{width, 1, 0, 1};
      ASSERT_EQ(ExecuteC(plan, axis, src, {expected.data(), ptrdiff_t(width * sizeof(T))}, rows), VC_OK);
      if (horizontal)
        PrepareHorizontal(plan, table->lanes);
      const auto kernel = horizontal ? table->horizontal_integer : table->vertical;
      kernel(plan, src, {actual.data(), ptrdiff_t(width * sizeof(T))}, rows, 0, 0);
      ASSERT_EQ(actual, expected) << "bits=" << bits << " taps=" << taps;
      EXPECT_NE(std::find(actual.begin(), actual.end(), T(0)), actual.end());
      EXPECT_NE(std::find(actual.begin(), actual.end(), T(limit)), actual.end());
    }
  }
}
TEST(ResampleHighwayContract, SaturatingNarrowingStorageAndEffectiveDepth) {
  CheckSaturatingNarrowing<uint8_t>(8);
  for (int bits : {9, 10, 12, 13, 14, 15, 16})
    CheckSaturatingNarrowing<uint16_t>(bits);
}
TEST(ResampleHighwayContract, ExtremeCoefficientRowsUseWideAccumulator) {
  const auto* table = GetResampleKernels(VC_TARGET_NATIVE);
  const auto kernel = table ? table->vertical : nullptr;
  if (!kernel)
    GTEST_SKIP();
  Coefficients plan{32, 1, 16, 32, 32, {0}, {32}, std::vector<int16_t>(32, 32767), {}};
  constexpr int width = 65;
  std::vector<uint16_t> source(width * 32, 65535), actual(width), expected(width);
  const vc_const_plane src{source.data(), width * 2};
  ASSERT_EQ(ExecuteC(plan, Axis::Vertical, src, {expected.data(), width * 2}, {width, 1, 0, 1}), VC_OK);
  kernel(plan, src, {actual.data(), width * 2}, {width, 1, 0, 1}, 0, 0);
  EXPECT_EQ(actual, expected);
  EXPECT_EQ(actual.front(), 65535);
}
INSTANTIATE_TEST_SUITE_P(OriginalCoefficients, ResampleExecution, testing::ValuesIn(kCoefficientGolden),
                         [](const testing::TestParamInfo<CoefficientGolden>& info) { return info.param.name; });
TEST(ResampleExecutionContract, InvalidRequestsDoNotWriteAndEmptyRequestsDoNotAccess) {
  const auto plan = BuildCoefficients(TriangleFilter(), {3, 5, 0, 3, 8});
  uint8_t source[9] = {}, destination[15];
  std::fill_n(destination, 15, 42);
  EXPECT_EQ(ExecuteC(plan, Axis::Horizontal, {source, 3}, {destination, 5}, {5, 3, 0, 4}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(ExecuteC(plan, Axis::Horizontal, {source, 2}, {destination, 5}, {5, 3, 0, 3}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(ExecuteC(plan, Axis::Horizontal, {source, 3}, {destination, 5}, {4, 3, 0, 3}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(ExecuteC(plan, Axis::Horizontal, {nullptr, 0}, {nullptr, 0}, {5, 3, 3, 0}), VC_OK);
  for (auto value : destination)
    EXPECT_EQ(value, 42);
}
TEST(ResampleExecutionContract, IntegerDepthsAndSaturation) {
  for (int bits : {9, 10, 12, 14, 15, 16}) {
    const auto plan = BuildCoefficients(LanczosFilter(3), {8, 23, 0, 8, bits});
    std::vector<uint16_t> source(8, (1 << bits) - 1), output(23);
    EXPECT_EQ(ExecuteC(plan, Axis::Horizontal, {source.data(), 16}, {output.data(), 46}, {23, 1, 0, 1}), VC_OK);
    for (auto value : output)
      EXPECT_EQ(value, (1 << bits) - 1);
  }
}
} // namespace
} // namespace vc_test
