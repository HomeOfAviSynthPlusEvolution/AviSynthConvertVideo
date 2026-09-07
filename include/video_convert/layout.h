// Licensed under GPL version 2 or later with the inherited AviSynth
// linking exception. See the notices in src/layout/ and docs/SOURCE-PROVENANCE.md.
#ifndef VIDEO_CONVERT_LAYOUT_H
#define VIDEO_CONVERT_LAYOUT_H

#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct vc_const_rgb_planes {
  vc_const_plane r, g, b, a;
} vc_const_rgb_planes;

typedef struct vc_rgb_planes {
  vc_plane r, g, b, a;
} vc_rgb_planes;

typedef struct vc_const_yuv_planes {
  vc_const_plane y, u, v;
} vc_const_yuv_planes;

typedef struct vc_yuv_planes {
  vc_plane y, u, v;
} vc_yuv_planes;

// Full image geometry and a destination row range [first_row, first_row + row_count).
// width/height must be positive; first_row/row_count must be nonnegative and
// contained in height. A valid empty range succeeds without accessing buffers.
typedef struct vc_rows {
  int width, height, first_row, row_count;
} vc_rows;

// All pointers refer to logical image row zero, including for partial execution.
// Strides are signed BYTES, with magnitude at least the active row byte count.
// Data and strides must be naturally aligned for storage. Buffers must cover the
// full described image and must not overlap. Allocation size and overlap are
// caller responsibilities; arithmetic, geometry, null pointers and alignment
// are validated before writing. No padding is accessed. Descriptors are unchanged.
// These ordinary C kernels allocate nothing, throw nothing and have no shared state.
// U16 is native-endian; samples are copied without depth/range conversion.

// components is 3 (BGR) or 4 (BGRA). A null destination alpha discards alpha;
// otherwise BGRA alpha is copied, or BGR uses alpha_fill (in the storage domain).
int vc_unpack_bgr(vc_const_plane source, vc_rgb_planes destination, int storage, int components, uint32_t alpha_fill,
                  vc_rows rows);

// A 3-component destination drops alpha. For 4 components, a null source alpha
// requests alpha_fill; otherwise source alpha is copied. Unused alpha descriptors
// and unused alpha_fill are ignored. No implicit full-range/opaque default.
int vc_pack_bgr(vc_const_rgb_planes source, vc_plane destination, int storage, int components, uint32_t alpha_fill,
                vc_rows rows);

// Direct packed BGR/BGRA copy, add-alpha or drop-alpha, without planar scratch.
// Source/destination components are each 3 or 4. Fill is used only for 3->4.
int vc_repack_bgr(vc_const_plane source, vc_plane destination, int storage, int source_components,
                  int destination_components, uint32_t alpha_fill, vc_rows rows);

// U8 YUY2 is Y0 U0 Y1 V0. Width is the even, full luma pixel width; U/V width is
// width/2. U and V may have different strides. No resampling or range conversion.
int vc_unpack_yuy2(vc_const_plane source, vc_yuv_planes destination, vc_rows rows);
int vc_pack_yuy2(vc_const_yuv_planes source, vc_plane destination, vc_rows rows);

// Copy YUY2 luma to an independent U8 plane, without reading or writing padding.
int vc_extract_yuy2_luma(vc_const_plane source, vc_plane destination, vc_rows rows);
// In-place exception to the non-overlap rule above: preserve every luma byte and
// replace every chroma byte by 128. Only the selected rows are accessed.
int vc_neutralize_yuy2_chroma(vc_plane image, vc_rows rows);

// Resolve once per filter/operation, then reuse for any row ranges. Each entry
// validates the same buffer contract as the ordinary C functions above.
typedef struct vc_layout_functions {
  int (*unpack_bgr)(vc_const_plane, vc_rgb_planes, int, int, uint32_t, vc_rows);
  int (*pack_bgr)(vc_const_rgb_planes, vc_plane, int, int, uint32_t, vc_rows);
  int (*unpack_yuy2)(vc_const_plane, vc_yuv_planes, vc_rows);
  int (*pack_yuy2)(vc_const_yuv_planes, vc_plane, vc_rows);
  int (*repack_bgr)(vc_const_plane, vc_plane, int, int, int, uint32_t, vc_rows);
  int (*extract_yuy2_luma)(vc_const_plane, vc_plane, vc_rows);
  int (*neutralize_yuy2_chroma)(vc_plane, vc_rows);
} vc_layout_functions;

// Masks cover the complete layout family, excluding C and emulated SIMD.
int64_t vc_layout_compiled_targets(void);
int64_t vc_layout_supported_targets(void);
int64_t vc_layout_choose_target(int64_t allowed_targets);
// target: C, native, or one compiled and hardware-supported Highway target bit.
// Invalid/unavailable targets return NULL. Native falls back to ordinary C.
// The immutable returned table lives for the lifetime of the linked library.
const vc_layout_functions* vc_get_layout_functions(int64_t target);

#ifdef __cplusplus
}
#endif
#endif
