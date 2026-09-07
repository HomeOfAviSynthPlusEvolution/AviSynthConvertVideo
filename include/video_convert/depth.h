// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_DEPTH_H
#define VIDEO_CONVERT_DEPTH_H
#include "layout.h"
#ifdef __cplusplus
extern "C" {
#endif
// One channel without dithering. Use chroma=0 for RGB, luma and alpha;
// alpha uses full ranges on both sides. Integer input must fit its declared depth.
typedef struct vc_depth_config {
  int source_bits, destination_bits; // 8 = U8, 9..16 = native-endian U16, 32 = F32.
  int source_full, destination_full; // Exactly 0 (limited) or 1 (full).
  int chroma;                        // Exactly 0 or 1; F32 chroma is centered on zero.
} vc_depth_config;
typedef struct vc_depth_plan vc_depth_plan;
// Immutable ordinary C plan. No exception crosses this boundary; on failure
// a non-null output argument is cleared. Destroy accepts NULL.
int vc_depth_create(const vc_depth_config* config, vc_depth_plan** output);
// Same target contract as layout.h: C, native (SIMD or C), or one supported bit.
// Selection is bound per plan and does not modify global Highway dispatch state.
int64_t vc_depth_supported_targets(void);
int vc_depth_create_for_target(const vc_depth_config* config, int64_t target, vc_depth_plan** output);
void vc_depth_destroy(vc_depth_plan* plan);
// Uses the layout.h row/plane contract, independent signed byte strides and
// naturally aligned samples. Source/destination must not overlap. No allocation
// during execution; concurrent calls may share a plan with disjoint outputs.
// Integer output rounds half upward and saturates before narrowing. NaN maps
// to code zero, infinities to the corresponding endpoint. F32 is unclipped.
// Identical depth/range copies sample representations. Valid empty row bands
// access no buffers; invalid descriptors are rejected before any output write.
int vc_depth_execute(const vc_depth_plan* plan, vc_const_plane source, vc_plane destination, vc_rows rows);
#ifdef __cplusplus
}
#endif
#endif
