// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_RESAMPLE_COEFFICIENTS_H
#define VIDEO_CONVERT_RESAMPLE_COEFFICIENTS_H
#include "functions.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace vc::resample {
struct CoefficientRequest {
  int source_size;
  int target_size;
  double crop_start;
  double crop_size;
  int bits_per_sample;
  double source_center = 0.5;
  double destination_center = 0.5;
};

struct HorizontalBlock {
  int source_start, window_size, taps;
  size_t coefficient_start;
  size_t pair_start;
  bool linear_indices;
  bool sliding_window;
  bool stride_two;
};
struct HorizontalPacking {
  size_t lanes = 0;
  int dot_outputs = 0;
  std::vector<int16_t> dot_weights;
  std::vector<HorizontalBlock> blocks;
  std::vector<int32_t> indices, integer_weights;
  std::vector<float> float_weights;
  std::vector<int16_t> pair_indices, pair_weights;
};

// Private, unpadded mathematical plan. Coefficients for destination i begin at
// i * filter_size; sizes[i] entries are active, remaining entries are zero.
// Offsets/sizes describe a contiguous source interval after edge folding.
// Later target-specific packing must preserve these active values.
struct Coefficients {
  int source_size;
  int target_size;
  int bits_per_sample;
  int filter_size;
  int filter_size_real;
  std::vector<int> offsets;
  std::vector<int> sizes;
  std::vector<int16_t> integers;
  std::vector<float> floats;
  // Maximum per-output sum of absolute integer coefficients; -1 means unknown.
  int64_t max_abs_sum = -1;
  HorizontalPacking horizontal;
};

// Prepare once before publishing a horizontal SIMD plan. Mathematical data stays intact.
void PrepareHorizontal(Coefficients& plan, size_t lanes);
// Allocates only while preparing. Invalid/nonfinite requests or unrepresentable
// coefficients throw standard exceptions privately; the C boundary translates
// them into status codes. The function and request are never mutated.
Coefficients BuildCoefficients(const ResamplingFunction& function, const CoefficientRequest& request);
} // namespace vc::resample
#endif
