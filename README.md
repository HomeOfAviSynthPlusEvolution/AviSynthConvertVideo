# AviSynth — Convert Video

**English** | [简体中文](README.zh-CN.md) | [日本語](README.ja.md)

AviSynth — Convert Video is AviSynthMinus's independent video conversion and image resampling module. It builds without AviSynth and integrates into the core through static linking. It provides ordinary C implementations and cross-platform SIMD kernels using Google Highway.

The public interface uses C types and functions; the implementation uses C++17. It has no dependency on the AviSynth SDK, AvsCore, or AvsSimd.

## Why separate video conversion?

Separating computational kernels from the frameserver allows their interfaces, numerical behavior, tests, and performance to be maintained independently. The module evolves alongside AviSynthMinus and is normally included as a pinned Git submodule.

The host owns clips, script registration, frame allocation, properties, colorspace interpretation, and scheduling. The library operates on explicit buffers, conversion configurations, and row ranges. It supplies building blocks for the host's filter matrix, rather than registering AviSynth filters itself.

## Supported operations

| Area | Capabilities |
|---|---|
| Storage layouts | Packed BGR/BGRA and planar RGB/RGBA conversion, alpha handling, YUY2 packing/unpacking, luma extraction and chroma neutralization. |
| Color matrices | RGB/YUV and luma calculations, integer and floating-point paths, explicit range and matrix configuration. |
| Bit depth | Integer and floating-point conversion, range scaling, rounding and saturation. |
| Dithering | Bayer ordered dithering and AviSynth-compatible serpentine Floyd–Steinberg error diffusion. |
| Resampling | Horizontal and vertical filtering, crop offsets, explicit sample centers, and reusable coefficient plans. |

Storage types are U8, U16, and F32. Supported effective bit depths and layouts depend on the operation; the public headers document their exact restrictions. Resampling provides Point, Triangle, Bicubic, Lanczos, Blackman, Spline16/36/64, Gaussian, Sinc, SinPower, SincLin2, and UserDefined2 filters. The host composes these operations for chroma subsampling, packed formats, and complete resize filters.

Numerical behavior follows the reviewed C implementations and regression tests. Deliberately corrected upstream defects are not reproduced merely for compatibility. Matching a script name does not promise byte-identical results with every historical AviSynth version.

## SIMD and CPU restrictions

`VC_TARGET_C` selects ordinary C. `VC_TARGET_NATIVE` selects an available native implementation, falling back to C when necessary. Operation-specific target queries return compiled SIMD targets supported by the running CPU; explicit unavailable target requests fail. Targets use Highway bit values.

Selection is bound to a plan or function table and does not modify Highway's process-wide target restrictions. AvsSimd stays in the host, where it interprets `SetMaxCPU` and intersects the permitted targets with the library's available targets. `SetMaxCPU("none")` selects ordinary C. On non-x86 hosts, the integration uses C for `none` and native selection otherwise.

Floyd–Steinberg intentionally uses ordinary C. Its signed-error recurrence preserves AviSynth's modified serpentine weights and rounding. A context belongs to one frame/plane: row bands must arrive in order and must not execute concurrently on that context. Independent contexts can run concurrently.

## Building and integration

CMake 3.24 or later and a C++17 compiler are required. The interface is C-compatible and exposes no STL containers or Highway vector types. C++ exceptions do not cross the public boundary. Use matching headers and libraries: the C interface does not promise binary interchangeability between releases.

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DVC_BUILD_TESTS=ON
cmake --build build/release --config Release --parallel
ctest --test-dir build/release -C Release --output-on-failure
```

Tests fetch GoogleTest 1.17.0 unless the embedding project already provides `GTest::gtest_main`. Disable them with `-DVC_BUILD_TESTS=OFF` for a library-only build. `-DVC_SCALAR_ONLY=ON` disables SIMD selection for the whole conversion library, not just its tests. Optional benchmarks use `-DVC_BUILD_BENCHMARKS=ON`; upstream comparison targets may require an external reference checkout.

The static library is `VideoConvert`; its CMake alias is `AviSynth::ConvertVideo`. After adding the repository as a submodule, link the target directly:

```cmake
add_subdirectory(third_party/video_convert)
target_link_libraries(MyHost PRIVATE AviSynth::ConvertVideo)
```

Standalone builds use vendored Highway 1.4.0. Embedded builds reuse an existing compatible `hwy` target so Audio, Video, and the host can share one runtime. CMake propagates static link dependencies; consumers do not enumerate kernel sources. The supported integration is a joint CMake build, not an installed binary SDK package.

Public headers are under [include/video_convert](include/video_convert). Strides are signed byte counts. Follow each API's row-origin, buffer-overlap, and lifetime contracts. Most plans are immutable and support concurrent execution into disjoint output bands; Floyd–Steinberg contexts are the stateful exception. Resampling can report the source-row envelope required by an output band.

## Testing and performance

Independent tests cover coefficients, C/native equivalence, corrected numerical behavior, packing, ranges, quantization, irregular sizes, signed strides, row bands, and memory boundaries. A C consumer checks the public interface. AviSynthMinus separately tests public filters and metadata behavior.

The CI workflow covers Windows x64 and ARM64 with MSVC and clang-cl, Linux x64 and ARM64, macOS x64 and ARM64, a C-only configuration, and Linux ASan/UBSan.

The Windows x64 Release configuration contains **4,199 tests**, including a C interface test.

All measured results are in [PERFORMANCE.md](PERFORMANCE.md): **522 full-filter cases**, **584 kernel comparison rows**, and **108 supplementary long-support rows**. Supplementary profiles overlap the main audit. The following table covers every measured family. Ratios are **new / upstream time**; below 1 means faster. Medians are case-ratio medians, not a total-runtime speedup. Low-work cases are excluded.

| Family | Nontrivial cases | AVX2 median | AVX2 range | Native median | Native range |
|---|---|---|---|---|---|
| chroma | 25 | 1.004 | 0.647–1.390 | 0.943 | 0.692–1.461 |
| depth | 132 | 1.141 | 0.908–1.506 | 0.994 | 0.837–1.204 |
| depth-alpha | 13 | 1.067 | 0.976–3.307 | 1.028 | 0.904–2.917 |
| floyd | 17 | 2.154 | 1.612–2.849 | 2.113 | 1.592–2.739 |
| greyscale | 21 | 0.940 | 0.293–1.297 | 0.962 | 0.291–1.304 |
| interlaced | 9 | 1.007 | 0.215–1.168 | 1.081 | 0.227–1.352 |
| layout | 23 | 0.955 | 0.126–1.820 | 1.083 | 0.108–1.854 |
| luma | 15 | 0.906 | 0.598–1.047 | 0.869 | 0.592–1.325 |
| matrix-filter | 54 | 1.056 | 0.730–1.834 | 0.974 | 0.610–1.460 |
| ordered | 16 | 1.151 | 0.735–1.705 | 0.967 | 0.630–1.323 |
| resize | 144 | 1.028 | 0.284–2.875 | 0.960 | 0.406–1.770 |
| resize-composed | 19 | 1.008 | 0.599–1.644 | 0.879 | 0.475–1.068 |
| yuy2 | 11 | 0.894 | 0.249–1.250 | 0.891 | 0.224–1.222 |

Ryzen 9 7940H, Windows x64, clang-cl 22.1.3 Release, pinned logical CPU 12. Upstream reference: `5c82777b374bdef16e13007a11e77d735ac1e4eb`, native AVX512 where available; module native uses Highway `AVX3_ZEN4`. Full-filter times include GetFrame and output allocation. [Complete tables](PERFORMANCE.md) provide both times, exact workloads, versions, and output-equality qualifications. All measured module outputs match C; some upstream outputs differ numerically.

Performance depends on format, support width, CPU, and compiler. Long-support horizontal resampling and Floyd–Steinberg retain known performance gaps against upstream implementations on some workloads.

Compare equivalent inputs, compiler options, CPU restrictions, and timing scopes. Kernel-only timings and complete-filter timings are different measurements. Wider SIMD targets do not guarantee higher speed. Performance reports should include output comparisons as well as timings.

## Development and contributions

The maintainer directs development, reviews changes, and is responsible for releases. Bug reports, suggestions, and contributions are welcome. Discuss numerical semantics, public interface changes, and substantial architectural changes before implementation.

This project uses AI-assisted implementation, tests, and review. Contributions should explain the problem, approach, validation, and how AI was involved. Reports should include the commit, OS, CPU, compiler, build options, input/output formats, and a minimal reproducer; performance reports should also describe dimensions, CPU targets, and the measurement method.

## Acknowledgments and license

This module builds on AviSynth, AviSynth+, AviSynthMinus, and their contributors, and uses Google Highway for SIMD. Thanks to the original authors and everyone contributing tests, reports, and improvements.

Thanks to [SB.SB](https://sb.sb) for sponsoring the LLM subscription used in this project's development.

The project uses GPL version 2 or later with the inherited AviSynth linking exception, retaining its original wording and scope. See [LICENSE](LICENSE). Source files retain their copyright notices; third-party components have their own licenses. Providing a new C interface does not expand the inherited exception.
