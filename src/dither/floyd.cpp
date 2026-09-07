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

#include "video_convert/dither.h"
#include "depth/transform.h"
#include "layout/buffer.h"
#include <algorithm>
#include <new>
#include <vector>
struct vc_floyd_context {
  vc_floyd_config config;
  vc::depth::Transform range;
  int width, height, next_row = 0, next_error = 0;
  std::vector<int> errors;
};
namespace {
int FloorDivide(int value, int divisor) {
  return value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor);
}
template <class S, class D, bool low>
void Execute(vc_floyd_context& p, vc_const_plane source, vc_plane destination, vc_rows rows) {
  const auto& c = p.config.depth;
  const int divisor = 1 << (c.source_bits - p.config.quantization_bits), rounder = divisor / 2;
  const int maximum = (1 << c.destination_bits) - 1, source_max = (1 << c.source_bits) - 1;
  const int quantized_max = (1 << p.config.quantization_bits) - 1;
  const int upscale = 1 << (c.destination_bits - p.config.quantization_bits);
  const float backscale = float(maximum) / float(quantized_max);
  int* errors = p.errors.data() + 1;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = vc::Row<S>(source, y);
    auto* dst = vc::Row<D>(destination, y);
    const int direction = (y & 1) ? -1 : 1, begin = direction == 1 ? 0 : rows.width - 1,
              end = direction == 1 ? rows.width : -1;
    for (int x = begin; x != end; x += direction) {
      int value = src[x];
      if (c.source_full != c.destination_full) {
        const float scaled = (float(value) - p.range.source_offset) * p.range.factor;
        value = std::clamp(int(scaled + (p.range.destination_offset + .5f)), 0, source_max);
      }
      int error = p.next_error;
      if constexpr (low)
        error -= rounder;
      const int sum = value + error;
      int quantized = FloorDivide(sum + rounder, divisor);
      // Multiplication also defines negative values, unlike signed left shift.
      error = sum - quantized * divisor;
      if constexpr (low)
        quantized = int(float(std::min(quantized, quantized_max)) * backscale + .5f);
      else
        quantized *= upscale;
      dst[x] = static_cast<D>(std::clamp(quantized, 0, maximum));
      // Preserve the inherited optimized serpentine coefficients (0,4,5,7)/16
      // and their individually rounded integer residual, including row edges.
      const int e3 = FloorDivide(error * 4 + 8, 16), e5 = FloorDivide(error * 5 + 8, 16), e7 = error - e3 - e5;
      p.next_error = errors[x + direction] + e7;
      errors[x - direction] += e3;
      errors[x] += e5;
      errors[x + direction] = 0;
    }
  }
  p.next_row += rows.row_count;
}
template <class S, class D>
void ChooseLow(vc_floyd_context& c, vc_const_plane s, vc_plane d, vc_rows r) {
  if (c.config.quantization_bits < 8)
    Execute<S, D, true>(c, s, d, r);
  else
    Execute<S, D, false>(c, s, d, r);
}
bool Flag(int x) {
  return x == 0 || x == 1;
}
} // namespace
int vc_floyd_create(const vc_floyd_config* config, int width, int height, vc_floyd_context** output) {
  if (!output)
    return VC_INVALID_ARGUMENT;
  *output = nullptr;
  if (!config || width <= 0 || height <= 0)
    return VC_INVALID_ARGUMENT;
  const auto& c = config->depth;
  const int q = config->quantization_bits;
  if (c.source_bits < 8 || c.source_bits > 16 || c.destination_bits < 8 || c.destination_bits > c.source_bits ||
      q < 1 || q > c.destination_bits || q >= c.source_bits || !Flag(c.source_full) || !Flag(c.destination_full) ||
      !Flag(c.chroma))
    return VC_INVALID_ARGUMENT;
  try {
    auto rc = c;
    rc.destination_bits = c.source_bits;
    *output = new vc_floyd_context{*config, vc::depth::BuildTransform(rc),         width, height, 0,
                                   0,       std::vector<int>(size_t(width) + 2, 0)};
    return VC_OK;
  } catch (const std::bad_alloc&) {
    return VC_OUT_OF_MEMORY;
  } catch (...) {
    return VC_INTERNAL_ERROR;
  }
}
void vc_floyd_destroy(vc_floyd_context* context) {
  delete context;
}
void vc_floyd_reset(vc_floyd_context* context) {
  if (context) {
    context->next_row = 0;
    context->next_error = 0;
    std::fill(context->errors.begin(), context->errors.end(), 0);
  }
}
int vc_floyd_execute(vc_floyd_context* context, vc_const_plane source, vc_plane destination, vc_rows rows) {
  if (!context || !vc::ValidRows(rows) || rows.width != context->width || rows.height != context->height ||
      rows.first_row != context->next_row)
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  const int sb = context->config.depth.source_bits == 8 ? 1 : 2,
            db = context->config.depth.destination_bits == 8 ? 1 : 2;
  if (!vc::ValidPlane(source.data, source.stride, rows.width, rows.height, sb, sb) ||
      !vc::ValidPlane(destination.data, destination.stride, rows.width, rows.height, db, db))
    return VC_INVALID_ARGUMENT;
  if (sb == 1)
    ChooseLow<uint8_t, uint8_t>(*context, source, destination, rows);
  else if (db == 1)
    ChooseLow<uint16_t, uint8_t>(*context, source, destination, rows);
  else
    ChooseLow<uint16_t, uint16_t>(*context, source, destination, rows);
  return VC_OK;
}
