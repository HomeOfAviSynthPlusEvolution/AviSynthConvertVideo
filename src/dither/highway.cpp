// Highway ordered dithering developed in AviSynthConvertVideo.
// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "dither/ordered.h"
#include "layout/buffer.h"
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
template <class S, class D, bool remap, bool low>
void Ordered(const vc_ordered_plan& p, vc_const_plane source, vc_plane destination, vc_rows rows) {
  const hn::CappedTag<int32_t, 16> di;
  const hn::Rebind<float, decltype(di)> df;
  const hn::Rebind<S, decltype(di)> ds;
  const hn::Rebind<D, decltype(di)> dd;
  const size_t n = hn::Lanes(di), width = size_t(rows.width);
  const auto zero = hn::Zero(di), ceiling = hn::Set(di, p.output_max);
  const auto srcmax = hn::Set(di, (1 << p.config.depth.source_bits) - 1);
  const auto qmax = hn::Set(di, p.quantized_max);
  const auto src_offset = hn::Set(df, p.range.source_offset), factor = hn::Set(df, p.range.factor);
  const auto dst_offset = hn::Set(df, p.range.destination_offset + .5f);
  const auto center = hn::Set(df, float((1 << p.shift) - 1) * .5f);
  const auto backscale = hn::Set(df, p.backscale), half = hn::Set(df, .5f);
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<S>(source, y);
    auto* dst = Row<D>(destination, y);
    size_t x = 0;
    auto quantize = [&](size_t at) HWY_ATTR {
      auto value = hn::PromoteTo(di, hn::LoadU(ds, src + at));
      if constexpr (remap) {
        const auto scaled = hn::Mul(hn::Sub(hn::ConvertTo(df, value), src_offset), factor);
        value = hn::Min(hn::Max(hn::ConvertInRangeTo(di, hn::Add(scaled, dst_offset)), zero), srcmax);
      }
      const auto corr = hn::LoadU(di, p.thresholds.data() + (y & 15) * 32 + (at & 15));
      auto sum = hn::Add(value, corr);
      if constexpr (low)
        sum = hn::ConvertInRangeTo(di, hn::Sub(hn::ConvertTo(df, sum), center));
      auto q = hn::ShiftRightSame(sum, p.shift);
      if (p.config.depth.destination_bits != p.config.quantization_bits) {
        q = hn::Min(q, qmax);
        if constexpr (low)
          q = hn::ConvertInRangeTo(di, hn::Add(hn::Mul(hn::ConvertTo(df, q), backscale), half));
        else
          q = hn::ShiftLeftSame(q, p.config.depth.destination_bits - p.config.quantization_bits);
      }
      return hn::Min(hn::Max(q, zero), ceiling);
    };
    if constexpr (sizeof(D) == 1) {
      const hn::Repartition<int16_t, decltype(di)> d16;
      const hn::Repartition<uint8_t, decltype(di)> d8;
      for (; x + 4 * n <= width; x += 4 * n) {
        const auto a = hn::OrderedDemote2To(d16, quantize(x), quantize(x + n));
        const auto b = hn::OrderedDemote2To(d16, quantize(x + 2 * n), quantize(x + 3 * n));
        hn::StoreU(hn::OrderedDemote2To(d8, a, b), d8, dst + x);
      }
    } else {
      const hn::Repartition<uint16_t, decltype(di)> d16;
      for (; x + 2 * n <= width; x += 2 * n)
        hn::StoreU(hn::OrderedDemote2To(d16, quantize(x), quantize(x + n)), d16, dst + x);
    }
    for (; x + n <= width; x += n)
      hn::StoreU(hn::DemoteTo(dd, quantize(x)), dd, dst + x);
    for (; x < width; ++x)
      dst[x] = static_cast<D>(dither::Quantize(p, src[x], int(x), y));
  }
}
// Ordinary ordered reduction never needs signed or floating lanes. Saturating
// the addition at 65535 is exact after the quantized-code ceiling is applied.
template <class S, class D>
void OrderedInteger(const vc_ordered_plan& p, vc_const_plane source, vc_plane destination, vc_rows rows) {
  using D16 = hn::CappedTag<uint16_t, 32>;
  using V16 = hn::VFromD<D16>;
  const D16 d;
  const hn::Rebind<S, decltype(d)> ds;
  const size_t n = hn::Lanes(d), width = size_t(rows.width);
  const auto cap = hn::Set(d, uint16_t(p.quantized_max));
  const int backshift = p.config.depth.destination_bits - p.config.quantization_bits;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<S>(source, y);
    auto* dst = Row<D>(destination, y);
    size_t x = 0;
    auto quantize = [&](size_t at) HWY_ATTR {
      V16 value;
      if constexpr (sizeof(S) == 1)
        value = hn::PromoteTo(d, hn::LoadU(ds, src + at));
      else
        value = hn::LoadU(d, src + at);
      const auto corr = hn::LoadU(d, p.thresholds16.data() + (y & 15) * 64 + (at & 15));
      return hn::ShiftLeftSame(hn::Min(hn::ShiftRightSame(hn::SaturatedAdd(value, corr), p.shift), cap), backshift);
    };
    if constexpr (sizeof(D) == 1) {
      const hn::Repartition<uint8_t, decltype(d)> d8;
      const hn::Rebind<int16_t, decltype(d)> di16;
      for (; x + 2 * n <= width; x += 2 * n)
        hn::StoreU(hn::OrderedDemote2To(d8, hn::BitCast(di16, quantize(x)), hn::BitCast(di16, quantize(x + n))), d8,
                   dst + x);
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
    return c.quantization_bits < 8 ? Ordered<S, D, true, true> : Ordered<S, D, true, false>;
  return c.quantization_bits < 8 ? Ordered<S, D, false, true> : OrderedInteger<S, D>;
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
