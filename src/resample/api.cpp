// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "video_convert/resample.h"
#include "execute.h"
#include "highway.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
struct vc_resample_plan {
  vc::resample::Coefficients coefficients;
  vc::resample::Axis axis;
  int width, height;
  vc::resample::RowKernel kernel = nullptr;
};
namespace {
using namespace vc::resample;
std::unique_ptr<ResamplingFunction> MakeFilter(const vc_filter_spec& filter) {
  const auto& p = filter.parameters;
  for (double value : p)
    if (!std::isfinite(value))
      throw std::invalid_argument("Nonfinite filter parameter");
  auto taps = [&] {
    if (p[0] < 1 || p[0] > std::numeric_limits<int>::max() || std::floor(p[0]) != p[0])
      throw std::invalid_argument("Invalid tap count");
    return int(p[0]);
  };
  switch (filter.kind) {
    case VC_POINT:
      return std::make_unique<PointFilter>();
    case VC_TRIANGLE:
      return std::make_unique<TriangleFilter>();
    case VC_BICUBIC:
      return std::make_unique<MitchellNetravaliFilter>(p[0], p[1]);
    case VC_LANCZOS:
      return std::make_unique<LanczosFilter>(taps());
    case VC_BLACKMAN:
      return std::make_unique<BlackmanFilter>(taps());
    case VC_SPLINE16:
      return std::make_unique<Spline16Filter>();
    case VC_SPLINE36:
      return std::make_unique<Spline36Filter>();
    case VC_SPLINE64:
      return std::make_unique<Spline64Filter>();
    case VC_GAUSSIAN:
      return std::make_unique<GaussianFilter>(p[0], p[1], p[2]);
    case VC_SINC:
      return std::make_unique<SincFilter>(taps());
    case VC_SINPOWER:
      return std::make_unique<SinPowerFilter>(p[0]);
    case VC_SINCLIN2:
      return std::make_unique<SincLin2Filter>(taps());
    case VC_USER_DEFINED2:
      return std::make_unique<UserDefined2Filter>(p[0], p[1], p[2]);
    default:
      throw std::invalid_argument("Unknown filter");
  }
}
} // namespace
int vc_resample_filter_support(const vc_filter_spec* filter, double* support) {
  if (!filter || !support)
    return VC_INVALID_ARGUMENT;
  try {
    const auto function = MakeFilter(*filter);
    *support = function->support();
    return VC_OK;
  } catch (const std::bad_alloc&) {
    return VC_OUT_OF_MEMORY;
  } catch (const std::invalid_argument&) {
    return VC_INVALID_ARGUMENT;
  } catch (...) {
    return VC_INTERNAL_ERROR;
  }
}
int64_t vc_resample_supported_targets(void) {
  return vc::resample::ResampleSupportedTargets();
}
int vc_resample_create(const vc_resample_config* config, vc_resample_plan** output) {
  return vc_resample_create_for_target(config, VC_TARGET_C, output);
}
int vc_resample_create_for_target(const vc_resample_config* config, int64_t target, vc_resample_plan** output) {
  if (!output)
    return VC_INVALID_ARGUMENT;
  *output = nullptr;
  if (!config || config->source_width <= 0 || config->source_height <= 0 ||
      (config->axis != VC_HORIZONTAL && config->axis != VC_VERTICAL))
    return VC_INVALID_ARGUMENT;
  try {
    const auto* kernels = target == VC_TARGET_C ? nullptr : GetResampleKernels(target);
    if (target != VC_TARGET_C && target != VC_TARGET_NATIVE && !kernels)
      return VC_INVALID_ARGUMENT;
    const auto function = MakeFilter(config->filter);
    const bool horizontal = config->axis == VC_HORIZONTAL;
    auto plan = std::make_unique<vc_resample_plan>();
    plan->axis = horizontal ? Axis::Horizontal : Axis::Vertical;
    plan->width = horizontal ? config->target_size : config->source_width;
    plan->height = horizontal ? config->source_height : config->target_size;
    plan->coefficients = BuildCoefficients(
        *function, {horizontal ? config->source_width : config->source_height, config->target_size, config->crop_start,
                    config->crop_size, config->bits_per_sample, config->source_center, config->destination_center});
    if (kernels) {
      plan->kernel = horizontal
                         ? (config->bits_per_sample == 32 ? kernels->horizontal_float : kernels->horizontal_integer)
                         : kernels->vertical;
      if (horizontal)
        PrepareHorizontal(plan->coefficients, kernels->lanes);
    }
    *output = plan.release();
    return VC_OK;
  } catch (const std::bad_alloc&) {
    return VC_OUT_OF_MEMORY;
  } catch (const std::length_error&) {
    return VC_INVALID_ARGUMENT;
  } catch (const std::invalid_argument&) {
    return VC_INVALID_ARGUMENT;
  } catch (...) {
    return VC_INTERNAL_ERROR;
  }
}
void vc_resample_destroy(vc_resample_plan* plan) {
  delete plan;
}
int vc_resample_required_rows(const vc_resample_plan* plan, vc_row_range output, vc_row_range* required) {
  if (!plan || !required || output.first_row < 0 || output.row_count < 0 || output.first_row > plan->height ||
      output.row_count > plan->height - output.first_row)
    return VC_INVALID_ARGUMENT;
  if (output.row_count == 0) {
    *required = {0, 0};
    return VC_OK;
  }
  if (plan->axis == Axis::Horizontal) {
    *required = output;
    return VC_OK;
  }
  int first = plan->coefficients.source_size, end = 0;
  for (int y = output.first_row; y < output.first_row + output.row_count; ++y) {
    first = std::min(first, plan->coefficients.offsets[y]);
    end = std::max(end, plan->coefficients.offsets[y] + plan->coefficients.sizes[y]);
  }
  *required = {first, end - first};
  return VC_OK;
}
int vc_resample_execute(const vc_resample_plan* plan, vc_const_row_band source, vc_row_band destination) {
  if (!plan || source.rows.first_row < 0 || source.rows.row_count < 0)
    return VC_INVALID_ARGUMENT;
  const vc_rows rows{plan->width, plan->height, destination.rows.first_row, destination.rows.row_count};
  if (!plan->kernel)
    return ExecuteC(plan->coefficients, plan->axis, source.plane, destination.plane, rows, source.rows.first_row,
                    source.rows.row_count, destination.rows.first_row, destination.rows.row_count);
  const int status =
      ValidateExecution(plan->coefficients, plan->axis, source.plane, destination.plane, rows, source.rows.first_row,
                        source.rows.row_count, destination.rows.first_row, destination.rows.row_count);
  if (status != VC_OK || rows.row_count == 0)
    return status;
  plan->kernel(plan->coefficients, source.plane, destination.plane, rows, source.rows.first_row,
               destination.rows.first_row);
  return VC_OK;
}
