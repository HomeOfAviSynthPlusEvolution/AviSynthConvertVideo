// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_DITHER_ORDERED_H
#define VIDEO_CONVERT_DITHER_ORDERED_H
#include "video_convert/dither.h"
#include "depth/transform.h"
#include <algorithm>
#include <array>
struct vc_ordered_plan {
  vc_ordered_config config;
  vc::depth::Transform range;
  std::array<int, 512> thresholds;
  int period, shift, output_max, quantized_max;
  float backscale;
  std::array<uint16_t, 1024> thresholds16{};
  void (*kernel)(const vc_ordered_plan&, vc_const_plane, vc_plane, vc_rows) = nullptr;
};
namespace vc::dither {
using RowKernel = decltype(vc_ordered_plan::kernel);
int64_t SupportedTargets();
RowKernel GetKernel(int64_t target, const vc_ordered_config& config);
inline int FloorDivide(int value, int divisor) {
  // Explicit floor division for negative low-depth corrections: C++17 signed
  // right shifts are implementation-defined. Values are bounded by 16-bit input.
  return value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor);
}

inline int Quantize(const vc_ordered_plan& p, int input, int x, int y) {
  const auto& c = p.config.depth;
  const int source_max = (1 << c.source_bits) - 1, divisor = 1 << p.shift;
  int value = input;
  if (c.source_full != c.destination_full) {
    const float scaled = (float(value) - p.range.source_offset) * p.range.factor;
    value = std::clamp(int(scaled + (p.range.destination_offset + .5f)), 0, source_max);
  }
  const int corr = p.thresholds[(y & 15) * 32 + (x & 15)];
  int quantized;
  if (p.config.quantization_bits < 8)
    quantized = FloorDivide(int(float(value) + float(corr) - float(divisor - 1) * .5f), divisor);
  else
    quantized = (value + corr) / divisor;
  if (c.destination_bits != p.config.quantization_bits) {
    quantized = std::min(quantized, p.quantized_max);
    if (p.config.quantization_bits < 8)
      quantized = int(float(quantized) * p.backscale + .5f);
    else
      quantized *= 1 << (c.destination_bits - p.config.quantization_bits);
  }
  return std::clamp(quantized, 0, p.output_max);
}
} // namespace vc::dither
#endif
