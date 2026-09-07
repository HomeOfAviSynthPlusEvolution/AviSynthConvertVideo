// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#pragma once
#include "coefficients.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
namespace vc::matrix {
struct IntegerTransform {
  std::array<std::array<int, 3>, 3> weights;
  std::array<int, 3> input_offsets;
  std::array<int64_t, 3> biases, output_offsets;
  int limit;
};
inline IntegerTransform MakeIntegerTransform(const Config& config, const Coefficients& m) {
  const bool f = config.direction != Direction::YuvToRgb;
  const int center = 1 << (config.bits_per_sample - 1);
  const int64_t scale = int64_t{1} << config.precision;
  const int64_t rounding = config.precision ? scale / 2 : 0;
  IntegerTransform t{};
  t.weights = {{{m.y_b, f ? m.y_g : m.u_b, f ? m.y_r : m.v_b},
                {f ? m.u_b : m.y_g, m.u_g, f ? m.u_r : m.v_g},
                {f ? m.v_b : m.y_r, f ? m.v_g : m.u_r, m.v_r}}};
  const int offsets[3] = {f ? m.offset_rgb : m.offset_y, f ? m.offset_rgb : -center, f ? m.offset_rgb : -center};
  t.input_offsets = {-center, -center, -center};
  t.biases = {rounding + int64_t(f ? m.offset_y : m.offset_rgb) * scale,
              rounding + int64_t(f ? center : m.offset_rgb) * scale,
              rounding + int64_t(f ? center : m.offset_rgb) * scale};
  for (int c = 0; c < 3; ++c) {
    // Centering keeps common 16-bit products within i32. Split the constant
    // into quotient/remainder so adding its large fixed-point value cannot
    // overflow: floor((dot + q*scale + r)/scale) == floor((dot+r)/scale)+q.
    for (int k = 0; k < 3; ++k)
      t.biases[c] += int64_t(t.weights[c][k]) * (center + offsets[k]);
    const int64_t bias = t.biases[c];
    const int64_t q = bias >= 0 ? bias / scale : -((-bias + scale - 1) / scale);
    t.output_offsets[c] = q;
    t.biases[c] -= q * scale;
  }
  t.limit = (1 << config.bits_per_sample) - 1;
  return t;
}
inline bool FitsInt32(const IntegerTransform& t, int precision) {
  const auto maximum = int64_t(std::numeric_limits<int32_t>::max());
  for (int c = 0; c < 3; ++c) {
    const int64_t bias = t.biases[c];
    int64_t bound = bias < 0 ? -bias : bias;
    for (int k = 0; k < 3; ++k) {
      const int64_t lo = t.input_offsets[k], hi = lo + t.limit;
      const int64_t magnitude = std::max(lo < 0 ? -lo : lo, hi < 0 ? -hi : hi);
      const int64_t w = t.weights[c][k];
      bound += magnitude * (w < 0 ? -w : w);
    }
    // Bound every intermediate product/addition, and the post-shift addition.
    const int64_t output = t.output_offsets[c];
    if (bound > maximum || (bound >> precision) + 1 + (output < 0 ? -output : output) > maximum)
      return false;
  }
  return true;
}
} // namespace vc::matrix
