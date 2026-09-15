// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "coefficients.h"
#include <algorithm>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
namespace vc::resample {
namespace {
void CompactPairs(HorizontalPacking& packed) {
  if (packed.pair_indices.empty() || packed.dot_outputs)
    return;
  decltype(packed.pair_indices) indices, weights;
  std::vector<size_t> starts(packed.blocks.size());
  std::map<std::string, size_t> unique;
  std::string key;
  for (size_t i = 0; i < packed.blocks.size(); ++i) {
    const auto& block = packed.blocks[i];
    if (!block.window_size && !block.stride_two && !block.single_sliding)
      continue;
    const size_t first = block.pair_start;
    const size_t count = (size_t(block.taps) / 2 + block.taps % 2) * 2 * packed.lanes;
    // Both relative indices and every coefficient must match exactly. Absolute
    // source starts remain per block; no phase approximation or tap reordering.
    key.clear();
    key.reserve(2 * count * sizeof(int16_t));
    key.append(reinterpret_cast<const char*>(packed.pair_indices.data() + first), count * sizeof(int16_t));
    key.append(reinterpret_cast<const char*>(packed.pair_weights.data() + first), count * sizeof(int16_t));
    // Duplicate keys are not moved, so their scratch allocation can be reused.
    const auto entry = unique.try_emplace(std::move(key), indices.size());
    starts[i] = entry.first->second;
    if (entry.second) {
      indices.insert(indices.end(), packed.pair_indices.begin() + first, packed.pair_indices.begin() + first + count);
      weights.insert(weights.end(), packed.pair_weights.begin() + first, packed.pair_weights.begin() + first + count);
    }
  }
  if (indices.size() == packed.pair_indices.size())
    return;
  for (size_t i = 0; i < packed.blocks.size(); ++i)
    packed.blocks[i].pair_start = starts[i];
  packed.pair_indices = std::move(indices);
  packed.pair_weights = std::move(weights);
}
} // namespace
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
    // Four-sample spacing can deinterleave four taps from the same input
    // vectors. Prove even the remainder's complete four-vector loads fit;
    // the extra samples must be in the row, not merely in its padding.
    bool stride_four = linear && !window && !sliding && taps >= 8 &&
                       int64_t(start) + int64_t(4 * lanes) + taps - 1 <= plan.source_size;
    for (size_t i = 0; i < lanes; ++i)
      stride_four = stride_four && plan.offsets[first + i] - start == int(4 * i);
    packed.has_stride_four = packed.has_stride_four || stride_four;
    if (!window && !sliding && !stride_four)
      start = 0;
    // Regular float pairs can share one deinterleave, even when the entire
    // support fits a larger register window. Prove the odd tail's full load.
    bool stride_two = linear && taps >= 8 && int64_t(start) + int64_t(2 * lanes) + taps - 1 <= plan.source_size;
    for (size_t i = 0; i < lanes; ++i)
      stride_two = stride_two && plan.offsets[first + i] - start == int(2 * i);
    packed.has_stride_two_float = packed.has_stride_two_float || stride_two;
    if (plan.bits_per_sample != 32) {
      // At a two-sample stride, adjacent taps for all outputs form one
      // contiguous vector. Include the zero-weight mate of an odd last tap
      // in the load-bound proof; it must still be inside the source row.
      if (!window)
        start = plan.offsets[first];
      stride_two = 4 * lanes <= size_t(std::numeric_limits<int16_t>::max()) && start == plan.offsets[first] &&
                   int64_t(start) + int64_t(2 * lanes) + ((int64_t(taps) + 1) / 2) * 2 - 2 <= plan.source_size;
      for (size_t i = 0; i < lanes; ++i)
        stride_two = stride_two && plan.offsets[first + i] - start == int(2 * i);
      if (!window && !stride_two)
        start = 0;
      packed.has_long_stride_two = packed.has_long_stride_two || (!window && stride_two);
    }
    // Upsampling often needs fewer than one vector of source positions per
    // tap. Slide that vector instead of repeatedly selecting from four tables.
    const int first_offset = plan.offsets[first];
    bool single_sliding = linear && taps >= 8 && (window == int(4 * lanes) || !window) &&
                          int64_t(first_offset) + int64_t(lanes) + taps - 1 <= plan.source_size;
    for (size_t i = 0; i < lanes; ++i)
      single_sliding = single_sliding && plan.offsets[first + i] >= first_offset &&
                       plan.offsets[first + i] - first_offset < int(lanes);
    // Long integer supports can slide one vector of adjacent sample pairs.
    // Only the first pair's indices are needed; the final full load includes
    // the zero-weight mate of an odd tap and must remain inside the row.
    if (plan.bits_per_sample != 32 && !window && !stride_two) {
      single_sliding =
          taps >= 8 && 4 * lanes <= size_t(std::numeric_limits<int16_t>::max()) &&
          int64_t(first_offset) + int64_t(2 * lanes) + ((int64_t(taps) + 1) / 2) * 2 - 2 <= plan.source_size;
      for (size_t i = 0; i < lanes; ++i)
        single_sliding = single_sliding && plan.sizes[first + i] == taps && plan.offsets[first + i] >= first_offset &&
                         plan.offsets[first + i] - first_offset + 1 < int(2 * lanes);
      if (single_sliding)
        start = first_offset;
    }
    packed.has_single_sliding = packed.has_single_sliding || single_sliding;
    packed.blocks.push_back({start, window, taps, packed.indices.size(), packed.pair_indices.size(), linear, sliding,
                             stride_two, stride_four, single_sliding});
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
    if (plan.bits_per_sample != 32 && (window || stride_two || single_sliding) &&
        4 * lanes <= size_t(std::numeric_limits<int16_t>::max())) {
      for (int k = 0; k < taps; k += 2)
        for (size_t i = 0; i < lanes; ++i)
          for (int j = 0; j < 2; ++j) {
            const size_t position = first + i;
            // Direct pair loads need no indices; long supports need not fit i16.
            packed.pair_indices.push_back(
                stride_two || (single_sliding && k > 0)
                    ? 0
                    : int16_t(plan.offsets[position] + std::min(k + j, plan.sizes[position] - 1) - start));
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
  // Keep the existing regular-pair and short-dot kernels in their own loops.
  if (plan.bits_per_sample != 32 &&
      (packed.dot_outputs || std::any_of(packed.blocks.begin(), packed.blocks.end(),
                                         [](const HorizontalBlock& block) { return block.stride_two; })))
    packed.has_single_sliding = false;
  // A uniform short support needs no per-block window or tap-loop dispatch.
  // Pair packing already bounds every lookup, including zero-weight edge mates.
  const int pairs = plan.filter_size / 2 + plan.filter_size % 2;
  if (plan.bits_per_sample != 32 && pairs >= 1 && pairs <= 5 &&
      4 * lanes <= size_t(std::numeric_limits<int16_t>::max()) && !packed.blocks.empty() && !packed.dot_outputs &&
      std::all_of(packed.blocks.begin(), packed.blocks.end(), [&](const HorizontalBlock& block) {
        return block.window_size == int(2 * lanes) && !block.stride_two && block.taps / 2 + block.taps % 2 == pairs;
      }))
    packed.single_window_pairs = pairs;
  // Common short float supports can fit all source samples in one vector.
  // Prove every block before changing its window so the fixed-tap entry owns
  // the entire plan; other entries keep their original two/four-vector layout.
  const int float_taps = plan.filter_size;
  if (plan.bits_per_sample == 32 && (float_taps == 2 || float_taps == 6) && plan.source_size >= int(lanes) &&
      !packed.blocks.empty()) {
    bool single = true;
    for (size_t block = 0; block < blocks; ++block) {
      int first = plan.source_size, last = 0;
      for (size_t i = 0; i < lanes; ++i) {
        const size_t x = block * lanes + i;
        first = std::min(first, plan.offsets[x]);
        last = std::max(last, plan.offsets[x] + plan.sizes[x]);
      }
      single = single && last - first <= int(lanes) && packed.blocks[block].taps == float_taps;
    }
    if (single) {
      for (size_t block = 0; block < blocks; ++block) {
        auto& entry = packed.blocks[block];
        const auto begin = plan.offsets.begin() + block * lanes;
        const int start = std::min(*std::min_element(begin, begin + lanes), plan.source_size - int(lanes));
        for (size_t i = 0; i < size_t(float_taps) * lanes; ++i)
          packed.indices[entry.coefficient_start + i] += entry.source_start - start;
        entry.source_start = start;
        entry.window_size = int(lanes);
      }
      packed.single_window_float_taps = float_taps;
    }
  }
  CompactPairs(packed);
  plan.horizontal = std::move(packed);
}
} // namespace vc::resample
