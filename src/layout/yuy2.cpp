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

// ConvertPlanar (c) 2005 by Klaus Post

#include "validation.h"

namespace {
template <bool Neutralize>
int Luma(vc_const_plane source, vc_plane destination, vc_rows rows) {
  const int status = vc::CheckYuy2Luma(source, destination, rows, Neutralize);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = vc::Row<uint8_t>(source, y);
    auto* dst = vc::Row<uint8_t>(destination, y);
    for (ptrdiff_t x = 0; x < rows.width; ++x) {
      if constexpr (Neutralize)
        dst[x * 2 + 1] = 128;
      else
        dst[x] = src[x * 2];
    }
  }
  return VC_OK;
}
} // namespace

extern "C" int vc_extract_yuy2_luma(vc_const_plane source, vc_plane destination, vc_rows rows) {
  return Luma<false>(source, destination, rows);
}
extern "C" int vc_neutralize_yuy2_chroma(vc_plane image, vc_rows rows) {
  return Luma<true>({image.data, image.stride}, image, rows);
}

extern "C" int vc_unpack_yuy2(vc_const_plane source, vc_yuv_planes destination, vc_rows rows) {
  const int status = vc::CheckUnpackYuy2(source, destination, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  // Adapted from convert_yuy2_to_yv16_c; preserve the original byte mapping.
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = vc::Row<uint8_t>(source, y);
    auto* dst_y = vc::Row<uint8_t>(destination.y, y);
    auto* dst_u = vc::Row<uint8_t>(destination.u, y);
    auto* dst_v = vc::Row<uint8_t>(destination.v, y);
    for (ptrdiff_t x = 0; x < rows.width / 2; ++x) {
      dst_y[x * 2] = src[x * 4];
      dst_y[x * 2 + 1] = src[x * 4 + 2];
      dst_u[x] = src[x * 4 + 1];
      dst_v[x] = src[x * 4 + 3];
    }
  }
  return VC_OK;
}

extern "C" int vc_pack_yuy2(vc_const_yuv_planes source, vc_plane destination, vc_rows rows) {
  const int status = vc::CheckPackYuy2(source, destination, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  // Adapted from convert_yv16_to_yuy2_c.
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src_y = vc::Row<uint8_t>(source.y, y);
    const auto* src_u = vc::Row<uint8_t>(source.u, y);
    const auto* src_v = vc::Row<uint8_t>(source.v, y);
    auto* dst = vc::Row<uint8_t>(destination, y);
    for (ptrdiff_t x = 0; x < rows.width / 2; ++x) {
      dst[x * 4] = src_y[x * 2];
      dst[x * 4 + 1] = src_u[x];
      dst[x * 4 + 2] = src_y[x * 2 + 1];
      dst[x * 4 + 3] = src_v[x];
    }
  }
  return VC_OK;
}
