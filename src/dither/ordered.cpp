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

#include "ordered.h"
#include "depth/transform.h"
#include "layout/buffer.h"
#include <algorithm>
#include <array>
#include <new>
namespace {
int Bayer(int x, int y, int order) {
  int value = 0;
  // Preserve the existing 16x16 orientation. Generate the odd-difference
  // matrix by halving this exact permutation, avoiding hand-copied typos.
  if (order == 4)
    std::swap(x, y);
  for (int bit = 0; bit < order; ++bit) {
    const int xb = (x >> bit) & 1, yb = (y >> bit) & 1;
    value = 4 * value + 2 * (xb ^ yb) + yb;
  }
  return value;
}
template <class S, class D>
void Convert(const vc_ordered_plan& p, vc_const_plane source, vc_plane destination, vc_rows rows) {
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = vc::Row<S>(source, y);
    auto* dst = vc::Row<D>(destination, y);
    for (int x = 0; x < rows.width; ++x) {
      dst[x] = static_cast<D>(vc::dither::Quantize(p, src[x], x, y));
    }
  }
}
bool Flag(int x) {
  return x == 0 || x == 1;
}
} // namespace
int64_t vc_ordered_supported_targets(void) {
  return vc::dither::SupportedTargets();
}
int vc_ordered_create(const vc_ordered_config* config, vc_ordered_plan** output) {
  return vc_ordered_create_for_target(config, VC_TARGET_C, output);
}
int vc_ordered_create_for_target(const vc_ordered_config* config, int64_t target, vc_ordered_plan** output) {
  if (!output)
    return VC_INVALID_ARGUMENT;
  *output = nullptr;
  if (!config)
    return VC_INVALID_ARGUMENT;
  const auto& c = config->depth;
  const int q = config->quantization_bits;
  if (c.source_bits < 8 || c.source_bits > 16 || c.destination_bits < 8 || c.destination_bits > c.source_bits ||
      q < 1 || q > c.destination_bits || q >= c.source_bits || c.source_bits - q > 8 || !Flag(c.source_full) ||
      !Flag(c.destination_full) || !Flag(c.chroma))
    return VC_INVALID_ARGUMENT;
  try {
    const auto kernel = target == VC_TARGET_C ? nullptr : vc::dither::GetKernel(target, *config);
    if (target != VC_TARGET_C && target != VC_TARGET_NATIVE && !kernel)
      return VC_INVALID_ARGUMENT;
    auto range_config = c;
    range_config.destination_bits = c.source_bits;
    const int shift = c.source_bits - q, order = (shift + 1) / 2;
    auto* plan = new vc_ordered_plan{*config,      vc::depth::BuildTransform(range_config),
                                     {},           1 << order,
                                     shift,        (1 << c.destination_bits) - 1,
                                     (1 << q) - 1, float((1 << c.destination_bits) - 1) / float((1 << q) - 1)};
    for (int y = 0; y < 16; ++y)
      for (int x = 0; x < 32; ++x)
        plan->thresholds[y * 32 + x] = Bayer(x % plan->period, y % plan->period, order) >> (shift & 1);
    for (int y = 0; y < 16; ++y)
      for (int x = 0; x < 64; ++x)
        plan->thresholds16[y * 64 + x] = uint16_t(plan->thresholds[y * 32 + (x & 15)]);
    plan->kernel = kernel;
    *output = plan;
    return VC_OK;
  } catch (const std::bad_alloc&) {
    return VC_OUT_OF_MEMORY;
  } catch (...) {
    return VC_INTERNAL_ERROR;
  }
}
void vc_ordered_destroy(vc_ordered_plan* plan) {
  delete plan;
}
int vc_ordered_execute(const vc_ordered_plan* plan, vc_const_plane source, vc_plane destination, vc_rows rows) {
  if (!plan || !vc::ValidRows(rows))
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  const int sb = plan->config.depth.source_bits == 8 ? 1 : 2, db = plan->config.depth.destination_bits == 8 ? 1 : 2;
  if (!vc::ValidPlane(source.data, source.stride, rows.width, rows.height, sb, sb) ||
      !vc::ValidPlane(destination.data, destination.stride, rows.width, rows.height, db, db))
    return VC_INVALID_ARGUMENT;
  if (plan->kernel)
    plan->kernel(*plan, source, destination, rows);
  else if (sb == 1)
    Convert<uint8_t, uint8_t>(*plan, source, destination, rows);
  else if (db == 1)
    Convert<uint16_t, uint8_t>(*plan, source, destination, rows);
  else
    Convert<uint16_t, uint16_t>(*plan, source, destination, rows);
  return VC_OK;
}
