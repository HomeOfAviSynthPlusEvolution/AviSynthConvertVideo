// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_RESAMPLE_HIGHWAY_H
#define VIDEO_CONVERT_RESAMPLE_HIGHWAY_H
#include "execute.h"
namespace vc::resample {
// Private raw kernels: caller validates geometry, bands and storage first.
using RowKernel = void (*)(const Coefficients&, vc_const_plane, vc_plane, vc_rows, int, int);
struct ResampleKernels {
  size_t lanes;
  RowKernel horizontal_float, horizontal_integer, vertical;
};
// Immutable native/explicit SIMD table. C, unsupported and unavailable targets
// return null; caller uses ExecuteC. No process-global target overrides.
const ResampleKernels* GetResampleKernels(int64_t target);
int64_t ResampleSupportedTargets();
} // namespace vc::resample
#endif
