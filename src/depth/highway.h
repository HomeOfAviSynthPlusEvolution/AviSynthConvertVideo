// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#pragma once
#include "transform.h"
namespace vc::depth {
int64_t SupportedTargets();
RowKernel GetKernel(int64_t target, const Transform& transform);
} // namespace vc::depth
