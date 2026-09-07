// Highway matrix kernels developed in AviSynthConvertVideo.
// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "matrix/highway.h"
#include "matrix/transform.h"
#include "layout/buffer.h"
#include <algorithm>
#include <type_traits>
#include <hwy/targets.h>
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "matrix/highway.cpp"
#include <hwy/foreach_target.h>
#include <hwy/highway.h>
HWY_BEFORE_NAMESPACE();
namespace vc {
namespace HWY_NAMESPACE {
namespace hn = hwy::HWY_NAMESPACE;
#if HWY_TARGET != HWY_SCALAR && HWY_TARGET != HWY_EMU128
template <class T, int outputs = 3>
void MatrixIntegerTail(const matrix::Config& config, const matrix::IntegerTransform& t, const T* const (&src)[3],
                       T* const (&dst)[outputs], size_t x, size_t width) {
  for (; x < width; ++x) {
    const int64_t values[3] = {int64_t(src[0][x]) + t.input_offsets[0], int64_t(src[1][x]) + t.input_offsets[1],
                               int64_t(src[2][x]) + t.input_offsets[2]};
    for (int c = 0; c < outputs; ++c) {
      const int64_t sum =
          values[0] * t.weights[c][0] + values[1] * t.weights[c][1] + values[2] * t.weights[c][2] + t.biases[c];
      const int64_t scale = int64_t{1} << config.precision;
      const int64_t shifted = sum >= 0 ? sum / scale : -((-sum + scale - 1) / scale);
      dst[c][x] = T(std::clamp(shifted + t.output_offsets[c], int64_t{0}, int64_t(t.limit)));
    }
  }
}
// Storage, accumulator width and direction share one integer matrix family.
// Coefficients and offsets are already signed; no reinterpretation of U16 data.
template <class T, class A, int outputs = 3>
void MatrixInteger(const matrix::Config& config, const matrix::Coefficients& m,
                   const std::array<vc_const_plane, 3>& source, const std::array<vc_plane, 3>& destination,
                   vc_rows rows) {
  const auto t = matrix::MakeIntegerTransform(config, m);
  const hn::ScalableTag<A> d;
  const hn::Rebind<T, decltype(d)> ds;
  const hn::Rebind<int32_t, decltype(d)> d32;
  const size_t lanes = hn::Lanes(d), width = size_t(rows.width);
  const auto zero = hn::Zero(d), ceiling = hn::Set(d, A(t.limit));
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const T* src[3] = {Row<T>(source[0], y), Row<T>(source[1], y), Row<T>(source[2], y)};
    T* dst[outputs];
    for (int c = 0; c < outputs; ++c)
      dst[c] = Row<T>(destination[c], y);
    size_t x = 0;
    for (; x + lanes <= width; x += lanes) {
      std::array<hn::VFromD<decltype(d)>, 3> values;
      for (int k = 0; k < 3; ++k) {
        const auto loaded = hn::PromoteTo(d32, hn::LoadU(ds, src[k] + x));
        if constexpr (sizeof(A) == 8)
          values[k] = hn::Add(hn::PromoteTo(d, loaded), hn::Set(d, A(t.input_offsets[k])));
        else
          values[k] = hn::Add(loaded, hn::Set(d, A(t.input_offsets[k])));
      }
      for (int c = 0; c < outputs; ++c) {
        auto sum = hn::Mul(values[0], hn::Set(d, A(t.weights[c][0])));
        sum = hn::Add(sum, hn::Mul(values[1], hn::Set(d, A(t.weights[c][1]))));
        sum = hn::Add(sum, hn::Mul(values[2], hn::Set(d, A(t.weights[c][2]))));
        sum = hn::Add(sum, hn::Set(d, A(t.biases[c])));
        const auto shifted = hn::ShiftRightSame(sum, config.precision);
        const auto value = hn::Min(hn::Max(hn::Add(shifted, hn::Set(d, A(t.output_offsets[c]))), zero), ceiling);
        if constexpr (sizeof(A) == 8)
          hn::StoreU(hn::DemoteTo(ds, hn::DemoteTo(d32, value)), ds, dst[c] + x);
        else
          hn::StoreU(hn::DemoteTo(ds, value), ds, dst[c] + x);
      }
    }
    MatrixIntegerTail<T, outputs>(config, t, src, dst, x, width);
  }
}
// Adjacent signed 16-bit products share a widening multiply-add when all
// coefficients fit i16 and the proven i32 accumulator bound still holds.
// x86 widening pairs can stay in 128-bit block order until the saturating pack.
// Other targets retain the portable whole-vector order; ReorderDemote2To does
// not promise the same permutation on every architecture.
template <class D, class V>
HWY_INLINE auto MatrixPairLower(D d, V a, V b) {
#if HWY_ARCH_X86
  return hn::InterleaveLower(d, a, b);
#else
  return hn::InterleaveWholeLower(d, a, b);
#endif
}
template <class D, class V>
HWY_INLINE auto MatrixPairUpper(D d, V a, V b) {
#if HWY_ARCH_X86
  return hn::InterleaveUpper(d, a, b);
#else
  return hn::InterleaveWholeUpper(d, a, b);
#endif
}
template <class D, class V>
HWY_INLINE auto MatrixPairDemote(D d, V a, V b) {
#if HWY_ARCH_X86
  return hn::ReorderDemote2To(d, a, b);
#else
  return hn::OrderedDemote2To(d, a, b);
#endif
}
template <class T, bool full_storage, int outputs = 3, bool folded_offsets = false>
void MatrixPairs(const matrix::Config& config, const matrix::Coefficients& m,
                 const std::array<vc_const_plane, 3>& source, const std::array<vc_plane, 3>& destination,
                 vc_rows rows) {
  auto t = matrix::MakeIntegerTransform(config, m);
  if constexpr (folded_offsets) {
    for (int c = 0; c < outputs; ++c) {
      t.biases[c] += t.output_offsets[c] * (int64_t{1} << config.precision);
      t.output_offsets[c] = 0;
    }
  }
  const hn::ScalableTag<int16_t> d16;
  const hn::Repartition<int32_t, decltype(d16)> d32;
  const hn::Rebind<T, decltype(d16)> ds;
  const size_t lanes = hn::Lanes(d16), width = size_t(rows.width);
  const auto offset = hn::Set(d16, int16_t(t.input_offsets[0]));
  const auto limit = hn::Set(d32, t.limit);
  const int precision = config.precision;
  std::array<hn::VFromD<decltype(d16)>, 3> wbg, wrz;
  std::array<hn::VFromD<decltype(d32)>, 3> biases, offsets;
  for (int c = 0; c < outputs; ++c) {
    wbg[c] =
        hn::InterleaveWholeLower(d16, hn::Set(d16, int16_t(t.weights[c][0])), hn::Set(d16, int16_t(t.weights[c][1])));
    wrz[c] = hn::InterleaveWholeLower(d16, hn::Set(d16, int16_t(t.weights[c][2])), hn::Zero(d16));
    biases[c] = hn::Set(d32, int32_t(t.biases[c]));
    offsets[c] = hn::Set(d32, int32_t(t.output_offsets[c]));
  }
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const T* src[3] = {Row<T>(source[0], y), Row<T>(source[1], y), Row<T>(source[2], y)};
    T* dst[outputs];
    for (int c = 0; c < outputs; ++c)
      dst[c] = Row<T>(destination[c], y);
    size_t x = 0;
#if HWY_ARCH_X86
    if constexpr (sizeof(T) == 1) {
      const hn::Repartition<uint8_t, decltype(d16)> db;
      const auto zero = hn::Zero(db);
      for (; x + 2 * lanes <= width; x += 2 * lanes) {
        std::array<hn::VFromD<decltype(d16)>, 3> a, b;
        for (int k = 0; k < 3; ++k) {
          const auto bytes = hn::LoadU(db, src[k] + x);
          // Preserve 128-bit block order through both widening stages so the
          // final two packs restore contiguous bytes without a lane shuffle.
          a[k] = hn::Add(hn::BitCast(d16, hn::InterleaveLower(db, bytes, zero)), offset);
          b[k] = hn::Add(hn::BitCast(d16, hn::InterleaveUpper(db, bytes, zero)), offset);
        }
        const auto a0 = MatrixPairLower(d16, a[0], a[1]);
        const auto a1 = MatrixPairUpper(d16, a[0], a[1]);
        const auto a2 = MatrixPairLower(d16, a[2], hn::Zero(d16));
        const auto a3 = MatrixPairUpper(d16, a[2], hn::Zero(d16));
        const auto b0 = MatrixPairLower(d16, b[0], b[1]);
        const auto b1 = MatrixPairUpper(d16, b[0], b[1]);
        const auto b2 = MatrixPairLower(d16, b[2], hn::Zero(d16));
        const auto b3 = MatrixPairUpper(d16, b[2], hn::Zero(d16));
        for (int c = 0; c < outputs; ++c) {
          const auto calculate = [&](auto p0, auto p1, auto p2, auto p3) HWY_ATTR {
            auto lo = hn::Add(hn::WidenMulPairwiseAdd(d32, p0, wbg[c]), hn::WidenMulPairwiseAdd(d32, p2, wrz[c]));
            auto hi = hn::Add(hn::WidenMulPairwiseAdd(d32, p1, wbg[c]), hn::WidenMulPairwiseAdd(d32, p3, wrz[c]));
            lo = hn::ShiftRightSame(hn::Add(lo, biases[c]), precision);
            hi = hn::ShiftRightSame(hn::Add(hi, biases[c]), precision);
            if constexpr (!folded_offsets) {
              lo = hn::Add(lo, offsets[c]);
              hi = hn::Add(hi, offsets[c]);
            }
            if constexpr (!full_storage) {
              lo = hn::Min(lo, limit);
              hi = hn::Min(hi, limit);
            }
            return MatrixPairDemote(d16, lo, hi);
          };
          hn::StoreU(hn::ReorderDemote2To(db, calculate(a0, a1, a2, a3), calculate(b0, b1, b2, b3)), db, dst[c] + x);
        }
      }
    }
#endif
    for (; x + lanes <= width; x += lanes) {
      std::array<hn::VFromD<decltype(d16)>, 3> values;
      for (int k = 0; k < 3; ++k) {
        if constexpr (sizeof(T) == 1)
          values[k] = hn::Add(hn::PromoteTo(d16, hn::LoadU(ds, src[k] + x)), offset);
        else
          values[k] = hn::Add(hn::BitCast(d16, hn::LoadU(ds, src[k] + x)), offset);
      }
      const auto bg0 = MatrixPairLower(d16, values[0], values[1]);
      const auto bg1 = MatrixPairUpper(d16, values[0], values[1]);
      const auto rz0 = MatrixPairLower(d16, values[2], hn::Zero(d16));
      const auto rz1 = MatrixPairUpper(d16, values[2], hn::Zero(d16));
      for (int c = 0; c < outputs; ++c) {
        auto lo = hn::Add(hn::WidenMulPairwiseAdd(d32, bg0, wbg[c]), hn::WidenMulPairwiseAdd(d32, rz0, wrz[c]));
        auto hi = hn::Add(hn::WidenMulPairwiseAdd(d32, bg1, wbg[c]), hn::WidenMulPairwiseAdd(d32, rz1, wrz[c]));
        lo = hn::ShiftRightSame(hn::Add(lo, biases[c]), precision);
        hi = hn::ShiftRightSame(hn::Add(hi, biases[c]), precision);
        if constexpr (!folded_offsets) {
          lo = hn::Add(lo, offsets[c]);
          hi = hn::Add(hi, offsets[c]);
        }
        // Saturating demotion supplies the storage bounds, including zero.
        if constexpr (!full_storage) {
          lo = hn::Min(lo, limit);
          hi = hn::Min(hi, limit);
        }
        if constexpr (sizeof(T) == 1)
          hn::StoreU(hn::DemoteTo(ds, MatrixPairDemote(d16, lo, hi)), ds, dst[c] + x);
        else
          hn::StoreU(MatrixPairDemote(ds, lo, hi), ds, dst[c] + x);
      }
    }
    MatrixIntegerTail<T, outputs>(config, t, src, dst, x, width);
  }
}
template <bool forward, int outputs = 3>
void MatrixFloat(const matrix::Config&, const matrix::Coefficients& m, const std::array<vc_const_plane, 3>& source,
                 const std::array<vc_plane, 3>& destination, vc_rows rows) {
  const float weights[3][3] = {{m.y_b_f, forward ? m.y_g_f : m.u_b_f, forward ? m.y_r_f : m.v_b_f},
                               {forward ? m.u_b_f : m.y_g_f, m.u_g_f, forward ? m.u_r_f : m.v_g_f},
                               {forward ? m.v_b_f : m.y_r_f, forward ? m.v_g_f : m.u_r_f, m.v_r_f}};
  const hn::ScalableTag<float> d;
  const size_t lanes = hn::Lanes(d), width = size_t(rows.width);
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const float* src[3] = {Row<float>(source[0], y), Row<float>(source[1], y), Row<float>(source[2], y)};
    float* dst[outputs];
    for (int c = 0; c < outputs; ++c)
      dst[c] = Row<float>(destination[c], y);
    size_t x = 0;
    for (; x + lanes <= width; x += lanes) {
      auto a = hn::LoadU(d, src[0] + x);
      auto b = hn::LoadU(d, src[1] + x);
      auto r = hn::LoadU(d, src[2] + x);
      if constexpr (forward) {
        if (m.offset_rgb_f != 0) {
          const auto offset = hn::Set(d, m.offset_rgb_f);
          a = hn::Add(a, offset);
          b = hn::Add(b, offset);
          r = hn::Add(r, offset);
        }
      } else {
        a = hn::Add(a, hn::Set(d, m.offset_y_f));
      }
      for (int c = 0; c < outputs; ++c) {
        // Deliberately separate multiplication/addition, matching C rounding.
        auto sum = hn::Add(hn::Mul(hn::Set(d, weights[c][0]), a), hn::Mul(hn::Set(d, weights[c][1]), b));
        sum = hn::Add(sum, hn::Mul(hn::Set(d, weights[c][2]), r));
        sum = hn::Add(sum, hn::Set(d, forward ? (c == 0 ? m.offset_y_f : 0.f) : m.offset_rgb_f));
        const auto lo = hn::Set(d, forward && c > 0 ? -.5f : 0.f);
        const auto hi = hn::Set(d, forward && c > 0 ? .5f : 1.f);
        if constexpr (outputs == 1)
          hn::StoreU(sum, d, dst[c] + x);
        else
          hn::StoreU(hn::Min(hn::Max(sum, lo), hi), d, dst[c] + x);
      }
    }
    for (; x < width; ++x) {
      float a = src[0][x], b = src[1][x], r = src[2][x];
      if constexpr (forward) {
        if (m.offset_rgb_f != 0) {
          a += m.offset_rgb_f;
          b += m.offset_rgb_f;
          r += m.offset_rgb_f;
        }
      } else {
        a += m.offset_y_f;
      }
      for (int c = 0; c < outputs; ++c) {
        const float sum = weights[c][0] * a + weights[c][1] * b + weights[c][2] * r;
        const float value = sum + (forward ? (c == 0 ? m.offset_y_f : 0.f) : m.offset_rgb_f);
        dst[c][x] =
            outputs == 1 ? value : std::clamp(value, forward && c > 0 ? -.5f : 0.f, forward && c > 0 ? .5f : 1.f);
      }
    }
  }
}
template <int outputs>
matrix::RowKernel ChooseMatrixKernelImpl(const matrix::Config& config, const matrix::Coefficients& m) {
  if (config.bits_per_sample == 32)
    return config.direction != matrix::Direction::YuvToRgb ? MatrixFloat<true, outputs> : MatrixFloat<false, outputs>;
  const auto transform = matrix::MakeIntegerTransform(config, m);
  const bool narrow = matrix::FitsInt32(transform, config.precision);
  bool paired = narrow;
  for (const auto& row : transform.weights)
    for (const int weight : row)
      paired = paired && weight >= -32768 && weight <= 32767;
  if (paired) {
    // Folding an integer output offset into the fixed-point bias is exact.
    // Prove the larger pre-shift sum safe before removing the vector additions.
    auto folded = transform;
    for (int c = 0; c < 3; ++c) {
      folded.biases[c] += folded.output_offsets[c] * (int64_t{1} << config.precision);
      folded.output_offsets[c] = 0;
    }
    if (matrix::FitsInt32(folded, config.precision)) {
      if (config.bits_per_sample == 8)
        return MatrixPairs<uint8_t, true, outputs, true>;
      return config.bits_per_sample == 16 ? MatrixPairs<uint16_t, true, outputs, true>
                                          : MatrixPairs<uint16_t, false, outputs, true>;
    }
    if (config.bits_per_sample == 8)
      return MatrixPairs<uint8_t, true, outputs>;
    return config.bits_per_sample == 16 ? MatrixPairs<uint16_t, true, outputs> : MatrixPairs<uint16_t, false, outputs>;
  }
  if (config.bits_per_sample == 8)
    return narrow ? MatrixInteger<uint8_t, int32_t, outputs> : MatrixInteger<uint8_t, int64_t, outputs>;
  return narrow ? MatrixInteger<uint16_t, int32_t, outputs> : MatrixInteger<uint16_t, int64_t, outputs>;
}
matrix::RowKernel ChooseMatrixKernel(const matrix::Config& config, const matrix::Coefficients& m) {
  return config.direction == matrix::Direction::RgbToY ? ChooseMatrixKernelImpl<1>(config, m)
                                                       : ChooseMatrixKernelImpl<3>(config, m);
}
#endif
} // namespace HWY_NAMESPACE
} // namespace vc
HWY_AFTER_NAMESPACE();
#if HWY_ONCE
namespace vc::matrix {
int64_t MatrixSupportedTargets() {
  int64_t mask = 0;
#define VC_TARGET(target, choose) mask |= target;
#include "targets.inc"
#undef VC_TARGET
  return mask & hwy::SupportedTargets();
}
RowKernel GetMatrixKernel(int64_t target, const Config& config, const Coefficients& coefficients) {
  const auto supported = MatrixSupportedTargets();
  if (target == VC_TARGET_NATIVE)
    target = supported ? supported & -supported : 0;
  if (target <= 0 || (target & (target - 1)) || !(target & supported))
    return nullptr;
  switch (target) {
#define VC_TARGET(target, choose)                                                                                      \
  case target:                                                                                                         \
    return (choose(ChooseMatrixKernel))(config, coefficients);
#include "targets.inc"
#undef VC_TARGET
    default:
      return nullptr;
  }
}
} // namespace vc::matrix
#endif
