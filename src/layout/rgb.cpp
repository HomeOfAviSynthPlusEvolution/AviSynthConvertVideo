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

#include "validation.h"

namespace vc {
namespace {
// Adapted from convert_rgb_to_rgbp_c / convert_rgbp_to_rgb_c. AVS row inversion
// and G/B/R pointer-array indexing are now explicit descriptors and named planes.
template <class T, int Components>
void Unpack(vc_const_plane source, vc_rgb_planes destination, T fill, vc_rows rows) {
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const T* src = Row<T>(source, y);
    T* r = Row<T>(destination.r, y);
    T* g = Row<T>(destination.g, y);
    T* b = Row<T>(destination.b, y);
    T* a = destination.a.data ? Row<T>(destination.a, y) : nullptr;
    for (ptrdiff_t x = 0; x < rows.width; ++x) {
      b[x] = src[x * Components];
      g[x] = src[x * Components + 1];
      r[x] = src[x * Components + 2];
      if (a)
        a[x] = Components == 4 ? src[x * Components + 3] : fill;
    }
  }
}

template <class T, int Components>
void Pack(vc_const_rgb_planes source, vc_plane destination, T fill, vc_rows rows) {
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const T* r = Row<T>(source.r, y);
    const T* g = Row<T>(source.g, y);
    const T* b = Row<T>(source.b, y);
    const T* a = Components == 4 && source.a.data ? Row<T>(source.a, y) : nullptr;
    T* dst = Row<T>(destination, y);
    for (ptrdiff_t x = 0; x < rows.width; ++x) {
      dst[x * Components] = b[x];
      dst[x * Components + 1] = g[x];
      dst[x * Components + 2] = r[x];
      if constexpr (Components == 4)
        dst[x * Components + 3] = a ? a[x] : fill;
    }
  }
}
} // namespace
} // namespace vc

extern "C" int vc_unpack_bgr(vc_const_plane source, vc_rgb_planes destination, int storage, int components,
                             uint32_t alpha_fill, vc_rows rows) {
  const int status = vc::CheckUnpackBgr(source, destination, storage, components, alpha_fill, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  if (storage == VC_U8) {
    if (components == 3)
      vc::Unpack<uint8_t, 3>(source, destination, uint8_t(alpha_fill), rows);
    else
      vc::Unpack<uint8_t, 4>(source, destination, uint8_t(alpha_fill), rows);
  } else {
    if (components == 3)
      vc::Unpack<uint16_t, 3>(source, destination, uint16_t(alpha_fill), rows);
    else
      vc::Unpack<uint16_t, 4>(source, destination, uint16_t(alpha_fill), rows);
  }
  return VC_OK;
}

extern "C" int vc_pack_bgr(vc_const_rgb_planes source, vc_plane destination, int storage, int components,
                           uint32_t alpha_fill, vc_rows rows) {
  const int status = vc::CheckPackBgr(source, destination, storage, components, alpha_fill, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  if (storage == VC_U8) {
    if (components == 3)
      vc::Pack<uint8_t, 3>(source, destination, uint8_t(alpha_fill), rows);
    else
      vc::Pack<uint8_t, 4>(source, destination, uint8_t(alpha_fill), rows);
  } else {
    if (components == 3)
      vc::Pack<uint16_t, 3>(source, destination, uint16_t(alpha_fill), rows);
    else
      vc::Pack<uint16_t, 4>(source, destination, uint16_t(alpha_fill), rows);
  }
  return VC_OK;
}
