// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_DITHER_H
#define VIDEO_CONVERT_DITHER_H
#include "depth.h"
#ifdef __cplusplus
extern "C" {
#endif
// Ordered quantization of one integer channel. Alpha uses ordinary depth conversion.
// Input samples must fit source_bits. The matrix phase uses logical frame x/y;
// row bands may execute in any order and concurrently into disjoint outputs.
typedef struct vc_ordered_config {
  vc_depth_config depth; // Integer depths 8..16; destination_bits <= source_bits.
  int quantization_bits; // 1..destination_bits, strictly less than source_bits.
                         // The source/quantization difference must be <= 8.
} vc_ordered_config;
typedef struct vc_ordered_plan vc_ordered_plan;
int vc_ordered_create(const vc_ordered_config* config, vc_ordered_plan** output);
// Bind the same C/native/explicit-target policy as depth.h.
int64_t vc_ordered_supported_targets(void);
int vc_ordered_create_for_target(const vc_ordered_config* config, int64_t target, vc_ordered_plan** output);
void vc_ordered_destroy(vc_ordered_plan* plan);
// Immutable C plan, signed strides and valid empty bands as in depth.h.
// No allocation during execute. Invalid descriptors fail before writing output.
// Input/output buffers must not overlap. Quantization below 8 bits uses centered
// thresholds and full-code backscaling, matching the existing artistic mode.
int vc_ordered_execute(const vc_ordered_plan* plan, vc_const_plane source, vc_plane destination, vc_rows rows);
// Stateful serpentine error diffusion. Unlike ordered dithering, row bands must
// start at the next unprocessed row. A context belongs to one frame/plane and
// must not be shared by concurrent executions. Independent contexts may run in
// parallel. This recurrence uses ordinary C; adjacent samples are dependent.
typedef struct vc_floyd_config {
  vc_depth_config depth; // Integer 8..16; destination_bits <= source_bits.
  int quantization_bits; // 1..destination_bits and < source_bits; difference <=15.
} vc_floyd_config;
typedef struct vc_floyd_context vc_floyd_context;
// Allocate all error storage up front. Output is cleared on failure.
int vc_floyd_create(const vc_floyd_config* config, int width, int height, vc_floyd_context** output);
void vc_floyd_destroy(vc_floyd_context* context);
// Start another plane/frame with the same config/geometry and zero error state.
void vc_floyd_reset(vc_floyd_context* context);
// Geometry must match creation; first_row must equal the next unprocessed row.
// Empty bands access no buffers. Validation precedes any pixel/state mutation.
// No allocation during execution. Source and destination must not overlap.
int vc_floyd_execute(vc_floyd_context* context, vc_const_plane source, vc_plane destination, vc_rows rows);
#ifdef __cplusplus
}
#endif
#endif
