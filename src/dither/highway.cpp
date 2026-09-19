// Highway ordered dithering developed in AviSynthConvertVideo.
// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "dither/ordered.h"
#include "layout/buffer.h"
#include <hwy/cache_control.h>
#include <hwy/targets.h>
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "dither/highway.cpp"
#include <hwy/foreach_target.h>
#include <hwy/highway.h>
HWY_BEFORE_NAMESPACE();
namespace vc {
namespace HWY_NAMESPACE {
namespace hn = hwy::HWY_NAMESPACE;
#if HWY_TARGET != HWY_SCALAR && HWY_TARGET != HWY_EMU128
// Ordinary ordered reduction never needs signed or floating lanes. Saturating
// the addition at 65535 is exact after the quantized-code ceiling is applied.
// Keep type-dependent branches in a function template: MSVC v141 checks
// discarded branches inside the former lambda against the wrong source type.
template <class S, bool low, bool remap>
HWY_INLINE hn::VFromD<hn::CappedTag<uint16_t, 32>> OrderedQuantize(
    const vc_ordered_plan& p, const S* src, int y, size_t at) {
  using D16 = hn::CappedTag<uint16_t, 32>;
  using V16 = hn::VFromD<D16>;
  const D16 d;
  const hn::Rebind<S, decltype(d)> ds;
  const size_t n = hn::Lanes(d);
  const auto cap = hn::Set(d, uint16_t(p.quantized_max));
  const hn::Rebind<int16_t, decltype(d)> di;
  const auto center = hn::Set(d, uint16_t(1 << (p.shift - 1)));
  const auto scale_integer = hn::Set(di, p.low_scale_integer);
  const auto scale_fraction = hn::Set(di, p.low_scale_fraction);
  const hn::Repartition<int32_t, decltype(d)> di32;
  const hn::Rebind<float, decltype(di32)> df32;
  const hn::Rebind<S, decltype(di32)> half_source;
  const auto source_max = hn::Set(di32, (1 << p.config.depth.source_bits) - 1);
  const auto source_offset = hn::Set(df32, p.range.source_offset);
  const auto range_factor = hn::Set(df32, p.range.factor);
  const auto destination_offset = hn::Set(df32, p.range.destination_offset + .5f);
  const int backshift = p.config.depth.destination_bits - p.config.quantization_bits;
  V16 value;
  if constexpr (remap) {
    auto map = [&](size_t index) HWY_ATTR {
      const auto input = hn::PromoteTo(di32, hn::LoadU(half_source, src + index));
      const auto scaled = hn::Mul(hn::Sub(hn::ConvertTo(df32, input), source_offset), range_factor);
      return hn::Min(hn::Max(hn::ConvertInRangeTo(di32, hn::Add(scaled, destination_offset)), hn::Zero(di32)),
                     source_max);
    };
    value = hn::OrderedDemote2To(d, map(at), map(at + n / 2));
  } else if constexpr (sizeof(S) == 1)
    value = hn::PromoteTo(d, hn::LoadU(ds, src + at));
  else
    value = hn::LoadU(d, src + at);
  const auto corr = hn::LoadU(d, p.thresholds16.data() + (y & 15) * 64 + (at & 15));
  auto sum = hn::SaturatedAdd(value, corr);
  if constexpr (low)
    // After the final zero clamp, subtracting the half-step in unsigned
    // lanes equals truncating the old signed half-integer correction.
    sum = hn::SaturatedSub(sum, center);
  const auto q = hn::Min(hn::ShiftRightSame(sum, p.shift), cap);
  if constexpr (low) {
    const auto signed_q = hn::BitCast(di, q);
    return hn::BitCast(d, hn::Add(hn::Mul(signed_q, scale_integer), hn::MulFixedPoint15(signed_q, scale_fraction)));
  } else
    return hn::ShiftLeftSame(q, backshift);
}

template <class S, class D, bool low = false, bool remap = false>
void Ordered(const vc_ordered_plan& p, vc_const_plane source, vc_plane destination, vc_rows rows) {
  const hn::CappedTag<uint16_t, 32> d;
  const size_t n = hn::Lanes(d), width = size_t(rows.width);
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<S>(source, y);
    auto* dst = Row<D>(destination, y);
    size_t x = 0;
    auto quantize = [&](size_t at) HWY_ATTR {
      return OrderedQuantize<S, low, remap>(p, src, y, at);
    };
    if constexpr (sizeof(D) == 1) {
      const hn::Repartition<uint8_t, decltype(d)> d8;
      const hn::Rebind<int16_t, decltype(d)> di16;
      for (; x + 2 * n <= width; x += 2 * n) {
#if HWY_TARGET == HWY_AVX2
        // The narrower remap loop can expose L2-to-L1 input latency on SPR.
        // Touch each upcoming cache line once, with the address inside the row.
        if constexpr (remap && !low && sizeof(S) == 2)
          if (x + 8 * n < width)
            hwy::Prefetch(src + x + 8 * n);
#endif
        hn::StoreU(hn::OrderedDemote2To(d8, hn::BitCast(di16, quantize(x)), hn::BitCast(di16, quantize(x + n))), d8,
                   dst + x);
      }
      const hn::Rebind<uint8_t, decltype(d)> half8;
      for (; x + n <= width; x += n)
        hn::StoreU(hn::DemoteTo(half8, quantize(x)), half8, dst + x);
    } else
      for (; x + n <= width; x += n)
        hn::StoreU(quantize(x), d, dst + x);
    for (; x < width; ++x)
      dst[x] = static_cast<D>(dither::Quantize(p, src[x], int(x), y));
  }
}

template <class S, class D>
dither::RowKernel ChooseOrderedFlags(const vc_ordered_config& c) {
  if (c.depth.source_full != c.depth.destination_full)
    return c.quantization_bits < 8 ? Ordered<S, D, true, true> : Ordered<S, D, false, true>;
  return c.quantization_bits < 8 ? Ordered<S, D, true> : Ordered<S, D>;
}
dither::RowKernel ChooseOrdered(const vc_ordered_config& c) {
  if (c.depth.source_bits == 8)
    return ChooseOrderedFlags<uint8_t, uint8_t>(c);
  if (c.depth.destination_bits == 8)
    return ChooseOrderedFlags<uint16_t, uint8_t>(c);
  return ChooseOrderedFlags<uint16_t, uint16_t>(c);
}
#endif
} // namespace HWY_NAMESPACE
} // namespace vc
HWY_AFTER_NAMESPACE();
#if HWY_ONCE
namespace vc::dither {
int64_t SupportedTargets() {
  int64_t mask = 0;
#define VC_TARGET(target, choose) mask |= target;
#include "targets.inc"
#undef VC_TARGET
  return mask & hwy::SupportedTargets();
}
RowKernel GetKernel(int64_t target, const vc_ordered_config& config) {
  const auto supported = SupportedTargets();
  if (target == VC_TARGET_NATIVE)
    target = supported ? supported & -supported : 0;
  if (target <= 0 || (target & (target - 1)) || !(target & supported))
    return nullptr;
  switch (target) {
#define VC_TARGET(target, choose)                                                                                      \
  case target:                                                                                                         \
    return (choose(ChooseOrdered))(config);
#include "targets.inc"
#undef VC_TARGET
    default:
      return nullptr;
  }
}
} // namespace vc::dither
#endif
