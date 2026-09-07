// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "video_convert/matrix.h"
#include "execute.h"
#include "highway.h"
#include <memory>
#include <new>
#include <stdexcept>
struct vc_matrix_plan {
  vc::matrix::Config config;
  vc::matrix::Coefficients coefficients;
  vc::matrix::RowKernel kernel;
};
int64_t vc_matrix_supported_targets(void) {
  return vc::matrix::MatrixSupportedTargets();
}
int vc_matrix_create(const vc_matrix_config* config, vc_matrix_plan** output) {
  return vc_matrix_create_for_target(config, VC_TARGET_C, output);
}
int vc_matrix_create_for_target(const vc_matrix_config* config, int64_t target, vc_matrix_plan** output) {
  if (!output)
    return VC_INVALID_ARGUMENT;
  *output = nullptr;
  if (!config || (config->source_full != 0 && config->source_full != 1) ||
      (config->destination_full != 0 && config->destination_full != 1) ||
      (config->direction != VC_RGB_TO_YUV && config->direction != VC_YUV_TO_RGB && config->direction != VC_RGB_TO_Y))
    return VC_INVALID_ARGUMENT;
  try {
    const vc::matrix::Config internal{config->kr,
                                      config->kb,
                                      config->bits_per_sample,
                                      config->precision,
                                      config->source_full != 0,
                                      config->destination_full != 0,
                                      config->direction == VC_RGB_TO_YUV   ? vc::matrix::Direction::RgbToYuv
                                      : config->direction == VC_YUV_TO_RGB ? vc::matrix::Direction::YuvToRgb
                                                                           : vc::matrix::Direction::RgbToY};
    const auto coefficients = vc::matrix::BuildCoefficients(internal);
    const auto kernel = target == VC_TARGET_C ? nullptr : vc::matrix::GetMatrixKernel(target, internal, coefficients);
    if (target != VC_TARGET_C && target != VC_TARGET_NATIVE && !kernel)
      return VC_INVALID_ARGUMENT;
    auto plan = std::make_unique<vc_matrix_plan>(vc_matrix_plan{internal, coefficients, kernel});
    *output = plan.release();
    return VC_OK;
  } catch (const std::bad_alloc&) {
    return VC_OUT_OF_MEMORY;
  } catch (const std::invalid_argument&) {
    return VC_INVALID_ARGUMENT;
  } catch (...) {
    return VC_INTERNAL_ERROR;
  }
}
void vc_matrix_destroy(vc_matrix_plan* plan) {
  delete plan;
}
int vc_matrix_rgb_to_yuv(const vc_matrix_plan* plan, vc_const_rgb_planes source, vc_yuv_planes destination,
                         vc_rows rows) {
  if (!plan || plan->config.direction != vc::matrix::Direction::RgbToYuv)
    return VC_INVALID_ARGUMENT;
  return vc::matrix::Execute(plan->config, plan->coefficients, {source.b, source.g, source.r},
                             {destination.y, destination.u, destination.v}, rows, plan->kernel);
}
int vc_matrix_yuv_to_rgb(const vc_matrix_plan* plan, vc_const_yuv_planes source, vc_rgb_planes destination,
                         vc_rows rows) {
  if (!plan || plan->config.direction != vc::matrix::Direction::YuvToRgb)
    return VC_INVALID_ARGUMENT;
  return vc::matrix::Execute(plan->config, plan->coefficients, {source.y, source.u, source.v},
                             {destination.b, destination.g, destination.r}, rows, plan->kernel);
}

int vc_matrix_rgb_to_y(const vc_matrix_plan* plan, vc_const_rgb_planes source, vc_plane destination, vc_rows rows) {
  if (!plan || plan->config.direction != vc::matrix::Direction::RgbToY)
    return VC_INVALID_ARGUMENT;
  return vc::matrix::Execute(plan->config, plan->coefficients, {source.b, source.g, source.r}, {destination, {}, {}},
                             rows, plan->kernel);
}
