# AviSynth — Convert Video

[English](README.md) | **简体中文** | [日本語](README.ja.md)

AviSynth — Convert Video 是 AviSynthMinus 的独立视频转换与图像重采样模块。它可以脱离 AviSynth 构建，也可以静态链接回核心，提供普通 C 实现和使用 Google Highway 的跨平台 SIMD 内核。

公开接口使用 C 类型和函数，内部使用 C++17，不依赖 AviSynth SDK、AvsCore 或 AvsSimd。

## 为什么拆分视频转换？

将计算内核从帧服务器中分离，可以独立维护接口、数值行为、测试和性能。本模块与 AviSynthMinus 同步演进，通常以固定版本的 Git 子模块集成。

宿主负责剪辑、脚本注册、帧分配、属性、色彩空间解释和调度。库处理显式的缓冲区、转换配置和行范围，为宿主组合滤镜功能提供基础操作，不自行注册 AviSynth 滤镜。

## 支持的操作

| 类别 | 能力 |
|---|---|
| 存储布局 | Packed BGR/BGRA 与 planar RGB/RGBA 转换、alpha 处理、YUY2 打包与解包、亮度提取及色度中和。 |
| 色彩矩阵 | RGB/YUV 与亮度计算，整数和浮点路径，显式范围及矩阵配置。 |
| 位深 | 整数与浮点转换、范围缩放、舍入和饱和处理。 |
| 抖动 | Bayer ordered 抖动和兼容 AviSynth 的蛇形 Floyd–Steinberg 误差扩散。 |
| 重采样 | 横向与纵向滤波、裁剪偏移、显式采样中心和可复用系数计划。 |

存储类型为 U8、U16 和 F32。不同操作支持的有效位深和布局有所不同，精确限制见公开头文件。重采样包含 Point、Triangle、Bicubic、Lanczos、Blackman、Spline16/36/64、Gaussian、Sinc、SinPower、SincLin2 和 UserDefined2。宿主组合这些操作，完成色度子采样、packed 格式处理和完整缩放滤镜。

数值行为以审核后的 C 实现和回归测试为准，不为兼容而重现已经明确修正的上游缺陷。脚本名称相同不代表与所有历史 AviSynth 版本逐字节一致。

## SIMD 与 CPU 限制

`VC_TARGET_C` 选择普通 C；`VC_TARGET_NATIVE` 选择可用的本机实现，必要时回退到 C。各操作的目标查询返回当前 CPU 支持且库已编译的 SIMD 目标；显式请求不可用目标会失败。目标标识使用 Highway 的位值。

分派绑定到计划或函数表，不改变 Highway 的进程级目标限制。AvsSimd 留在宿主，解释 `SetMaxCPU` 并与库提供的目标求交集。`SetMaxCPU("none")` 选择普通 C；非 x86 宿主中，`none` 使用 C，其他设置使用 native 选择。

Floyd–Steinberg 有意保留普通 C 实现，保持 AviSynth 修改版蛇形权重、有符号误差及舍入规则。每个 context 属于一个帧/平面，行段必须按顺序执行，不能并发使用同一 context；独立 context 可以并行。

## 构建与集成

需要 CMake 3.24 或更新版本及支持 C++17 的编译器。接口兼容 C，不暴露 STL 容器或 Highway 向量类型，C++ 异常不会跨越公开边界。升级时使用配套头文件和库；C 接口不承诺不同版本的预编译库可以互换。

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DVC_BUILD_TESTS=ON
cmake --build build/release --config Release --parallel
ctest --test-dir build/release -C Release --output-on-failure
```

测试在宿主未提供 `GTest::gtest_main` 时获取 GoogleTest 1.17.0。只构建库可指定 `-DVC_BUILD_TESTS=OFF`。`-DVC_SCALAR_ONLY=ON` 对整个转换库禁用 SIMD 选择，而不只是影响测试。`-DVC_BUILD_BENCHMARKS=ON` 启用可选基准测试；上游对比目标可能需要外部参考源码。

静态库名为 `VideoConvert`，CMake 别名为 `AviSynth::ConvertVideo`。作为子模块加入后，直接链接目标：

```cmake
add_subdirectory(third_party/video_convert)
target_link_libraries(MyHost PRIVATE AviSynth::ConvertVideo)
```

独立构建使用 vendored Highway 1.4.0。嵌入构建复用已有的兼容 `hwy` 目标，让 Audio、Video 和宿主共用一份运行时。CMake 传播静态链接依赖，宿主无需列举内核源文件。当前支持的是联合 CMake 构建，不是安装式二进制 SDK 包。

公开头文件位于 [include/video_convert](include/video_convert)。stride 是有符号字节数。请遵守各接口的行起点、缓冲区重叠和生命周期契约。大多数计划不可变，可并发写入互不重叠的输出行段；Floyd–Steinberg context 是有状态的例外。重采样可以查询输出行段需要的源行范围。

## 测试与性能

独立测试覆盖系数、C/native 一致性、修正后的数值行为、布局、范围、量化、非整倍数尺寸、有符号 stride、行段和内存边界；C 消费者测试检查公开接口。AviSynthMinus 另行验证公开滤镜和元数据行为。

CI 配置覆盖 Windows x64/ARM64 的 MSVC 与 clang-cl、Linux x64/ARM64、macOS x64/ARM64、纯 C 配置及 Linux ASan/UBSan。

Windows x64 Release 配置包含 **4,199 项测试**，其中包括 C 接口测试。

全部已测结果见 [PERFORMANCE.md](PERFORMANCE.md)：**522 个完整滤镜场景**、**584 行内核比较**及 **108 行长核补充记录**，补充记录与主审计有重叠。下表覆盖全部已测类别。数值为**新 / 上游耗时比**，小于 1 表示更快；中位数是各场景比例的中位数，不是总耗时加速比。极低工作量场景不参与汇总。

| 类别 | 有效场景数 | AVX2 中位数 | AVX2 范围 | Native 中位数 | Native 范围 |
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

环境为 Ryzen 9 7940H、Windows x64、clang-cl 22.1.3 Release，固定逻辑 CPU 12。上游为 `5c82777b374bdef16e13007a11e77d735ac1e4eb`，native 使用各路径可用的 AVX512；本模块 native 使用 Highway `AVX3_ZEN4`。完整滤镜计时包含 GetFrame 和输出分配。[完整表格](PERFORMANCE.md) 提供双方耗时、精确负载、测量版本及输出一致性说明。已测模块输出均匹配 C，部分上游输出存在数值差异。

性能取决于格式、核宽度、CPU 和编译器。长核横向重采样与 Floyd–Steinberg 在部分负载下仍与上游实现存在已知性能差距。

比较时应保持输入、编译选项、CPU 限制和计时范围一致。内核测速与完整滤镜测速不同，更宽的 SIMD 也不保证更快。性能报告应同时提供输出比较和耗时。

## 开发与贡献

维护者负责技术方向、变更审核和发布。欢迎问题报告、建议与贡献；修改数值语义、公开接口或重要架构前，建议先讨论目标和方案。

本项目使用 AI 辅助实现、测试和审查。贡献应说明问题、方案、验证方法和 AI 参与方式。报告问题请提供提交版本、系统、CPU、编译器、构建选项、输入输出格式及最小复现；性能报告还应包含尺寸、CPU 目标和测量方法。

## 致谢与许可证

本模块建立在 AviSynth、AviSynth+、AviSynthMinus 及其贡献者的工作上，使用 Google Highway 提供 SIMD 能力。感谢原作者和参与测试、报告问题及改进的开发者与用户。

感谢 [烧饼论坛](https://sb.sb) 赞助本项目开发使用的 LLM 订阅。

项目采用 GPL 第 2 版或更新版本，保留继承自 AviSynth 的链接例外原文和适用范围，完整条款见 [LICENSE](LICENSE)。源文件保留版权声明，第三方组件遵循各自许可证。提供新的 C 接口不会扩大原有例外的适用范围。
