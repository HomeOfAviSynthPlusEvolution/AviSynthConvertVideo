// Highway bit-depth kernels developed in AviSynthConvertVideo.
// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "depth/highway.h"
#include "layout/buffer.h"
#include <algorithm>
#include <type_traits>
#include <hwy/targets.h>
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "depth/highway.cpp"
#include <hwy/foreach_target.h>
#include <hwy/highway.h>
HWY_BEFORE_NAMESPACE();
namespace vc {
namespace HWY_NAMESPACE {
namespace hn = hwy::HWY_NAMESPACE;
#if HWY_TARGET != HWY_SCALAR && HWY_TARGET != HWY_EMU128
// Integer shifts and exact 8->16 expansion operate at storage width. Saturating
// addition is valid before a right shift because overflow is already above the
// destination ceiling; a second clamp covers sub-16-bit source ceilings.
template <class S, class D, bool expand257>
void DepthShift(const depth::Transform& t, vc_const_plane source, vc_plane destination, vc_rows rows) {
  const hn::ScalableTag<uint16_t> d;
  const hn::Rebind<S, decltype(d)> ds;
  const hn::Rebind<D, decltype(d)> dd;
  const size_t lanes = hn::Lanes(d), width = size_t(rows.width);
  const int shift = t.config.destination_bits - t.config.source_bits;
  const int max = (1 << t.config.destination_bits) - 1;
  const auto ceiling = hn::Set(d, uint16_t(max));
  const auto rounding = hn::Set(d, uint16_t(shift < 0 ? 1 << (-shift - 1) : 0));
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<S>(source, y);
    auto* dst = Row<D>(destination, y);
    size_t x = 0;
    for (; x + lanes <= width; x += lanes) {
      hn::VFromD<decltype(d)> value;
      if constexpr (sizeof(S) == 1)
        value = hn::PromoteTo(d, hn::LoadU(ds, src + x));
      else
        value = hn::LoadU(d, src + x);
      if constexpr (expand257)
        value = hn::Mul(value, hn::Set(d, 257));
      else if (shift > 0)
        value = hn::ShiftLeftSame(value, shift);
      else if (shift < 0)
        value = hn::Min(hn::ShiftRightSame(hn::SaturatedAdd(value, rounding), -shift), ceiling);
      if constexpr (sizeof(D) == 1)
        hn::StoreU(hn::DemoteTo(dd, value), dd, dst + x);
      else
        hn::StoreU(value, dd, dst + x);
    }
    for (; x < width; ++x) {
      if constexpr (expand257)
        dst[x] = D(int(src[x]) * 257);
      else
        dst[x] = D(shift > 0   ? int(src[x]) << shift
                   : shift < 0 ? std::min((int(src[x]) + (1 << (-shift - 1))) >> -shift, max)
                               : src[x]);
    }
  }
}
// Integer source ranges keep the affine result well inside int32. Saturating
// packed narrowing supplies clipping; only F32 sources need pre-cast sanitizing.
template <bool subtract, class S, class DF>
HWY_INLINE auto DepthIntegerVector(DF d, const S* src, hn::VFromD<DF> offset, hn::VFromD<DF> factor,
                                   hn::VFromD<DF> bias, hn::VFromD<DF> ceiling) {
  const hn::Rebind<int32_t, DF> di;
  const hn::Rebind<S, DF> ds;
  hn::VFromD<DF> input;
  if constexpr (std::is_same_v<S, float>)
    input = hn::LoadU(d, src);
  else
    input = hn::ConvertTo(d, hn::PromoteTo(di, hn::LoadU(ds, src)));
  if constexpr (subtract)
    input = hn::Sub(input, offset);
  auto value = hn::Add(hn::Mul(input, factor), bias);
  if constexpr (std::is_same_v<S, float>)
    value = hn::Min(hn::MaxNumber(value, hn::Zero(d)), ceiling);
  // Sanitized F32 and bounded integer affine results are representable in int32.
  return hn::ConvertInRangeTo(di, value);
}
template <class S, class D, bool subtract>
void DepthAffine(const depth::Transform& t, vc_const_plane source, vc_plane destination, vc_rows rows) {
  const hn::ScalableTag<float> d;
  const hn::Rebind<int32_t, decltype(d)> di;
  const hn::Rebind<S, decltype(d)> ds;
  const hn::Rebind<D, decltype(d)> dd;
  const size_t lanes = hn::Lanes(d), width = size_t(rows.width);
  const auto offset = hn::Set(d, t.source_offset), factor = hn::Set(d, t.factor);
  const float out_offset = t.destination_offset + (std::is_same_v<D, float> ? 0.f : .5f);
  const auto bias = hn::Set(d, out_offset);
  const int maximum = std::is_same_v<D, float> ? 0 : (1 << t.config.destination_bits) - 1;
  const auto ceiling = hn::Set(d, float(maximum));
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<S>(source, y);
    auto* dst = Row<D>(destination, y);
    size_t x = 0;
    if constexpr (!std::is_same_v<D, float>) {
      // Assemble full stores instead of narrowing/storing one short vector.
      if constexpr (sizeof(D) == 1) {
        const hn::Repartition<int16_t, decltype(d)> d16;
        const hn::Repartition<uint8_t, decltype(d)> d8;
        for (; x + 4 * lanes <= width; x += 4 * lanes) {
          const auto a = DepthIntegerVector<subtract>(d, src + x, offset, factor, bias, ceiling);
          const auto b = DepthIntegerVector<subtract>(d, src + x + lanes, offset, factor, bias, ceiling);
          const auto c = DepthIntegerVector<subtract>(d, src + x + 2 * lanes, offset, factor, bias, ceiling);
          const auto e = DepthIntegerVector<subtract>(d, src + x + 3 * lanes, offset, factor, bias, ceiling);
          hn::StoreU(hn::OrderedDemote2To(d8, hn::OrderedDemote2To(d16, a, b), hn::OrderedDemote2To(d16, c, e)), d8,
                     dst + x);
        }
      } else {
        const hn::Repartition<uint16_t, decltype(d)> d16;
        const auto limit = hn::Set(d16, uint16_t(maximum));
        for (; x + 2 * lanes <= width; x += 2 * lanes) {
          const auto a = DepthIntegerVector<subtract>(d, src + x, offset, factor, bias, ceiling);
          const auto b = DepthIntegerVector<subtract>(d, src + x + lanes, offset, factor, bias, ceiling);
          if constexpr (std::is_same_v<S, float>)
            hn::StoreU(hn::OrderedDemote2To(d16, a, b), d16, dst + x);
          else
            hn::StoreU(hn::Min(hn::OrderedDemote2To(d16, a, b), limit), d16, dst + x);
        }
      }
    }
    for (; x + lanes <= width; x += lanes) {
      if constexpr (std::is_same_v<D, float>) {
        hn::VFromD<decltype(d)> input;
        if constexpr (std::is_same_v<S, float>)
          input = hn::LoadU(d, src + x);
        else
          input = hn::ConvertTo(d, hn::PromoteTo(di, hn::LoadU(ds, src + x)));
        if constexpr (subtract)
          input = hn::Sub(input, offset);
        hn::StoreU(hn::Add(hn::Mul(input, factor), bias), d, dst + x);
      } else {
        const auto value = DepthIntegerVector<subtract>(d, src + x, offset, factor, bias, ceiling);
        hn::StoreU(hn::Min(hn::DemoteTo(dd, value), hn::Set(dd, D(maximum))), dd, dst + x);
      }
    }
    for (; x < width; ++x) {
      const float value = (float(src[x]) - t.source_offset) * t.factor + out_offset;
      if constexpr (std::is_same_v<D, float>)
        dst[x] = value;
      else
        dst[x] = !(value > 0) ? D(0) : value >= float(maximum) ? D(maximum) : D(value);
    }
  }
}
template <class S, class D>
depth::RowKernel ChooseDepthStorage(const depth::Transform& t) {
  if constexpr (!std::is_same_v<S, float> && !std::is_same_v<D, float>) {
    if (!t.config.source_full && !t.config.destination_full)
      return DepthShift<S, D, false>;
    if constexpr (sizeof(S) == 1 && sizeof(D) == 2)
      if (t.config.destination_bits == 16 && !t.config.chroma && t.config.source_full && t.config.destination_full)
        return DepthShift<S, D, true>;
  }
  return t.source_offset != 0 ? DepthAffine<S, D, true> : DepthAffine<S, D, false>;
}
template <class S>
depth::RowKernel ChooseDepthDestination(const depth::Transform& t) {
  if (t.config.destination_bits == 32)
    return ChooseDepthStorage<S, float>(t);
  if (t.config.destination_bits == 8)
    return ChooseDepthStorage<S, uint8_t>(t);
  return ChooseDepthStorage<S, uint16_t>(t);
}
depth::RowKernel ChooseDepthKernel(const depth::Transform& t) {
  if (t.config.source_bits == 32)
    return ChooseDepthDestination<float>(t);
  if (t.config.source_bits == 8)
    return ChooseDepthDestination<uint8_t>(t);
  return ChooseDepthDestination<uint16_t>(t);
}
#endif
} // namespace HWY_NAMESPACE
} // namespace vc
HWY_AFTER_NAMESPACE();
#if HWY_ONCE
namespace vc::depth {
int64_t SupportedTargets() {
  int64_t mask = 0;
#define VC_TARGET(target, choose) mask |= target;
#include "targets.inc"
#undef VC_TARGET
  return mask & hwy::SupportedTargets();
}
RowKernel GetKernel(int64_t target, const Transform& transform) {
  const auto supported = SupportedTargets();
  if (target == VC_TARGET_NATIVE)
    target = supported ? supported & -supported : 0;
  if (target <= 0 || (target & (target - 1)) || !(target & supported))
    return nullptr;
  switch (target) {
#define VC_TARGET(target, choose)                                                                                      \
  case target:                                                                                                         \
    return (choose(ChooseDepthKernel))(transform);
#include "targets.inc"
#undef VC_TARGET
    default:
      return nullptr;
  }
}
} // namespace vc::depth
#endif
