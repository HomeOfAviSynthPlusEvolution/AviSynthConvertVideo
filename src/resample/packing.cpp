// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "coefficients.h"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace vc::resample {
void PrepareHorizontal(Coefficients& plan, size_t lanes) {
  if (lanes == 0 || lanes > size_t(std::numeric_limits<int>::max() / 4))
    throw std::invalid_argument("Invalid SIMD lane count");
  HorizontalPacking packed;
  packed.lanes = lanes;
  const size_t blocks = size_t(plan.target_size) / lanes;
  packed.blocks.reserve(blocks);
  for (size_t block = 0; block < blocks; ++block) {
    const size_t first = block * lanes;
    int start = plan.source_size, end = 0, taps = 0;
    for (size_t i = 0; i < lanes; ++i) {
      start = std::min(start, plan.offsets[first + i]);
      end = std::max(end, plan.offsets[first + i] + plan.sizes[first + i]);
      taps = std::max(taps, plan.sizes[first + i]);
    }
    int window = 0;
    for (int size : {int(lanes) * 2, int(lanes) * 4})
      if (plan.source_size >= size && end - start <= size) {
        window = size;
        break;
      }
    if (window)
      start = std::min(start, plan.source_size - window);
    else
      start = 0;
    bool linear = plan.bits_per_sample == 32;
    for (size_t i = 0; i < lanes; ++i)
      linear = linear && plan.sizes[first + i] == taps;
    // Long float supports can slide a two-vector input window per tap without
    // holding the entire support in registers. Keep the original sum order.
    if (linear && !window)
      start = plan.offsets[first];
    bool sliding = linear && taps >= 8 && ((lanes == 8 && window == int(4 * lanes)) || !window) &&
                   start <= plan.source_size - int(2 * lanes) - (taps - 1);
    for (size_t i = 0; i < lanes; ++i)
      sliding = sliding && plan.offsets[first + i] - start < int(2 * lanes);
    if (!window && !sliding)
      start = 0;
    bool stride_two = sliding;
    for (size_t i = 0; i < lanes; ++i)
      stride_two = stride_two && plan.offsets[first + i] - start == int(2 * i);
    packed.blocks.push_back(
        {start, window, taps, packed.indices.size(), packed.pair_indices.size(), linear, sliding, stride_two});
    for (int k = 0; k < taps; ++k)
      for (size_t i = 0; i < lanes; ++i) {
        const size_t position = first + i;
        packed.indices.push_back(plan.offsets[position] + std::min(k, plan.sizes[position] - 1) - start);
        const size_t index = position * plan.filter_size + k;
        if (plan.bits_per_sample == 32)
          packed.float_weights.push_back(plan.floats[index]);
        else
          packed.integer_weights.push_back(plan.integers[index]);
      }
    if (plan.bits_per_sample != 32 && window && 4 * lanes <= size_t(std::numeric_limits<int16_t>::max())) {
      for (int k = 0; k < taps; k += 2)
        for (size_t i = 0; i < lanes; ++i)
          for (int j = 0; j < 2; ++j) {
            const size_t position = first + i;
            packed.pair_indices.push_back(
                int16_t(plan.offsets[position] + std::min(k + j, plan.sizes[position] - 1) - start));
            packed.pair_weights.push_back(
                k + j < plan.sizes[position] ? plan.integers[position * plan.filter_size + k + j] : 0);
          }
    }
  }
  if (plan.bits_per_sample != 32 && lanes == 8 && plan.filter_size >= 8 && plan.filter_size <= 16) {
    if (size_t(plan.target_size) > packed.dot_weights.max_size() / 16)
      throw std::length_error("Horizontal coefficient packing is too large");
    packed.dot_weights.resize(size_t(plan.target_size) * 16);
    for (int x = 0; x < plan.target_size; ++x)
      std::copy_n(plan.integers.data() + size_t(x) * plan.filter_size, plan.filter_size,
                  packed.dot_weights.data() + size_t(x) * 16);
    for (int x = 0; plan.target_size - x >= 4; x += 4) {
      bool safe = true;
      for (int i = 0; i < 4; ++i)
        safe = safe && plan.offsets[x + i] <= plan.source_size - 16;
      if (!safe)
        break;
      packed.dot_outputs = x + 4;
    }
  }
  plan.horizontal = std::move(packed);
}
} // namespace vc::resample
