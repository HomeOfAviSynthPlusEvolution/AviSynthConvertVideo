// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "transform.h"
#include "highway.h"
#include <new>
struct vc_depth_plan {
  vc::depth::Transform transform;
  vc::depth::RowKernel kernel;
};
namespace {
bool ValidDepth(int bits) {
  return (bits >= 8 && bits <= 16) || bits == 32;
}
bool Flag(int value) {
  return value == 0 || value == 1;
}
} // namespace
int64_t vc_depth_supported_targets(void) {
  return vc::depth::SupportedTargets();
}
int vc_depth_create(const vc_depth_config* config, vc_depth_plan** output) {
  return vc_depth_create_for_target(config, VC_TARGET_C, output);
}
int vc_depth_create_for_target(const vc_depth_config* config, int64_t target, vc_depth_plan** output) {
  if (!output)
    return VC_INVALID_ARGUMENT;
  *output = nullptr;
  if (!config || !ValidDepth(config->source_bits) || !ValidDepth(config->destination_bits) ||
      !Flag(config->source_full) || !Flag(config->destination_full) || !Flag(config->chroma))
    return VC_INVALID_ARGUMENT;
  try {
    const auto transform = vc::depth::BuildTransform(*config);
    const auto kernel = target == VC_TARGET_C ? nullptr : vc::depth::GetKernel(target, transform);
    if (target != VC_TARGET_C && target != VC_TARGET_NATIVE && !kernel)
      return VC_INVALID_ARGUMENT;
    *output = new vc_depth_plan{transform, kernel};
    return VC_OK;
  } catch (const std::bad_alloc&) {
    return VC_OUT_OF_MEMORY;
  } catch (...) {
    return VC_INTERNAL_ERROR;
  }
}
void vc_depth_destroy(vc_depth_plan* plan) {
  delete plan;
}
int vc_depth_execute(const vc_depth_plan* plan, vc_const_plane source, vc_plane destination, vc_rows rows) {
  return plan ? vc::depth::Execute(plan->transform, source, destination, rows, plan->kernel) : VC_INVALID_ARGUMENT;
}
