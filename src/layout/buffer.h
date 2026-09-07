// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_LAYOUT_BUFFER_H
#define VIDEO_CONVERT_LAYOUT_BUFFER_H

#include "video_convert/layout.h"
#include <cstddef>
#include <cstdint>
#include <limits>

namespace vc {
inline bool ValidRows(vc_rows rows) {
  return rows.width > 0 && rows.height > 0 && rows.first_row >= 0 && rows.first_row <= rows.height &&
         rows.row_count >= 0 && rows.row_count <= rows.height - rows.first_row;
}

inline bool ValidPlane(const void* data, ptrdiff_t stride, int width, int height, int bytes_per_pixel, int alignment) {
  constexpr auto limit = std::numeric_limits<ptrdiff_t>::max();
  if (!data || reinterpret_cast<uintptr_t>(data) % alignment != 0 || stride % alignment != 0 ||
      stride == std::numeric_limits<ptrdiff_t>::min() || width > limit / bytes_per_pixel)
    return false;
  const ptrdiff_t row_bytes = ptrdiff_t(width) * bytes_per_pixel;
  const ptrdiff_t pitch = stride < 0 ? -stride : stride;
  return pitch >= row_bytes && (height == 1 || pitch <= (limit - row_bytes) / (height - 1));
}

template <class T>
const T* Row(vc_const_plane plane, int y) {
  return reinterpret_cast<const T*>(static_cast<const uint8_t*>(plane.data) + ptrdiff_t(y) * plane.stride);
}

template <class T>
T* Row(vc_plane plane, int y) {
  return reinterpret_cast<T*>(static_cast<uint8_t*>(plane.data) + ptrdiff_t(y) * plane.stride);
}
} // namespace vc
#endif
