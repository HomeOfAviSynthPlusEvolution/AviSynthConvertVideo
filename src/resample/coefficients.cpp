// Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
// http://avisynth.nl

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.

#include "coefficients.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace vc::resample {
namespace {
size_t CheckedCount(int target, int taps, size_t bytes) {
  const size_t limit = size_t(std::numeric_limits<ptrdiff_t>::max()) / bytes;
  if (size_t(target) > limit / size_t(taps))
    throw std::length_error("Coefficient allocation is too large");
  return size_t(target) * size_t(taps);
}
int CheckedTruncate(double value) {
  if (!std::isfinite(value) || value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
    throw std::invalid_argument("Coefficient/coordinate is outside integer range");
  return int(value);
}
int16_t CheckedInt16(int64_t value) {
  if (value < -32768 || value > 32767)
    throw std::invalid_argument("Coefficient is outside signed 16-bit range");
  return int16_t(value);
}
int16_t CheckedDifference(double current, double previous, int scale) {
  return CheckedInt16(int64_t(CheckedTruncate(current * scale + 0.5)) - CheckedTruncate(previous * scale + 0.5));
}
float CheckedFloat(double value) {
  if (!std::isfinite(value) || std::abs(value) > std::numeric_limits<float>::max())
    throw std::invalid_argument("Coefficient is outside finite float range");
  return float(value);
}
} // namespace

Coefficients BuildCoefficients(const ResamplingFunction& function, const CoefficientRequest& request) {
  const int source_size = request.source_size, target_size = request.target_size,
            bits_per_pixel = request.bits_per_sample;
  const double crop_start = request.crop_start, crop_size = request.crop_size;
  const double center_pos_src = request.source_center, center_pos_dst = request.destination_center;
  if (source_size <= 0 || target_size <= 0 || !std::isfinite(crop_start) || !std::isfinite(crop_size) ||
      crop_size <= 0 || !std::isfinite(center_pos_src) || !std::isfinite(center_pos_dst) ||
      !((bits_per_pixel >= 8 && bits_per_pixel <= 16) || bits_per_pixel == 32))
    throw std::invalid_argument("Invalid coefficient request");
  // edge condition ideas from fmtconv, thanks.
  double src_step = crop_size / double(target_size);    // Distance between source pixels for adjacent dest pixels
  double zc_size = std::max(src_step, 1.0) / 1.0;       // Size of filter unit step (kernel_scale=1.0 in our case)
  double imp_step = 1.0 / zc_size;                      // Corresponding distance in the impulse
  double filter_support = function.support() * zc_size; // Number of source pixels covered by the FIR

  if (!std::isfinite(src_step) || src_step <= 0 || !std::isfinite(filter_support) || filter_support < 0 ||
      !std::isfinite(imp_step) || imp_step <= 0)
    throw std::invalid_argument("Invalid sampling scale/support");
  const double requested_size = std::ceil(filter_support * 2);
  if (!std::isfinite(requested_size) || requested_size > std::numeric_limits<int>::max())
    throw std::length_error("Sampling support is too large");
  int fir_filter_size = std::max(int(requested_size), 1);
  int max_kernel_size = 0;

  const int last_line = source_size - 1;

  // Initial position calculation

  double pos = crop_start;

  /*
  pre 3.7.4 logic:

  Now in 2025, let's fact-check this comment.

    pos = crop_start + ((crop_size - target_size) / (target_size*2)); // TODO this look wrong, gotta check
    ==>
    pos = crop_start + 1/2 * (crop_size / target_size - 1)
    ==>
    pos = crop_start + src_step * 0.5 - 1 * 0.5

    fmtconv generic formula:

    pos = crop_start + src_step * center_pos_dst - 1 * center_pos_src; // 3.7.4- fmtconv

    Solved: center_pos_dst = 0.5, center_pos_src = 0.5 in old Avisynth

  */

  // Introduces an offset because samples are located at the center of the
  // pixels, not on their boundaries. Excepted for pointresize.
  if (filter_support > 0) {
    // Pre 3.7.4 Avisynth worked with fixed center_pos_dst = center_pos_src = 0.5
    // Now it's externally configurable. In our use case they are always the same.
    pos += src_step * center_pos_dst - 1 * center_pos_src;
  } else {
    // In case of PointResize(), which now returns real 0 for function.support().
    // Avisynth heritage.
    filter_support = 0.0001;
  }

  const int current_FPScale = (bits_per_pixel > 8 && bits_per_pixel <= 16) ? (1 << 13) : (1 << 14);

  // Reject impossible initial coordinates before allocating coefficient storage.
  (void)CheckedTruncate(pos + filter_support);
  const size_t count =
      CheckedCount(target_size, fir_filter_size, bits_per_pixel == 32 ? sizeof(float) : sizeof(int16_t));
  Coefficients program{source_size, target_size, bits_per_pixel, fir_filter_size, fir_filter_size, {}, {}, {}, {}};
  program.offsets.resize(target_size);
  program.sizes.resize(target_size);
  if (bits_per_pixel == 32)
    program.floats.resize(count);
  else
    program.integers.resize(count);

  std::vector<double> coef_tmp;
  for (int i = 0; i < target_size; ++i) {
    coef_tmp.clear();

    const int64_t start = int64_t(CheckedTruncate(pos + filter_support)) - fir_filter_size + 1;
    if (start < std::numeric_limits<int>::min() || start + fir_filter_size - 1 > std::numeric_limits<int>::max())
      throw std::invalid_argument("Sampling coordinates are out of range");
    int start_pos = int(start);
    program.offsets[i] = std::clamp(start_pos, 0, last_line);

    // First pass: Accumulate all coefficients for weighting
    double total = 0.0;
    for (int k = 0; k < fir_filter_size; ++k) {
      const int p = start_pos + k;
      double val = function.f((pos - p) * imp_step);
      coef_tmp.push_back(val);
      if (!std::isfinite(val))
        throw std::invalid_argument("Nonfinite filter value");
      total += val;
    }

    if (!std::isfinite(total))
      throw std::invalid_argument("Nonfinite coefficient total");
    if (total == 0.0) {
      // Shouldn't happen for valid positions.
      total = 1.0;
    }

    const size_t coeff_arr_base_index = size_t(i) * fir_filter_size;

    // Second pass: Generate real coefficients, handling edge conditions
    double accu = 0.0;
    double prev_value = 0.0;

    int kernel_size = 0;

    if (bits_per_pixel == 32) {
      // Float version
      for (int k = 0; k < fir_filter_size; ++k) {
        const int p = start_pos + k;
        double val = coef_tmp[k];
        accu += val;
        if (p >= 0 && p <= last_line) {
          program.floats[coeff_arr_base_index + kernel_size] = CheckedFloat(accu / total);
          ++kernel_size;
          accu = 0;
        }
      }
    } else {
      // Integer version - using upscaled integer arithmetic (FPScale/FPScale16)
      for (int k = 0; k < fir_filter_size; ++k) {
        const int p = start_pos + k;
        double val = coef_tmp[k];
        accu += val;
        if (p >= 0 && p <= last_line) {
          double new_value = prev_value + accu / total;
          // differential approach ensures the filter coefficients sum to exactly FPScale)
          // The subtraction method guarantees that no matter how many terms we add, the
          // final sum will be exactly equal to the fixed-point representation of 1.0.
          program.integers[coeff_arr_base_index + kernel_size] =
              CheckedDifference(new_value, prev_value, current_FPScale);
          prev_value = new_value;
          ++kernel_size;
          accu = 0;
        }
      }
    }

    // We even haven't reached any valid line,
    // or gathered accu values from past last line.
    if (accu != 0) {
      if (kernel_size > 0) {
        // Assign the remaining accumulator to the last line, just like we put
        // the accumulator before the first valid line to the first line.
        if (bits_per_pixel == 32) {
          const float folded = program.floats[coeff_arr_base_index + kernel_size - 1] + CheckedFloat(accu / total);
          if (!std::isfinite(folded))
            throw std::invalid_argument("Nonfinite folded coefficient");
          program.floats[coeff_arr_base_index + kernel_size - 1] = folded;
        } else {
          double new_value = prev_value + accu / total;
          program.integers[coeff_arr_base_index + kernel_size - 1] =
              CheckedInt16(int64_t(program.integers[coeff_arr_base_index + kernel_size - 1]) +
                           CheckedDifference(new_value, prev_value, current_FPScale));
        }
        // no change in kernel_size
      } else {
        // new entry, accu/total must be 1.0 here (we always normalize)
        if (bits_per_pixel == 32)
          program.floats[coeff_arr_base_index + kernel_size] = CheckedFloat(accu / total);
        else
          program.integers[coeff_arr_base_index + kernel_size] =
              CheckedInt16(CheckedTruncate(accu / total * current_FPScale + 0.5));
        ++kernel_size;
      }
    }

    if (kernel_size == 0) {
      // write a single 1.0 coeff entry
      if (bits_per_pixel == 32)
        program.floats[coeff_arr_base_index + kernel_size] = 1.0f;
      else
        program.integers[coeff_arr_base_index + kernel_size] = int16_t(current_FPScale);
      ++kernel_size;
    }

    program.sizes[i] = kernel_size;
    if (kernel_size > max_kernel_size)
      max_kernel_size = kernel_size;

    pos += src_step;
  }

  // the different kernel sizes and coeff table will be later postprocessed
  // to have aligned and equally sized coefficients.

  program.filter_size_real = max_kernel_size;
  // can be less than original filter size if source dimensions are small

  if (bits_per_pixel != 32) {
    program.max_abs_sum = 0;
    for (int i = 0; i < target_size; ++i) {
      int64_t absolute = 0;
      for (int k = 0; k < program.sizes[i]; ++k)
        absolute += std::abs(int(program.integers[size_t(i) * program.filter_size + k]));
      program.max_abs_sum = std::max(program.max_abs_sum, absolute);
    }
  }
  return program;
}

} // namespace vc::resample
