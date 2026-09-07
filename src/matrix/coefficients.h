// Avisynth v2.5.  Copyright 2002-2009 Ben Rudiak-Gould et al.
// http://avisynth.nl

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.

#pragma once
namespace vc::matrix {
enum class Direction { RgbToYuv, YuvToRgb, RgbToY };
struct Config {
  double kr, kb;
  int bits_per_sample, precision;
  bool source_full, destination_full;
  Direction direction;
};
// Private imported coefficient layout. Channel labels retain their original
// meaning in each direction; no AVS structures or ABI layout dependency.
struct Coefficients {
  int y_r, y_g, y_b;
  // for grayscale conversion these may not needed
  int u_r, u_g, u_b;
  int v_r, v_g, v_b;

  float y_r_f, y_g_f, y_b_f;
  float u_r_f, u_g_f, u_b_f;
  float v_r_f, v_g_f, v_b_f;

  int offset_y;
  float offset_y_f;
  int offset_rgb;
  float offset_rgb_f;
};

// Valid depths: 8..16 and 32; fixed-point precision: 0..20. Luma weights
// must be finite, nonnegative and sum to less than one. Reject coefficients
// outside the signed 32-bit representation instead of invoking cast overflow.
Coefficients BuildCoefficients(const Config& config);
} // namespace vc::matrix
