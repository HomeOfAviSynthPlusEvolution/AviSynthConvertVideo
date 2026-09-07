// Highway layout kernels developed in AviSynthConvertVideo.
// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "validation.h"
#include <hwy/targets.h>
#include <hwy/cache_control.h>

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "layout/highway.cpp"
#include <hwy/foreach_target.h>
#include <hwy/highway.h>

HWY_BEFORE_NAMESPACE();
namespace vc {
namespace HWY_NAMESPACE {
namespace hn = hwy::HWY_NAMESPACE;

#if HWY_TARGET != HWY_SCALAR && HWY_TARGET != HWY_EMU128

template <class T, int Components>
void UnpackBgr(vc_const_plane source, vc_rgb_planes destination, T fill, vc_rows rows) {
  const hn::ScalableTag<T> d;
  const size_t lanes = hn::Lanes(d);
  const size_t width = static_cast<size_t>(rows.width);
  const size_t end = width - width % lanes;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const T* src = Row<T>(source, y);
    T* r = Row<T>(destination.r, y);
    T* g = Row<T>(destination.g, y);
    T* b = Row<T>(destination.b, y);
    T* a = destination.a.data ? Row<T>(destination.a, y) : nullptr;
    size_t x = 0;
    for (; x < end; x += lanes) {
      auto vb = hn::Zero(d), vg = vb, vr = vb, va = hn::Set(d, fill);
      if constexpr (Components == 3)
        hn::LoadInterleaved3(d, src + x * Components, vb, vg, vr);
      else
        hn::LoadInterleaved4(d, src + x * Components, vb, vg, vr, va);
      hn::StoreU(vb, d, b + x);
      hn::StoreU(vg, d, g + x);
      hn::StoreU(vr, d, r + x);
      if (a)
        hn::StoreU(va, d, a + x);
    }
    for (; x < width; ++x) {
      b[x] = src[x * Components];
      g[x] = src[x * Components + 1];
      r[x] = src[x * Components + 2];
      if (a)
        a[x] = Components == 4 ? src[x * Components + 3] : fill;
    }
  }
}

template <class T, int Components>
void PackBgr(vc_const_rgb_planes source, vc_plane destination, T fill, vc_rows rows) {
  const hn::ScalableTag<T> d;
  const size_t lanes = hn::Lanes(d);
  const size_t width = static_cast<size_t>(rows.width);
  const size_t end = width - width % lanes;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const T* r = Row<T>(source.r, y);
    const T* g = Row<T>(source.g, y);
    const T* b = Row<T>(source.b, y);
    const T* a = Components == 4 && source.a.data ? Row<T>(source.a, y) : nullptr;
    T* dst = Row<T>(destination, y);
    size_t x = 0;
    for (; x < end; x += lanes) {
      const auto vb = hn::LoadU(d, b + x), vg = hn::LoadU(d, g + x), vr = hn::LoadU(d, r + x);
      if constexpr (Components == 3)
        hn::StoreInterleaved3(vb, vg, vr, d, dst + x * Components);
      else {
        const auto va = a ? hn::LoadU(d, a + x) : hn::Set(d, fill);
        hn::StoreInterleaved4(vb, vg, vr, va, d, dst + x * Components);
      }
    }
    for (; x < width; ++x) {
      dst[x * Components] = b[x];
      dst[x * Components + 1] = g[x];
      dst[x * Components + 2] = r[x];
      if constexpr (Components == 4)
        dst[x * Components + 3] = a ? a[x] : fill;
    }
  }
}

template <class T, int SC, int DC>
void Repack(vc_const_plane source, vc_plane destination, T fill, vc_rows rows) {
  const hn::ScalableTag<T> d;
  const size_t n = hn::Lanes(d), w = size_t(rows.width);
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<T>(source, y);
    auto* dst = Row<T>(destination, y);
    size_t x = 0;
    for (; x + n <= w; x += n) {
      auto b = hn::Zero(d), g = b, r = b, a = hn::Set(d, fill);
      if constexpr (SC == 3)
        hn::LoadInterleaved3(d, src + x * SC, b, g, r);
      else
        hn::LoadInterleaved4(d, src + x * SC, b, g, r, a);
      if constexpr (DC == 3)
        hn::StoreInterleaved3(b, g, r, d, dst + x * DC);
      else
        hn::StoreInterleaved4(b, g, r, a, d, dst + x * DC);
    }
    for (; x < w; ++x) {
      for (int c = 0; c < 3; ++c)
        dst[x * DC + c] = src[x * SC + c];
      if constexpr (DC == 4)
        dst[x * DC + 3] = SC == 4 ? src[x * SC + 3] : fill;
    }
  }
}
template <class T>
void SelectRepack(vc_const_plane source, vc_plane destination, int sc, int dc, T fill, vc_rows rows) {
  if (sc == 3) {
    if (dc == 3)
      Repack<T, 3, 3>(source, destination, fill, rows);
    else
      Repack<T, 3, 4>(source, destination, fill, rows);
  } else {
    if (dc == 3)
      Repack<T, 4, 3>(source, destination, fill, rows);
    else
      Repack<T, 4, 4>(source, destination, fill, rows);
  }
}
int RepackBgrEntry(vc_const_plane source, vc_plane destination, int storage, int sc, int dc, uint32_t fill,
                   vc_rows rows) {
  const int status = CheckRepackBgr(source, destination, storage, sc, dc, fill, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  if (storage == VC_U8)
    SelectRepack(source, destination, sc, dc, uint8_t(fill), rows);
  else
    SelectRepack(source, destination, sc, dc, uint16_t(fill), rows);
  return VC_OK;
}

int UnpackBgrEntry(vc_const_plane source, vc_rgb_planes destination, int storage, int components, uint32_t alpha_fill,
                   vc_rows rows) {
  const int status = CheckUnpackBgr(source, destination, storage, components, alpha_fill, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  if (storage == VC_U8) {
    if (components == 3)
      UnpackBgr<uint8_t, 3>(source, destination, uint8_t(alpha_fill), rows);
    else
      UnpackBgr<uint8_t, 4>(source, destination, uint8_t(alpha_fill), rows);
  } else {
    if (components == 3)
      UnpackBgr<uint16_t, 3>(source, destination, uint16_t(alpha_fill), rows);
    else
      UnpackBgr<uint16_t, 4>(source, destination, uint16_t(alpha_fill), rows);
  }
  return VC_OK;
}

int PackBgrEntry(vc_const_rgb_planes source, vc_plane destination, int storage, int components, uint32_t alpha_fill,
                 vc_rows rows) {
  const int status = CheckPackBgr(source, destination, storage, components, alpha_fill, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  if (storage == VC_U8) {
    if (components == 3)
      PackBgr<uint8_t, 3>(source, destination, uint8_t(alpha_fill), rows);
    else
      PackBgr<uint8_t, 4>(source, destination, uint8_t(alpha_fill), rows);
  } else {
    if (components == 3)
      PackBgr<uint16_t, 3>(source, destination, uint16_t(alpha_fill), rows);
    else
      PackBgr<uint16_t, 4>(source, destination, uint16_t(alpha_fill), rows);
  }
  return VC_OK;
}

template <bool Neutralize>
int Luma(vc_const_plane source, vc_plane destination, vc_rows rows) {
  const int status = CheckYuy2Luma(source, destination, rows, Neutralize);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  const hn::ScalableTag<uint8_t> d;
  const size_t lanes = hn::Lanes(d);
  const size_t end = size_t(rows.width) - size_t(rows.width) % lanes;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<uint8_t>(source, y);
    auto* dst = Row<uint8_t>(destination, y);
    size_t x = 0;
    for (; x < end; x += lanes) {
      hn::Vec<decltype(d)> luma, chroma;
      hn::LoadInterleaved2(d, src + 2 * x, luma, chroma);
      if constexpr (Neutralize)
        hn::StoreInterleaved2(luma, hn::Set(d, uint8_t{128}), d, dst + 2 * x);
      else
        hn::StoreU(luma, d, dst + x);
    }
    for (; x < size_t(rows.width); ++x) {
      if constexpr (Neutralize)
        dst[x * 2 + 1] = 128;
      else
        dst[x] = src[x * 2];
    }
  }
  return VC_OK;
}
int ExtractYuy2LumaEntry(vc_const_plane source, vc_plane destination, vc_rows rows) {
  return Luma<false>(source, destination, rows);
}
int NeutralizeYuy2ChromaEntry(vc_plane image, vc_rows rows) {
  return Luma<true>({image.data, image.stride}, image, rows);
}

int UnpackYuy2Entry(vc_const_plane source, vc_yuv_planes destination, vc_rows rows) {
  const int status = CheckUnpackYuy2(source, destination, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  const hn::ScalableTag<uint8_t> d;
  const size_t lanes = hn::Lanes(d);
  const size_t pairs = static_cast<size_t>(rows.width) / 2;
  const size_t end = pairs - pairs % lanes;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<uint8_t>(source, y);
    auto* dst_y = Row<uint8_t>(destination.y, y);
    auto* dst_u = Row<uint8_t>(destination.u, y);
    auto* dst_v = Row<uint8_t>(destination.v, y);
    size_t x = 0;
    for (; x < end; x += lanes) {
      auto y0 = hn::Zero(d), u = y0, y1 = y0, v = y0;
      hn::LoadInterleaved4(d, src + x * 4, y0, u, y1, v);
      hn::StoreInterleaved2(y0, y1, d, dst_y + x * 2);
      hn::StoreU(u, d, dst_u + x);
      hn::StoreU(v, d, dst_v + x);
    }
    for (; x < pairs; ++x) {
      dst_y[x * 2] = src[x * 4];
      dst_y[x * 2 + 1] = src[x * 4 + 2];
      dst_u[x] = src[x * 4 + 1];
      dst_v[x] = src[x * 4 + 3];
    }
  }
  return VC_OK;
}

int PackYuy2Entry(vc_const_yuv_planes source, vc_plane destination, vc_rows rows) {
  const int status = CheckPackYuy2(source, destination, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  const hn::ScalableTag<uint8_t> d;
  const size_t lanes = hn::Lanes(d);
  const size_t pairs = static_cast<size_t>(rows.width) / 2;
  const size_t end = pairs - pairs % lanes;
  // Large complete row bands exceed private caches. Keep small streaming-pipeline
  // bands cached; use non-temporal stores only on cache-line-aligned output.
  const bool stream = uint64_t(rows.width) * rows.row_count * 2 >= 2 * 1024 * 1024 &&
                      reinterpret_cast<uintptr_t>(destination.data) % 64 == 0 && destination.stride % 64 == 0;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src_y = Row<uint8_t>(source.y, y);
    const auto* src_u = Row<uint8_t>(source.u, y);
    const auto* src_v = Row<uint8_t>(source.v, y);
    auto* dst = Row<uint8_t>(destination, y);
    size_t x = 0;
    for (; x < end; x += lanes) {
      const auto u = hn::LoadU(d, src_u + x), v = hn::LoadU(d, src_v + x);
      const auto uv0 = hn::InterleaveWholeLower(d, u, v);
      const auto uv1 = hn::InterleaveWholeUpper(d, u, v);
      const auto y0 = hn::LoadU(d, src_y + x * 2), y1 = hn::LoadU(d, src_y + x * 2 + lanes);
      const auto a = hn::InterleaveWholeLower(d, y0, uv0), b = hn::InterleaveWholeUpper(d, y0, uv0);
      const auto c = hn::InterleaveWholeLower(d, y1, uv1), e = hn::InterleaveWholeUpper(d, y1, uv1);
      if (stream) {
        hn::Stream(a, d, dst + x * 4);
        hn::Stream(b, d, dst + x * 4 + lanes);
        hn::Stream(c, d, dst + x * 4 + lanes * 2);
        hn::Stream(e, d, dst + x * 4 + lanes * 3);
      } else {
        hn::StoreU(a, d, dst + x * 4);
        hn::StoreU(b, d, dst + x * 4 + lanes);
        hn::StoreU(c, d, dst + x * 4 + lanes * 2);
        hn::StoreU(e, d, dst + x * 4 + lanes * 3);
      }
    }
    for (; x < pairs; ++x) {
      dst[x * 4] = src_y[x * 2];
      dst[x * 4 + 1] = src_u[x];
      dst[x * 4 + 2] = src_y[x * 2 + 1];
      dst[x * 4 + 3] = src_v[x];
    }
  }
  if (stream)
    hwy::FlushStream();
  return VC_OK;
}
#endif
} // namespace HWY_NAMESPACE
} // namespace vc
HWY_AFTER_NAMESPACE();

#if HWY_ONCE
namespace vc {
const vc_layout_functions* LayoutTable(int64_t target) {
  if (target == VC_TARGET_C) {
    static const vc_layout_functions c{vc_unpack_bgr, vc_pack_bgr,          vc_unpack_yuy2,           vc_pack_yuy2,
                                       vc_repack_bgr, vc_extract_yuy2_luma, vc_neutralize_yuy2_chroma};
    return &c;
  }
  switch (target) {
#define VC_TARGET(target, choose)                                                                                      \
  case target: {                                                                                                       \
    static const vc_layout_functions table{                                                                            \
        choose(UnpackBgrEntry), choose(PackBgrEntry),         choose(UnpackYuy2Entry),          choose(PackYuy2Entry), \
        choose(RepackBgrEntry), choose(ExtractYuy2LumaEntry), choose(NeutralizeYuy2ChromaEntry)};                      \
    return &table;                                                                                                     \
  }
#include "targets.inc"
#undef VC_TARGET
    default:
      return nullptr;
  }
}
} // namespace vc

extern "C" int64_t vc_layout_compiled_targets(void) {
  int64_t mask = 0;
#define VC_TARGET(target, choose) mask |= target;
#include "targets.inc"
#undef VC_TARGET
  return mask;
}
extern "C" int64_t vc_layout_supported_targets(void) {
  return vc_layout_compiled_targets() & hwy::SupportedTargets();
}
extern "C" int64_t vc_layout_choose_target(int64_t allowed_targets) {
  const int64_t candidates = allowed_targets & vc_layout_supported_targets();
  return candidates ? candidates & -candidates : VC_TARGET_C;
}
extern "C" const vc_layout_functions* vc_get_layout_functions(int64_t target) {
  if (target == VC_TARGET_NATIVE)
    target = vc_layout_choose_target(VC_TARGET_NATIVE);
  else if (target != VC_TARGET_C &&
           (target < 0 || (target & (target - 1)) != 0 || (vc_layout_supported_targets() & target) == 0))
    return nullptr;
  return vc::LayoutTable(target);
}
#endif
