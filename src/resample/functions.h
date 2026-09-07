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

#ifndef VIDEO_CONVERT_RESAMPLE_FUNCTIONS_H
#define VIDEO_CONVERT_RESAMPLE_FUNCTIONS_H

// Private mathematical functions. No AVS types, coefficient storage or C++ ABI
// is exposed to callers. Sampling plans apply support and boundary rules.
namespace vc::resample {

class ResamplingFunction {
public:
  virtual double f(double x) const = 0;
  virtual double support() const = 0;
  virtual ~ResamplingFunction() = default;
};

class PointFilter : public ResamplingFunction
/**
  * Nearest neighbour (point sampler), used in PointResize
 **/
{
public:
  double f(double x) const override;
  double support() const override { return 0; }
  // Pre 3.7.4 : 0.0001. Comment: 0.0 crashes it.
  // 3.7.4- this 0 is specially handled in GetResamplingProgram
};

class TriangleFilter : public ResamplingFunction
/**
  * Simple triangle filter, used in BilinearResize
 **/
{
public:
  double f(double x) const override;
  double support() const override { return 1.0; }
};

class MitchellNetravaliFilter : public ResamplingFunction
/**
  * Mitchell-Netraveli filter, used in BicubicResize
 **/
{
public:
  MitchellNetravaliFilter(double b = 1. / 3., double c = 1. / 3.);
  double f(double x) const override;
  double support() const override { return 2.0; }

private:
  double p0, p2, p3, q0, q1, q2, q3;
};

class LanczosFilter : public ResamplingFunction
/**
  * Lanczos filter, used in LanczosResize
 **/
{
public:
  LanczosFilter(int _taps = 3);
  double f(double x) const override;
  double support() const override { return taps; };

private:
  double sinc(double value) const;
  double taps;
};

class BlackmanFilter : public ResamplingFunction
/**
  * Blackman filter, used in BlackmanResize
 **/
{
public:
  BlackmanFilter(int _taps = 4);
  double f(double x) const override;
  double support() const override { return taps; };

private:
  double taps, rtaps;
};

// Spline16
class Spline16Filter : public ResamplingFunction
/**
  * Spline16 of Panorama Tools is a cubic-spline, with derivative set to 0 at the edges (4x4 pixels).
 **/
{
public:
  double f(double x) const override;
  double support() const override { return 2.0; };

private:
};

// Spline36
class Spline36Filter : public ResamplingFunction
/**
  * Spline36 is like Spline16,  except that it uses 6x6=36 pixels.
 **/
{
public:
  double f(double x) const override;
  double support() const override { return 3.0; };

private:
};

// Spline64
class Spline64Filter : public ResamplingFunction
/**
  * Spline64 is like Spline36,  except that it uses 8x8=64 pixels.
 **/
{
public:
  double f(double x) const override;
  double support() const override { return 4.0; };

private:
};

class GaussianFilter : public ResamplingFunction
/**
  * GaussianFilter, from swscale.
 **/
{
public:
  GaussianFilter(double p = 30.0, double _b = 2.0, double _s = 4.0);
  double f(double x) const override;
  double support() const override { return s; }; // <3.7.4 was fixed at 4.0

private:
  double param;
  double b; // base value since 3.7.4
  double s; // variable support since 3.7.4
};

class SincFilter : public ResamplingFunction
/**
  * Sinc filter, used in SincResize
 **/
{
public:
  SincFilter(int _taps = 4);
  double f(double x) const override;
  double support() const override { return taps; };

private:
  double taps;
};

class SinPowerFilter : public ResamplingFunction
// SinPow kernel, used in SinPowResize
{
public:
  SinPowerFilter(double p = 2.5);
  double f(double x) const override;
  double support() const override { return 2.0; }; // 2 very important, 4 cause bugs

private:
  double param;
};

class SincLin2Filter : public ResamplingFunction
/**
  * SincLin2 filter, used in SincLin2Resize
  **/
{
public:
  SincLin2Filter(int _taps = 15);
  double f(double x) const override;
  double support() const override { return taps; };

private:
  double sinc(double value) const;
  double taps;
};

class UserDefined2Filter : public ResamplingFunction
/**
	  * User-defined by 2 samples filter, used in UDef2Resize
	 **/
{
public:
  UserDefined2Filter(double _b, double _c, double _s);
  double f(double x) const override;
  double support() const override { return s; }

private:
  double sinc(double value) const;
  double a, b, c;
  double s; // 'support' as a parameter
};

} // namespace vc::resample
#endif
