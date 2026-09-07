// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_DEPTH_TRANSFORM_H
#define VIDEO_CONVERT_DEPTH_TRANSFORM_H
#include "video_convert/depth.h"
namespace vc::depth {
struct Transform {
  vc_depth_config config;
  float source_offset, factor, destination_offset;
};
Transform BuildTransform(vc_depth_config config);
using RowKernel = void (*)(const Transform&, vc_const_plane, vc_plane, vc_rows);
int Execute(const Transform&, vc_const_plane, vc_plane, vc_rows, RowKernel);
int ExecuteC(const Transform& transform, vc_const_plane source, vc_plane destination, vc_rows rows);
} // namespace vc::depth
#endif
