// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#pragma once
#include "coefficients.h"
#include "video_convert/layout.h"
#include <array>
namespace vc::matrix {
using RowKernel = void (*)(const Config&, const Coefficients&, const std::array<vc_const_plane, 3>&,
                           const std::array<vc_plane, 3>&, vc_rows);
int Execute(const Config& config, const Coefficients& coefficients, const std::array<vc_const_plane, 3>& source,
            const std::array<vc_plane, 3>& destination, vc_rows rows, RowKernel kernel);
// Private validated full-resolution row execution. RGB arrays use B,G,R;
// YUV arrays use Y,U,V. Alpha is outside the transform. Coefficients must have
// been built from config. All plane pointers describe logical row zero.
int ExecuteC(const Config& config, const Coefficients& coefficients, const std::array<vc_const_plane, 3>& source,
             const std::array<vc_plane, 3>& destination, vc_rows rows);
} // namespace vc::matrix
