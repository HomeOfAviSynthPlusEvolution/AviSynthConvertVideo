// Packed channel-count conversion developed in AviSynthConvertVideo.
// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "validation.h"
namespace vc {
namespace {
template <class T, int SC, int DC>
void Repack(vc_const_plane source, vc_plane destination, T fill, vc_rows rows) {
  for (int y = rows.first_row; y < rows.first_row + rows.row_count; ++y) {
    const auto* src = Row<T>(source, y);
    auto* dst = Row<T>(destination, y);
    for (ptrdiff_t x = 0; x < rows.width; ++x) {
      for (int c = 0; c < 3; ++c)
        dst[x * DC + c] = src[x * SC + c];
      if constexpr (DC == 4)
        dst[x * DC + 3] = SC == 4 ? src[x * SC + 3] : fill;
    }
  }
}
template <class T>
void SelectRepack(vc_const_plane source, vc_plane destination, int sc, int dc, T fill, vc_rows rows) {
  if (sc == 3) {
    if (dc == 3)
      Repack<T, 3, 3>(source, destination, fill, rows);
    else
      Repack<T, 3, 4>(source, destination, fill, rows);
  } else {
    if (dc == 3)
      Repack<T, 4, 3>(source, destination, fill, rows);
    else
      Repack<T, 4, 4>(source, destination, fill, rows);
  }
}
} // namespace
} // namespace vc
int vc_repack_bgr(vc_const_plane source, vc_plane destination, int storage, int sc, int dc, uint32_t fill,
                  vc_rows rows) {
  const int status = vc::CheckRepackBgr(source, destination, storage, sc, dc, fill, rows);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  if (storage == VC_U8)
    vc::SelectRepack(source, destination, sc, dc, uint8_t(fill), rows);
  else
    vc::SelectRepack(source, destination, sc, dc, uint16_t(fill), rows);
  return VC_OK;
}
