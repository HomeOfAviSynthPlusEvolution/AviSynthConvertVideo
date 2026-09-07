// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_LAYOUT_VALIDATION_H
#define VIDEO_CONVERT_LAYOUT_VALIDATION_H
#include "buffer.h"
#include <initializer_list>
namespace vc {
inline int CheckUnpackBgr(vc_const_plane source, vc_rgb_planes destination, int storage, int components,
                          uint32_t alpha_fill, vc_rows rows) {
  if (!vc::ValidRows(rows) || (storage != VC_U8 && storage != VC_U16) || (components != 3 && components != 4))
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  const int bytes = storage == VC_U8 ? 1 : 2;
  if (destination.a.data && components == 3 && alpha_fill > (bytes == 1 ? 255u : 65535u))
    return VC_INVALID_ARGUMENT;
  if (!vc::ValidPlane(source.data, source.stride, rows.width, rows.height, bytes * components, bytes))
    return VC_INVALID_ARGUMENT;
  for (auto plane : {destination.r, destination.g, destination.b})
    if (!vc::ValidPlane(plane.data, plane.stride, rows.width, rows.height, bytes, bytes))
      return VC_INVALID_ARGUMENT;
  if (destination.a.data &&
      !vc::ValidPlane(destination.a.data, destination.a.stride, rows.width, rows.height, bytes, bytes))
    return VC_INVALID_ARGUMENT;
  return VC_OK;
}

inline int CheckPackBgr(vc_const_rgb_planes source, vc_plane destination, int storage, int components,
                        uint32_t alpha_fill, vc_rows rows) {
  if (!vc::ValidRows(rows) || (storage != VC_U8 && storage != VC_U16) || (components != 3 && components != 4))
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  const int bytes = storage == VC_U8 ? 1 : 2;
  if (!source.a.data && components == 4 && alpha_fill > (bytes == 1 ? 255u : 65535u))
    return VC_INVALID_ARGUMENT;
  if (!vc::ValidPlane(destination.data, destination.stride, rows.width, rows.height, bytes * components, bytes))
    return VC_INVALID_ARGUMENT;
  for (auto plane : {source.r, source.g, source.b})
    if (!vc::ValidPlane(plane.data, plane.stride, rows.width, rows.height, bytes, bytes))
      return VC_INVALID_ARGUMENT;
  if (components == 4 && source.a.data &&
      !vc::ValidPlane(source.a.data, source.a.stride, rows.width, rows.height, bytes, bytes))
    return VC_INVALID_ARGUMENT;
  return VC_OK;
}

inline int CheckRepackBgr(vc_const_plane source, vc_plane destination, int storage, int sc, int dc, uint32_t fill,
                          vc_rows rows) {
  if (!ValidRows(rows) || (storage != VC_U8 && storage != VC_U16) || (sc != 3 && sc != 4) || (dc != 3 && dc != 4))
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  const int bytes = storage == VC_U8 ? 1 : 2;
  if (sc == 3 && dc == 4 && fill > (bytes == 1 ? 255u : 65535u))
    return VC_INVALID_ARGUMENT;
  if (!ValidPlane(source.data, source.stride, rows.width, rows.height, sc * bytes, bytes) ||
      !ValidPlane(destination.data, destination.stride, rows.width, rows.height, dc * bytes, bytes))
    return VC_INVALID_ARGUMENT;
  return VC_OK;
}

inline int CheckUnpackYuy2(vc_const_plane source, vc_yuv_planes destination, vc_rows rows) {
  if (!vc::ValidRows(rows) || rows.width % 2 != 0)
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  if (!vc::ValidPlane(source.data, source.stride, rows.width, rows.height, 2, 1) ||
      !vc::ValidPlane(destination.y.data, destination.y.stride, rows.width, rows.height, 1, 1) ||
      !vc::ValidPlane(destination.u.data, destination.u.stride, rows.width / 2, rows.height, 1, 1) ||
      !vc::ValidPlane(destination.v.data, destination.v.stride, rows.width / 2, rows.height, 1, 1))
    return VC_INVALID_ARGUMENT;
  return VC_OK;
}

inline int CheckYuy2Luma(vc_const_plane source, vc_plane destination, vc_rows rows, bool in_place) {
  if (!ValidRows(rows) || rows.width % 2 != 0)
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  if (!ValidPlane(source.data, source.stride, rows.width, rows.height, 2, 1) ||
      !ValidPlane(destination.data, destination.stride, rows.width, rows.height, in_place ? 2 : 1, 1))
    return VC_INVALID_ARGUMENT;
  return VC_OK;
}

inline int CheckPackYuy2(vc_const_yuv_planes source, vc_plane destination, vc_rows rows) {
  if (!vc::ValidRows(rows) || rows.width % 2 != 0)
    return VC_INVALID_ARGUMENT;
  if (rows.row_count == 0)
    return VC_OK;
  if (!vc::ValidPlane(destination.data, destination.stride, rows.width, rows.height, 2, 1) ||
      !vc::ValidPlane(source.y.data, source.y.stride, rows.width, rows.height, 1, 1) ||
      !vc::ValidPlane(source.u.data, source.u.stride, rows.width / 2, rows.height, 1, 1) ||
      !vc::ValidPlane(source.v.data, source.v.stride, rows.width / 2, rows.height, 1, 1))
    return VC_INVALID_ARGUMENT;
  return VC_OK;
}

} // namespace vc
#endif
