add_executable(video_layout_bench layout_bench.cpp)
target_compile_features(video_layout_bench PRIVATE cxx_std_17)
target_link_libraries(video_layout_bench PRIVATE AviSynth::ConvertVideo hwy)

# Optional comparison against an unchanged AVS checkout. Never required by the
# library, tests, or default standalone build; do not copy old SIMD into production.
set(VC_AVS_REFERENCE_DIR "" CACHE PATH "AviSynth checkout for old layout SIMD comparisons")
set(VC_AVS_REFERENCE_BUILD_DIR "" CACHE PATH "Configured AviSynth build containing generated avs_core headers")
if(VC_AVS_REFERENCE_DIR)
  if(NOT EXISTS "${VC_AVS_REFERENCE_BUILD_DIR}/avs_core/version.h")
    message(FATAL_ERROR "Set VC_AVS_REFERENCE_BUILD_DIR to a configured AVS build containing avs_core/version.h")
  endif()
  add_library(video_layout_reference STATIC
    reference_support.cpp
    ${VC_AVS_REFERENCE_DIR}/avs_core/convert/intel/convert_rgb_sse.cpp
    ${VC_AVS_REFERENCE_DIR}/avs_core/convert/intel/convert_rgb_avx2.cpp
    ${VC_AVS_REFERENCE_DIR}/avs_core/convert/intel/convert_planar_sse.cpp)
  target_compile_features(video_layout_reference PRIVATE cxx_std_17)
  target_compile_definitions(video_layout_reference PRIVATE BUILDING_AVSCORE INTEL_INTRINSICS)
  target_include_directories(video_layout_reference PRIVATE
    ${VC_AVS_REFERENCE_DIR}/avs_core/include
    ${VC_AVS_REFERENCE_BUILD_DIR}/avs_core
    ${VC_AVS_REFERENCE_DIR}/avs_core
    ${VC_AVS_REFERENCE_DIR}/avs_core/convert)
  if(MSVC)
    target_compile_options(video_layout_reference PRIVATE /Gy)
    set_source_files_properties(${VC_AVS_REFERENCE_DIR}/avs_core/convert/intel/convert_rgb_avx2.cpp
      PROPERTIES COMPILE_OPTIONS /arch:AVX2)
  else()
    set_source_files_properties(${VC_AVS_REFERENCE_DIR}/avs_core/convert/intel/convert_rgb_avx2.cpp
      PROPERTIES COMPILE_OPTIONS "-mavx2")
  endif()
  target_compile_definitions(video_layout_bench PRIVATE VC_BENCH_AVS_REFERENCE)
  target_link_libraries(video_layout_bench PRIVATE video_layout_reference)
endif()

# Optional unmodified upstream snapshot (for example tmp/upstream-performance).
set(VC_AVS_AVX512_REFERENCE_DIR "" CACHE PATH "Upstream source snapshot with AVX-512 layout kernels")
if(VC_AVS_AVX512_REFERENCE_DIR)
  if(NOT TARGET video_layout_reference)
    message(FATAL_ERROR "AVX-512 reference requires VC_AVS_REFERENCE_DIR and its configured headers")
  endif()
  set(avx512_source "${VC_AVS_AVX512_REFERENCE_DIR}/avs_core/convert/intel/convert_rgb_avx512.cpp")
  target_sources(video_layout_reference PRIVATE "${avx512_source}")
  if(MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_source_files_properties("${avx512_source}" PROPERTIES COMPILE_OPTIONS "/arch:AVX512;/clang:-mavx512vbmi")
  elseif(MSVC)
    set_source_files_properties("${avx512_source}" PROPERTIES COMPILE_OPTIONS /arch:AVX512)
  else()
    set_source_files_properties("${avx512_source}" PROPERTIES COMPILE_OPTIONS "-mavx512f;-mavx512bw;-mavx512dq;-mavx512vl;-mavx512vbmi")
  endif()
  target_compile_definitions(video_layout_bench PRIVATE VC_BENCH_AVX512_REFERENCE)
endif()
