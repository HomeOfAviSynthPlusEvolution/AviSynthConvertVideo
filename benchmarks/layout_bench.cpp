// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "video_convert/layout.h"
#include <hwy/targets.h>
#include <hwy/cache_control.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <new>
#include <string>
#include <vector>

#ifdef VC_BENCH_AVS_REFERENCE
template <class T, bool Alpha>
void convert_rgb_to_rgbp_avx2(const uint8_t*, uint8_t* (&)[4], int, int (&)[4], int, int, int);
template <class T, bool Alpha>
void convert_rgba_to_rgbp_ssse3(const uint8_t*, uint8_t* (&)[4], int, int (&)[4], int, int);
template <class T, bool Alpha>
void convert_rgbp_to_rgba_sse2(const uint8_t* (&)[4], uint8_t*, int (&)[4], int, int, int);
void convert_yuy2_to_yv16_sse2(const uint8_t*, uint8_t*, uint8_t*, uint8_t*, size_t, size_t, size_t, size_t, size_t);
void convert_yv16_to_yuy2_sse2(const uint8_t*, const uint8_t*, const uint8_t*, uint8_t*, size_t, size_t, size_t, size_t,
                               size_t);
#endif

#ifdef VC_BENCH_AVX512_REFERENCE
template <class T, bool Alpha>
void convert_rgba_to_rgbp_avx512vbmi(const uint8_t*, uint8_t* (&)[4], int, int (&)[4], int, int);
#endif

namespace {
struct Buffer {
  uint8_t* data;
  int pitch;
  size_t size;
  Buffer(int bytes, int height) : pitch((bytes + 63) & ~63), size(size_t(pitch) * height + 64) {
    data = static_cast<uint8_t*>(::operator new(size, std::align_val_t(64)));
    for (size_t i = 0; i < size; ++i)
      data[i] = uint8_t(i * 37 + i / 127);
  }
  ~Buffer() { ::operator delete(data, std::align_val_t(64)); }
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
};
using Operation = std::function<void()>;
volatile uint64_t sink = 0;

double Measure(const Operation& operation) {
  using Clock = std::chrono::steady_clock;
  operation();
  const auto start = Clock::now();
  for (int i = 0; i < 4; ++i)
    operation();
  const double estimate = std::chrono::duration<double>(Clock::now() - start).count() / 4;
  const int iterations = std::max(2, std::min(100000, int(0.015 / std::max(estimate, 1e-9))));
  std::array<double, 5> samples{};
  for (double& sample : samples) {
    const auto begin = Clock::now();
    for (int i = 0; i < iterations; ++i)
      operation();
    sample = std::chrono::duration<double, std::micro>(Clock::now() - begin).count() / iterations;
  }
  std::sort(samples.begin(), samples.end());
  return samples[2];
}

void Require(int status) {
  if (status != VC_OK) {
    std::fprintf(stderr, "layout rejected benchmark input\n");
    std::exit(2);
  }
}

void Compare(const char* route, int width, int height, const std::vector<std::pair<std::string, Operation>>& operations,
             const std::vector<std::pair<Buffer*, int>>& outputs) {
  operations.front().second();
  std::vector<std::vector<uint8_t>> expected;
  for (const auto& output : outputs) {
    std::vector<uint8_t> active(size_t(output.second) * height);
    for (int y = 0; y < height; ++y)
      std::memcpy(active.data() + size_t(y) * output.second, output.first->data + size_t(y) * output.first->pitch,
                  output.second);
    expected.push_back(std::move(active));
  }
  for (const auto& operation : operations) {
    for (const auto& output : outputs)
      std::memset(output.first->data, 0xA5, output.first->size);
    operation.second();
    for (size_t p = 0; p < outputs.size(); ++p)
      for (int y = 0; y < height; ++y)
        if (std::memcmp(expected[p].data() + size_t(y) * outputs[p].second,
                        outputs[p].first->data + size_t(y) * outputs[p].first->pitch, outputs[p].second)) {
          std::fprintf(stderr, "output mismatch: %s %s\n", route, operation.first.c_str());
          std::exit(3);
        }
    const double us = Measure(operation.second);
    sink += outputs.front().first->data[0];
    std::printf("%s,%d,%d,%s,%.3f\n", route, width, height, operation.first.c_str(), us);
    std::fflush(stdout);
  }
}

template <class T>
void Rgb(int width, int height, int components, bool alpha, bool unpack) {
  constexpr int storage = sizeof(T) == 1 ? VC_U8 : VC_U16;
  const uint32_t fill = sizeof(T) == 1 ? 255 : 65535;
  Buffer packed(width * components * sizeof(T), height);
  Buffer r(width * sizeof(T), height), g(width * sizeof(T), height), b(width * sizeof(T), height),
      a(width * sizeof(T), height);
  const auto rows = vc_rows{width, height, 0, height};
  // Match AVS bottom-up packed storage for both new and old kernels.
  vc_const_plane packed_src{packed.data + size_t(height - 1) * packed.pitch, -packed.pitch};
  vc_plane packed_dst{packed.data + size_t(height - 1) * packed.pitch, -packed.pitch};
  vc_rgb_planes dst{{r.data, r.pitch}, {g.data, g.pitch}, {b.data, b.pitch}, {alpha ? a.data : nullptr, a.pitch}};
  vc_const_rgb_planes src{{r.data, r.pitch}, {g.data, g.pitch}, {b.data, b.pitch}, {alpha ? a.data : nullptr, a.pitch}};
  std::vector<std::pair<std::string, Operation>> operations;
  for (const auto& target :
       {std::pair<const char*, int64_t>{"C", VC_TARGET_C}, {"AVX2", HWY_AVX2}, {"native", VC_TARGET_NATIVE}}) {
    const auto* functions = vc_get_layout_functions(target.second);
    if (!functions)
      continue;
    operations.emplace_back(target.first, [=] {
      Require(unpack ? functions->unpack_bgr(packed_src, dst, storage, components, fill, rows)
                     : functions->pack_bgr(src, packed_dst, storage, components, fill, rows));
    });
  }
#ifdef VC_BENCH_AVS_REFERENCE
  const auto supported = hwy::SupportedTargets();
  if ((unpack && components == 3 && (supported & HWY_AVX2)) || (unpack && components == 4 && (supported & HWY_SSSE3)) ||
      (!unpack && components == 4)) {
    operations.emplace_back("old_simd", [&] {
      uint8_t* planes[4]{g.data, b.data, r.data, a.data};
      const uint8_t* inputs[4]{g.data, b.data, r.data, a.data};
      int pitches[4]{g.pitch, b.pitch, r.pitch, alpha ? a.pitch : 0};
      if (unpack && components == 3) {
        if (alpha)
          convert_rgb_to_rgbp_avx2<T, true>(static_cast<const uint8_t*>(packed_src.data), planes, packed.pitch, pitches,
                                            width, height, sizeof(T) * 8);
        else
          convert_rgb_to_rgbp_avx2<T, false>(static_cast<const uint8_t*>(packed_src.data), planes, packed.pitch,
                                             pitches, width, height, sizeof(T) * 8);
      } else if (unpack) {
        if (alpha)
          convert_rgba_to_rgbp_ssse3<T, true>(static_cast<const uint8_t*>(packed_src.data), planes, packed.pitch,
                                              pitches, width, height);
        else
          convert_rgba_to_rgbp_ssse3<T, false>(static_cast<const uint8_t*>(packed_src.data), planes, packed.pitch,
                                               pitches, width, height);
      } else {
        if (alpha)
          convert_rgbp_to_rgba_sse2<T, true>(inputs, static_cast<uint8_t*>(packed_dst.data), pitches, packed.pitch,
                                             width, height);
        else
          convert_rgbp_to_rgba_sse2<T, false>(inputs, static_cast<uint8_t*>(packed_dst.data), pitches, packed.pitch,
                                              width, height);
      }
    });
  }
#endif
#ifdef VC_BENCH_AVX512_REFERENCE
  if (unpack && components == 4 && (hwy::SupportedTargets() & (HWY_AVX3_DL | HWY_AVX3_ZEN4 | HWY_AVX3_SPR))) {
    operations.emplace_back("upstream_AVX512", [&] {
      uint8_t* planes[4]{g.data, b.data, r.data, a.data};
      int pitches[4]{g.pitch, b.pitch, r.pitch, a.pitch};
      if (alpha)
        convert_rgba_to_rgbp_avx512vbmi<T, true>(static_cast<const uint8_t*>(packed_src.data), planes, packed.pitch,
                                                 pitches, width, height);
      else
        convert_rgba_to_rgbp_avx512vbmi<T, false>(static_cast<const uint8_t*>(packed_src.data), planes, packed.pitch,
                                                  pitches, width, height);
    });
  }
#endif
  std::vector<std::pair<Buffer*, int>> outputs;
  if (unpack) {
    outputs = {{&r, width * sizeof(T)}, {&g, width * sizeof(T)}, {&b, width * sizeof(T)}};
    if (alpha)
      outputs.emplace_back(&a, width * sizeof(T));
  } else
    outputs.emplace_back(&packed, width * components * sizeof(T));
  const auto route = std::string(unpack ? "unpack_" : "pack_") + (components == 3 ? "BGR" : "BGRA") +
                     (sizeof(T) == 1 ? "8" : "16") + (alpha ? "_alpha" : "_noalpha");
  Compare(route.c_str(), width, height, operations, outputs);
}

void Yuy2(int width, int height, bool unpack) {
  Buffer packed(width * 2, height), y(width, height), u(width / 2, height), v(width / 2, height);
  const vc_rows rows{width, height, 0, height};
  const vc_const_plane packed_src{packed.data, packed.pitch};
  const vc_plane packed_dst{packed.data, packed.pitch};
  const vc_yuv_planes dst{{y.data, y.pitch}, {u.data, u.pitch}, {v.data, v.pitch}};
  const vc_const_yuv_planes src{{y.data, y.pitch}, {u.data, u.pitch}, {v.data, v.pitch}};
  std::vector<std::pair<std::string, Operation>> operations;
  for (const auto& target :
       {std::pair<const char*, int64_t>{"C", VC_TARGET_C}, {"AVX2", HWY_AVX2}, {"native", VC_TARGET_NATIVE}}) {
    const auto* functions = vc_get_layout_functions(target.second);
    if (!functions)
      continue;
    operations.emplace_back(target.first, [=] {
      Require(unpack ? functions->unpack_yuy2(packed_src, dst, rows) : functions->pack_yuy2(src, packed_dst, rows));
    });
  }
#ifdef VC_BENCH_AVS_REFERENCE
  operations.emplace_back("old_simd", [&] {
    if (unpack)
      convert_yuy2_to_yv16_sse2(packed.data, y.data, u.data, v.data, packed.pitch, y.pitch, u.pitch, width, height);
    else
      convert_yv16_to_yuy2_sse2(y.data, u.data, v.data, packed.data, y.pitch, u.pitch, packed.pitch, width, height);
    // The old packer uses non-temporal stores without an internal fence.
    // Complete them before observing the output or finishing the timed call.
    if (!unpack)
      hwy::FlushStream();
  });
#endif
  Compare(unpack ? "unpack_YUY2" : "pack_YUY2", width, height, operations,
          unpack ? std::vector<std::pair<Buffer*, int>>{{&y, width}, {&u, width / 2}, {&v, width / 2}}
                 : std::vector<std::pair<Buffer*, int>>{{&packed, width * 2}});
}
} // namespace

int main(int argc, char** argv) {
  const bool yuy2_only = argc == 2 && std::strcmp(argv[1], "--yuy2-only") == 0;
  std::fprintf(stderr, "native target: %s\n", hwy::TargetName(vc_layout_choose_target(VC_TARGET_NATIVE)));
  std::puts("route,width,height,variant,microseconds");
  for (const auto& size : {std::pair<int, int>{256, 32}, {1920, 1080}, {3840, 2160}}) {
    if (!yuy2_only)
      for (int components : {3, 4})
        for (bool alpha : {false, true})
          for (bool unpack : {false, true}) {
            Rgb<uint8_t>(size.first, size.second, components, alpha, unpack);
            Rgb<uint16_t>(size.first, size.second, components, alpha, unpack);
          }
    Yuy2(size.first, size.second, false);
    Yuy2(size.first, size.second, true);
  }
}
