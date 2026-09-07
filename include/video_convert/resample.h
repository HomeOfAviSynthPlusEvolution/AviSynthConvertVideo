// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_RESAMPLE_H
#define VIDEO_CONVERT_RESAMPLE_H
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif
enum vc_resample_axis { VC_HORIZONTAL = 0, VC_VERTICAL = 1 };
enum vc_filter_kind {
  VC_POINT,
  VC_TRIANGLE,
  VC_BICUBIC,
  VC_LANCZOS,
  VC_BLACKMAN,
  VC_SPLINE16,
  VC_SPLINE36,
  VC_SPLINE64,
  VC_GAUSSIAN,
  VC_SINC,
  VC_SINPOWER,
  VC_SINCLIN2,
  VC_USER_DEFINED2
};
// Parameters: Bicubic b,c; Lanczos/Blackman/Sinc/SincLin2 integer taps;
// Gaussian p,b,s; SinPower p; UserDefined2 b,c,s. Others use no parameters.
// All three slots must be finite. Parameters are explicit; no script defaults.
typedef struct vc_filter_spec {
  int kind;
  double parameters[3];
} vc_filter_spec;
// Query the effective support radius, including filter parameter normalization.
// The output is unchanged on failure. This query does not select a CPU target.
int vc_resample_filter_support(const vc_filter_spec* filter, double* support);

typedef struct vc_resample_config {
  int axis, source_width, source_height, target_size, bits_per_sample;
  double crop_start, crop_size, source_center, destination_center;
  vc_filter_spec filter;
} vc_resample_config;
typedef struct vc_resample_plan vc_resample_plan;
typedef struct vc_row_range {
  int first_row, row_count;
} vc_row_range;
// data denotes first_row, not global row zero. Stride is signed bytes.
// Storage must cover row_count full-width rows. Input/output may not overlap.
typedef struct vc_const_row_band {
  vc_const_plane plane;
  vc_row_range rows;
} vc_const_row_band;
typedef struct vc_row_band {
  vc_plane plane;
  vc_row_range rows;
} vc_row_band;
// Creates an immutable, ordinary C plan. bits: 8 (U8), 9..16 (U16), 32 (F32).
// All exceptions are translated. *output is null on failure if output is nonnull.
// Caller retains no configuration storage. Destroy may accept null, but must not
// race execution/query calls. No AVS dependency, global dispatch, or shared scratch.
int vc_resample_create(const vc_resample_config* config, vc_resample_plan** output);
// Available compiled SIMD targets supported by this CPU. Zero in scalar-only
// builds. Values are Highway target bits; the host owns its CPU policy.
int64_t vc_resample_supported_targets(void);
// Bind an immutable plan at creation. C always uses ordinary C; NATIVE chooses
// an available native target or C when none exists. An explicit SIMD target must
// be one available bit; unavailable/mixed/invalid selections are rejected.
// Preparing another plan never changes an existing plan or process-global state.
int vc_resample_create_for_target(const vc_resample_config* config, int64_t target, vc_resample_plan** output);
void vc_resample_destroy(vc_resample_plan* plan);
// Returns a contiguous source-row envelope for a global output range. It may
// include gaps. Empty requests return {0,0}. No sample buffer is accessed.
int vc_resample_required_rows(const vc_resample_plan* plan, vc_row_range output, vc_row_range* required);
// Execute exactly destination.rows. Source must cover the required envelope;
// it may cover more rows. No allocations or scratch. Disjoint output bands can
// execute concurrently with the same plan. Finite F32 inputs retain sequential
// coefficient order without clipping. U16 samples must fit the declared depth.
int vc_resample_execute(const vc_resample_plan* plan, vc_const_row_band source, vc_row_band destination);
#ifdef __cplusplus
}
#endif
#endif
