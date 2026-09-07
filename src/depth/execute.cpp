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

#include "transform.h"
#include "layout/buffer.h"
#include <algorithm>
#include <cstring>
#include <type_traits>
namespace vc::depth {
namespace {
struct Range {
  float offset, span;
};
Range GetRange(int bits, bool full, bool chroma) {
  if (chroma) {
    if (bits == 32)
      return {0, full ? .5f : 112.f / 255};
    return {float(1 << (bits - 1)), full ? float((1 << bits) - 1) / 2 : float(112 << (bits - 8))};
  }
  if (bits == 32)
    return {full ? 0.f : 16.f / 255, full ? 1.f : 219.f / 255};
  return {full ? 0.f : float(16 << (bits - 8)), full ? float((1 << bits) - 1) : float(219 << (bits - 8))};
}
template <class S, class D>
void Convert(const Transform& t, vc_const_plane source, vc_plane destination, vc_rows rows) {
  const auto& c = t.config;
  const int maximum = std::is_same_v<D, float> ? 0 : (1 << c.destination_bits) - 1;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<S>(source, y);
    auto* dst = Row<D>(destination, y);
    for (int x = 0; x < rows.width; ++x) {
      if constexpr (!std::is_same_v<S, float> && !std::is_same_v<D, float>) {
        if (!c.source_full && !c.destination_full) {
          const int shift = c.destination_bits - c.source_bits;
          dst[x] = static_cast<D>(shift > 0 ? int(src[x]) << shift
                                            : std::min((int(src[x]) + (1 << (-shift - 1))) >> -shift, maximum));
          continue;
        }
        if (c.source_bits == 8 && c.destination_bits == 16 && !c.chroma && c.source_full && c.destination_full) {
          dst[x] = static_cast<D>(int(src[x]) * 257);
          continue;
        }
      }
      const float centered = float(src[x]) - t.source_offset;
      const float scaled = centered * t.factor;
      if constexpr (std::is_same_v<D, float>)
        dst[x] = scaled + t.destination_offset;
      else {
        const float value = scaled + (t.destination_offset + .5f);
        // Compare before conversion: no floating-to-integer overflow, including
        // huge finite values. An unordered value selects the lower endpoint.
        dst[x] = !(value > 0) ? D(0) : value >= float(maximum) ? static_cast<D>(maximum) : static_cast<D>(value);
      }
    }
  }
}
template <class S>
void Destination(const Transform& t, vc_const_plane s, vc_plane d, vc_rows rows) {
  if (t.config.destination_bits == 32)
    Convert<S, float>(t, s, d, rows);
  else if (t.config.destination_bits == 8)
    Convert<S, uint8_t>(t, s, d, rows);
  else
    Convert<S, uint16_t>(t, s, d, rows);
}
} // namespace
Transform BuildTransform(vc_depth_config config) {
  const auto s = GetRange(config.source_bits, config.source_full != 0, config.chroma != 0);
  const auto d = GetRange(config.destination_bits, config.destination_full != 0, config.chroma != 0);
  return {config, s.offset, d.span / s.span, d.offset};
}
int Execute(const Transform& t, vc_const_plane source, vc_plane destination, vc_rows rows, RowKernel kernel) {
  if (!ValidRows(rows))
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  const int sb = t.config.source_bits == 32 ? 4 : t.config.source_bits == 8 ? 1 : 2;
  const int db = t.config.destination_bits == 32 ? 4 : t.config.destination_bits == 8 ? 1 : 2;
  if (!ValidPlane(source.data, source.stride, rows.width, rows.height, sb, sb) ||
      !ValidPlane(destination.data, destination.stride, rows.width, rows.height, db, db))
    return VC_INVALID_ARGUMENT;
  if (t.config.source_bits == t.config.destination_bits && t.config.source_full == t.config.destination_full) {
    for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y)
      std::memcpy(Row<uint8_t>(destination, y), Row<uint8_t>(source, y), size_t(rows.width) * sb);
  } else if (kernel)
    kernel(t, source, destination, rows);
  else if (sb == 4)
    Destination<float>(t, source, destination, rows);
  else if (sb == 1)
    Destination<uint8_t>(t, source, destination, rows);
  else
    Destination<uint16_t>(t, source, destination, rows);
  return VC_OK;
}
int ExecuteC(const Transform& t, vc_const_plane s, vc_plane d, vc_rows rows) {
  return Execute(t, s, d, rows, nullptr);
}
} // namespace vc::depth
