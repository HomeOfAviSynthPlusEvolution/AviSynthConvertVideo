// Avisynth v2.5.  Copyright 2002-2009 Ben Rudiak-Gould et al.
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
namespace vc::matrix {
namespace {
template <class T, bool rgb_to_yuv, int outputs = 3>
void IntegerRows(const Config& config, const Coefficients& m, const std::array<vc_const_plane, 3>& source,
                 const std::array<vc_plane, 3>& destination, vc_rows rows) {
  const int limit = (1 << config.bits_per_sample) - 1;
  const int center = 1 << (config.bits_per_sample - 1);
  const int64_t scale = int64_t{1} << config.precision;
  const int64_t rounding = config.precision ? scale / 2 : 0;
  const int weights[3][3] = {{m.y_b, rgb_to_yuv ? m.y_g : m.u_b, rgb_to_yuv ? m.y_r : m.v_b},
                             {rgb_to_yuv ? m.u_b : m.y_g, m.u_g, rgb_to_yuv ? m.u_r : m.v_g},
                             {rgb_to_yuv ? m.v_b : m.y_r, rgb_to_yuv ? m.v_g : m.u_r, m.v_r}};
  const int offsets[3] = {rgb_to_yuv ? m.offset_y : m.offset_rgb, rgb_to_yuv ? center : m.offset_rgb,
                          rgb_to_yuv ? center : m.offset_rgb};
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const T* s[3] = {Row<T>(source[0], y), Row<T>(source[1], y), Row<T>(source[2], y)};
    T* d[outputs];
    for (int c = 0; c < outputs; ++c)
      d[c] = Row<T>(destination[c], y);
    for (int x = 0; x < rows.width; ++x) {
      const int values[3] = {int(s[0][x]) + (rgb_to_yuv ? m.offset_rgb : m.offset_y),
                             int(s[1][x]) + (rgb_to_yuv ? m.offset_rgb : -center),
                             int(s[2][x]) + (rgb_to_yuv ? m.offset_rgb : -center)};
      for (int c = 0; c < outputs; ++c) {
        const int64_t sum = int64_t(weights[c][0]) * values[0] + int64_t(weights[c][1]) * values[1] +
                            int64_t(weights[c][2]) * values[2] + rounding + int64_t(offsets[c]) * scale;
        // Clamp before division: matches arithmetic right shift plus clipping,
        // without relying on a signed shift of negative values in C++17.
        d[c][x] = static_cast<T>(std::clamp(sum, int64_t{0}, int64_t(limit) * scale) / scale);
      }
    }
  }
}
template <bool rgb_to_yuv, int outputs = 3>
void FloatRows(const Coefficients& m, const std::array<vc_const_plane, 3>& source,
               const std::array<vc_plane, 3>& destination, vc_rows rows) {
  const float weights[3][3] = {{m.y_b_f, rgb_to_yuv ? m.y_g_f : m.u_b_f, rgb_to_yuv ? m.y_r_f : m.v_b_f},
                               {rgb_to_yuv ? m.u_b_f : m.y_g_f, m.u_g_f, rgb_to_yuv ? m.u_r_f : m.v_g_f},
                               {rgb_to_yuv ? m.v_b_f : m.y_r_f, rgb_to_yuv ? m.v_g_f : m.u_r_f, m.v_r_f}};
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const float* s[3] = {Row<float>(source[0], y), Row<float>(source[1], y), Row<float>(source[2], y)};
    float* d[outputs];
    for (int c = 0; c < outputs; ++c)
      d[c] = Row<float>(destination[c], y);
    for (int x = 0; x < rows.width; ++x) {
      float values[3] = {s[0][x], s[1][x], s[2][x]};
      if constexpr (rgb_to_yuv) {
        if (m.offset_rgb_f != 0)
          for (float& value : values)
            value += m.offset_rgb_f;
      } else {
        values[0] += m.offset_y_f;
        values[1] -= 0.0f;
        values[2] -= 0.0f;
      }
      for (int c = 0; c < outputs; ++c) {
        const float sum = weights[c][0] * values[0] + weights[c][1] * values[1] + weights[c][2] * values[2];
        const float value = rgb_to_yuv ? (c == 0 ? m.offset_y_f : 0.0f) + sum : sum + m.offset_rgb_f;
        d[c][x] = outputs == 1 ? value
                               : std::clamp(value, rgb_to_yuv && c > 0 ? -.5f : 0.0f, rgb_to_yuv && c > 0 ? .5f : 1.0f);
      }
    }
  }
}
} // namespace
int Execute(const Config& config, const Coefficients& coefficients, const std::array<vc_const_plane, 3>& source,
            const std::array<vc_plane, 3>& destination, vc_rows rows, RowKernel kernel) {
  if (!ValidRows(rows) ||
      (config.bits_per_sample != 32 && (config.bits_per_sample < 8 || config.bits_per_sample > 16)) ||
      config.precision < 0 || config.precision > 20 ||
      (config.direction != Direction::RgbToYuv && config.direction != Direction::YuvToRgb &&
       config.direction != Direction::RgbToY))
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  const int bytes = config.bits_per_sample == 32 ? 4 : config.bits_per_sample == 8 ? 1 : 2;
  for (const auto& plane : source)
    if (!ValidPlane(plane.data, plane.stride, rows.width, rows.height, bytes, bytes))
      return VC_INVALID_ARGUMENT;
  const int outputs = config.direction == Direction::RgbToY ? 1 : 3;
  for (int c = 0; c < outputs; ++c)
    if (!ValidPlane(destination[c].data, destination[c].stride, rows.width, rows.height, bytes, bytes))
      return VC_INVALID_ARGUMENT;
  if (kernel) {
    kernel(config, coefficients, source, destination, rows);
    return VC_OK;
  }
  if (config.direction == Direction::RgbToY) {
    if (bytes == 4)
      FloatRows<true, 1>(coefficients, source, destination, rows);
    else if (bytes == 1)
      IntegerRows<uint8_t, true, 1>(config, coefficients, source, destination, rows);
    else
      IntegerRows<uint16_t, true, 1>(config, coefficients, source, destination, rows);
    return VC_OK;
  }
  const bool forward = config.direction != Direction::YuvToRgb;
  if (bytes == 4) {
    if (forward)
      FloatRows<true>(coefficients, source, destination, rows);
    else
      FloatRows<false>(coefficients, source, destination, rows);
  } else if (bytes == 1) {
    if (forward)
      IntegerRows<uint8_t, true>(config, coefficients, source, destination, rows);
    else
      IntegerRows<uint8_t, false>(config, coefficients, source, destination, rows);
  } else {
    if (forward)
      IntegerRows<uint16_t, true>(config, coefficients, source, destination, rows);
    else
      IntegerRows<uint16_t, false>(config, coefficients, source, destination, rows);
  }
  return VC_OK;
}
int ExecuteC(const Config& config, const Coefficients& coefficients, const std::array<vc_const_plane, 3>& source,
             const std::array<vc_plane, 3>& destination, vc_rows rows) {
  return Execute(config, coefficients, source, destination, rows, nullptr);
}
} // namespace vc::matrix
