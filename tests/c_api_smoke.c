#include "video_convert/dither.h"
#include "video_convert/matrix.h"
#include "video_convert/depth.h"
#include "video_convert/layout.h"
#include "video_convert/resample.h"

int main(void) {
  const uint8_t packed[] = {1, 2, 3, 4};
  uint8_t y[2] = {0}, u = 0, v = 0, output[4] = {0};
  const vc_rows rows = {2, 1, 0, 1};
  const vc_const_plane input = {packed, 4};
  const vc_yuv_planes planar = {{y, 2}, {&u, 1}, {&v, 1}};
  const vc_const_yuv_planes source = {{y, 2}, {&u, 1}, {&v, 1}};
  const vc_plane destination = {output, 4};
  if (vc_unpack_yuy2(input, planar, rows) != VC_OK || y[0] != 1 || y[1] != 3 || u != 2 || v != 4)
    return 1;
  if (vc_pack_yuy2(source, destination, rows) != VC_OK)
    return 2;
  for (int i = 0; i < 4; ++i)
    if (output[i] != packed[i])
      return 3;
  const vc_layout_functions* functions = vc_get_layout_functions(VC_TARGET_NATIVE);
  if (!functions || functions->unpack_yuy2(input, planar, rows) != VC_OK ||
      functions->pack_yuy2(source, destination, rows) != VC_OK)
    return 4;
  for (int i = 0; i < 4; ++i)
    if (output[i] != packed[i])
      return 5;
  vc_resample_config config = {VC_HORIZONTAL, 2, 1, 4, 8, 0, 2, 0.5, 0.5, {VC_POINT, {0, 0, 0}}};
  vc_resample_plan* plan = NULL;
  if (vc_resample_create(&config, &plan) != VC_OK)
    return 6;
  vc_row_range required;
  const vc_row_range range = {0, 1};
  if (vc_resample_required_rows(plan, range, &required) != VC_OK) {
    vc_resample_destroy(plan);
    return 7;
  }
  const vc_const_row_band band = {{y, 2}, required};
  const vc_row_band dest = {{output, 4}, range};
  int status = vc_resample_execute(plan, band, dest);
  vc_resample_destroy(plan);
  if (status != VC_OK || output[0] != 1 || output[1] != 1 || output[2] != 3 || output[3] != 3)
    return 8;
  if (vc_resample_supported_targets() < 0 || vc_resample_create_for_target(&config, VC_TARGET_NATIVE, &plan) != VC_OK)
    return 9;
  status = vc_resample_execute(plan, band, dest);
  vc_resample_destroy(plan);
  if (status != VC_OK || output[0] != 1 || output[1] != 1 || output[2] != 3 || output[3] != 3)
    return 10;
  const vc_matrix_config matrix_config = {0.299, 0.114, 8, 15, 1, 0, VC_RGB_TO_YUV};
  vc_matrix_plan* matrix_plan = NULL;
  if (vc_matrix_create(&matrix_config, &matrix_plan) != VC_OK)
    return 11;
  const uint8_t rgb[2] = {0, 255};
  uint8_t my[2] = {0}, mu[2] = {0}, mv[2] = {0};
  const vc_const_rgb_planes rgb_source = {{rgb, 2}, {rgb, 2}, {rgb, 2}, {NULL, 0}};
  const vc_yuv_planes yuv_destination = {{my, 2}, {mu, 2}, {mv, 2}};
  status = vc_matrix_rgb_to_yuv(matrix_plan, rgb_source, yuv_destination, rows);
  vc_matrix_destroy(matrix_plan);
  if (status != VC_OK || my[0] != 16 || my[1] != 235 || mu[0] != 128 || mu[1] != 128 || mv[0] != 128 || mv[1] != 128)
    return 12;
  if (vc_matrix_supported_targets() < 0 ||
      vc_matrix_create_for_target(&matrix_config, VC_TARGET_NATIVE, &matrix_plan) != VC_OK)
    return 13;
  status = vc_matrix_rgb_to_yuv(matrix_plan, rgb_source, yuv_destination, rows);
  vc_matrix_destroy(matrix_plan);
  if (status != VC_OK || my[0] != 16 || my[1] != 235 || mu[0] != 128 || mu[1] != 128 || mv[0] != 128 || mv[1] != 128)
    return 14;
  vc_matrix_config luma_config = matrix_config;
  luma_config.direction = VC_RGB_TO_Y;
  if (vc_matrix_create_for_target(&luma_config, VC_TARGET_NATIVE, &matrix_plan) != VC_OK)
    return 19;
  status = vc_matrix_rgb_to_y(matrix_plan, rgb_source, yuv_destination.y, rows);
  vc_matrix_destroy(matrix_plan);
  if (status != VC_OK)
    return 20;
  const vc_depth_config depth_config = {8, 16, 1, 1, 0};
  vc_depth_plan* depth_plan = NULL;
  uint16_t depth_output[2] = {0};
  if (vc_depth_create(&depth_config, &depth_plan) != VC_OK)
    return 21;
  status = vc_depth_execute(depth_plan, rgb_source.r, (vc_plane){depth_output, 4}, rows);
  vc_depth_destroy(depth_plan);
  if (status != VC_OK || depth_output[0] != 0 || depth_output[1] != 65535)
    return 22;
  if (vc_depth_supported_targets() < 0 ||
      vc_depth_create_for_target(&depth_config, VC_TARGET_NATIVE, &depth_plan) != VC_OK)
    return 23;
  status = vc_depth_execute(depth_plan, rgb_source.r, (vc_plane){depth_output, 4}, rows);
  vc_depth_destroy(depth_plan);
  if (status != VC_OK || depth_output[0] != 0 || depth_output[1] != 65535)
    return 24;
  const vc_ordered_config ordered_config = {{16, 8, 1, 1, 0}, 8};
  vc_ordered_plan* ordered_plan = NULL;
  uint8_t ordered_output[2] = {1, 1};
  if (vc_ordered_create(&ordered_config, &ordered_plan) != VC_OK)
    return 25;
  status = vc_ordered_execute(ordered_plan, (vc_const_plane){depth_output, 4}, (vc_plane){ordered_output, 2}, rows);
  vc_ordered_destroy(ordered_plan);
  if (status != VC_OK || ordered_output[0] != 0 || ordered_output[1] != 255)
    return 26;
  const vc_floyd_config floyd_config = {{16, 8, 1, 1, 0}, 8};
  vc_floyd_context* floyd_context = NULL;
  if (vc_floyd_create(&floyd_config, 2, 1, &floyd_context) != VC_OK)
    return 27;
  status = vc_floyd_execute(floyd_context, (vc_const_plane){depth_output, 4}, (vc_plane){ordered_output, 2}, rows);
  vc_floyd_reset(floyd_context);
  vc_floyd_destroy(floyd_context);
  if (status != VC_OK || ordered_output[0] != 0 || ordered_output[1] != 255)
    return 28;
  const uint8_t packed_bgr[3] = {11, 22, 33};
  uint8_t packed_bgra[4] = {0};
  if (vc_repack_bgr((vc_const_plane){packed_bgr, 3}, (vc_plane){packed_bgra, 4}, VC_U8, 3, 4, 255,
                    (vc_rows){1, 1, 0, 1}) != VC_OK ||
      packed_bgra[3] != 255 || packed_bgra[2] != 33)
    return 29;
  uint8_t yuy2_gray[4] = {17, 33, 99, 77}, luma[2] = {0};
  if (vc_extract_yuy2_luma((vc_const_plane){yuy2_gray, 4}, (vc_plane){luma, 2}, (vc_rows){2, 1, 0, 1}) != VC_OK ||
      vc_neutralize_yuy2_chroma((vc_plane){yuy2_gray, 4}, (vc_rows){2, 1, 0, 1}) != VC_OK || luma[0] != 17 ||
      luma[1] != 99 || yuy2_gray[0] != 17 || yuy2_gray[1] != 128 || yuy2_gray[2] != 99 || yuy2_gray[3] != 128)
    return 30;
  return 0;
}
