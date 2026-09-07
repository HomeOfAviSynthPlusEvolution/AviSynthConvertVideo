// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_MATRIX_H
#define VIDEO_CONVERT_MATRIX_H
#include "layout.h"
#ifdef __cplusplus
extern "C" {
#endif

enum vc_matrix_direction { VC_RGB_TO_YUV = 0, VC_YUV_TO_RGB = 1, VC_RGB_TO_Y = 2 };
typedef struct vc_matrix_config {
  double kr, kb;
  // 8 selects U8, 9..16 select native-endian U16, 32 selects F32.
  // Integer inputs must fit the declared depth. F32 RGB/Y use 0..1 and
  // chroma uses -0.5..0.5; finite out-of-range inputs are clipped at RGB/YUV output.
  // RGB_TO_Y preserves the grayscale F32 contract: no output clipping.
  int bits_per_sample;
  // Fixed-point precision 0..20. Existing planar AVS routes use 15 for
  // RGB->YUV and 13 for YUV->RGB. Ignored by F32 execution but validated.
  int precision;
  // Exactly 0 (limited) or 1 (full); these select coefficient range scaling.
  int source_full, destination_full;
  int direction;
} vc_matrix_config;
typedef struct vc_matrix_plan vc_matrix_plan;

// Immutable ordinary C plan, independent of AVS. Kr/Kb must be finite,
// nonnegative and sum to less than one. No exception crosses this boundary.
// On failure a non-null output argument is set to NULL. Destroy accepts NULL.
int vc_matrix_create(const vc_matrix_config* config, vc_matrix_plan** output);
// Available runtime SIMD bits, excluding scalar/emulated targets. Target selection
// follows layout.h: C is ordinary scalar, native chooses available SIMD or C,
// and an explicit single bit must be supported. No global dispatch state changes.
int64_t vc_matrix_supported_targets(void);
int vc_matrix_create_for_target(const vc_matrix_config* config, int64_t target, vc_matrix_plan** output);
void vc_matrix_destroy(vc_matrix_plan* plan);

// Full-resolution RGB <-> YUV444, using the layout plane/row buffer contract.
// Plans are reusable for arbitrary image dimensions; all three channels have
// equal geometry and independent signed byte strides. Alpha descriptors are
// ignored. No allocation or shared mutable state during execution. Concurrent
// calls may share a plan with disjoint output storage. Wrong-direction plans,
// invalid rows or planes are rejected before writing. Do not destroy a plan
// while a call is active. NaN payload and signed-zero identity are unspecified.
int vc_matrix_rgb_to_yuv(const vc_matrix_plan* plan, vc_const_rgb_planes source, vc_yuv_planes destination,
                         vc_rows rows);
int vc_matrix_yuv_to_rgb(const vc_matrix_plan* plan, vc_const_yuv_planes source, vc_rgb_planes destination,
                         vc_rows rows);
// Luma-only plan (VC_RGB_TO_Y), sharing forward coefficients. Integer output is
// clipped to the declared depth; F32 output is intentionally not clipped.
int vc_matrix_rgb_to_y(const vc_matrix_plan* plan, vc_const_rgb_planes source, vc_plane destination, vc_rows rows);
#ifdef __cplusplus
}
#endif
#endif
