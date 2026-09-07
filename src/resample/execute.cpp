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

#include "execute.h"
#include "layout/buffer.h"
#include <algorithm>
#include <cstdint>
#include <type_traits>

namespace vc::resample {
namespace {
template <class T, bool horizontal>
void Run(const Coefficients& plan, vc_const_plane source, vc_plane destination, vc_rows rows, int source_first,
         int destination_first) {
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    T* dst = Row<T>(destination, y - destination_first);
    for (int x = 0; x < rows.width; ++x) {
      const int position = horizontal ? x : y;
      const int offset = plan.offsets[position];
      const size_t base = size_t(position) * plan.filter_size;
      const int taps = plan.sizes[position];
      const T* src = Row<T>(source, (horizontal ? y : offset) - source_first) + (horizontal ? offset : x);
      const ptrdiff_t step = horizontal ? 1 : source.stride / ptrdiff_t(sizeof(T));
      if constexpr (std::is_same_v<T, float>) {
        float sum = 0;
        for (int k = 0; k < taps; ++k)
          sum += src[ptrdiff_t(k) * step] * plan.floats[base + k];
        dst[x] = sum;
      } else {
        constexpr int shift = sizeof(T) == 1 ? 14 : 13;
        // Retain the original signed-domain treatment of full 16-bit samples.
        // A wider accumulator defines behavior for extreme negative-lobe filters
        // without relying on signed 32-bit overflow.
        const int bias = plan.bits_per_sample == 16 ? 32768 : 0;
        int64_t sum = int64_t(1) << (shift - 1);
        for (int k = 0; k < taps; ++k)
          sum += int64_t(int(src[ptrdiff_t(k) * step]) - bias) * plan.integers[base + k];
        sum += int64_t(bias) * (int64_t(1) << shift);
        // Clamping before division avoids implementation-defined negative shifts.
        const int64_t limit = (int64_t(1) << plan.bits_per_sample) - 1;
        dst[x] = T(std::clamp(sum, int64_t(0), limit << shift) / (int64_t(1) << shift));
      }
    }
  }
}
template <class T>
void Dispatch(const Coefficients& plan, Axis axis, vc_const_plane source, vc_plane destination, vc_rows rows,
              int source_first, int destination_first) {
  if (axis == Axis::Horizontal)
    Run<T, true>(plan, source, destination, rows, source_first, destination_first);
  else
    Run<T, false>(plan, source, destination, rows, source_first, destination_first);
}
} // namespace
int ValidateExecution(const Coefficients& plan, Axis axis, vc_const_plane source, vc_plane destination, vc_rows rows,
                      int source_first, int source_count, int destination_first, int destination_count) {
  if (!ValidRows(rows) || (axis != Axis::Horizontal && axis != Axis::Vertical) ||
      (axis == Axis::Horizontal ? rows.width : rows.height) != plan.target_size)
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  const int bytes = plan.bits_per_sample == 32 ? 4 : plan.bits_per_sample == 8 ? 1 : 2;
  const int source_width = axis == Axis::Horizontal ? plan.source_size : rows.width;
  const int source_height = axis == Axis::Vertical ? plan.source_size : rows.height;
  if (source_count == -1)
    source_count = source_height;
  if (destination_count == -1)
    destination_count = rows.height;
  if (source_first < 0 || source_count < 0 || source_first > source_height ||
      source_count > source_height - source_first || destination_first < 0 || destination_count < 0 ||
      destination_first > rows.height || destination_count > rows.height - destination_first ||
      rows.first_row < destination_first || rows.first_row + rows.row_count > destination_first + destination_count)
    return VC_INVALID_ARGUMENT;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const int first = axis == Axis::Horizontal ? y : plan.offsets[y];
    const int count = axis == Axis::Horizontal ? 1 : plan.sizes[y];
    if (first < source_first || first + count > source_first + source_count)
      return VC_INVALID_ARGUMENT;
  }
  if (!ValidPlane(source.data, source.stride, source_width, source_count, bytes, bytes) ||
      !ValidPlane(destination.data, destination.stride, rows.width, destination_count, bytes, bytes))
    return VC_INVALID_ARGUMENT;
  return VC_OK;
}
int ExecuteC(const Coefficients& plan, Axis axis, vc_const_plane source, vc_plane destination, vc_rows rows,
             int source_first, int source_count, int destination_first, int destination_count) {
  const int status = ValidateExecution(plan, axis, source, destination, rows, source_first, source_count,
                                       destination_first, destination_count);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  const int bytes = plan.bits_per_sample == 32 ? 4 : plan.bits_per_sample == 8 ? 1 : 2;
  if (bytes == 1)
    Dispatch<uint8_t>(plan, axis, source, destination, rows, source_first, destination_first);
  else if (bytes == 2)
    Dispatch<uint16_t>(plan, axis, source, destination, rows, source_first, destination_first);
  else
    Dispatch<float>(plan, axis, source, destination, rows, source_first, destination_first);
  return VC_OK;
}
} // namespace vc::resample
