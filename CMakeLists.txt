cmake_minimum_required(VERSION 3.24)
project(AviSynthConvertVideo VERSION 0.1.0 LANGUAGES C CXX)

option(VC_BUILD_TESTS "Build independent video conversion tests" ${PROJECT_IS_TOP_LEVEL})
option(VC_SCALAR_ONLY "Build ordinary C fallback without SIMD targets" OFF)
option(VC_BUILD_BENCHMARKS "Build layout performance comparisons" OFF)

if(NOT TARGET hwy)
  set(HWY_ENABLE_CONTRIB OFF CACHE BOOL "" FORCE)
  set(HWY_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(HWY_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  set(HWY_ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
  set(HWY_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  set(HWY_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)
  set(HWY_FORCE_STATIC_LIBS ON CACHE BOOL "" FORCE)
  set(HWY_CMAKE_RVV OFF CACHE BOOL "" FORCE)
  add_subdirectory(third_party/highway EXCLUDE_FROM_ALL)
endif()

add_library(VideoConvert STATIC
  src/dither/floyd.cpp src/dither/highway.cpp src/dither/ordered.cpp
  src/depth/execute.cpp src/depth/api.cpp src/depth/highway.cpp
  src/layout/repack.cpp src/layout/rgb.cpp src/layout/yuy2.cpp src/layout/highway.cpp
  src/matrix/coefficients.cpp src/matrix/execute.cpp src/matrix/highway.cpp src/matrix/api.cpp
  src/resample/functions.cpp src/resample/coefficients.cpp src/resample/execute.cpp src/resample/api.cpp src/resample/highway.cpp src/resample/packing.cpp)
add_library(AviSynth::ConvertVideo ALIAS VideoConvert)
target_compile_features(VideoConvert PRIVATE cxx_std_17)
target_link_libraries(VideoConvert PRIVATE hwy)
if(VC_SCALAR_ONLY)
  target_compile_definitions(VideoConvert PRIVATE HWY_COMPILE_ONLY_SCALAR)
endif()
set_target_properties(VideoConvert PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(VideoConvert PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(VC_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
if(VC_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()

# Preserve separate multiply/add rounding in the C baseline and Highway kernels.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  if(MSVC)
    set_source_files_properties(src/dither/floyd.cpp src/dither/highway.cpp src/dither/ordered.cpp src/depth/highway.cpp src/depth/execute.cpp src/resample/execute.cpp src/resample/highway.cpp src/matrix/coefficients.cpp src/matrix/execute.cpp src/matrix/highway.cpp PROPERTIES COMPILE_OPTIONS /clang:-ffp-contract=off)
  else()
    set_source_files_properties(src/dither/floyd.cpp src/dither/highway.cpp src/dither/ordered.cpp src/depth/highway.cpp src/depth/execute.cpp src/resample/execute.cpp src/resample/highway.cpp src/matrix/coefficients.cpp src/matrix/execute.cpp src/matrix/highway.cpp PROPERTIES COMPILE_OPTIONS -ffp-contract=off)
  endif()
endif()
