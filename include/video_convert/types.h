// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_TYPES_H
#define VIDEO_CONVERT_TYPES_H
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum vc_status { VC_OK = 0, VC_INVALID_ARGUMENT = 1, VC_OUT_OF_MEMORY = 2, VC_INTERNAL_ERROR = 3 };
enum vc_storage { VC_U8 = 1, VC_U16 = 2, VC_F32 = 3 };

#define VC_TARGET_C INT64_C(0)
#define VC_TARGET_NATIVE INT64_C(-1)

typedef struct vc_const_plane {
  const void* data;
  ptrdiff_t stride;
} vc_const_plane;

typedef struct vc_plane {
  void* data;
  ptrdiff_t stride;
} vc_plane;

#ifdef __cplusplus
}
#endif
#endif
