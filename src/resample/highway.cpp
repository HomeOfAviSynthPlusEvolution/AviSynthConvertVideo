// Highway resampling kernels developed in AviSynthConvertVideo.
// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "resample/highway.h"
#include "layout/buffer.h"
#include <algorithm>
#include <array>
#include <limits>
#include <type_traits>
#include <hwy/targets.h>
#include <hwy/cache_control.h>
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "resample/highway.cpp"
#include <hwy/foreach_target.h>
#include <hwy/highway.h>
HWY_BEFORE_NAMESPACE();
namespace vc {
namespace HWY_NAMESPACE {
namespace hn = hwy::HWY_NAMESPACE;
#if HWY_TARGET != HWY_SCALAR && HWY_TARGET != HWY_EMU128

template <class D, class V, class T>
void StoreOutput(D d, V value, T* output, bool stream) {
  if constexpr (hn::MaxLanes(d) * sizeof(T) >= 16) {
    if (stream) {
      hn::Stream(value, d, output);
      return;
    }
  }
  hn::StoreU(value, d, output);
}

template <class T, class D, class V>
auto LimitEffectiveBits(D d, V value, int bits) {
  // Saturating narrowing already handles negative values and the storage limit.
  // Only sub-16-bit U16 formats need an additional upper bound.
  if constexpr (std::is_same_v<T, uint16_t>) {
    if (bits < 16)
      return hn::Min(value, hn::Set(d, (1 << bits) - 1));
  }
  return value;
}

template <class T, class D>
auto LoadSigned16(D d, const T* source, int bias) {
  const hn::Rebind<T, D> ds;
  if constexpr (sizeof(T) == 1)
    return hn::PromoteTo(d, hn::LoadU(ds, source));
  else
    return hn::BitCast(d, hn::Xor(hn::LoadU(ds, source), hn::Set(ds, uint16_t(bias))));
}

void VerticalU8Pair(const uint8_t* source0, const uint8_t* source1, uint8_t* destination, size_t width, int16_t weight,
                    bool stream) {
  const hn::ScalableTag<int16_t> d;
  const hn::Rebind<uint8_t, decltype(d)> ds;
  const hn::Repartition<uint8_t, decltype(d)> dout;
  const size_t lanes = hn::Lanes(d);
  const auto w = hn::Set(d, weight);
  size_t x = 0;
  for (; x + 2 * lanes <= width; x += 2 * lanes) {
    const auto a0 = hn::PromoteTo(d, hn::LoadU(ds, source0 + x));
    const auto b0 = hn::PromoteTo(d, hn::LoadU(ds, source1 + x));
    const auto a1 = hn::PromoteTo(d, hn::LoadU(ds, source0 + x + lanes));
    const auto b1 = hn::PromoteTo(d, hn::LoadU(ds, source1 + x + lanes));
    // round((b-a)*weight/16384) exactly matches the original positive-rounder
    // fixed-point rule, including negative lobes. Differences times two fit i16.
    const auto lo = hn::Add(a0, hn::MulFixedPoint15(hn::ShiftLeft<1>(hn::Sub(b0, a0)), w));
    const auto hi = hn::Add(a1, hn::MulFixedPoint15(hn::ShiftLeft<1>(hn::Sub(b1, a1)), w));
    StoreOutput(dout, hn::OrderedDemote2To(dout, lo, hi), destination + x, stream);
  }
  for (; x < width; ++x) {
    const int sum = (16384 - weight) * int(source0[x]) + weight * int(source1[x]) + 8192;
    destination[x] = uint8_t(std::clamp(sum, 0, 255 * 16384) / 16384);
  }
}

void VerticalU16Pair(const uint16_t* source0, const uint16_t* source1, uint16_t* destination, size_t width,
                     int16_t weight, int limit, bool stream) {
  const hn::ScalableTag<int32_t> d;
  const hn::Rebind<uint16_t, decltype(d)> ds;
  const hn::Repartition<uint16_t, decltype(d)> dout;
  const size_t lanes = hn::Lanes(d);
  const auto w = hn::Set(d, int32_t(weight));
  size_t x = 0;
  for (; x + 4 * lanes <= width; x += 4 * lanes) {
    hn::VFromD<decltype(d)> sums[4];
    for (size_t i = 0; i < 4; ++i) {
      const auto a = hn::PromoteTo(d, hn::LoadU(ds, source0 + x + i * lanes));
      const auto b = hn::PromoteTo(d, hn::LoadU(ds, source1 + x + i * lanes));
      // |b-a| <= 65535 and |weight| <= 32768. The normalized pair's actual
      // weight range is [-24575,32767], so the product plus 4096 fits i32.
      const auto delta = hn::ShiftRight<13>(hn::Add(hn::Mul(hn::Sub(b, a), w), hn::Set(d, 4096)));
      sums[i] = hn::Add(a, delta);
      if (limit < 65535)
        sums[i] = hn::Min(sums[i], hn::Set(d, limit));
    }
    for (size_t i = 0; i < 4; i += 2)
      StoreOutput(dout, hn::OrderedDemote2To(dout, sums[i], sums[i + 1]), destination + x + i * lanes, stream);
  }
  for (; x < width; ++x) {
    const int64_t sum = int64_t(8192 - weight) * source0[x] + int64_t(weight) * source1[x] + 4096;
    destination[x] = uint16_t(std::clamp(sum, int64_t(0), int64_t(limit) * 8192) / 8192);
  }
}

template <class T>
void Vertical(const resample::Coefficients& plan, vc_const_plane source, vc_plane destination, vc_rows rows,
              int source_first, int destination_first) {
  using Accumulator = std::conditional_t<std::is_same_v<T, float>, float, int32_t>;
  const hn::ScalableTag<Accumulator> d;
  const hn::Rebind<T, decltype(d)> ds;
  const size_t lanes = hn::Lanes(d);
  const size_t width = size_t(rows.width), end = width - width % lanes;
  const bool stream = uint64_t(rows.width) * rows.row_count * sizeof(T) >= 512 * 1024 &&
                      reinterpret_cast<uintptr_t>(destination.data) % 64 == 0 && destination.stride % 64 == 0;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const int offset = plan.offsets[y], taps = plan.sizes[y];
    const size_t base = size_t(y) * plan.filter_size;
    T* dst = Row<T>(destination, y - destination_first);
    if constexpr (std::is_same_v<T, uint8_t>) {
      if (taps == 2 && int(plan.integers[base]) + int(plan.integers[base + 1]) == 16384) {
        VerticalU8Pair(Row<T>(source, offset - source_first), Row<T>(source, offset + 1 - source_first), dst, width,
                       plan.integers[base + 1], stream);
        continue;
      }
    }
    if constexpr (std::is_same_v<T, uint16_t>) {
      if (lanes <= 8 && taps == 2 && int(plan.integers[base]) + int(plan.integers[base + 1]) == 8192) {
        VerticalU16Pair(Row<T>(source, offset - source_first), Row<T>(source, offset + 1 - source_first), dst, width,
                        plan.integers[base + 1], (1 << plan.bits_per_sample) - 1, stream);
        continue;
      }
    }
    const int bias = plan.bits_per_sample == 16 ? 32768 : 0;
    constexpr int shift = sizeof(T) == 1 ? 14 : 13;
    size_t vector_end = end;
    if constexpr (!std::is_same_v<T, float>) {
      // Bound every partial sum, including the rounder and final bias restoration.
      // Rare extreme filters use the defined wide C accumulator for this row.
      int64_t absolute = 0;
      for (int k = 0; k < taps; ++k)
        absolute += std::abs(int(plan.integers[base + k]));
      const int64_t sample_limit = bias ? 32768 : ((int64_t(1) << plan.bits_per_sample) - 1);
      if (absolute * sample_limit + (int64_t(bias) << shift) + (int64_t(1) << (shift - 1)) >
          std::numeric_limits<int32_t>::max()) {
        vector_end = 0;
      }
    }
    size_t x = 0;
    // Independent output vectors hide multiply/add latency without regrouping
    // any pixel's tap sum. Reuse each coefficient and source-row address.
    constexpr size_t vectors = std::is_same_v<T, float> ? 4 : 8;
    for (; x + vectors * lanes <= vector_end; x += vectors * lanes) {
      hn::VFromD<decltype(d)> sums[vectors];
      for (auto& sum : sums) {
        if constexpr (std::is_same_v<T, float>)
          sum = hn::Zero(d);
        else
          sum = hn::Set(d, int32_t(1) << (shift - 1));
      }
      int k = 0;
      if constexpr (!std::is_same_v<T, float>) {
        const hn::Repartition<int16_t, decltype(d)> df16;
        for (; k + 1 < taps; k += 2) {
          const T* src0 = Row<T>(source, offset + k - source_first) + x;
          const T* src1 = Row<T>(source, offset + k + 1 - source_first) + x;
          const auto w0 = hn::Set(df16, plan.integers[base + k]), w1 = hn::Set(df16, plan.integers[base + k + 1]);
          const auto weights = hn::InterleaveWholeLower(df16, w0, w1);
          for (size_t i = 0; i < vectors; i += 2) {
            const auto v0 = LoadSigned16(df16, src0 + i * lanes, bias), v1 = LoadSigned16(df16, src1 + i * lanes, bias);
            sums[i] = hn::Add(sums[i], hn::WidenMulPairwiseAdd(d, hn::InterleaveWholeLower(df16, v0, v1), weights));
            sums[i + 1] =
                hn::Add(sums[i + 1], hn::WidenMulPairwiseAdd(d, hn::InterleaveWholeUpper(df16, v0, v1), weights));
          }
        }
      }
      for (; k < taps; ++k) {
        const T* src = Row<T>(source, offset + k - source_first) + x;
        if constexpr (std::is_same_v<T, float>) {
          const auto weight = hn::Set(d, plan.floats[base + k]);
          for (size_t i = 0; i < vectors; ++i)
            sums[i] = hn::Add(sums[i], hn::Mul(hn::LoadU(ds, src + i * lanes), weight));
        } else {
          const auto weight = hn::Set(d, int32_t(plan.integers[base + k]));
          for (size_t i = 0; i < vectors; ++i) {
            const auto values = hn::Sub(hn::PromoteTo(d, hn::LoadU(ds, src + i * lanes)), hn::Set(d, bias));
            sums[i] = hn::Add(sums[i], hn::Mul(values, weight));
          }
        }
      }
      if constexpr (std::is_same_v<T, float>) {
        for (size_t i = 0; i < vectors; ++i)
          StoreOutput(ds, sums[i], dst + x + i * lanes, stream);
      } else {
        for (auto& sum : sums) {
          sum = hn::ShiftRight<shift>(hn::Add(sum, hn::Set(d, bias << shift)));
          sum = LimitEffectiveBits<T>(d, sum, plan.bits_per_sample);
        }
        for (size_t i = 0; i < vectors; i += 4) {
          if constexpr (sizeof(T) == 1) {
            const hn::Repartition<int16_t, decltype(d)> d16;
            const hn::Repartition<uint8_t, decltype(d)> d8;
            const auto lo = hn::OrderedDemote2To(d16, sums[i], sums[i + 1]);
            const auto hi = hn::OrderedDemote2To(d16, sums[i + 2], sums[i + 3]);
            StoreOutput(d8, hn::OrderedDemote2To(d8, lo, hi), dst + x + i * lanes, stream);
          } else {
            const hn::Repartition<uint16_t, decltype(d)> d16;
            StoreOutput(d16, hn::OrderedDemote2To(d16, sums[i], sums[i + 1]), dst + x + i * lanes, stream);
            StoreOutput(d16, hn::OrderedDemote2To(d16, sums[i + 2], sums[i + 3]), dst + x + (i + 2) * lanes, stream);
          }
        }
      }
    }

    for (; x < vector_end; x += lanes) {
      auto sum = hn::Zero(d);
      if constexpr (!std::is_same_v<T, float>)
        sum = hn::Set(d, int32_t(1) << (shift - 1));
      for (int k = 0; k < taps; ++k) {
        const T* src = Row<T>(source, offset + k - source_first) + x;
        if constexpr (std::is_same_v<T, float>) {
          sum = hn::Add(sum, hn::Mul(hn::LoadU(ds, src), hn::Set(d, plan.floats[base + k])));
        } else {
          const auto values = hn::Sub(hn::PromoteTo(d, hn::LoadU(ds, src)), hn::Set(d, bias));
          sum = hn::Add(sum, hn::Mul(values, hn::Set(d, int32_t(plan.integers[base + k]))));
        }
      }
      if constexpr (std::is_same_v<T, float>)
        StoreOutput(ds, sum, dst + x, stream);
      else {
        sum = hn::Add(sum, hn::Set(d, bias << shift));
        sum = hn::ShiftRight<shift>(sum);
        sum = LimitEffectiveBits<T>(d, sum, plan.bits_per_sample);
        StoreOutput(ds, hn::DemoteTo(ds, sum), dst + x, stream);
      }
    }
    for (; x < width; ++x) {
      if constexpr (std::is_same_v<T, float>) {
        float sum = 0;
        for (int k = 0; k < taps; ++k)
          sum += Row<T>(source, offset + k - source_first)[x] * plan.floats[base + k];
        dst[x] = sum;
      } else {
        int64_t sum = int64_t(1) << (shift - 1);
        for (int k = 0; k < taps; ++k)
          sum += int64_t(int(Row<T>(source, offset + k - source_first)[x]) - bias) * plan.integers[base + k];
        sum += int64_t(bias) << shift;
        const int64_t limit = (int64_t(1) << plan.bits_per_sample) - 1;
        dst[x] = T(std::clamp(sum, int64_t(0), limit << shift) / (int64_t(1) << shift));
      }
    }
  }
  if (stream)
    hwy::FlushStream();
}
// Read a native-endian narrow sample from an in-bounds four-byte window.
// GatherOffset's generic implementation uses CopyBytes, so unaligned windows
// do not impose uint32 alignment or typed dereferences on the source allocation.
template <class T, class D, class V>
auto GatherNarrow(D d, const T* source, V indices, int last_window) {
  const hn::Rebind<uint32_t, D> du;
  const auto byte_offsets = hn::Mul(indices, hn::Set(d, int32_t(sizeof(T))));
  const auto windows = hn::Min(byte_offsets, hn::Set(d, last_window));
  auto delta = hn::Sub(byte_offsets, windows);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  delta = hn::Sub(hn::Set(d, int32_t(4 - sizeof(T))), delta);
#endif
  const auto words = hn::GatherOffset(du, reinterpret_cast<const uint32_t*>(source), windows);
  const auto values = words >> hn::BitCast(du, hn::ShiftLeft<3>(delta));
  return hn::BitCast(d, hn::And(values, hn::Set(du, sizeof(T) == 1 ? 255u : 65535u)));
}
template <class T, class D>
auto LoadAccumulator(D d, const T* ptr) {
  const hn::Rebind<T, D> ds;
  if constexpr (std::is_same_v<T, float>)
    return hn::LoadU(ds, ptr);
  else
    return hn::PromoteTo(d, hn::LoadU(ds, ptr));
}
template <class T, class D>
auto PairedHorizontalSum(D d, const resample::Coefficients& plan, const resample::HorizontalBlock& block,
                         const T* source, int bias) {
  const hn::Repartition<int16_t, D> d16;
  const size_t lanes16 = hn::Lanes(d16);
  const auto v0 = LoadSigned16(d16, source + block.source_start, bias);
  auto v1 = hn::Zero(d16);
  if (block.window_size == int(2 * lanes16))
    v1 = LoadSigned16(d16, source + block.source_start + lanes16, bias);
  constexpr int shift = sizeof(T) == 1 ? 14 : 13;
  auto sum = hn::Zero(d), odd = hn::Zero(d);
  for (int k = 0; k < block.taps; k += 2) {
    const size_t offset = block.pair_start + size_t(k / 2) * lanes16;
    const auto index = hn::LoadU(d16, plan.horizontal.pair_indices.data() + offset);
    const auto lookup = hn::IndicesFromVec(d16, index);
    const auto samples =
        block.window_size == int(lanes16) ? hn::TableLookupLanes(v0, lookup) : hn::TwoTablesLookupLanes(v0, v1, lookup);
    const auto weights = hn::LoadU(d16, plan.horizontal.pair_weights.data() + offset);
    sum = hn::ReorderWidenMulAccumulate(d, samples, weights, sum, odd);
  }
  return hn::Add(hn::RearrangeToOddPlusEven(sum, odd), hn::Set(d, 1 << (shift - 1)));
}

template <class T, class D>
auto PackedHorizontalSum(D d, const resample::Coefficients& plan, const resample::HorizontalBlock& block,
                         const T* source, int bias) {
  const hn::Rebind<int32_t, D> di;
  const size_t lanes = hn::Lanes(d);
  if constexpr (!std::is_same_v<T, float>) {
    if (block.window_size && 4 * lanes <= size_t(std::numeric_limits<int16_t>::max()))
      return PairedHorizontalSum(d, plan, block, source, bias);
  }
  if constexpr (std::is_same_v<T, float>) {
    if (block.sliding_window) {
      const auto indices = hn::LoadU(di, plan.horizontal.indices.data() + block.coefficient_start);
      const auto lookup = hn::IndicesFromVec(d, indices);
      auto sum = hn::Zero(d);
      for (int k = 0; k < block.taps; ++k) {
        const T* src = source + block.source_start + k;
        const auto lo = hn::LoadU(d, src), hi = hn::LoadU(d, src + lanes);
        const auto samples = block.stride_two ? hn::ConcatEven(d, hi, lo) : hn::TwoTablesLookupLanes(lo, hi, lookup);
        const auto weights =
            hn::LoadU(d, plan.horizontal.float_weights.data() + block.coefficient_start + size_t(k) * lanes);
        sum = hn::Add(sum, hn::Mul(samples, weights));
      }
      return sum;
    }
  }
  const auto& packing = plan.horizontal;
  auto v0 = hn::Zero(d), v1 = v0, v2 = v0, v3 = v0;
  if (block.window_size) {
    v0 = LoadAccumulator(d, source + block.source_start);
    v1 = LoadAccumulator(d, source + block.source_start + lanes);
    if (block.window_size == int(4 * lanes)) {
      v2 = LoadAccumulator(d, source + block.source_start + 2 * lanes);
      v3 = LoadAccumulator(d, source + block.source_start + 3 * lanes);
    }
  }
  auto sum = hn::Zero(d);
  constexpr int shift = sizeof(T) == 1 ? 14 : 13;
  if constexpr (!std::is_same_v<T, float>)
    sum = hn::Set(d, 1 << (shift - 1));
  // Wider float vectors benefit from reusing the first index. Narrow vectors
  // and paired integer kernels perform better with prepared tap indices.
  const auto initial = hn::LoadU(di, packing.indices.data() + block.coefficient_start);
  for (int k = 0; k < block.taps; ++k) {
    const size_t offset = block.coefficient_start + size_t(k) * lanes;
    const auto index = std::is_same_v<T, float> && lanes >= 16 && block.linear_indices
                           ? hn::Add(initial, hn::Set(di, k))
                           : hn::LoadU(di, packing.indices.data() + offset);
    auto values = hn::Zero(d);
    if (block.window_size) {
      const auto upper_mask = hn::Ge(index, hn::Set(di, int32_t(2 * lanes)));
      const auto local = hn::Sub(index, hn::IfThenElseZero(upper_mask, hn::Set(di, int32_t(2 * lanes))));
      const auto lookup = hn::IndicesFromVec(d, local);
      values = hn::TwoTablesLookupLanes(v0, v1, lookup);
      if (block.window_size == int(4 * lanes)) {
        const auto upper = hn::TwoTablesLookupLanes(v2, v3, lookup);
        values = hn::IfThenElse(hn::RebindMask(d, upper_mask), upper, values);
      }
    } else {
      if constexpr (std::is_same_v<T, float>)
        values = hn::GatherIndex(d, source, index);
      else
        values = GatherNarrow(d, source, index, plan.source_size * int(sizeof(T)) - 4);
    }
    if constexpr (std::is_same_v<T, float>)
      sum = hn::Add(sum, hn::Mul(values, hn::LoadU(d, packing.float_weights.data() + offset)));
    else
      sum = hn::Add(sum,
                    hn::Mul(hn::Sub(values, hn::Set(d, bias)), hn::LoadU(d, packing.integer_weights.data() + offset)));
  }
  return sum;
}

// Long supports no longer fit the output-parallel register lookup window.
// Vectorize consecutive taps instead. The caller's absolute-sum proof bounds
// every partial sum, so regrouping integer products is exact and overflow-free.
template <class T, class D>
void HorizontalLongInteger(D d, const resample::Coefficients& plan, const T* source, T* destination, size_t first,
                           size_t count, int bias) {
  const hn::Repartition<int16_t, D> d16;
  const int step = int(hn::Lanes(d16));
  constexpr int shift = sizeof(T) == 1 ? 14 : 13;
  const int limit = (1 << plan.bits_per_sample) - 1;
  for (size_t x = first; x < first + count; ++x) {
    const auto* src = source + plan.offsets[x];
    const auto* weights = plan.integers.data() + x * plan.filter_size;
    auto sum = hn::Zero(d);
    int k = 0;
    for (; k + step <= plan.sizes[x]; k += step)
      sum = hn::Add(sum, hn::WidenMulPairwiseAdd(d, LoadSigned16(d16, src + k, bias), hn::LoadU(d16, weights + k)));
    if (k < plan.sizes[x]) {
      const hn::Rebind<T, decltype(d16)> ds;
      const size_t remaining = size_t(plan.sizes[x] - k);
      const auto raw = hn::LoadN(ds, src + k, remaining);
      const auto values = [&]() HWY_ATTR {
        if constexpr (sizeof(T) == 1)
          return hn::PromoteTo(d16, raw);
        else
          return hn::BitCast(d16, hn::Xor(raw, hn::Set(ds, uint16_t(bias))));
      }();
      sum = hn::Add(sum, hn::WidenMulPairwiseAdd(d, values, hn::LoadN(d16, weights + k, remaining)));
    }
    int total = hn::ReduceSum(d, sum);
    total += (bias << shift) + (1 << (shift - 1));
    destination[x] = T(std::clamp(total, 0, limit << shift) >> shift);
  }
}

template <class T>
void HorizontalInteger(const resample::Coefficients& plan, vc_const_plane source, vc_plane destination, vc_rows rows,
                       int source_first, int destination_first) {
  const hn::ScalableTag<int32_t> d;
  const hn::Rebind<T, decltype(d)> ds;
  const size_t lanes = hn::Lanes(d), width = size_t(rows.width);
  constexpr int shift = sizeof(T) == 1 ? 14 : 13;
  const int bias = plan.bits_per_sample == 16 ? 32768 : 0;
  const int limit = (1 << plan.bits_per_sample) - 1;
  const int64_t bound = (int64_t(std::numeric_limits<int32_t>::max()) - (int64_t(bias) << shift) - (1 << (shift - 1))) /
                        (bias ? 32768 : limit);
  const bool gather_safe =
      plan.max_abs_sum >= 0 && plan.max_abs_sum <= bound && plan.source_size >= int(4 / sizeof(T)) &&
      plan.source_size <= std::numeric_limits<int32_t>::max() / int(sizeof(T)) && plan.integers.size() >= 2 &&
      plan.integers.size() <= size_t(std::numeric_limits<int32_t>::max() / 2);
  const size_t end = gather_safe ? width - width % lanes : 0;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const T* src = Row<T>(source, y - source_first);
    T* dst = Row<T>(destination, y - destination_first);
    if (gather_safe && lanes == 8 && plan.horizontal.lanes == lanes && plan.horizontal.dot_outputs) {
      const hn::Repartition<int16_t, decltype(d)> d16;
      const hn::Half<decltype(d)> dh;
      const hn::Rebind<T, decltype(dh)> dout;
      size_t x = 0;
      for (; x < size_t(plan.horizontal.dot_outputs); x += 4) {
        hn::VFromD<decltype(d)> products[4];
        for (size_t i = 0; i < 4; ++i) {
          const auto values = LoadSigned16(d16, src + plan.offsets[x + i], bias);
          const auto weights = hn::LoadU(d16, plan.horizontal.dot_weights.data() + (x + i) * 16);
          products[i] = hn::WidenMulPairwiseAdd(d, values, weights);
        }
        const auto ab = hn::PairwiseAdd128(d, products[0], products[1]);
        const auto cd = hn::PairwiseAdd128(d, products[2], products[3]);
        const auto combined = hn::PairwiseAdd128(d, ab, cd);
        auto sum = hn::Add(hn::LowerHalf(dh, combined), hn::UpperHalf(dh, combined));
        sum = hn::ShiftRight<shift>(hn::Add(sum, hn::Set(dh, (bias << shift) + (1 << (shift - 1)))));
        sum = LimitEffectiveBits<T>(dh, sum, plan.bits_per_sample);
        hn::StoreU(hn::DemoteTo(dout, sum), dout, dst + x);
      }
      for (; x < width; ++x) {
        int64_t sum = int64_t(1) << (shift - 1);
        const size_t base = x * plan.filter_size;
        for (int k = 0; k < plan.sizes[x]; ++k)
          sum += int64_t(int(src[plan.offsets[x] + k]) - bias) * plan.integers[base + k];
        sum += int64_t(bias) << shift;
        dst[x] = T(std::clamp(sum, int64_t(0), int64_t(limit) << shift) / (int64_t(1) << shift));
      }
      continue;
    }
    size_t x = 0;
    for (; x < end; x += lanes) {
      if (plan.horizontal.lanes == lanes) {
        if (!plan.horizontal.blocks[x / lanes].window_size) {
          HorizontalLongInteger(d, plan, src, dst, x, lanes, bias);
          continue;
        }
        auto sum = PackedHorizontalSum(d, plan, plan.horizontal.blocks[x / lanes], src, bias);
        sum = hn::ShiftRight<shift>(hn::Add(sum, hn::Set(d, bias << shift)));
        sum = LimitEffectiveBits<T>(d, sum, plan.bits_per_sample);
        hn::StoreU(hn::DemoteTo(ds, sum), ds, dst + x);
        continue;
      }
      const auto offsets = hn::LoadU(d, plan.offsets.data() + x);
      const auto sizes = hn::LoadU(d, plan.sizes.data() + x);
      const auto bases = hn::Mul(hn::Iota(d, int32_t(x)), hn::Set(d, plan.filter_size));
      auto sum = hn::Set(d, 1 << (shift - 1));
      const int taps = hn::ReduceMax(d, sizes);
      for (int k = 0; k < taps; ++k) {
        const auto step = hn::Min(hn::Set(d, k), hn::Sub(sizes, hn::Set(d, 1)));
        const auto values = hn::Sub(GatherNarrow(d, src, hn::Add(offsets, step), plan.source_size * int(sizeof(T)) - 4),
                                    hn::Set(d, bias));
        const auto raw =
            GatherNarrow(d, plan.integers.data(), hn::Add(bases, hn::Set(d, k)), int(plan.integers.size() * 2 - 4));
        const auto weights = hn::ShiftRight<16>(hn::ShiftLeft<16>(raw));
        sum = hn::Add(sum, hn::Mul(values, weights));
      }
      sum = hn::ShiftRight<shift>(hn::Add(sum, hn::Set(d, bias << shift)));
      sum = LimitEffectiveBits<T>(d, sum, plan.bits_per_sample);
      hn::StoreU(hn::DemoteTo(ds, sum), ds, dst + x);
    }
    for (; x < width; ++x) {
      int64_t sum = int64_t(1) << (shift - 1);
      const size_t base = x * plan.filter_size;
      for (int k = 0; k < plan.sizes[x]; ++k)
        sum += int64_t(int(src[plan.offsets[x] + k]) - bias) * plan.integers[base + k];
      sum += int64_t(bias) << shift;
      dst[x] = T(std::clamp(sum, int64_t(0), int64_t(limit) << shift) / (int64_t(1) << shift));
    }
  }
}
void RunHorizontalInteger(const resample::Coefficients& plan, vc_const_plane source, vc_plane destination, vc_rows rows,
                          int source_first, int destination_first) {
  if (plan.bits_per_sample == 8)
    HorizontalInteger<uint8_t>(plan, source, destination, rows, source_first, destination_first);
  else
    HorizontalInteger<uint16_t>(plan, source, destination, rows, source_first, destination_first);
}
void RunHorizontalFloat(const resample::Coefficients& plan, vc_const_plane source, vc_plane destination, vc_rows rows,
                        int source_first, int destination_first) {
  const hn::ScalableTag<float> d;
  const hn::Rebind<int32_t, decltype(d)> di;
  const size_t lanes = hn::Lanes(d);
  const size_t width = size_t(rows.width);
  // Gather indices must fit signed 32 bits. Very large plans retain scalar access.
  const size_t end = plan.floats.size() <= size_t(std::numeric_limits<int32_t>::max()) ? width - width % lanes : 0;
  const bool stream = uint64_t(rows.width) * rows.row_count * sizeof(float) >= 512 * 1024 &&
                      reinterpret_cast<uintptr_t>(destination.data) % 64 == 0 && destination.stride % 64 == 0;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const float* src = Row<float>(source, y - source_first);
    float* dst = Row<float>(destination, y - destination_first);
    size_t x = 0;
    for (; x < end; x += lanes) {
      if (plan.horizontal.lanes == lanes) {
        StoreOutput(d, PackedHorizontalSum(d, plan, plan.horizontal.blocks[x / lanes], src, 0), dst + x, stream);
        continue;
      }
      const auto offsets = hn::LoadU(di, plan.offsets.data() + x);
      const auto sizes = hn::LoadU(di, plan.sizes.data() + x);
      const auto bases = hn::Mul(hn::Iota(di, int32_t(x)), hn::Set(di, plan.filter_size));
      auto sum = hn::Zero(d);
      const int taps = hn::ReduceMax(di, sizes);
      for (int k = 0; k < taps; ++k) {
        const auto valid = hn::Lt(hn::Set(di, k), sizes);
        // Inactive lanes reread the last valid sample with a zero coefficient.
        // No gather may cross the source boundary, even with zero weight.
        const auto step = hn::Min(hn::Set(di, k), hn::Sub(sizes, hn::Set(di, 1)));
        const auto samples = hn::GatherIndex(d, src, hn::Add(offsets, step));
        const auto weights = hn::GatherIndex(d, plan.floats.data(), hn::Add(bases, hn::Set(di, k)));
        const auto product = hn::Mul(samples, weights);
        sum = hn::IfThenElse(hn::RebindMask(d, valid), hn::Add(sum, product), sum);
      }
      StoreOutput(d, sum, dst + x, stream);
    }
    for (; x < width; ++x) {
      float sum = 0;
      const size_t base = x * plan.filter_size;
      for (int k = 0; k < plan.sizes[x]; ++k)
        sum += src[plan.offsets[x] + k] * plan.floats[base + k];
      dst[x] = sum;
    }
  }
  if (stream)
    hwy::FlushStream();
}
size_t ResampleLanes() {
  return hn::Lanes(hn::ScalableTag<int32_t>());
}
void RunVertical(const resample::Coefficients& plan, vc_const_plane source, vc_plane destination, vc_rows rows,
                 int source_first, int destination_first) {
  if (plan.bits_per_sample == 8)
    Vertical<uint8_t>(plan, source, destination, rows, source_first, destination_first);
  else if (plan.bits_per_sample == 32)
    Vertical<float>(plan, source, destination, rows, source_first, destination_first);
  else
    Vertical<uint16_t>(plan, source, destination, rows, source_first, destination_first);
}
#endif
} // namespace HWY_NAMESPACE
} // namespace vc
HWY_AFTER_NAMESPACE();
#if HWY_ONCE
namespace vc::resample {
int64_t ResampleSupportedTargets() {
  int64_t mask = 0;
#define VC_TARGET(target, choose) mask |= target;
#include "targets.inc"
#undef VC_TARGET
  return mask & hwy::SupportedTargets();
}
const ResampleKernels* GetResampleKernels(int64_t target) {
  const auto supported = ResampleSupportedTargets();
  if (target == VC_TARGET_NATIVE)
    target = supported ? supported & -supported : 0;
  if (target <= 0 || (target & (target - 1)) || !(target & supported))
    return nullptr;
  switch (target) {
#define VC_TARGET(target, choose)                                                                                      \
  case target: {                                                                                                       \
    static const ResampleKernels kernels{(choose(ResampleLanes))(), choose(RunHorizontalFloat),                        \
                                         choose(RunHorizontalInteger), choose(RunVertical)};                           \
    return &kernels;                                                                                                   \
  }
#include "targets.inc"
#undef VC_TARGET
    default:
      return nullptr;
  }
}

} // namespace vc::resample
#endif
