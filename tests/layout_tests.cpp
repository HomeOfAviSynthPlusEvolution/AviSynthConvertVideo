#include "video_convert/layout.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

namespace {
// Logical row views with independent pitches, orientation and guard/padding checks.
template <class T>
struct Buffer {
  static constexpr T guard = T(0xA5);
  int width, height, pitch;
  bool inverted;
  std::vector<T> data;
  Buffer(int w, int h, int padding, bool reverse)
      : width(w), height(h), pitch(w + padding), inverted(reverse), data((w + padding) * h + 2, guard) {}
  T* row(int y) { return data.data() + 1 + (inverted ? height - 1 - y : y) * pitch; }
  ptrdiff_t stride() const { return ptrdiff_t(pitch) * sizeof(T) * (inverted ? -1 : 1); }
  vc_plane view() { return {row(0), stride()}; }
  vc_const_plane read() { return {row(0), stride()}; }
  void check_guards() const {
    EXPECT_EQ(data.front(), guard);
    EXPECT_EQ(data.back(), guard);
    for (int y = 0; y < height; ++y)
      for (int x = width; x < pitch; ++x)
        EXPECT_EQ(data[1 + y * pitch + x], guard);
  }
};

// Like the AVS packed-layout reference, use distinct channel anchors. Expected
// values come from coordinates, not a pack/unpack round trip or the new kernels.
template <class T>
T sample(int x, int y, int channel) {
  return T((x * 37 + y * 59 + channel * 71) * (sizeof(T) == 2 ? 257 : 1));
}

using RgbCase = std::tuple<int, int, bool, bool, int, int64_t>;
class RgbLayout : public testing::TestWithParam<RgbCase> {};

template <class T>
void CheckRgb(int components, bool alpha, bool reverse, int width, const vc_layout_functions& functions) {
  constexpr int height = 5;
  constexpr int storage = sizeof(T) == 1 ? VC_U8 : VC_U16;
  constexpr T fill = T(123);
  Buffer<T> packed(width * components, height, 3, reverse);
  Buffer<T> r(width, height, 1, !reverse), g(width, height, 2, reverse), b(width, height, 4, false);
  Buffer<T> a(width, height, 5, true), output(width * components, height, 7, !reverse);
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
      for (int c = 0; c < components; ++c)
        packed.row(y)[x * components + c] = sample<T>(x, y, c);
  const auto original = packed.data;
  vc_rgb_planes dst{r.view(), g.view(), b.view(), alpha ? a.view() : vc_plane{nullptr, 0}};
  const auto original_dst = dst;
  // Out-of-order bands use global row coordinates and leave other rows untouched.
  ASSERT_EQ(functions.unpack_bgr(packed.read(), dst, storage, components, fill, {width, height, 2, 2}), VC_OK);
  for (int y : {0, 1, 4})
    for (int x = 0; x < width; ++x) {
      EXPECT_EQ(r.row(y)[x], Buffer<T>::guard);
      EXPECT_EQ(g.row(y)[x], Buffer<T>::guard);
      EXPECT_EQ(b.row(y)[x], Buffer<T>::guard);
    }
  ASSERT_EQ(functions.unpack_bgr(packed.read(), dst, storage, components, fill, {width, height, 0, 2}), VC_OK);
  ASSERT_EQ(functions.unpack_bgr(packed.read(), dst, storage, components, fill, {width, height, 4, 1}), VC_OK);
  EXPECT_EQ(dst.r.data, original_dst.r.data);
  EXPECT_EQ(dst.r.stride, original_dst.r.stride);
  EXPECT_EQ(packed.data, original);
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x) {
      EXPECT_EQ(b.row(y)[x], sample<T>(x, y, 0));
      EXPECT_EQ(g.row(y)[x], sample<T>(x, y, 1));
      EXPECT_EQ(r.row(y)[x], sample<T>(x, y, 2));
      EXPECT_EQ(a.row(y)[x], alpha ? (components == 4 ? sample<T>(x, y, 3) : fill) : Buffer<T>::guard);
      // Independent planar input for the reverse direction.
      a.row(y)[x] = sample<T>(x, y, 3);
    }
  vc_const_rgb_planes src{r.read(), g.read(), b.read(), alpha ? a.read() : vc_const_plane{nullptr, 0}};
  const auto before_r = r.data, before_g = g.data, before_b = b.data, before_a = a.data;
  ASSERT_EQ(functions.pack_bgr(src, output.view(), storage, components, fill, {width, height, 0, height}), VC_OK);
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
      for (int c = 0; c < components; ++c)
        EXPECT_EQ(output.row(y)[x * components + c], c == 3 && !alpha ? fill : sample<T>(x, y, c));
  EXPECT_EQ(r.data, before_r);
  EXPECT_EQ(g.data, before_g);
  EXPECT_EQ(b.data, before_b);
  EXPECT_EQ(a.data, before_a);
  // Compare full-plane and reverse single-row execution byte-for-byte, padding included.
  Buffer<T> banded(width * components, height, 7, !reverse);
  for (int y = height - 1; y >= 0; --y)
    ASSERT_EQ(functions.pack_bgr(src, banded.view(), storage, components, fill, {width, height, y, 1}), VC_OK);
  EXPECT_EQ(output.data, banded.data);
  packed.check_guards();
  r.check_guards();
  g.check_guards();
  b.check_guards();
  a.check_guards();
  output.check_guards();
}

TEST_P(RgbLayout, ExactSamplesAlphaOrientationAndBands) {
  const auto [bytes, components, alpha, reverse, width, target] = GetParam();
  const auto* functions = vc_get_layout_functions(target);
  ASSERT_NE(functions, nullptr);
  if (bytes == 1)
    CheckRgb<uint8_t>(components, alpha, reverse, width, *functions);
  else
    CheckRgb<uint16_t>(components, alpha, reverse, width, *functions);
}
INSTANTIATE_TEST_SUITE_P(All, RgbLayout,
                         testing::Combine(testing::Values(1, 2), testing::Values(3, 4), testing::Bool(),
                                          testing::Bool(), testing::Values(1, 2, 7, 16, 17, 33, 65, 129),
                                          testing::Values(VC_TARGET_C, VC_TARGET_NATIVE)));

class Yuy2Layout : public testing::TestWithParam<std::tuple<int, bool, int64_t>> {};
TEST_P(Yuy2Layout, ExactSamplesIndependentChromaStridesAndBands) {
  const auto [width, reverse, target] = GetParam();
  const auto* functions = vc_get_layout_functions(target);
  ASSERT_NE(functions, nullptr);
  constexpr int height = 5;
  Buffer<uint8_t> packed(width * 2, height, 0, reverse), output(width * 2, height, 3, !reverse);
  Buffer<uint8_t> y(width, height, 1, !reverse), u(width / 2, height, 2, reverse), v(width / 2, height, 4, true);
  for (int row = 0; row < height; ++row)
    for (int x = 0; x < width * 2; ++x)
      packed.row(row)[x] = uint8_t(row * 43 + x);
  const auto original = packed.data;
  vc_yuv_planes dst{y.view(), u.view(), v.view()};
  for (int row = height - 1; row >= 0; --row)
    ASSERT_EQ(functions->unpack_yuy2(packed.read(), dst, {width, height, row, 1}), VC_OK);
  for (int row = 0; row < height; ++row)
    for (int x = 0; x < width / 2; ++x) {
      EXPECT_EQ(y.row(row)[2 * x], uint8_t(row * 43 + 4 * x));
      EXPECT_EQ(y.row(row)[2 * x + 1], uint8_t(row * 43 + 4 * x + 2));
      EXPECT_EQ(u.row(row)[x], uint8_t(row * 43 + 4 * x + 1));
      EXPECT_EQ(v.row(row)[x], uint8_t(row * 43 + 4 * x + 3));
    }
  vc_const_yuv_planes src{y.read(), u.read(), v.read()};
  ASSERT_EQ(functions->pack_yuy2(src, output.view(), {width, height, 0, height}), VC_OK);
  for (int row = 0; row < height; ++row)
    for (int x = 0; x < width * 2; ++x)
      EXPECT_EQ(output.row(row)[x], uint8_t(row * 43 + x));
  EXPECT_EQ(packed.data, original);
  packed.check_guards();
  output.check_guards();
  y.check_guards();
  u.check_guards();
  v.check_guards();
}
INSTANTIATE_TEST_SUITE_P(All, Yuy2Layout,
                         testing::Combine(testing::Values(2, 6, 16, 18, 34, 66, 130, 258), testing::Bool(),
                                          testing::Values(VC_TARGET_C, VC_TARGET_NATIVE)));

TEST(LayoutValidation, InvalidRequestsDoNotWrite) {
  std::array<uint16_t, 32> source{}, destination{};
  destination.fill(0xBEEF);
  const auto before = destination;
  vc_plane dst{destination.data(), 8};
  vc_const_plane src{source.data(), 8};
  vc_rgb_planes rgb{dst, dst, dst, {nullptr, 0}};
  EXPECT_EQ(vc_unpack_bgr(src, rgb, VC_U16, 2, 0, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr(src, rgb, 99, 3, 0, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr(src, rgb, VC_U16, 3, 0, {0, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr(src, rgb, VC_U16, 3, 0, {1, 1, -1, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr(src, rgb, VC_U16, 3, 0, {1, 1, 0, 2}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr({source.data(), 5}, rgb, VC_U16, 3, 0, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr({source.data(), 4}, rgb, VC_U16, 3, 0, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr({reinterpret_cast<uint8_t*>(source.data()) + 1, 8}, rgb, VC_U16, 3, 0, {1, 1, 0, 1}),
            VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr({nullptr, 8}, rgb, VC_U16, 3, 0, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr({source.data(), PTRDIFF_MIN}, rgb, VC_U16, 3, 0, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_bgr({source.data(), PTRDIFF_MAX - 1}, rgb, VC_U16, 3, 0, {1, 3, 0, 1}), VC_INVALID_ARGUMENT);
  rgb.a = dst;
  EXPECT_EQ(vc_unpack_bgr(src, rgb, VC_U16, 3, 65536, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_yuy2(src, {dst, dst, dst}, {3, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(destination, before);
}

TEST(LayoutValidation, EmptyRangeDoesNotAccessNullBuffers) {
  const vc_rows rows{2, 3, 3, 0};
  EXPECT_EQ(vc_unpack_bgr({}, {}, VC_U8, 3, 0, rows), VC_OK);
  EXPECT_EQ(vc_pack_bgr({}, {}, VC_U16, 4, 0, rows), VC_OK);
  EXPECT_EQ(vc_unpack_yuy2({}, {}, rows), VC_OK);
  EXPECT_EQ(vc_pack_yuy2({}, {}, rows), VC_OK);
  EXPECT_EQ(vc_unpack_yuy2({}, {}, {2, 3, 4, 0}), VC_INVALID_ARGUMENT);
}

TEST(LayoutValidation, ReverseRoutesValidateEveryPlaneBeforeWriting) {
  std::array<uint8_t, 32> input{}, output{};
  output.fill(0xA5);
  const auto before = output;
  vc_const_plane source{input.data(), 8};
  vc_plane destination{output.data(), 8};
  vc_const_rgb_planes rgb{source, source, {nullptr, 8}, {}};
  EXPECT_EQ(vc_pack_bgr(rgb, destination, VC_U8, 3, 0, {2, 1, 0, 1}), VC_INVALID_ARGUMENT);
  rgb.b = source;
  EXPECT_EQ(vc_pack_bgr(rgb, destination, VC_U8, 4, 256, {2, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_pack_bgr(rgb, {output.data(), 5}, VC_U8, 3, 0, {2, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_pack_bgr(rgb, destination, VC_U8, 3, 0, {2, 1, 0, -1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_pack_yuy2({source, source, {nullptr, 8}}, destination, {2, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_pack_yuy2({source, source, source}, destination, {3, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_unpack_yuy2(source, {destination, destination, {nullptr, 8}}, {2, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(output, before);
}

TEST(LayoutAlpha, DroppedAlphaDescriptorAndUnusedFillAreIgnored) {
  const uint8_t r = 3, g = 2, b = 1, a = 17;
  std::array<uint8_t, 4> output{};
  // A non-null descriptor with an otherwise invalid stride is never accessed
  // when the destination has no alpha component.
  vc_const_rgb_planes src{{&r, 1}, {&g, 1}, {&b, 1}, {&a, PTRDIFF_MIN}};
  ASSERT_EQ(vc_pack_bgr(src, {output.data(), 3}, VC_U8, 3, UINT32_MAX, {1, 1, 0, 1}), VC_OK);
  EXPECT_EQ(output, (std::array<uint8_t, 4>{1, 2, 3, 0}));
  src.a.stride = 1;
  ASSERT_EQ(vc_pack_bgr(src, {output.data(), 4}, VC_U8, 4, UINT32_MAX, {1, 1, 0, 1}), VC_OK);
  EXPECT_EQ(output[3], 17);
}

TEST(LayoutBounds, SinglePixelUsesExactlyThreeComponents) {
  // Exact allocations make any packed tail overread visible under ASan.
  const std::vector<uint16_t> input{65535, 32768, 1023};
  std::vector<uint16_t> r(1), g(1), b(1), a(1), output(3);
  vc_rgb_planes dst{{r.data(), 2}, {g.data(), 2}, {b.data(), 2}, {a.data(), 2}};
  ASSERT_EQ(vc_unpack_bgr({input.data(), 6}, dst, VC_U16, 3, 4095, {1, 1, 0, 1}), VC_OK);
  EXPECT_EQ(r[0], 1023);
  EXPECT_EQ(g[0], 32768);
  EXPECT_EQ(b[0], 65535);
  EXPECT_EQ(a[0], 4095);
  ASSERT_EQ(
      vc_pack_bgr({{r.data(), 2}, {g.data(), 2}, {b.data(), 2}, {}}, {output.data(), 6}, VC_U16, 3, 0, {1, 1, 0, 1}),
      VC_OK);
  EXPECT_EQ(output, input);
}

TEST(LayoutDispatch, CNativeAndTargetValidation) {
  const auto* c = vc_get_layout_functions(VC_TARGET_C);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(c->unpack_bgr, vc_unpack_bgr);
  EXPECT_EQ(c->pack_bgr, vc_pack_bgr);
  EXPECT_EQ(c->unpack_yuy2, vc_unpack_yuy2);
  EXPECT_EQ(c->pack_yuy2, vc_pack_yuy2);
  const auto* native = vc_get_layout_functions(VC_TARGET_NATIVE);
  ASSERT_NE(native, nullptr);
  const int64_t selected = vc_layout_choose_target(VC_TARGET_NATIVE);
  EXPECT_EQ(native, vc_get_layout_functions(selected));
  EXPECT_EQ(vc_layout_choose_target(0), VC_TARGET_C);
  EXPECT_EQ(vc_layout_supported_targets() & ~vc_layout_compiled_targets(), 0);
  EXPECT_EQ(vc_get_layout_functions(-2), nullptr);
  EXPECT_EQ(vc_get_layout_functions(INT64_MAX), nullptr);
  const int64_t missing = (~vc_layout_supported_targets()) & INT64_MAX;
  if (missing)
    EXPECT_EQ(vc_get_layout_functions(missing & -missing), nullptr);
  if (vc_layout_supported_targets() == 0)
    EXPECT_EQ(native, c);
  else
    EXPECT_NE(native->unpack_bgr, c->unpack_bgr);
}

TEST(LayoutDispatch, NativeRejectsInvalidBuffersAndAcceptsEmptyRanges) {
  const auto* native = vc_get_layout_functions(VC_TARGET_NATIVE);
  ASSERT_NE(native, nullptr);
  EXPECT_EQ(native->unpack_bgr({}, {}, VC_U8, 3, 0, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(native->pack_bgr({}, {}, VC_U16, 4, 0, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(native->unpack_yuy2({}, {}, {2, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(native->pack_yuy2({}, {}, {2, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(native->unpack_bgr({}, {}, VC_U8, 3, 0, {1, 1, 1, 0}), VC_OK);
  EXPECT_EQ(native->pack_bgr({}, {}, VC_U16, 4, 0, {1, 1, 1, 0}), VC_OK);
  EXPECT_EQ(native->unpack_yuy2({}, {}, {2, 1, 1, 0}), VC_OK);
  EXPECT_EQ(native->pack_yuy2({}, {}, {2, 1, 1, 0}), VC_OK);
}

TEST(LayoutBounds, LargeAlignedYuy2OutputPreservesTailPaddingAndNegativeStride) {
  constexpr int width = 4098, height = 1024, pitch = (width * 2 + 63) & ~63;
  std::vector<uint8_t> y(size_t(width) * height, 73), u(size_t(width / 2) * height, 19),
      v(size_t(width / 2) * height, 211), output(size_t(pitch) * height + 128, 0xA5);
  auto* aligned = reinterpret_cast<uint8_t*>((reinterpret_cast<uintptr_t>(output.data()) + 63) & ~uintptr_t(63));
  const vc_const_yuv_planes source{{y.data(), width}, {u.data(), width / 2}, {v.data(), width / 2}};
  const vc_plane destination{aligned + size_t(height - 1) * pitch, -pitch};
  const auto* functions = vc_get_layout_functions(VC_TARGET_NATIVE);
  ASSERT_NE(functions, nullptr);
  ASSERT_EQ(functions->pack_yuy2(source, destination, {width, height, 0, height}), VC_OK);
  std::vector<uint8_t> expected(output.size(), 0xA5);
  const size_t offset = size_t(aligned - output.data());
  for (int row = 0; row < height; ++row)
    for (int x = 0; x < width / 2; ++x) {
      auto* p = expected.data() + offset + size_t(row) * pitch + x * 4;
      p[0] = 73;
      p[1] = 19;
      p[2] = 73;
      p[3] = 211;
    }
  EXPECT_EQ(output, expected);
}

TEST(LayoutBounds, NativePackedTailsUseExactAllocations) {
  const auto* native = vc_get_layout_functions(VC_TARGET_NATIVE);
  ASSERT_NE(native, nullptr);
  for (int width : {1, 15, 16, 17, 63, 64, 65, 127, 128, 129}) {
    for (int components : {3, 4}) {
      std::vector<uint8_t> packed(width * components, 31), output(width * components);
      std::vector<uint8_t> r(width), g(width), b(width), a(width);
      vc_rgb_planes dst{{r.data(), width}, {g.data(), width}, {b.data(), width}, {a.data(), width}};
      ASSERT_EQ(native->unpack_bgr({packed.data(), width * components}, dst, VC_U8, components, 31, {width, 1, 0, 1}),
                VC_OK);
      vc_const_rgb_planes src{{r.data(), width}, {g.data(), width}, {b.data(), width}, {a.data(), width}};
      ASSERT_EQ(native->pack_bgr(src, {output.data(), width * components}, VC_U8, components, 31, {width, 1, 0, 1}),
                VC_OK);
      EXPECT_EQ(output, packed);
    }
  }
}
} // namespace

namespace {
template <class T>
void CheckRepack() {
  for (int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE})
    for (int sc : {3, 4})
      for (int dc : {3, 4})
        for (int width : {1, 7, 16, 17, 33, 65})
          for (bool flip : {false, true}) {
            const auto* functions = vc_get_layout_functions(target);
            ASSERT_NE(functions, nullptr);
            Buffer<T> src(width * sc, 3, 5, flip), dst(width * dc, 3, 7, !flip);
            for (int y = 0; y < 3; ++y)
              for (int x = 0; x < width; ++x)
                for (int c = 0; c < sc; ++c)
                  src.row(y)[x * sc + c] = sample<T>(x, y, c);
            const auto saved = src.data;
            const T fill = sizeof(T) == 1 ? T(231) : T(59000);
            for (int y : {2, 0, 1})
              ASSERT_EQ(functions->repack_bgr(src.read(), dst.view(), sizeof(T) == 1 ? VC_U8 : VC_U16, sc, dc, fill,
                                              {width, 3, y, 1}),
                        VC_OK);
            for (int y = 0; y < 3; ++y)
              for (int x = 0; x < width; ++x)
                for (int c = 0; c < dc; ++c)
                  EXPECT_EQ(dst.row(y)[x * dc + c], c == 3 && sc == 3 ? fill : sample<T>(x, y, c));
            EXPECT_EQ(src.data, saved);
            src.check_guards();
            dst.check_guards();
          }
}
TEST(LayoutRepack, ChannelsAlphaIndependentStridesAndBands) {
  CheckRepack<uint8_t>();
  CheckRepack<uint16_t>();
}
TEST(LayoutRepack, InvalidCallsDoNotWrite) {
  for (int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE}) {
    const auto* f = vc_get_layout_functions(target);
    uint8_t src[3] = {1, 2, 3}, dst[4] = {9, 9, 9, 9};
    EXPECT_EQ(f->repack_bgr({src, 3}, {dst, 4}, VC_U8, 3, 4, 256, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
    EXPECT_EQ(f->repack_bgr({src, 3}, {dst, 4}, VC_U8, 2, 4, 255, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
    EXPECT_EQ(f->repack_bgr({src, 3}, {dst, 3}, VC_U8, 3, 4, 255, {1, 1, 0, 1}), VC_INVALID_ARGUMENT);
    for (auto v : dst)
      EXPECT_EQ(v, 9);
    EXPECT_EQ(f->repack_bgr({nullptr, 0}, {nullptr, 0}, VC_U8, 3, 4, 255, {1, 1, 1, 0}), VC_OK);
  }
}
template <class T>
void CheckExactRepack() {
  for (int w : {1, 3, 4, 5, 7, 8, 9, 16, 17, 33, 65})
    for (int sc : {3, 4})
      for (int dc : {3, 4}) {
        std::vector<T> source(w * sc, 99), destination(w * dc, 0);
        ASSERT_EQ(vc_get_layout_functions(VC_TARGET_NATIVE)
                      ->repack_bgr({source.data(), ptrdiff_t(w * sc * sizeof(T))},
                                   {destination.data(), ptrdiff_t(w * dc * sizeof(T))}, sizeof(T) == 1 ? VC_U8 : VC_U16,
                                   sc, dc, 255, {w, 1, 0, 1}),
                  VC_OK);
        for (int x = 0; x < w; ++x)
          for (int c = 0; c < dc; ++c)
            EXPECT_EQ(destination[x * dc + c], c == 3 && sc == 3 ? 255 : 99);
      }
}
TEST(LayoutRepack, ExactAllocationBoundaries) {
  CheckExactRepack<uint8_t>();
  CheckExactRepack<uint16_t>();
}
TEST(Yuy2Luma, BandsStridesAndSourcePreservation) {
  for (int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE}) {
    const auto* f = vc_get_layout_functions(target);
    ASSERT_NE(f, nullptr);
    for (int width : {2, 6, 14, 16, 18, 30, 32, 34, 62, 64, 66, 130})
      for (bool reverse : {false, true}) {
        Buffer<uint8_t> source(width * 2, 3, 7, reverse), luma(width, 3, 5, !reverse);
        for (int y = 0; y < 3; ++y)
          for (int x = 0; x < width * 2; ++x)
            source.row(y)[x] = sample<uint8_t>(x, y, 1);
        const auto before = source.data;
        for (int y : {2, 0, 1}) {
          ASSERT_EQ(f->extract_yuy2_luma(source.read(), luma.view(), {width, 3, y, 1}), VC_OK);
          for (int x = 0; x < width; ++x)
            EXPECT_EQ(luma.row(y)[x], sample<uint8_t>(x * 2, y, 1));
        }
        EXPECT_EQ(source.data, before);
        ASSERT_EQ(f->neutralize_yuy2_chroma(source.view(), {width, 3, 1, 1}), VC_OK);
        for (int y = 0; y < 3; ++y)
          for (int x = 0; x < width * 2; ++x)
            EXPECT_EQ(source.row(y)[x], y == 1 && x % 2 ? 128 : sample<uint8_t>(x, y, 1));
        for (int y : {2, 0})
          ASSERT_EQ(f->neutralize_yuy2_chroma(source.view(), {width, 3, y, 1}), VC_OK);
        for (int y = 0; y < 3; ++y)
          for (int x = 0; x < width * 2; ++x)
            EXPECT_EQ(source.row(y)[x], x % 2 ? 128 : sample<uint8_t>(x, y, 1));
        source.check_guards();
        luma.check_guards();
      }
  }
}
TEST(Yuy2Luma, InvalidArgumentsDoNotWriteAndEmptyBandsDoNotAccessBuffers) {
  for (int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE}) {
    const auto* f = vc_get_layout_functions(target);
    std::array<uint8_t, 16> source{}, destination{};
    source.fill(73);
    destination.fill(91);
    for (vc_rows rows : {vc_rows{3, 1, 0, 1}, vc_rows{4, 1, 1, 1}, vc_rows{4, 1, -1, 1}}) {
      EXPECT_EQ(f->extract_yuy2_luma({source.data(), 8}, {destination.data(), 4}, rows), VC_INVALID_ARGUMENT);
      EXPECT_EQ(f->neutralize_yuy2_chroma({source.data(), 8}, rows), VC_INVALID_ARGUMENT);
    }
    EXPECT_EQ(f->extract_yuy2_luma({source.data(), 7}, {destination.data(), 4}, {4, 1, 0, 1}), VC_INVALID_ARGUMENT);
    EXPECT_EQ(f->extract_yuy2_luma({source.data(), 8}, {destination.data(), 3}, {4, 1, 0, 1}), VC_INVALID_ARGUMENT);
    EXPECT_EQ(f->neutralize_yuy2_chroma({source.data(), 7}, {4, 1, 0, 1}), VC_INVALID_ARGUMENT);
    EXPECT_EQ(f->extract_yuy2_luma({nullptr, 0}, {nullptr, 0}, {4, 1, 1, 0}), VC_OK);
    EXPECT_EQ(f->neutralize_yuy2_chroma({nullptr, 0}, {4, 1, 1, 0}), VC_OK);
    for (auto v : source)
      EXPECT_EQ(v, 73);
    for (auto v : destination)
      EXPECT_EQ(v, 91);
  }
}
TEST(Yuy2Luma, ExactAllocationBoundaries) {
  for (int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE}) {
    const auto* f = vc_get_layout_functions(target);
    for (int w : {2, 6, 14, 16, 18, 30, 32, 34, 62, 64, 66, 130}) {
      std::vector<uint8_t> source(w * 2, 99), luma(w, 0);
      ASSERT_EQ(f->extract_yuy2_luma({source.data(), w * 2}, {luma.data(), w}, {w, 1, 0, 1}), VC_OK);
      ASSERT_EQ(f->neutralize_yuy2_chroma({source.data(), w * 2}, {w, 1, 0, 1}), VC_OK);
      for (int x = 0; x < w; ++x) {
        EXPECT_EQ(luma[x], 99);
        EXPECT_EQ(source[x * 2], 99);
        EXPECT_EQ(source[x * 2 + 1], 128);
      }
    }
  }
}
} // namespace
