#ifndef VIDEO_CONVERT_TESTS_RESAMPLE_HELPERS_H
#define VIDEO_CONVERT_TESTS_RESAMPLE_HELPERS_H
#include "resample/functions.h"
#include "resample_math_golden.h"
#include <memory>
namespace vc_test {
inline std::unique_ptr<vc::resample::ResamplingFunction> MakeFunction(const MathGolden& golden) {
  using namespace vc::resample;
  const auto& p = golden.parameters;
  switch (golden.kind) {
    case FilterKind::Point:
      return std::make_unique<PointFilter>();
    case FilterKind::Triangle:
      return std::make_unique<TriangleFilter>();
    case FilterKind::Bicubic:
      return std::make_unique<MitchellNetravaliFilter>(p[0], p[1]);
    case FilterKind::Lanczos:
      return std::make_unique<LanczosFilter>(int(p[0]));
    case FilterKind::Blackman:
      return std::make_unique<BlackmanFilter>(int(p[0]));
    case FilterKind::Spline16:
      return std::make_unique<Spline16Filter>();
    case FilterKind::Spline36:
      return std::make_unique<Spline36Filter>();
    case FilterKind::Spline64:
      return std::make_unique<Spline64Filter>();
    case FilterKind::Gaussian:
      return std::make_unique<GaussianFilter>(p[0], p[1], p[2]);
    case FilterKind::Sinc:
      return std::make_unique<SincFilter>(int(p[0]));
    case FilterKind::SinPower:
      return std::make_unique<SinPowerFilter>(p[0]);
    case FilterKind::SincLin2:
      return std::make_unique<SincLin2Filter>(int(p[0]));
    case FilterKind::UserDefined2:
      return std::make_unique<UserDefined2Filter>(p[0], p[1], p[2]);
  }
  return nullptr;
}

} // namespace vc_test
#endif
