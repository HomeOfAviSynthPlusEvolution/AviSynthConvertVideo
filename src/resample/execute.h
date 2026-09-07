// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_RESAMPLE_EXECUTE_H
#define VIDEO_CONVERT_RESAMPLE_EXECUTE_H
#include "coefficients.h"
#include "video_convert/layout.h"
namespace vc::resample {
enum class Axis { Horizontal, Vertical };
// Prepared coefficients must come from BuildCoefficients. Plan and source are
// immutable. Rows describe the full output image and a global output row range.
// H requires rows.width == target_size; V requires rows.height == target_size.
// Source/destination pointers refer to their supplied first rows (default zero).
// Counts of -1 privately mean full image height. No scratch or allocation.
// Common pre-write validation for ordinary C and target-bound SIMD execution.
int ValidateExecution(const Coefficients& plan, Axis axis, vc_const_plane source, vc_plane destination, vc_rows rows,
                      int source_first, int source_count, int destination_first, int destination_count);
int ExecuteC(const Coefficients& plan, Axis axis, vc_const_plane source, vc_plane destination, vc_rows rows,
             int source_first = 0, int source_count = -1, int destination_first = 0, int destination_count = -1);
} // namespace vc::resample
#endif
