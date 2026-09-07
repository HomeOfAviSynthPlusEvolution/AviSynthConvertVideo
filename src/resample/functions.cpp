// Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
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

#include "functions.h"
#include <algorithm>
#include <cmath>

namespace vc::resample {
namespace {
// Same binary64 constant as the original resample_functions.h M_PI macro.
constexpr double kPi = 3.14159265358979323846;
} // namespace

double PointFilter::f(double x) const {
  (void)x;
  return 1.0;
}

/***************************
 ***** Triangle filter *****
 **************************/

double TriangleFilter::f(double x) const {
  x = fabs(x);
  return (x < 1.0) ? 1.0 - x : 0.0;
}

/*********************************
 *** Mitchell-Netravali filter ***
 *********************************/

MitchellNetravaliFilter::MitchellNetravaliFilter(double b, double c) {
  p0 = (6. - 2. * b) / 6.;
  p2 = (-18. + 12. * b + 6. * c) / 6.;
  p3 = (12. - 9. * b - 6. * c) / 6.;
  q0 = (8. * b + 24. * c) / 6.;
  q1 = (-12. * b - 48. * c) / 6.;
  q2 = (6. * b + 30. * c) / 6.;
  q3 = (-b - 6. * c) / 6.;
}

double MitchellNetravaliFilter::f(double x) const {
  x = fabs(x);
  return (x < 1) ? (p0 + x * x * (p2 + x * p3)) : (x < 2) ? (q0 + x * (q1 + x * (q2 + x * q3))) : 0.0;
}

/***********************
 *** Lanczos3 filter ***
 ***********************/
LanczosFilter::LanczosFilter(int _taps) {
  taps = (double)std::clamp(_taps, 1, 100);
}

double LanczosFilter::sinc(double value) const {
  if (value > 0.000001) {
    value *= kPi;
    return sin(value) / value;
  } else {
    return 1.0;
  }
}

double LanczosFilter::f(double value) const {
  value = fabs(value);

  if (value < taps) {
    return (sinc(value) * sinc(value / taps));
  } else {
    return 0.0;
  }
}

/***********************
 *** Blackman filter ***
 ***********************/
BlackmanFilter::BlackmanFilter(int _taps) {
  taps = (double)std::clamp(_taps, 1, 100);
  rtaps = 1.0 / taps;
}

double BlackmanFilter::f(double value) const {
  value = fabs(value);

  if (value < taps) {
    if (value > 0.000001) {
      value *= kPi;
      return (sin(value) / value) * (0.42 + 0.5 * cos(value * rtaps) + 0.08 * cos(2 * value * rtaps));
    } else {
      return 1.0;
    }
  } else {
    return 0.0;
  }
}

/***********************
 *** Spline16 filter ***
 ***********************/

double Spline16Filter::f(double value) const {
  value = fabs(value);

  if (value < 1.0) {
    return ((value - 9.0 / 5.0) * value - 1.0 / 5.0) * value + 1.0;
  } else if (value < 2.0) {
    return ((-1.0 / 3.0 * (value - 1.0) + 4.0 / 5.0) * (value - 1.0) - 7.0 / 15.0) * (value - 1.0);
  }
  return 0.0;
}

/***********************
 *** Spline36 filter ***
 ***********************/

double Spline36Filter::f(double value) const {
  value = fabs(value);

  if (value < 1.0) {
    return ((13.0 / 11.0 * (value)-453.0 / 209.0) * (value)-3.0 / 209.0) * (value) + 1.0;
  } else if (value < 2.0) {
    return ((-6.0 / 11.0 * (value - 1.0) + 270.0 / 209.0) * (value - 1.0) - 156.0 / 209.0) * (value - 1.0);
  } else if (value < 3.0) {
    return ((1.0 / 11.0 * (value - 2.0) - 45.0 / 209.0) * (value - 2.0) + 26.0 / 209.0) * (value - 2.0);
  }
  return 0.0;
}

/***********************
 *** Spline64 filter ***
 ***********************/

double Spline64Filter::f(double value) const {
  value = fabs(value);

  if (value < 1.0) {
    return ((49.0 / 41.0 * (value)-6387.0 / 2911.0) * (value)-3.0 / 2911.0) * (value) + 1.0;
  } else if (value < 2.0) {
    return ((-24.0 / 41.0 * (value - 1.0) + 4032.0 / 2911.0) * (value - 1.0) - 2328.0 / 2911.0) * (value - 1.0);
  } else if (value < 3.0) {
    return ((6.0 / 41.0 * (value - 2.0) - 1008.0 / 2911.0) * (value - 2.0) + 582.0 / 2911.0) * (value - 2.0);
  } else if (value < 4.0) {
    return ((-1.0 / 41.0 * (value - 3.0) + 168.0 / 2911.0) * (value - 3.0) - 97.0 / 2911.0) * (value - 3.0);
  }
  return 0.0;
}

/***********************
 *** Gaussian filter ***
 ***********************/

/* Solve taps from p*value*value < 9 as pow(2.0, -9.0) == 1.0/512.0 i.e 0.5 bit
                     value*value < 9/p       p = param*0.1;
                     value*value < 90/param
                     value*value < 90/{0.1, 22.5, 30.0, 100.0}
                     value*value < {900, 4.0, 3.0, 0.9}
                     value       < {30, 2.0, 1.73, 0.949}         */

GaussianFilter::GaussianFilter(double p, double _b, double _s) {
  param = std::clamp(p, 0.01, 100.0);
  b = std::clamp(_b, 1.5, 3.5);
  s = _s;
  if (_s == 0) // auto-support signal
  {
    // get support from b and param for 0.01 of resudual kernel value
    // equatiion is s = sqrt(-ln(0.01)/(param*ln(b))
    // where ln(0.01) is about -4.6 and -ln(0.01) is 4.6
    s = sqrt(4.6 / ((param * 0.1) * log(b)));
  }
  s = std::clamp(s, 0.1, 150.0);
}

double GaussianFilter::f(double value) const {
  double p = param * 0.1;
  return pow(b, -p * value * value); // <3.7.4: b was fixed at 2.0
}

/***********************
 *** Sinc filter ***
 ***********************/
SincFilter::SincFilter(int _taps) {
  taps = (double)std::clamp(_taps, 1, 150);
}

double SincFilter::f(double value) const {
  value = fabs(value);

  if (value > 0.000001) {
    value *= kPi;
    return sin(value) / value;
  } else {
    return 1.0;
  }
}

/**********************
*** SinPower filter ***
***********************/

SinPowerFilter::SinPowerFilter(double p) {
  param = std::clamp(p, 1.0, 10.0);
}

double SinPowerFilter::f(double value) const {
  value = fabs(value);
  value *= kPi / param;

  if (value < (kPi / 2))
    return pow(cos(value), 1.8);
  else {
    if (value < kPi)
      return -(cos(value) * cos(value)) / (0.9 * value);
    else
      return 0;
  }
}

/***********************
*** SincLin2 filter ***
***********************/

SincLin2Filter::SincLin2Filter(int _taps) {
  taps = (double)std::clamp(_taps, 1, 30);
}

double SincLin2Filter::sinc(double value) const {
  if (value > 0.000001) {
    value *= kPi;
    return sin(value) / value;
  } else
    return 1.0;
}

double SincLin2Filter::f(double value) const {
  value = fabs(value);

  if (value < (taps / 2.0))
    return sinc(value);
  else
    return sinc(value) * ((2.0 - (2.0 * value / taps)));
}

/*********************************
 *** UserDefined2 filter ***
 *********************************/

UserDefined2Filter::UserDefined2Filter(double _b, double _c, double _s) {
  a = 1.0;                                  // 0 sample = 1
  b = (double)std::clamp(_b, -50.0, 250.0); // 1 and -1  sample
  c = (double)std::clamp(_c, -50.0, 250.0); // 2 and -2 sample
  b = (b - 16.0) / 219.0;
  c = (c - 16.0) / 219.0;
  s = (double)std::clamp(_s, 1.5, 15.0); // filter support for resampler
}

double UserDefined2Filter::sinc(double value) const {

  if (fabs(value) > 0.000001) {
    value *= kPi;
    return sin(value) / value;
  } else
    return 1.0;
}

double UserDefined2Filter::f(double x) const {
  x = fabs(x);

  return c * sinc(x + 2) + b * sinc(x + 1) + a * sinc(x) + b * sinc(x - 1) + c * sinc(x - 2);
}

} // namespace vc::resample
