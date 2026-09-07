// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#pragma once
#include "execute.h"
namespace vc::matrix {
int64_t MatrixSupportedTargets();
RowKernel GetMatrixKernel(int64_t target, const Config& config, const Coefficients& coefficients);
} // namespace vc::matrix
