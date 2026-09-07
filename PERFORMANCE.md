# Video conversion benchmark results

All saved comparison rows are included: **522 full-filter cases**, **584 kernel rows**, and **108 supplementary long-support resampling rows** (54 profiles × two targets). The supplementary set overlaps the filter audit. Every potentially affected filter family and all resampling kernel profiles have been remeasured after the long-support fix; current tables contain no pre-fix resampling timings. Missing upstream counterparts are explicitly marked, not counted as wins. Coverage is the measured workload set, not every possible parameter combination. Times are milliseconds.

## Reference and method

- Current resampling refresh: module `baa1cdfd3d8fe6c5fe8cf4289993c17a01a6242d`, host `bda0aab8bd9b366da41e946d41236735bae839af`. Production kernels are unchanged from `aab7884`. Upstream reference: `5c82777b374bdef16e13007a11e77d735ac1e4eb`.
- Ryzen 9 7940H, Windows x64, clang-cl 22.1.3 Release. All benchmark processes pinned to logical CPU 12 (0x1000). No concurrent build or test work during timing.
- All 273 full-filter cases in resize, resize-composed, chroma, matrix-filter, interlaced, and yuy2 were remeasured on both DLLs under AVX2 and native, three observations per side/target. Side order and target order alternate between rounds. This includes conservative coverage beyond paths that necessarily execute the changed horizontal branch. The other 249 cases retain valid measurements because their implementations do not call the changed resampling code.
- All 126 resampling kernel profiles (252 target comparison rows) were rebuilt and remeasured three times, including vertical controls, horizontal axes, non-integer ratios, and two-axis pipelines. This harness measures new AVX2/native then upstream AVX2/AVX512 per profile in a fixed order. All other kernel rows retain their previous valid measurements.
- AVX2 uses SetMaxCPU("avx2") on both DLLs. Native uses available upstream AVX512 where implemented and module Highway AVX3_ZEN4. Floyd–Steinberg remains C.
- Full-filter timing includes GetFrame processing and output allocation, excluding input generation, construction, and output hashing. A deterministic source is reused; increasing output frame numbers avoid output-cache hits. No Prefetch. Each observation is the median of five calibrated samples of approximately 15 ms, with 2–100 calls per sample. Three observations are reduced to a median separately for each side. These are warm single-thread measurements, not multithread throughput or constructor benchmarks.
- Resampling kernel timing excludes allocation and coefficient construction; each observation is the median of five calibrated samples. New output is checked against C before timing. Kernel and full-filter timing scopes must not be mixed.
- Retained non-resampling full-filter observations have one or three rounds, recorded per row. Retained matrix U8 kernels have five rounds, other matrix/depth/ordered comparisons have three, and layout uses its saved five-sample harness. Their source version is module 6984ce1 / host 287cd4f4; relevant implementations are unchanged. The supplementary 54-profile long-support set uses aab7884 / f9c088cc, also unchanged since measurement.
- Every refreshed new full-filter hash matches its new none=C baseline in all rounds. Every refreshed new kernel output matches C exactly on the timed inputs, including F32. Upstream outputs can differ due to historical rounding/clipping and corrected semantics; old/new equality is recorded separately. Hashes cover active pixels, excluding padding and frame properties.
- Cases with either side below 0.005 ms are retained but excluded from aggregate ratios. Ratios are new/upstream elapsed time; below 1 is faster. Medians are medians of case ratios, not a total-runtime speedup. Small differences are not established gains or regressions.

## Full-filter summary

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

## Complete full-filter comparisons

Expand each family. CPU cells are **upstream ms / new ms / ratio**; low-work ratios are omitted. Equality is **AVX2 / native** versus upstream. All new outputs match C.

<details>
<summary>chroma — 36 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| chroma-108 | YV24 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0002 / low-work | True / True | 3 / 3 |
| chroma-109 | YV24 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 1.4085 / 1.6222 / 1.152 | 1.0270 / 1.2096 / 1.178 | True / True | 3 / 3 |
| chroma-110 | YV24 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 1.6652 / 1.9535 / 1.173 | 1.4305 / 1.5208 / 1.063 | True / True | 3 / 3 |
| chroma-111 | YV16 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.0660 / 2.1382 / 0.697 | 1.2413 / 1.3981 / 1.126 | True / True | 3 / 3 |
| chroma-112 | YV16 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-113 | YV16 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.3727 / 0.5182 / 1.390 | 0.5286 / 0.4419 / 0.836 | True / True | 3 / 3 |
| chroma-114 | YV12 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 2.2820 / 2.0940 / 0.918 | 1.5901 / 1.5856 / 0.997 | True / True | 3 / 3 |
| chroma-115 | YV12 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.4294 / 0.5778 / 1.346 | 0.5857 / 0.5351 / 0.914 | True / True | 3 / 3 |
| chroma-116 | YV12 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0002 / low-work | True / True | 3 / 3 |
| chroma-117 | YV411 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.3337 / 2.2582 / 0.677 | 1.2527 / 1.3689 / 1.093 | True / True | 3 / 3 |
| chroma-118 | YV411 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 1.7955 / 1.1608 / 0.647 | 0.6343 / 0.6871 / 1.083 | True / True | 3 / 3 |
| chroma-119 | YV411 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 2.0725 / 1.5092 / 0.728 | 1.0809 / 1.0197 / 0.943 | True / True | 3 / 3 |
| chroma-120 | YUV444P16 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-121 | YUV444P16 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 1.8694 / 1.9615 / 1.049 | 1.7320 / 1.6919 / 0.977 | True / True | 3 / 3 |
| chroma-122 | YUV444P16 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 2.3940 / 2.3389 / 0.977 | 2.2615 / 1.9350 / 0.856 | True / True | 3 / 3 |
| chroma-123 | YUV422P16 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.5540 / 2.5957 / 0.730 | 1.4265 / 1.7394 / 1.219 | True / True | 3 / 3 |
| chroma-124 | YUV422P16 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-125 | YUV422P16 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.6135 / 0.7020 / 1.144 | 0.6715 / 0.5793 / 0.863 | True / True | 3 / 3 |
| chroma-126 | YUV420P16 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 2.9570 / 2.4317 / 0.822 | 1.9219 / 1.7586 / 0.915 | True / True | 3 / 3 |
| chroma-127 | YUV420P16 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.7285 / 0.6959 / 0.955 | 0.6793 / 0.6212 / 0.914 | True / True | 3 / 3 |
| chroma-128 | YUV420P16 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-129 | YUV444PS | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-130 | YUV444PS | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 3.0745 / 3.4803 / 1.132 | 4.4746 / 3.2906 / 0.735 | False / False | 3 / 3 |
| chroma-131 | YUV444PS | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 3.6483 / 4.0695 / 1.115 | 5.0919 / 4.0321 / 0.792 | False / False | 3 / 3 |
| chroma-132 | YUV422PS | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.4198 / 3.4506 / 1.009 | 4.2501 / 2.9420 / 0.692 | False / False | 3 / 3 |
| chroma-133 | YUV422PS | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-134 | YUV422PS | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 1.2223 / 1.3481 / 1.103 | 1.2954 / 1.3062 / 1.008 | False / False | 3 / 3 |
| chroma-135 | YUV420PS | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.4057 / 3.5330 / 1.037 | 3.9281 / 3.4179 / 0.870 | False / False | 3 / 3 |
| chroma-136 | YUV420PS | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 1.1525 / 1.2015 / 1.043 | 1.1508 / 1.2348 / 1.073 | False / False | 3 / 3 |
| chroma-137 | YUV420PS | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| extra-044 | YUV420P10 | 1920×1080 | `src.ConvertToYUV444(chromaresample="lanczos")` | 2.8778 / 2.6447 / 0.919 | 1.7372 / 1.8587 / 1.070 | True / True | 3 / 3 |
| extra-045 | YUV420P10 | 1920×1080 | `src.ConvertToYUV420(chromaresample="bilinear")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| extra-046 | YUV422P12 | 1920×1080 | `src.ConvertToYUV444(chromaresample="lanczos")` | 3.3453 / 2.7115 / 0.811 | 1.2608 / 1.8420 / 1.461 | True / True | 3 / 3 |
| extra-047 | YUV422P12 | 1920×1080 | `src.ConvertToYUV420(chromaresample="bilinear")` | 0.4055 / 0.4071 / 1.004 | 0.5115 / 0.3796 / 0.742 | True / True | 3 / 3 |
| extra-048 | YUV444P14 | 1920×1080 | `src.ConvertToYUV444(chromaresample="lanczos")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| extra-049 | YUV444P14 | 1920×1080 | `src.ConvertToYUV420(chromaresample="bilinear")` | 2.0774 / 1.7928 / 0.863 | 1.3794 / 1.0230 / 0.742 | True / True | 3 / 3 |

</details>

<details>
<summary>depth — 132 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| depth-146 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.1876 / 0.2025 / 1.079 | 0.1904 / 0.1812 / 0.952 | True / True | 1 / 1 |
| depth-147 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=false,dither=-1)` | 0.1773 / 0.1669 / 0.941 | 0.1784 / 0.1511 / 0.847 | True / True | 1 / 1 |
| depth-148 | Y8 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=-1)` | 0.1653 / 0.1683 / 1.018 | 0.1658 / 0.1455 / 0.877 | True / True | 1 / 1 |
| depth-149 | Y8 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=false,dither=-1)` | 0.0674 / 0.0732 / 1.086 | 0.0696 / 0.0679 / 0.975 | True / True | 3 / 3 |
| depth-150 | Y8 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=-1)` | 0.1734 / 0.2031 / 1.172 | 0.1805 / 0.1672 / 0.926 | True / True | 3 / 3 |
| depth-151 | Y8 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=false,dither=-1)` | 0.1657 / 0.1681 / 1.015 | 0.1684 / 0.1452 / 0.862 | True / True | 1 / 1 |
| depth-152 | Y8 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=true,dither=-1)` | 0.1651 / 0.1677 / 1.016 | 0.1658 / 0.1452 / 0.876 | True / True | 1 / 1 |
| depth-153 | Y8 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=false,dither=-1)` | 0.0630 / 0.0774 / 1.229 | 0.0680 / 0.0682 / 1.003 | True / True | 3 / 3 |
| depth-154 | Y8 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=true,dither=-1)` | 0.1743 / 0.2046 / 1.174 | 0.1781 / 0.1674 / 0.940 | True / True | 3 / 3 |
| depth-155 | Y8 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=false,dither=-1)` | 0.1655 / 0.1675 / 1.012 | 0.1657 / 0.1453 / 0.877 | True / True | 1 / 1 |
| depth-156 | Y8 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=true,dither=-1)` | 0.1670 / 0.1674 / 1.003 | 0.1661 / 0.1464 / 0.881 | True / True | 1 / 1 |
| depth-157 | Y8 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=false,dither=-1)` | 0.0676 / 0.0736 / 1.090 | 0.0620 / 0.0646 / 1.042 | True / True | 1 / 1 |
| depth-158 | Y8 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=true,dither=-1)` | 0.1726 / 0.2043 / 1.183 | 0.1751 / 0.1673 / 0.956 | True / True | 3 / 3 |
| depth-159 | Y8 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=false,dither=-1)` | 0.1659 / 0.1673 / 1.009 | 0.1671 / 0.1464 / 0.876 | True / True | 1 / 1 |
| depth-160 | Y8 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=true,dither=-1)` | 0.0716 / 0.0765 / 1.068 | 0.0701 / 0.0636 / 0.907 | True / True | 1 / 1 |
| depth-161 | Y8 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=false,dither=-1)` | 0.0716 / 0.0733 / 1.024 | 0.0785 / 0.0680 / 0.865 | True / True | 3 / 3 |
| depth-162 | Y8 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.1729 / 0.2037 / 1.178 | 0.1798 / 0.1678 / 0.933 | True / True | 3 / 3 |
| depth-163 | Y8 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=false,dither=-1)` | 0.1654 / 0.1674 / 1.012 | 0.1675 / 0.1458 / 0.870 | True / True | 1 / 1 |
| depth-164 | Y8 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=true,dither=-1)` | 0.1216 / 0.1709 / 1.406 | 0.1306 / 0.1268 / 0.971 | True / True | 3 / 3 |
| depth-165 | Y8 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=false,dither=-1)` | 0.1305 / 0.1853 / 1.420 | 0.1333 / 0.1337 / 1.003 | False / False | 3 / 3 |
| depth-166 | Y8 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 0.1272 / 0.1883 / 1.480 | 0.1336 / 0.1346 / 1.007 | True / True | 3 / 3 |
| depth-167 | Y8 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=false,dither=-1)` | 0.1183 / 0.1782 / 1.506 | 0.1373 / 0.1339 / 0.975 | False / False | 3 / 3 |
| depth-168 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=-1)` | 0.1602 / 0.1675 / 1.045 | 0.1607 / 0.1552 / 0.966 | True / True | 1 / 1 |
| depth-169 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=false,dither=-1)` | 0.0661 / 0.0819 / 1.239 | 0.0667 / 0.0752 / 1.128 | True / True | 3 / 3 |
| depth-170 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.1734 / 0.2038 / 1.175 | 0.1864 / 0.1811 / 0.972 | True / True | 3 / 3 |
| depth-171 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=false,dither=-1)` | 0.1628 / 0.1680 / 1.032 | 0.1627 / 0.1523 / 0.936 | True / True | 1 / 1 |
| depth-172 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=-1)` | 0.1585 / 0.2065 / 1.303 | 0.1605 / 0.1700 / 1.060 | True / True | 3 / 3 |
| depth-173 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=false,dither=-1)` | 0.1529 / 0.1705 / 1.115 | 0.1552 / 0.1472 / 0.948 | True / True | 1 / 1 |
| depth-174 | Y10 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=true,dither=-1)` | 0.1524 / 0.1696 / 1.113 | 0.1509 / 0.1470 / 0.974 | True / True | 1 / 1 |
| depth-175 | Y10 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=false,dither=-1)` | 0.0846 / 0.0882 / 1.042 | 0.0884 / 0.0955 / 1.080 | True / True | 1 / 1 |
| depth-176 | Y10 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=true,dither=-1)` | 0.1601 / 0.2100 / 1.312 | 0.1632 / 0.1694 / 1.038 | True / True | 3 / 3 |
| depth-177 | Y10 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=false,dither=-1)` | 0.1507 / 0.1695 / 1.124 | 0.1515 / 0.1467 / 0.968 | True / True | 1 / 1 |
| depth-178 | Y10 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=true,dither=-1)` | 0.1597 / 0.1703 / 1.067 | 0.1504 / 0.1481 / 0.985 | True / True | 1 / 1 |
| depth-179 | Y10 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=false,dither=-1)` | 0.0880 / 0.0875 / 0.995 | 0.0851 / 0.0909 / 1.069 | True / True | 1 / 1 |
| depth-180 | Y10 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=true,dither=-1)` | 0.1589 / 0.2050 / 1.290 | 0.1679 / 0.1712 / 1.019 | True / True | 3 / 3 |
| depth-181 | Y10 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=false,dither=-1)` | 0.1513 / 0.1698 / 1.122 | 0.1524 / 0.1468 / 0.963 | True / True | 1 / 1 |
| depth-182 | Y10 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=true,dither=-1)` | 0.1494 / 0.1717 / 1.149 | 0.1524 / 0.1458 / 0.956 | True / True | 1 / 1 |
| depth-183 | Y10 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=false,dither=-1)` | 0.0863 / 0.0956 / 1.108 | 0.0887 / 0.0910 / 1.026 | True / True | 3 / 3 |
| depth-184 | Y10 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.1584 / 0.2083 / 1.315 | 0.1638 / 0.1706 / 1.042 | True / True | 3 / 3 |
| depth-185 | Y10 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=false,dither=-1)` | 0.1505 / 0.1738 / 1.155 | 0.1541 / 0.1473 / 0.956 | True / True | 3 / 3 |
| depth-186 | Y10 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=true,dither=-1)` | 0.1619 / 0.2286 / 1.412 | 0.1626 / 0.1442 / 0.887 | True / True | 3 / 3 |
| depth-187 | Y10 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=false,dither=-1)` | 0.1627 / 0.2137 / 1.313 | 0.1614 / 0.1623 / 1.005 | False / False | 3 / 3 |
| depth-188 | Y10 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 0.1908 / 0.2131 / 1.117 | 0.1519 / 0.1659 / 1.093 | True / True | 1 / 1 |
| depth-189 | Y10 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=false,dither=-1)` | 0.1566 / 0.1962 / 1.253 | 0.1681 / 0.1520 / 0.904 | False / False | 3 / 3 |
| depth-190 | Y12 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=-1)` | 0.1607 / 0.1678 / 1.044 | 0.1650 / 0.1523 / 0.923 | True / True | 1 / 1 |
| depth-191 | Y12 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=false,dither=-1)` | 0.0687 / 0.0808 / 1.175 | 0.0685 / 0.0721 / 1.053 | True / True | 3 / 3 |
| depth-192 | Y12 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.1725 / 0.2072 / 1.201 | 0.1807 / 0.1816 / 1.005 | True / True | 3 / 3 |
| depth-193 | Y12 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=false,dither=-1)` | 0.1607 / 0.1712 / 1.065 | 0.1619 / 0.1518 / 0.938 | True / True | 1 / 1 |
| depth-194 | Y12 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=-1)` | 0.1513 / 0.1701 / 1.124 | 0.1507 / 0.1467 / 0.973 | True / True | 1 / 1 |
| depth-195 | Y12 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=false,dither=-1)` | 0.0893 / 0.0859 / 0.962 | 0.0858 / 0.0895 / 1.043 | False / False | 1 / 1 |
| depth-196 | Y12 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=-1)` | 0.1598 / 0.2076 / 1.300 | 0.1646 / 0.1702 / 1.034 | True / True | 3 / 3 |
| depth-197 | Y12 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=false,dither=-1)` | 0.1499 / 0.1711 / 1.142 | 0.1507 / 0.1479 / 0.981 | True / True | 1 / 1 |
| depth-198 | Y12 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=true,dither=-1)` | 0.1587 / 0.2071 / 1.305 | 0.1664 / 0.1706 / 1.025 | True / True | 3 / 3 |
| depth-199 | Y12 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=false,dither=-1)` | 0.1542 / 0.1707 / 1.107 | 0.1504 / 0.1482 / 0.985 | True / True | 1 / 1 |
| depth-200 | Y12 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=true,dither=-1)` | 0.1498 / 0.1705 / 1.138 | 0.1499 / 0.1481 / 0.989 | True / True | 1 / 1 |
| depth-201 | Y12 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=false,dither=-1)` | 0.0890 / 0.0883 / 0.992 | 0.0975 / 0.0944 / 0.968 | True / True | 3 / 3 |
| depth-202 | Y12 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=true,dither=-1)` | 0.1620 / 0.2047 / 1.263 | 0.1665 / 0.1716 / 1.031 | True / True | 3 / 3 |
| depth-203 | Y12 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=false,dither=-1)` | 0.1508 / 0.1731 / 1.148 | 0.1556 / 0.1474 / 0.947 | True / True | 3 / 3 |
| depth-204 | Y12 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=true,dither=-1)` | 0.1496 / 0.1699 / 1.136 | 0.1499 / 0.1463 / 0.976 | True / True | 1 / 1 |
| depth-205 | Y12 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=false,dither=-1)` | 0.0840 / 0.0866 / 1.032 | 0.0892 / 0.0922 / 1.033 | True / True | 1 / 1 |
| depth-206 | Y12 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.1583 / 0.2042 / 1.290 | 0.1681 / 0.1704 / 1.014 | True / True | 3 / 3 |
| depth-207 | Y12 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=false,dither=-1)` | 0.1491 / 0.1708 / 1.146 | 0.1501 / 0.1500 / 0.999 | True / True | 1 / 1 |
| depth-208 | Y12 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=true,dither=-1)` | 0.1556 / 0.1950 / 1.253 | 0.1597 / 0.1467 / 0.919 | True / True | 3 / 3 |
| depth-209 | Y12 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=false,dither=-1)` | 0.1581 / 0.2063 / 1.305 | 0.1666 / 0.1620 / 0.973 | False / False | 3 / 3 |
| depth-210 | Y12 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 0.1579 / 0.2037 / 1.290 | 0.1680 / 0.1684 / 1.002 | True / True | 3 / 3 |
| depth-211 | Y12 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=false,dither=-1)` | 0.1588 / 0.1910 / 1.203 | 0.1674 / 0.1443 / 0.862 | False / False | 3 / 3 |
| depth-212 | Y14 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=-1)` | 0.1631 / 0.1674 / 1.026 | 0.1631 / 0.1515 / 0.929 | True / True | 1 / 1 |
| depth-213 | Y14 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=false,dither=-1)` | 0.0638 / 0.0828 / 1.297 | 0.0654 / 0.0713 / 1.090 | True / True | 3 / 3 |
| depth-214 | Y14 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.1714 / 0.2065 / 1.204 | 0.1782 / 0.1814 / 1.018 | True / True | 3 / 3 |
| depth-215 | Y14 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=false,dither=-1)` | 0.1608 / 0.1667 / 1.037 | 0.1617 / 0.1517 / 0.938 | True / True | 1 / 1 |
| depth-216 | Y14 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=-1)` | 0.1503 / 0.1698 / 1.130 | 0.1511 / 0.1458 / 0.965 | True / True | 1 / 1 |
| depth-217 | Y14 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=false,dither=-1)` | 0.0850 / 0.0851 / 1.001 | 0.0868 / 0.0908 / 1.046 | False / False | 1 / 1 |
| depth-218 | Y14 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=-1)` | 0.1599 / 0.2053 / 1.284 | 0.1622 / 0.1725 / 1.063 | True / True | 3 / 3 |
| depth-219 | Y14 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=false,dither=-1)` | 0.1514 / 0.1709 / 1.129 | 0.1509 / 0.1465 / 0.971 | True / True | 1 / 1 |
| depth-220 | Y14 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=true,dither=-1)` | 0.1671 / 0.1703 / 1.019 | 0.1501 / 0.1466 / 0.977 | True / True | 1 / 1 |
| depth-221 | Y14 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=false,dither=-1)` | 0.0943 / 0.0856 / 0.908 | 0.0874 / 0.0902 / 1.033 | False / False | 1 / 1 |
| depth-222 | Y14 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=true,dither=-1)` | 0.1578 / 0.2044 / 1.295 | 0.1657 / 0.1716 / 1.036 | True / True | 3 / 3 |
| depth-223 | Y14 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=false,dither=-1)` | 0.1539 / 0.1691 / 1.099 | 0.1566 / 0.1463 / 0.934 | True / True | 1 / 1 |
| depth-224 | Y14 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=true,dither=-1)` | 0.1571 / 0.2046 / 1.303 | 0.1670 / 0.1702 / 1.019 | True / True | 3 / 3 |
| depth-225 | Y14 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=false,dither=-1)` | 0.1523 / 0.1725 / 1.133 | 0.1550 / 0.1464 / 0.944 | True / True | 1 / 1 |
| depth-226 | Y14 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=true,dither=-1)` | 0.1501 / 0.1697 / 1.131 | 0.1540 / 0.1473 / 0.956 | True / True | 1 / 1 |
| depth-227 | Y14 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=false,dither=-1)` | 0.0846 / 0.0857 / 1.013 | 0.0874 / 0.1003 / 1.146 | True / True | 1 / 1 |
| depth-228 | Y14 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.1618 / 0.2046 / 1.265 | 0.1638 / 0.1700 / 1.038 | True / True | 3 / 3 |
| depth-229 | Y14 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=false,dither=-1)` | 0.1497 / 0.1698 / 1.134 | 0.1521 / 0.1499 / 0.986 | True / True | 1 / 1 |
| depth-230 | Y14 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=true,dither=-1)` | 0.1597 / 0.1986 / 1.243 | 0.1706 / 0.1429 / 0.837 | True / True | 3 / 3 |
| depth-231 | Y14 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=false,dither=-1)` | 0.1603 / 0.2051 / 1.280 | 0.1671 / 0.1731 / 1.036 | False / False | 3 / 3 |
| depth-232 | Y14 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 0.1582 / 0.2032 / 1.285 | 0.1660 / 0.1686 / 1.016 | True / True | 3 / 3 |
| depth-233 | Y14 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=false,dither=-1)` | 0.1576 / 0.2110 / 1.339 | 0.1686 / 0.1448 / 0.859 | False / False | 3 / 3 |
| depth-234 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=-1)` | 0.1642 / 0.1676 / 1.021 | 0.1615 / 0.1525 / 0.944 | True / True | 1 / 1 |
| depth-235 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=false,dither=-1)` | 0.0668 / 0.0846 / 1.266 | 0.0680 / 0.0725 / 1.067 | True / True | 3 / 3 |
| depth-236 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.1741 / 0.2045 / 1.175 | 0.1827 / 0.1826 / 0.999 | True / True | 3 / 3 |
| depth-237 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=false,dither=-1)` | 0.1623 / 0.1676 / 1.032 | 0.1636 / 0.1517 / 0.927 | True / True | 1 / 1 |
| depth-238 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=-1)` | 0.1504 / 0.1698 / 1.129 | 0.1514 / 0.1463 / 0.966 | True / True | 1 / 1 |
| depth-239 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=false,dither=-1)` | 0.0875 / 0.0942 / 1.077 | 0.0891 / 0.1009 / 1.133 | True / True | 3 / 3 |
| depth-240 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=-1)` | 0.1631 / 0.2057 / 1.261 | 0.1656 / 0.1729 / 1.044 | True / True | 3 / 3 |
| depth-241 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=false,dither=-1)` | 0.1521 / 0.1734 / 1.140 | 0.1562 / 0.1465 / 0.938 | True / True | 3 / 3 |
| depth-242 | Y16 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=true,dither=-1)` | 0.1531 / 0.1701 / 1.111 | 0.1525 / 0.1459 / 0.956 | True / True | 1 / 1 |
| depth-243 | Y16 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=false,dither=-1)` | 0.0865 / 0.0847 / 0.979 | 0.0882 / 0.0928 / 1.053 | True / True | 1 / 1 |
| depth-244 | Y16 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=true,dither=-1)` | 0.1579 / 0.2041 / 1.293 | 0.1710 / 0.1695 / 0.991 | True / True | 3 / 3 |
| depth-245 | Y16 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=false,dither=-1)` | 0.1532 / 0.1704 / 1.112 | 0.1527 / 0.1480 / 0.969 | True / True | 1 / 1 |
| depth-246 | Y16 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=true,dither=-1)` | 0.1494 / 0.1690 / 1.132 | 0.1519 / 0.1462 / 0.962 | True / True | 1 / 1 |
| depth-247 | Y16 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=false,dither=-1)` | 0.0855 / 0.0863 / 1.010 | 0.0902 / 0.0944 / 1.046 | True / True | 3 / 3 |
| depth-248 | Y16 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=true,dither=-1)` | 0.1580 / 0.2068 / 1.309 | 0.1658 / 0.1689 / 1.019 | True / True | 3 / 3 |
| depth-249 | Y16 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=false,dither=-1)` | 0.1497 / 0.1701 / 1.136 | 0.1548 / 0.1471 / 0.950 | True / True | 1 / 1 |
| depth-250 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.1569 / 0.2042 / 1.302 | 0.1661 / 0.1711 / 1.030 | True / True | 3 / 3 |
| depth-251 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=false,dither=-1)` | 0.1518 / 0.1702 / 1.122 | 0.1519 / 0.1459 / 0.961 | True / True | 1 / 1 |
| depth-252 | Y16 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=true,dither=-1)` | 0.1549 / 0.1929 / 1.245 | 0.1652 / 0.1668 / 1.009 | True / True | 3 / 3 |
| depth-253 | Y16 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=false,dither=-1)` | 0.1625 / 0.2036 / 1.253 | 0.1615 / 0.1745 / 1.081 | False / False | 3 / 3 |
| depth-254 | Y16 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 0.1584 / 0.2248 / 1.419 | 0.1601 / 0.1801 / 1.125 | True / True | 3 / 3 |
| depth-255 | Y16 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=false,dither=-1)` | 0.1596 / 0.2046 / 1.282 | 0.1702 / 0.1618 / 0.951 | False / False | 3 / 3 |
| depth-256 | Y32 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=-1)` | 0.1645 / 0.1843 / 1.121 | 0.1646 / 0.1768 / 1.074 | True / True | 1 / 1 |
| depth-257 | Y32 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=false,dither=-1)` | 0.1943 / 0.2156 / 1.110 | 0.1982 / 0.2081 / 1.050 | True / True | 1 / 1 |
| depth-258 | Y32 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.2015 / 0.2200 / 1.092 | 0.2106 / 0.2225 / 1.057 | True / True | 3 / 3 |
| depth-259 | Y32 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=false,dither=-1)` | 0.1752 / 0.1809 / 1.032 | 0.1925 / 0.1920 / 0.998 | True / True | 3 / 3 |
| depth-260 | Y32 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=-1)` | 0.1554 / 0.1855 / 1.194 | 0.1766 / 0.1815 / 1.027 | True / True | 3 / 3 |
| depth-261 | Y32 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=false,dither=-1)` | 0.1870 / 0.2252 / 1.204 | 0.1979 / 0.2192 / 1.107 | True / True | 3 / 3 |
| depth-262 | Y32 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=-1)` | 0.1930 / 0.2221 / 1.151 | 0.2048 / 0.2097 / 1.024 | True / True | 3 / 3 |
| depth-263 | Y32 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=false,dither=-1)` | 0.1619 / 0.1798 / 1.111 | 0.1588 / 0.1717 / 1.081 | True / True | 1 / 1 |
| depth-264 | Y32 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=true,dither=-1)` | 0.1568 / 0.1796 / 1.145 | 0.1646 / 0.1717 / 1.044 | True / True | 1 / 1 |
| depth-265 | Y32 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=false,dither=-1)` | 0.1946 / 0.2328 / 1.196 | 0.2094 / 0.2100 / 1.002 | True / True | 3 / 3 |
| depth-266 | Y32 | 1920×1080 | `src.ConvertBits(12,fulls=false,fulld=true,dither=-1)` | 0.1938 / 0.2201 / 1.136 | 0.1848 / 0.2056 / 1.113 | True / True | 1 / 1 |
| depth-267 | Y32 | 1920×1080 | `src.ConvertBits(12,fulls=true,fulld=false,dither=-1)` | 0.1551 / 0.1814 / 1.169 | 0.1741 / 0.1783 / 1.024 | True / True | 3 / 3 |
| depth-268 | Y32 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=true,dither=-1)` | 0.1583 / 0.1970 / 1.244 | 0.1694 / 0.2000 / 1.181 | True / True | 3 / 3 |
| depth-269 | Y32 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=false,dither=-1)` | 0.1879 / 0.2208 / 1.175 | 0.2078 / 0.2503 / 1.204 | True / True | 3 / 3 |
| depth-270 | Y32 | 1920×1080 | `src.ConvertBits(14,fulls=false,fulld=true,dither=-1)` | 0.1894 / 0.2262 / 1.194 | 0.2151 / 0.2237 / 1.040 | True / True | 3 / 3 |
| depth-271 | Y32 | 1920×1080 | `src.ConvertBits(14,fulls=true,fulld=false,dither=-1)` | 0.1559 / 0.1903 / 1.220 | 0.1794 / 0.1829 / 1.020 | False / False | 3 / 3 |
| depth-272 | Y32 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=true,dither=-1)` | 0.1749 / 0.1830 / 1.047 | 0.1786 / 0.1796 / 1.006 | True / True | 3 / 3 |
| depth-273 | Y32 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=false,dither=-1)` | 0.1904 / 0.2174 / 1.142 | 0.1945 / 0.2205 / 1.133 | True / True | 1 / 1 |
| depth-274 | Y32 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.1895 / 0.2216 / 1.170 | 0.2040 / 0.2195 / 1.076 | True / True | 3 / 3 |
| depth-275 | Y32 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=false,dither=-1)` | 0.1576 / 0.2041 / 1.295 | 0.1792 / 0.1989 / 1.110 | False / False | 3 / 3 |
| depth-276 | Y32 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 0.3489 / 0.3354 / 0.962 | 0.3403 / 0.3201 / 0.941 | True / True | 1 / 1 |
| depth-277 | Y32 | 1920×1080 | `src.ConvertBits(32,fulls=true,fulld=false,dither=-1)` | 0.3469 / 0.3243 / 0.935 | 0.3411 / 0.3367 / 0.987 | True / True | 1 / 1 |

</details>

<details>
<summary>depth-alpha — 13 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| depth-alpha-278 | RGB32 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.7562 / 1.6151 / 2.136 | 0.7961 / 1.5551 / 1.953 | False / False | 3 / 3 |
| depth-alpha-279 | RGB32 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.7599 / 2.4516 / 3.226 | 0.8084 / 2.3522 / 2.910 | False / False | 3 / 3 |
| depth-alpha-280 | RGB32 | 1920×1080 | `src.ConvertToPlanarRGBA().ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 2.3478 / 2.2916 / 0.976 | 2.2197 / 2.1896 / 0.986 | True / True | 1 / 1 |
| depth-alpha-281 | RGB64 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.8169 / 2.5374 / 3.106 | 0.8699 / 2.2902 / 2.633 | False / False | 3 / 3 |
| depth-alpha-282 | RGB64 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.9189 / 3.0389 / 3.307 | 0.9654 / 2.8158 / 2.917 | False / False | 3 / 3 |
| depth-alpha-283 | RGB64 | 1920×1080 | `src.ConvertToPlanarRGBA().ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 3.2226 / 3.1834 / 0.988 | 3.0802 / 2.7834 / 0.904 | True / True | 1 / 1 |
| depth-alpha-284 | RGBAP16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.7985 / 0.8517 / 1.067 | 0.8200 / 0.8000 / 0.976 | True / True | 1 / 1 |
| depth-alpha-285 | RGBAP16 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.9057 / 0.9683 / 1.069 | 0.9232 / 0.9266 / 1.004 | True / True | 1 / 1 |
| depth-alpha-286 | RGBAP16 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 1.4973 / 1.5034 / 1.004 | 1.5440 / 1.5569 / 1.008 | True / True | 1 / 1 |
| depth-alpha-287 | YUVA444P16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.8075 / 0.8600 / 1.065 | 0.8724 / 0.8412 / 0.964 | True / True | 1 / 1 |
| depth-alpha-288 | YUVA444P16 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=-1)` | 0.9049 / 0.9545 / 1.055 | 0.9561 / 0.9826 / 1.028 | True / True | 1 / 1 |
| depth-alpha-289 | YUVA444P16 | 1920×1080 | `src.ConvertBits(32,fulls=false,fulld=true,dither=-1)` | 1.4884 / 1.4908 / 1.002 | 1.4822 / 1.6028 / 1.081 | True / True | 1 / 1 |
| depth-alpha-472 | RGB64 | 3840×2160 | `src.ConvertBits(8)` | 3.3417 / 9.9225 / 2.969 | 3.4261 / 8.8165 / 2.573 | True / True | 3 / 3 |

</details>

<details>
<summary>floyd — 17 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| floyd-306 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=8)` | 5.6087 / 9.6464 / 1.720 | 5.6990 / 9.6798 / 1.699 | True / True | 3 / 3 |
| floyd-307 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=8)` | 6.1734 / 13.6341 / 2.209 | 6.2729 / 13.6930 / 2.183 | True / True | 3 / 3 |
| floyd-308 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=8)` | 5.5793 / 12.0018 / 2.151 | 5.6631 / 12.0061 / 2.120 | True / True | 3 / 3 |
| floyd-309 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=8)` | 6.1718 / 16.9955 / 2.754 | 6.4427 / 16.4715 / 2.557 | True / True | 3 / 3 |
| floyd-310 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=1,dither_bits=10)` | 5.4962 / 11.6288 / 2.116 | 5.6238 / 11.6945 / 2.079 | True / True | 3 / 3 |
| floyd-311 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=1,dither_bits=10)` | 6.0377 / 16.7813 / 2.779 | 6.3431 / 16.9326 / 2.669 | True / True | 3 / 3 |
| floyd-312 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=true,dither=1,dither_bits=10)` | 5.9955 / 11.6479 / 1.943 | 6.1339 / 11.6249 / 1.895 | True / True | 3 / 3 |
| floyd-313 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=1,dither_bits=10)` | 5.8914 / 16.7853 / 2.849 | 6.1708 / 16.9033 / 2.739 | True / True | 3 / 3 |
| floyd-314 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=5)` | 6.0797 / 9.7999 / 1.612 | 6.2431 / 9.9367 / 1.592 | True / True | 3 / 3 |
| floyd-315 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=5)` | 8.1021 / 14.7816 / 1.824 | 8.1141 / 14.5277 / 1.790 | True / True | 3 / 3 |
| floyd-316 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=1,dither_bits=3)` | 6.0278 / 10.1709 / 1.687 | 6.2032 / 10.2028 / 1.645 | True / True | 3 / 3 |
| floyd-317 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=1,dither_bits=3)` | 7.7917 / 14.5558 / 1.868 | 8.1557 / 14.5486 / 1.784 | True / True | 3 / 3 |
| floyd-318 | YUV444P16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=8)` | 16.6596 / 35.9136 / 2.156 | 17.0280 / 35.8910 / 2.108 | True / True | 3 / 3 |
| floyd-319 | YUV444P16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=8)` | 18.5018 / 48.4629 / 2.619 | 19.2703 / 48.4004 / 2.512 | True / True | 3 / 3 |
| floyd-320 | RGBAP16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=8)` | 16.8681 / 36.3633 / 2.156 | 17.2342 / 36.4087 / 2.113 | True / True | 3 / 3 |
| floyd-321 | RGBAP16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=8)` | 18.6826 / 49.9410 / 2.673 | 19.6817 / 50.8556 / 2.584 | True / True | 3 / 3 |
| floyd-471 | Y16 | 3840×2160 | `src.ConvertBits(8,dither=1)` | 21.8065 / 46.9733 / 2.154 | 22.4093 / 48.5817 / 2.168 | True / True | 3 / 3 |

</details>

<details>
<summary>greyscale — 21 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| greyscale-031 | YUY2 | 1920×1080 | `src.Greyscale()` | 0.1855 / 0.1729 / 0.932 | 0.1762 / 0.1760 / 0.998 | True / True | 1 / 1 |
| greyscale-034 | YV16 | 1920×1080 | `src.Greyscale()` | 0.1144 / 0.1126 / 0.984 | 0.1202 / 0.1084 / 0.902 | True / True | 1 / 1 |
| greyscale-036 | RGB24 | 1920×1080 | `src.Greyscale()` | 2.9452 / 0.8669 / 0.294 | 2.9049 / 0.8858 / 0.305 | True / True | 3 / 3 |
| greyscale-038 | RGB32 | 1920×1080 | `src.Greyscale()` | 1.0374 / 1.0222 / 0.985 | 0.9629 / 1.0466 / 1.087 | True / True | 1 / 1 |
| greyscale-040 | RGB48 | 1920×1080 | `src.Greyscale()` | 3.2569 / 1.9939 / 0.612 | 3.2463 / 1.9195 / 0.591 | True / True | 1 / 1 |
| greyscale-042 | RGB64 | 1920×1080 | `src.Greyscale()` | 2.1675 / 2.3243 / 1.072 | 2.1951 / 2.1122 / 0.962 | True / True | 1 / 1 |
| greyscale-044 | RGBP8 | 1920×1080 | `src.Greyscale()` | 1.6155 / 0.4735 / 0.293 | 1.6153 / 0.4703 / 0.291 | True / True | 3 / 3 |
| greyscale-046 | RGBP16 | 1920×1080 | `src.Greyscale()` | 1.7692 / 1.0498 / 0.593 | 1.6700 / 1.0050 / 0.602 | True / True | 1 / 1 |
| greyscale-048 | RGBPS | 1920×1080 | `src.Greyscale()` | 1.9216 / 2.4920 / 1.297 | 1.9675 / 2.4933 / 1.267 | True / True | 3 / 3 |
| greyscale-050 | YV12 | 1920×1080 | `src.Greyscale()` | 0.0700 / 0.0713 / 1.019 | 0.0673 / 0.0689 / 1.023 | True / True | 1 / 1 |
| greyscale-052 | YUV420P16 | 1920×1080 | `src.Greyscale()` | 0.1682 / 0.1605 / 0.954 | 0.1625 / 0.1749 / 1.076 | True / True | 1 / 1 |
| greyscale-054 | YUV420PS | 1920×1080 | `src.Greyscale()` | 0.5696 / 0.5356 / 0.940 | 0.5553 / 0.5781 / 1.041 | True / True | 1 / 1 |
| greyscale-468 | YUY2 | 3840×2160 | `src.Greyscale()` | 1.2583 / 1.2478 / 0.992 | 1.2363 / 1.2205 / 0.987 | True / True | 1 / 1 |
| extra-029 | RGBP10 | 1920×1080 | `src.Greyscale()` | 1.7962 / 1.1236 / 0.626 | 1.9924 / 1.0291 / 0.517 | True / True | 1 / 1 |
| extra-031 | RGBP12 | 1920×1080 | `src.Greyscale()` | 1.9275 / 1.2652 / 0.656 | 1.7628 / 1.0925 / 0.620 | True / True | 1 / 1 |
| extra-033 | RGBP14 | 1920×1080 | `src.Greyscale()` | 1.7066 / 1.0629 / 0.623 | 1.7807 / 1.1423 / 0.642 | True / True | 1 / 1 |
| extra-035 | RGBAP8 | 1920×1080 | `src.Greyscale()` | 1.7611 / 0.5378 / 0.305 | 1.7685 / 0.5522 / 0.312 | True / True | 1 / 1 |
| extra-037 | RGBAP16 | 1920×1080 | `src.Greyscale()` | 1.8680 / 1.4529 / 0.778 | 1.9119 / 1.2368 / 0.647 | True / True | 1 / 1 |
| extra-039 | RGBAPS | 1920×1080 | `src.Greyscale()` | 2.2839 / 2.8387 / 1.243 | 2.3906 / 3.1169 / 1.304 | True / True | 3 / 3 |
| extra-041 | YUVA444P16 | 1920×1080 | `src.Greyscale()` | 0.9061 / 0.9330 / 1.030 | 0.9054 / 0.9499 / 1.049 | True / True | 1 / 1 |
| extra-043 | YUVA444PS | 1920×1080 | `src.Greyscale()` | 1.6466 / 1.6214 / 0.985 | 1.6042 / 1.8745 / 1.168 | True / True | 3 / 3 |

</details>

<details>
<summary>interlaced — 9 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| extra-019 | YUY2 | 1920×1080 | `src.ConvertToYV12(interlaced=true)` | 2.6480 / 0.8004 / 0.302 | 2.6670 / 0.7275 / 0.273 | True / True | 3 / 3 |
| extra-020 | YUY2 | 1920×1080 | `src.ConvertToYV16(interlaced=true)` | 0.4243 / 0.0910 / 0.215 | 0.4174 / 0.0948 / 0.227 | True / True | 3 / 3 |
| extra-021 | YUY2 | 1920×1080 | `src.ConvertToRGB32(interlaced=true)` | 5.4152 / 3.8956 / 0.719 | 3.0095 / 3.2548 / 1.081 | False / False | 3 / 3 |
| extra-022 | YV12 | 1920×1080 | `src.ConvertToYV16(interlaced=true)` | 0.4757 / 0.5433 / 1.142 | 0.5115 / 0.4709 / 0.921 | True / True | 3 / 3 |
| extra-023 | YV12 | 1920×1080 | `src.ConvertToRGB32(interlaced=true)` | 3.3230 / 3.3467 / 1.007 | 2.4924 / 2.8888 / 1.159 | False / False | 3 / 3 |
| extra-024 | YV16 | 1920×1080 | `src.ConvertToYV12(interlaced=true)` | 0.3428 / 0.4004 / 1.168 | 0.4399 / 0.3706 / 0.842 | True / True | 3 / 3 |
| extra-025 | YV16 | 1920×1080 | `src.ConvertToRGB32(interlaced=true)` | 3.6618 / 3.0857 / 0.843 | 1.8858 / 2.5488 / 1.352 | False / False | 3 / 3 |
| extra-026 | RGB32 | 1920×1080 | `src.ConvertToYV12(interlaced=true)` | 3.3151 / 3.5859 / 1.082 | 2.3597 / 2.6237 / 1.112 | True / True | 3 / 3 |
| extra-027 | RGB32 | 1920×1080 | `src.ConvertToYV16(interlaced=true)` | 2.8323 / 3.1666 / 1.118 | 1.9842 / 2.2149 / 1.116 | True / True | 3 / 3 |

</details>

<details>
<summary>layout — 29 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| layout-001 | RGB24 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.1734 / 0.3108 / 1.792 | 0.1930 / 0.1949 / 1.010 | True / True | 3 / 3 |
| layout-002 | RGB24 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.2111 / 0.3722 / 1.763 | 0.2303 / 0.2873 / 1.248 | True / True | 3 / 3 |
| layout-003 | RGB24 | 1920×1080 | `src.ConvertToRGB32()` | 0.1731 / 0.2592 / 1.498 | 0.2126 / 0.3453 / 1.624 | True / True | 3 / 3 |
| layout-004 | RGB32 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.5575 / 0.3492 / 0.626 | 0.4042 / 0.4577 / 1.132 | True / True | 1 / 1 |
| layout-005 | RGB32 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.8304 / 0.6989 / 0.842 | 0.7914 / 0.5733 / 0.724 | True / True | 1 / 1 |
| layout-006 | RGB32 | 1920×1080 | `src.ConvertToRGB24()` | 0.1524 / 0.2716 / 1.782 | 0.1785 / 0.3309 / 1.854 | True / True | 3 / 3 |
| layout-007 | RGBP8 | 1920×1080 | `src.ConvertToRGB24()` | 1.7670 / 0.2233 / 0.126 | 1.7667 / 0.1903 / 0.108 | True / True | 3 / 3 |
| layout-008 | RGBP8 | 1920×1080 | `src.ConvertToRGB32()` | 0.4555 / 0.2998 / 0.658 | 0.4221 / 0.4442 / 1.052 | True / True | 1 / 1 |
| layout-009 | RGBP8 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 1 / 1 |
| layout-010 | RGBP8 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.1477 / 0.1607 / 1.088 | 0.1588 / 0.1659 / 1.045 | True / True | 1 / 1 |
| layout-011 | RGBAP8 | 1920×1080 | `src.ConvertToRGB24()` | 1.7695 / 0.3632 / 0.205 | 1.7875 / 0.2048 / 0.115 | True / True | 3 / 3 |
| layout-012 | RGBAP8 | 1920×1080 | `src.ConvertToRGB32()` | 0.4795 / 0.4419 / 0.921 | 0.4542 / 0.4947 / 1.089 | True / True | 1 / 1 |
| layout-013 | RGBAP8 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| layout-014 | RGBAP8 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 1 / 1 |
| layout-015 | RGB48 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.6491 / 1.1813 / 1.820 | 0.6713 / 0.9719 / 1.448 | True / True | 3 / 3 |
| layout-016 | RGB48 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 1.2804 / 1.3074 / 1.021 | 1.3230 / 1.0610 / 0.802 | True / True | 1 / 1 |
| layout-017 | RGB48 | 1920×1080 | `src.ConvertToRGB64()` | 0.4729 / 0.7580 / 1.603 | 0.4295 / 0.7590 / 1.767 | True / True | 3 / 3 |
| layout-018 | RGB64 | 1920×1080 | `src.ConvertToPlanarRGB()` | 1.5071 / 1.4133 / 0.938 | 1.2303 / 1.0863 / 0.883 | True / True | 1 / 1 |
| layout-019 | RGB64 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 1.6310 / 1.5166 / 0.930 | 1.4925 / 1.2045 / 0.807 | True / True | 1 / 1 |
| layout-020 | RGB64 | 1920×1080 | `src.ConvertToRGB48()` | 0.5851 / 0.8624 / 1.474 | 0.5943 / 0.8497 / 1.430 | True / True | 3 / 3 |
| layout-021 | RGBP16 | 1920×1080 | `src.ConvertToRGB48()` | 2.0026 / 1.0104 / 0.505 | 1.9881 / 0.9013 / 0.453 | True / True | 1 / 1 |
| layout-022 | RGBP16 | 1920×1080 | `src.ConvertToRGB64()` | 1.1720 / 1.1193 / 0.955 | 1.0611 / 1.1857 / 1.117 | True / True | 1 / 1 |
| layout-023 | RGBP16 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 1 / 1 |
| layout-024 | RGBP16 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.5524 / 0.5476 / 0.991 | 0.5434 / 0.6110 / 1.124 | True / True | 1 / 1 |
| layout-025 | RGBAP16 | 1920×1080 | `src.ConvertToRGB48()` | 2.0013 / 0.9674 / 0.483 | 1.9672 / 0.8971 / 0.456 | True / True | 1 / 1 |
| layout-026 | RGBAP16 | 1920×1080 | `src.ConvertToRGB64()` | 1.2382 / 1.1531 / 0.931 | 1.1526 / 1.2477 / 1.083 | True / True | 1 / 1 |
| layout-027 | RGBAP16 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| layout-028 | RGBAP16 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 1 / 1 |
| layout-466 | RGB32 | 3840×2160 | `src.ConvertToRGB24()` | 1.2357 / 1.7546 / 1.420 | 1.2510 / 1.7082 / 1.365 | True / True | 3 / 3 |

</details>

<details>
<summary>luma — 21 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| luma-030 | YUY2 | 1920×1080 | `src.ConvertToY8()` | 0.0739 / 0.0626 / 0.847 | 0.0743 / 0.0688 / 0.926 | True / True | 1 / 1 |
| luma-033 | YV16 | 1920×1080 | `src.ConvertToY8()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| luma-035 | RGB24 | 1920×1080 | `src.ConvertToY()` | 0.4474 / 0.4636 / 1.036 | 0.4386 / 0.5812 / 1.325 | False / False | 3 / 3 |
| luma-037 | RGB32 | 1920×1080 | `src.ConvertToY()` | 1.0472 / 0.6261 / 0.598 | 0.8328 / 0.5553 / 0.667 | False / False | 1 / 1 |
| luma-039 | RGB48 | 1920×1080 | `src.ConvertToY()` | 1.3337 / 1.2800 / 0.960 | 1.1899 / 1.2010 / 1.009 | False / False | 1 / 1 |
| luma-041 | RGB64 | 1920×1080 | `src.ConvertToY()` | 1.9320 / 1.4489 / 0.750 | 1.7956 / 1.3066 / 0.728 | False / False | 3 / 3 |
| luma-043 | RGBP8 | 1920×1080 | `src.ConvertToY()` | 0.2122 / 0.2060 / 0.971 | 0.2115 / 0.2066 / 0.977 | False / False | 1 / 1 |
| luma-045 | RGBP16 | 1920×1080 | `src.ConvertToY()` | 0.4301 / 0.3282 / 0.763 | 0.4031 / 0.3417 / 0.848 | False / False | 1 / 1 |
| luma-047 | RGBPS | 1920×1080 | `src.ConvertToY()` | 0.7560 / 0.7918 / 1.047 | 0.7563 / 0.7504 / 0.992 | False / False | 1 / 1 |
| luma-049 | YV12 | 1920×1080 | `src.ConvertToY()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| luma-051 | YUV420P16 | 1920×1080 | `src.ConvertToY()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| luma-053 | YUV420PS | 1920×1080 | `src.ConvertToY()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| luma-467 | YUY2 | 3840×2160 | `src.ConvertToY8()` | 0.6492 / 0.5927 / 0.913 | 0.6792 / 0.7070 / 1.041 | True / True | 1 / 1 |
| extra-028 | RGBP10 | 1920×1080 | `src.ConvertToY()` | 0.3641 / 0.3394 / 0.932 | 0.3680 / 0.3110 / 0.845 | False / False | 1 / 1 |
| extra-030 | RGBP12 | 1920×1080 | `src.ConvertToY()` | 0.6561 / 0.4361 / 0.665 | 0.3409 / 0.3309 / 0.971 | False / False | 1 / 1 |
| extra-032 | RGBP14 | 1920×1080 | `src.ConvertToY()` | 0.4130 / 0.3695 / 0.895 | 0.3591 / 0.3119 / 0.869 | False / False | 1 / 1 |
| extra-034 | RGBAP8 | 1920×1080 | `src.ConvertToY()` | 0.2369 / 0.2147 / 0.906 | 0.2674 / 0.2060 / 0.770 | False / False | 1 / 1 |
| extra-036 | RGBAP16 | 1920×1080 | `src.ConvertToY()` | 0.5192 / 0.4217 / 0.812 | 0.5235 / 0.3101 / 0.592 | False / False | 1 / 1 |
| extra-038 | RGBAPS | 1920×1080 | `src.ConvertToY()` | 0.9739 / 0.9555 / 0.981 | 1.0173 / 0.8709 / 0.856 | False / False | 1 / 1 |
| extra-040 | YUVA444P16 | 1920×1080 | `src.ConvertToY()` | 0.0005 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| extra-042 | YUVA444PS | 1920×1080 | `src.ConvertToY()` | 0.0005 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |

</details>

<details>
<summary>matrix-filter — 54 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| matrix-filter-055 | RGB24 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.7446 / 0.9709 / 1.304 | 0.7461 / 1.0048 / 1.347 | True / True | 3 / 3 |
| matrix-filter-056 | RGB24 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.2282 / 2.7836 / 1.249 | 1.6689 / 2.0635 / 1.236 | True / True | 3 / 3 |
| matrix-filter-057 | RGB24 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 2.4129 / 3.0752 / 1.274 | 1.9184 / 2.3188 / 1.209 | True / True | 3 / 3 |
| matrix-filter-058 | RGB32 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 1.3379 / 1.2815 / 0.958 | 1.2510 / 1.0543 / 0.843 | True / True | 3 / 3 |
| matrix-filter-059 | RGB32 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.8288 / 2.9742 / 1.051 | 2.0732 / 2.0973 / 1.012 | True / True | 3 / 3 |
| matrix-filter-060 | RGB32 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 2.9937 / 3.1941 / 1.067 | 2.4539 / 2.3280 / 0.949 | True / True | 3 / 3 |
| matrix-filter-061 | RGB48 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 1.7886 / 2.1184 / 1.184 | 1.7760 / 1.8146 / 1.022 | False / False | 3 / 3 |
| matrix-filter-062 | RGB48 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 3.5707 / 4.1018 / 1.149 | 4.0377 / 3.3723 / 0.835 | False / False | 3 / 3 |
| matrix-filter-063 | RGB48 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.9283 / 4.2530 / 1.083 | 4.4995 / 3.5008 / 0.778 | False / False | 3 / 3 |
| matrix-filter-064 | RGB64 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 2.4155 / 2.2199 / 0.919 | 2.2413 / 1.9054 / 0.850 | False / False | 3 / 3 |
| matrix-filter-065 | RGB64 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 4.2674 / 4.1904 / 0.982 | 4.5671 / 3.4156 / 0.748 | False / False | 3 / 3 |
| matrix-filter-066 | RGB64 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 4.7095 / 4.4663 / 0.948 | 5.0869 / 3.5716 / 0.702 | False / False | 3 / 3 |
| matrix-filter-067 | RGBP8 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.4927 / 0.4605 / 0.935 | 0.5091 / 0.4651 / 0.914 | True / True | 3 / 3 |
| matrix-filter-068 | RGBP8 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 1.9739 / 2.1122 / 1.070 | 1.3964 / 1.4459 / 1.035 | True / True | 3 / 3 |
| matrix-filter-069 | RGBP8 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 2.2058 / 2.3624 / 1.071 | 1.6958 / 1.6166 / 0.953 | True / True | 3 / 3 |
| matrix-filter-070 | RGBP10 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.7785 / 0.7967 / 1.023 | 0.8383 / 0.7620 / 0.909 | True / True | 3 / 3 |
| matrix-filter-071 | RGBP10 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.6662 / 2.8211 / 1.058 | 2.0176 / 2.2604 / 1.120 | True / True | 3 / 3 |
| matrix-filter-072 | RGBP10 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.0362 / 3.1721 / 1.045 | 2.4305 / 2.4316 / 1.000 | True / True | 3 / 3 |
| matrix-filter-073 | RGBP12 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.7899 / 0.8021 / 1.016 | 0.7947 / 0.7414 / 0.933 | False / False | 3 / 3 |
| matrix-filter-074 | RGBP12 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.6362 / 2.8795 / 1.092 | 1.9698 / 2.2248 / 1.129 | False / False | 3 / 3 |
| matrix-filter-075 | RGBP12 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.0954 / 3.1753 / 1.026 | 2.4391 / 2.4188 / 0.992 | False / False | 3 / 3 |
| matrix-filter-076 | RGBP14 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.7645 / 0.7883 / 1.031 | 0.8089 / 0.7384 / 0.913 | False / False | 3 / 3 |
| matrix-filter-077 | RGBP14 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.6003 / 2.8932 / 1.113 | 2.0159 / 2.2793 / 1.131 | False / False | 3 / 3 |
| matrix-filter-078 | RGBP14 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 2.9920 / 3.1790 / 1.062 | 2.4101 / 2.4987 / 1.037 | False / False | 3 / 3 |
| matrix-filter-079 | RGBP16 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.8890 / 0.7799 / 0.877 | 0.9765 / 0.7046 / 0.722 | False / False | 3 / 3 |
| matrix-filter-080 | RGBP16 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 3.3444 / 2.7134 / 0.811 | 3.3373 / 2.1920 / 0.657 | False / False | 3 / 3 |
| matrix-filter-081 | RGBP16 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.4758 / 3.0096 / 0.866 | 3.9150 / 2.3899 / 0.610 | False / False | 3 / 3 |
| matrix-filter-082 | RGBPS | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 1.4842 / 1.6122 / 1.086 | 1.5691 / 1.7216 / 1.097 | False / False | 3 / 3 |
| matrix-filter-083 | RGBPS | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 4.0324 / 4.3731 / 1.084 | 5.0861 / 4.3152 / 0.848 | False / False | 3 / 3 |
| matrix-filter-084 | RGBPS | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 4.4318 / 4.8367 / 1.091 | 5.2850 / 4.7930 / 0.907 | False / False | 3 / 3 |
| matrix-filter-085 | RGBAP16 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 1.0853 / 0.9374 / 0.864 | 1.1031 / 0.9533 / 0.864 | False / False | 3 / 3 |
| matrix-filter-086 | RGBAP16 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 3.2149 / 3.0652 / 0.953 | 3.6654 / 2.5924 / 0.707 | False / False | 3 / 3 |
| matrix-filter-087 | RGBAP16 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.5749 / 3.3467 / 0.936 | 4.2971 / 2.8294 / 0.658 | False / False | 3 / 3 |
| matrix-filter-088 | YV24 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.4726 / 0.4658 / 0.986 | 0.5029 / 0.4794 / 0.953 | False / False | 3 / 3 |
| matrix-filter-089 | YV24 | 1920×1080 | `src.ConvertToRGB32(matrix="Rec709")` | 0.8788 / 1.0922 / 1.243 | 0.8875 / 1.1109 / 1.252 | False / False | 3 / 3 |
| matrix-filter-090 | YV16 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 3.1763 / 2.3399 / 0.737 | 1.4663 / 1.7188 / 1.172 | False / False | 3 / 3 |
| matrix-filter-091 | YV16 | 1920×1080 | `src.ConvertToRGB32(matrix="Rec709")` | 3.6093 / 3.1154 / 0.863 | 1.8460 / 2.4326 / 1.318 | False / False | 3 / 3 |
| matrix-filter-092 | YV12 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 2.4820 / 2.2279 / 0.898 | 1.7954 / 1.7473 / 0.973 | False / False | 3 / 3 |
| matrix-filter-093 | YV12 | 1920×1080 | `src.ConvertToRGB32(matrix="Rec709")` | 2.8661 / 3.0247 / 1.055 | 2.1703 / 2.4630 / 1.135 | False / False | 3 / 3 |
| matrix-filter-094 | YUV444P10 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.7423 / 0.7589 / 1.022 | 0.7860 / 0.7542 / 0.960 | False / False | 3 / 3 |
| matrix-filter-095 | YUV444P10 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 2.4399 / 3.1414 / 1.288 | 2.4365 / 3.0711 / 1.260 | False / False | 3 / 3 |
| matrix-filter-096 | YUV444P12 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.7524 / 0.8041 / 1.069 | 0.7875 / 0.7690 / 0.977 | False / False | 3 / 3 |
| matrix-filter-097 | YUV444P12 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 2.2828 / 3.1700 / 1.389 | 2.3204 / 3.0654 / 1.321 | False / False | 3 / 3 |
| matrix-filter-098 | YUV444P14 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.7352 / 0.7840 / 1.066 | 0.7728 / 0.7381 / 0.955 | False / False | 3 / 3 |
| matrix-filter-099 | YUV444P14 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 2.3179 / 3.2252 / 1.391 | 2.3859 / 3.1521 / 1.321 | False / False | 3 / 3 |
| matrix-filter-100 | YUV444P16 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.7419 / 0.8018 / 1.081 | 0.7778 / 0.7215 / 0.928 | False / False | 3 / 3 |
| matrix-filter-101 | YUV444P16 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 1.9779 / 2.0278 / 1.025 | 2.0263 / 2.3548 / 1.162 | False / False | 3 / 3 |
| matrix-filter-102 | YUV444PS | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 1.4314 / 1.6147 / 1.128 | 1.5718 / 1.5330 / 0.975 | False / False | 3 / 3 |
| matrix-filter-103 | YUV444PS | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 2.6645 / 4.8862 / 1.834 | 3.2910 / 4.8061 / 1.460 | False / False | 3 / 3 |
| matrix-filter-104 | YUV420P16 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 3.4421 / 2.8852 / 0.838 | 2.5429 / 2.4266 / 0.954 | False / False | 3 / 3 |
| matrix-filter-105 | YUV420P16 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 4.6571 / 4.2184 / 0.906 | 3.8199 / 3.9495 / 1.034 | False / False | 3 / 3 |
| matrix-filter-106 | YUV420PS | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 6.5137 / 4.7536 / 0.730 | 4.5857 / 4.6463 / 1.013 | False / False | 3 / 3 |
| matrix-filter-107 | YUV420PS | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 7.6010 / 8.0355 / 1.057 | 6.3003 / 7.6645 / 1.217 | False / False | 3 / 3 |
| matrix-filter-469 | RGB32 | 3840×2160 | `src.ConvertToYUV420()` | 12.0970 / 12.9594 / 1.071 | 10.8731 / 9.1133 / 0.838 | True / True | 3 / 3 |

</details>

<details>
<summary>ordered — 16 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| ordered-290 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=8)` | 0.0949 / 0.0724 / 0.763 | 0.0979 / 0.0692 / 0.706 | True / True | 1 / 1 |
| ordered-291 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=8)` | 0.2442 / 0.3526 / 1.444 | 0.2608 / 0.3060 / 1.173 | True / True | 3 / 3 |
| ordered-292 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=8)` | 0.0934 / 0.0686 / 0.735 | 0.1124 / 0.0708 / 0.630 | True / True | 1 / 1 |
| ordered-293 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=8)` | 0.2454 / 0.3485 / 1.420 | 0.2708 / 0.3059 / 1.129 | True / True | 3 / 3 |
| ordered-294 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=0,dither_bits=10)` | 0.0919 / 0.0973 / 1.058 | 0.1026 / 0.1126 / 1.097 | True / True | 1 / 1 |
| ordered-295 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=0,dither_bits=10)` | 0.2310 / 0.3414 / 1.478 | 0.2469 / 0.3009 / 1.219 | True / True | 3 / 3 |
| ordered-296 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=true,dither=0,dither_bits=10)` | 0.1002 / 0.0984 / 0.982 | 0.1163 / 0.1137 / 0.978 | True / True | 1 / 1 |
| ordered-297 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=0,dither_bits=10)` | 0.2582 / 0.4403 / 1.705 | 0.2749 / 0.3639 / 1.323 | True / True | 3 / 3 |
| ordered-298 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=5)` | 0.3963 / 0.3872 / 0.977 | 0.4126 / 0.3299 / 0.800 | True / True | 1 / 1 |
| ordered-299 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=5)` | 0.6280 / 0.7140 / 1.137 | 0.6410 / 0.5508 / 0.859 | True / True | 1 / 1 |
| ordered-300 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=0,dither_bits=3)` | 0.3399 / 0.3961 / 1.165 | 0.3581 / 0.3259 / 0.910 | False / False | 3 / 3 |
| ordered-301 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=0,dither_bits=3)` | 0.5561 / 0.7122 / 1.281 | 0.5750 / 0.5502 / 0.957 | False / False | 3 / 3 |
| ordered-302 | YUV444P16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=8)` | 0.4895 / 0.4657 / 0.951 | 0.5284 / 0.4293 / 0.812 | True / True | 1 / 1 |
| ordered-303 | YUV444P16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=8)` | 0.7690 / 1.0567 / 1.374 | 0.8219 / 0.9360 / 1.139 | True / True | 3 / 3 |
| ordered-304 | RGBAP16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=8)` | 0.7251 / 0.6937 / 0.957 | 0.8044 / 0.6384 / 0.794 | True / True | 1 / 1 |
| ordered-305 | RGBAP16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=8)` | 0.9657 / 1.2634 / 1.308 | 1.0209 / 1.1217 / 1.099 | True / True | 3 / 3 |

</details>

<details>
<summary>resize — 144 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| resize-322 | Y8 | 1920×1080 | `src.PointResize(960,540)` | 0.6798 / 0.3950 / 0.581 | 0.2951 / 0.2366 / 0.802 | True / True | 3 / 3 |
| resize-323 | Y8 | 1920×1080 | `src.PointResize(2880,1620)` | 2.2321 / 1.3572 / 0.608 | 0.7303 / 0.9616 / 1.317 | True / True | 3 / 3 |
| resize-324 | Y8 | 1920×1080 | `src.PointResize(960,1620)` | 1.1824 / 0.7590 / 0.642 | 0.5030 / 0.5338 / 1.061 | True / True | 3 / 3 |
| resize-325 | Y16 | 1920×1080 | `src.PointResize(960,540)` | 0.8958 / 0.4131 / 0.461 | 0.4511 / 0.2508 / 0.556 | True / True | 3 / 3 |
| resize-326 | Y16 | 1920×1080 | `src.PointResize(2880,1620)` | 2.5130 / 1.4561 / 0.579 | 1.7048 / 1.0696 / 0.627 | True / True | 3 / 3 |
| resize-327 | Y16 | 1920×1080 | `src.PointResize(960,1620)` | 1.3797 / 0.7972 / 0.578 | 0.7212 / 0.5567 / 0.772 | True / True | 3 / 3 |
| resize-328 | Y32 | 1920×1080 | `src.PointResize(960,540)` | 1.7089 / 0.4851 / 0.284 | 1.0368 / 0.4285 / 0.413 | True / True | 3 / 3 |
| resize-329 | Y32 | 1920×1080 | `src.PointResize(2880,1620)` | 5.1167 / 1.8109 / 0.354 | 2.4421 / 1.5844 / 0.649 | True / True | 3 / 3 |
| resize-330 | Y32 | 1920×1080 | `src.PointResize(960,1620)` | 2.8796 / 0.9199 / 0.319 | 2.1093 / 0.8561 / 0.406 | True / True | 3 / 3 |
| resize-331 | Y8 | 1920×1080 | `src.BilinearResize(960,540)` | 0.6852 / 0.6108 / 0.891 | 0.3395 / 0.2981 / 0.878 | True / True | 3 / 3 |
| resize-332 | Y8 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.2803 / 1.1887 / 0.521 | 0.9274 / 0.7840 / 0.845 | True / True | 3 / 3 |
| resize-333 | Y8 | 1920×1080 | `src.BilinearResize(960,1620)` | 1.1799 / 0.9317 / 0.790 | 0.6050 / 0.4608 / 0.762 | True / True | 3 / 3 |
| resize-334 | Y16 | 1920×1080 | `src.BilinearResize(960,540)` | 0.9131 / 0.6356 / 0.696 | 0.8397 / 0.3454 / 0.411 | True / True | 3 / 3 |
| resize-335 | Y16 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.8190 / 1.5488 / 0.549 | 1.3946 / 1.1124 / 0.798 | True / True | 3 / 3 |
| resize-336 | Y16 | 1920×1080 | `src.BilinearResize(960,1620)` | 1.5274 / 1.2178 / 0.797 | 1.1709 / 0.7661 / 0.654 | True / True | 3 / 3 |
| resize-337 | Y32 | 1920×1080 | `src.BilinearResize(960,540)` | 1.7943 / 1.0580 / 0.590 | 1.1074 / 0.9157 / 0.827 | False / False | 3 / 3 |
| resize-338 | Y32 | 1920×1080 | `src.BilinearResize(2880,1620)` | 5.3794 / 2.2997 / 0.427 | 2.7359 / 1.9045 / 0.696 | False / False | 3 / 3 |
| resize-339 | Y32 | 1920×1080 | `src.BilinearResize(960,1620)` | 3.0312 / 1.9661 / 0.649 | 2.1713 / 1.6541 / 0.762 | False / False | 3 / 3 |
| resize-340 | Y8 | 1920×1080 | `src.BicubicResize(960,540)` | 0.7206 / 0.8524 / 1.183 | 0.4024 / 0.4822 / 1.198 | True / True | 3 / 3 |
| resize-341 | Y8 | 1920×1080 | `src.BicubicResize(2880,1620)` | 2.3343 / 1.9332 / 0.828 | 1.2788 / 1.3464 / 1.053 | True / True | 3 / 3 |
| resize-342 | Y8 | 1920×1080 | `src.BicubicResize(960,1620)` | 1.3089 / 1.5748 / 1.203 | 0.8102 / 0.9477 / 1.170 | True / True | 3 / 3 |
| resize-343 | Y16 | 1920×1080 | `src.BicubicResize(960,540)` | 0.9362 / 0.8717 / 0.931 | 0.7138 / 0.5271 / 0.738 | True / True | 3 / 3 |
| resize-344 | Y16 | 1920×1080 | `src.BicubicResize(2880,1620)` | 2.9383 / 2.0843 / 0.709 | 1.9144 / 1.4534 / 0.759 | True / True | 3 / 3 |
| resize-345 | Y16 | 1920×1080 | `src.BicubicResize(960,1620)` | 1.6911 / 1.7921 / 1.060 | 1.3518 / 1.2705 / 0.940 | True / True | 3 / 3 |
| resize-346 | Y32 | 1920×1080 | `src.BicubicResize(960,540)` | 1.0781 / 1.2934 / 1.200 | 1.3239 / 1.1723 / 0.885 | False / False | 3 / 3 |
| resize-347 | Y32 | 1920×1080 | `src.BicubicResize(2880,1620)` | 5.6237 / 3.1064 / 0.552 | 2.8737 / 2.6100 / 0.908 | False / False | 3 / 3 |
| resize-348 | Y32 | 1920×1080 | `src.BicubicResize(960,1620)` | 2.1103 / 2.4255 / 1.149 | 2.7281 / 2.3195 / 0.850 | False / False | 3 / 3 |
| resize-349 | Y8 | 1920×1080 | `src.LanczosResize(960,540)` | 0.7702 / 0.9148 / 1.188 | 0.5860 / 0.6944 / 1.185 | True / True | 3 / 3 |
| resize-350 | Y8 | 1920×1080 | `src.LanczosResize(2880,1620)` | 2.6613 / 2.4364 / 0.915 | 1.8927 / 1.7129 / 0.905 | True / True | 3 / 3 |
| resize-351 | Y8 | 1920×1080 | `src.LanczosResize(960,1620)` | 1.4337 / 1.7960 / 1.253 | 1.2367 / 1.3585 / 1.098 | True / True | 3 / 3 |
| resize-352 | Y16 | 1920×1080 | `src.LanczosResize(960,540)` | 0.9902 / 0.9378 / 0.947 | 0.7162 / 0.7085 / 0.989 | True / True | 3 / 3 |
| resize-353 | Y16 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.3197 / 2.4810 / 0.747 | 2.3961 / 1.8484 / 0.771 | True / True | 3 / 3 |
| resize-354 | Y16 | 1920×1080 | `src.LanczosResize(960,1620)` | 1.8410 / 1.9786 / 1.075 | 1.3737 / 1.6122 / 1.174 | True / True | 3 / 3 |
| resize-355 | Y32 | 1920×1080 | `src.LanczosResize(960,540)` | 1.4957 / 1.6555 / 1.107 | 1.8703 / 1.5975 / 0.854 | False / False | 3 / 3 |
| resize-356 | Y32 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.4958 / 3.6021 / 1.030 | 3.4024 / 3.2132 / 0.944 | False / False | 3 / 3 |
| resize-357 | Y32 | 1920×1080 | `src.LanczosResize(960,1620)` | 2.7844 / 3.0095 / 1.081 | 3.6374 / 3.1701 / 0.872 | False / False | 3 / 3 |
| resize-358 | Y8 | 1920×1080 | `src.Lanczos4Resize(960,540)` | 0.8675 / 0.9725 / 1.121 | 0.6547 / 0.9294 / 1.419 | True / True | 3 / 3 |
| resize-359 | Y8 | 1920×1080 | `src.Lanczos4Resize(2880,1620)` | 3.2955 / 3.3158 / 1.006 | 2.2657 / 2.4177 / 1.067 | True / True | 3 / 3 |
| resize-360 | Y8 | 1920×1080 | `src.Lanczos4Resize(960,1620)` | 1.5631 / 1.9076 / 1.220 | 1.4571 / 1.7653 / 1.212 | True / True | 3 / 3 |
| resize-361 | Y16 | 1920×1080 | `src.Lanczos4Resize(960,540)` | 1.0659 / 0.9630 / 0.903 | 0.9042 / 0.9709 / 1.074 | True / True | 3 / 3 |
| resize-362 | Y16 | 1920×1080 | `src.Lanczos4Resize(2880,1620)` | 3.3931 / 3.2795 / 0.967 | 2.6651 / 2.1559 / 0.809 | True / True | 3 / 3 |
| resize-363 | Y16 | 1920×1080 | `src.Lanczos4Resize(960,1620)` | 1.9666 / 2.0101 / 1.022 | 1.7817 / 1.8750 / 1.052 | True / True | 3 / 3 |
| resize-364 | Y32 | 1920×1080 | `src.Lanczos4Resize(960,540)` | 1.6435 / 1.9617 / 1.194 | 2.0522 / 2.0210 / 0.985 | False / False | 3 / 3 |
| resize-365 | Y32 | 1920×1080 | `src.Lanczos4Resize(2880,1620)` | 3.8317 / 4.3624 / 1.139 | 3.8644 / 3.5806 / 0.927 | False / False | 3 / 3 |
| resize-366 | Y32 | 1920×1080 | `src.Lanczos4Resize(960,1620)` | 3.0492 / 3.5735 / 1.172 | 3.9017 / 3.6627 / 0.939 | False / False | 3 / 3 |
| resize-367 | Y8 | 1920×1080 | `src.BlackmanResize(960,540)` | 0.7988 / 0.9638 / 1.207 | 0.6543 / 0.8837 / 1.351 | True / True | 3 / 3 |
| resize-368 | Y8 | 1920×1080 | `src.BlackmanResize(2880,1620)` | 2.6851 / 3.3189 / 1.236 | 2.2287 / 2.1311 / 0.956 | True / True | 3 / 3 |
| resize-369 | Y8 | 1920×1080 | `src.BlackmanResize(960,1620)` | 1.5314 / 1.9034 / 1.243 | 1.4558 / 1.7067 / 1.172 | True / True | 3 / 3 |
| resize-370 | Y16 | 1920×1080 | `src.BlackmanResize(960,540)` | 1.0585 / 1.0022 / 0.947 | 0.9184 / 0.8783 / 0.956 | True / True | 3 / 3 |
| resize-371 | Y16 | 1920×1080 | `src.BlackmanResize(2880,1620)` | 3.2790 / 3.3134 / 1.011 | 2.6843 / 2.1561 / 0.803 | True / True | 3 / 3 |
| resize-372 | Y16 | 1920×1080 | `src.BlackmanResize(960,1620)` | 1.9318 / 1.9991 / 1.035 | 1.7237 / 1.8034 / 1.046 | True / True | 3 / 3 |
| resize-373 | Y32 | 1920×1080 | `src.BlackmanResize(960,540)` | 1.6204 / 1.9548 / 1.206 | 2.0081 / 2.0427 / 1.017 | False / False | 3 / 3 |
| resize-374 | Y32 | 1920×1080 | `src.BlackmanResize(2880,1620)` | 3.7956 / 4.3570 / 1.148 | 3.6420 / 3.9148 / 1.075 | False / False | 3 / 3 |
| resize-375 | Y32 | 1920×1080 | `src.BlackmanResize(960,1620)` | 3.0324 / 3.5705 / 1.177 | 3.8746 / 3.6660 / 0.946 | False / False | 3 / 3 |
| resize-376 | Y8 | 1920×1080 | `src.Spline16Resize(960,540)` | 0.7178 / 0.8526 / 1.188 | 0.3961 / 0.4894 / 1.236 | True / True | 3 / 3 |
| resize-377 | Y8 | 1920×1080 | `src.Spline16Resize(2880,1620)` | 2.3308 / 1.9201 / 0.824 | 1.3922 / 1.3505 / 0.970 | True / True | 3 / 3 |
| resize-378 | Y8 | 1920×1080 | `src.Spline16Resize(960,1620)` | 1.3169 / 1.5820 / 1.201 | 0.8203 / 0.9847 / 1.200 | True / True | 3 / 3 |
| resize-379 | Y16 | 1920×1080 | `src.Spline16Resize(960,540)` | 0.9440 / 0.8639 / 0.915 | 0.6090 / 0.5220 / 0.857 | True / True | 3 / 3 |
| resize-380 | Y16 | 1920×1080 | `src.Spline16Resize(2880,1620)` | 2.9790 / 2.0574 / 0.691 | 1.9962 / 1.5237 / 0.763 | True / True | 3 / 3 |
| resize-381 | Y16 | 1920×1080 | `src.Spline16Resize(960,1620)` | 1.8173 / 1.8100 / 0.996 | 1.5854 / 1.2777 / 0.806 | True / True | 3 / 3 |
| resize-382 | Y32 | 1920×1080 | `src.Spline16Resize(960,540)` | 1.3098 / 1.2257 / 0.936 | 1.1952 / 1.1814 / 0.988 | False / False | 3 / 3 |
| resize-383 | Y32 | 1920×1080 | `src.Spline16Resize(2880,1620)` | 5.7237 / 3.0650 / 0.536 | 3.0358 / 2.6623 / 0.877 | False / False | 3 / 3 |
| resize-384 | Y32 | 1920×1080 | `src.Spline16Resize(960,1620)` | 2.1304 / 2.4550 / 1.152 | 2.7151 / 2.2843 / 0.841 | False / False | 3 / 3 |
| resize-385 | Y8 | 1920×1080 | `src.Spline36Resize(960,540)` | 0.7855 / 0.9104 / 1.159 | 0.5871 / 0.6907 / 1.176 | True / True | 3 / 3 |
| resize-386 | Y8 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 2.7129 / 2.6081 / 0.961 | 1.8550 / 1.6781 / 0.905 | True / True | 3 / 3 |
| resize-387 | Y8 | 1920×1080 | `src.Spline36Resize(960,1620)` | 1.5344 / 1.8398 / 1.199 | 1.2480 / 1.3294 / 1.065 | True / True | 3 / 3 |
| resize-388 | Y16 | 1920×1080 | `src.Spline36Resize(960,540)` | 1.0232 / 0.9824 / 0.960 | 0.7260 / 0.7195 / 0.991 | True / True | 3 / 3 |
| resize-389 | Y16 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.2689 / 2.5450 / 0.779 | 2.4113 / 1.7851 / 0.740 | True / True | 3 / 3 |
| resize-390 | Y16 | 1920×1080 | `src.Spline36Resize(960,1620)` | 1.9002 / 1.9219 / 1.011 | 1.3576 / 1.5463 / 1.139 | True / True | 3 / 3 |
| resize-391 | Y32 | 1920×1080 | `src.Spline36Resize(960,540)` | 1.5437 / 1.7348 / 1.124 | 1.9536 / 1.6858 / 0.863 | False / False | 3 / 3 |
| resize-392 | Y32 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.5722 / 3.7397 / 1.047 | 3.2024 / 3.2108 / 1.003 | False / False | 3 / 3 |
| resize-393 | Y32 | 1920×1080 | `src.Spline36Resize(960,1620)` | 2.8494 / 3.0918 / 1.085 | 3.7067 / 3.0446 / 0.821 | False / False | 3 / 3 |
| resize-394 | Y8 | 1920×1080 | `src.Spline64Resize(960,540)` | 0.8251 / 0.9586 / 1.162 | 0.6457 / 0.8926 / 1.382 | True / True | 3 / 3 |
| resize-395 | Y8 | 1920×1080 | `src.Spline64Resize(2880,1620)` | 2.7744 / 3.3657 / 1.213 | 2.2239 / 2.1301 / 0.958 | True / True | 3 / 3 |
| resize-396 | Y8 | 1920×1080 | `src.Spline64Resize(960,1620)` | 1.5412 / 1.9257 / 1.250 | 1.4401 / 1.7309 / 1.202 | True / True | 3 / 3 |
| resize-397 | Y16 | 1920×1080 | `src.Spline64Resize(960,540)` | 1.0724 / 0.9780 / 0.912 | 0.9127 / 0.9056 / 0.992 | True / True | 3 / 3 |
| resize-398 | Y16 | 1920×1080 | `src.Spline64Resize(2880,1620)` | 3.4433 / 3.3597 / 0.976 | 2.6949 / 2.1410 / 0.794 | True / True | 3 / 3 |
| resize-399 | Y16 | 1920×1080 | `src.Spline64Resize(960,1620)` | 2.0697 / 2.0307 / 0.981 | 1.7929 / 1.9072 / 1.064 | True / True | 3 / 3 |
| resize-400 | Y32 | 1920×1080 | `src.Spline64Resize(960,540)` | 1.7629 / 1.9982 / 1.134 | 2.0882 / 2.0494 / 0.981 | False / False | 3 / 3 |
| resize-401 | Y32 | 1920×1080 | `src.Spline64Resize(2880,1620)` | 3.8120 / 4.3275 / 1.135 | 3.5669 / 3.8065 / 1.067 | False / False | 3 / 3 |
| resize-402 | Y32 | 1920×1080 | `src.Spline64Resize(960,1620)` | 3.0772 / 3.5723 / 1.161 | 3.8508 / 3.6570 / 0.950 | False / False | 3 / 3 |
| resize-403 | Y8 | 1920×1080 | `src.GaussResize(960,540)` | 0.8175 / 0.9690 / 1.185 | 0.6590 / 0.8784 / 1.333 | True / True | 3 / 3 |
| resize-404 | Y8 | 1920×1080 | `src.GaussResize(2880,1620)` | 2.7161 / 3.3355 / 1.228 | 2.2148 / 2.1016 / 0.949 | True / True | 3 / 3 |
| resize-405 | Y8 | 1920×1080 | `src.GaussResize(960,1620)` | 1.5451 / 1.8939 / 1.226 | 1.4352 / 1.7069 / 1.189 | True / True | 3 / 3 |
| resize-406 | Y16 | 1920×1080 | `src.GaussResize(960,540)` | 1.0839 / 0.9546 / 0.881 | 0.9097 / 0.8794 / 0.967 | True / True | 3 / 3 |
| resize-407 | Y16 | 1920×1080 | `src.GaussResize(2880,1620)` | 3.7006 / 3.3489 / 0.905 | 2.6110 / 2.0965 / 0.803 | True / True | 3 / 3 |
| resize-408 | Y16 | 1920×1080 | `src.GaussResize(960,1620)` | 2.1267 / 2.0105 / 0.945 | 1.7633 / 1.8252 / 1.035 | True / True | 3 / 3 |
| resize-409 | Y32 | 1920×1080 | `src.GaussResize(960,540)` | 1.7128 / 1.9646 / 1.147 | 2.0014 / 2.0537 / 1.026 | False / False | 3 / 3 |
| resize-410 | Y32 | 1920×1080 | `src.GaussResize(2880,1620)` | 4.0593 / 4.3107 / 1.062 | 3.6274 / 3.7293 / 1.028 | False / False | 3 / 3 |
| resize-411 | Y32 | 1920×1080 | `src.GaussResize(960,1620)` | 3.0458 / 3.5537 / 1.167 | 3.7938 / 3.7709 / 0.994 | False / False | 3 / 3 |
| resize-412 | Y8 | 1920×1080 | `src.SincResize(960,540)` | 0.8025 / 0.9622 / 1.199 | 0.6536 / 0.8843 / 1.353 | True / True | 3 / 3 |
| resize-413 | Y8 | 1920×1080 | `src.SincResize(2880,1620)` | 2.7375 / 3.3260 / 1.215 | 2.1922 / 2.1328 / 0.973 | True / True | 3 / 3 |
| resize-414 | Y8 | 1920×1080 | `src.SincResize(960,1620)` | 1.5236 / 1.9085 / 1.253 | 1.4538 / 1.7272 / 1.188 | True / True | 3 / 3 |
| resize-415 | Y16 | 1920×1080 | `src.SincResize(960,540)` | 1.0501 / 0.9732 / 0.927 | 0.9107 / 0.9373 / 1.029 | True / True | 3 / 3 |
| resize-416 | Y16 | 1920×1080 | `src.SincResize(2880,1620)` | 3.2920 / 3.3458 / 1.016 | 2.6467 / 2.2743 / 0.859 | True / True | 3 / 3 |
| resize-417 | Y16 | 1920×1080 | `src.SincResize(960,1620)` | 1.9782 / 1.9848 / 1.003 | 1.8187 / 1.8662 / 1.026 | True / True | 3 / 3 |
| resize-418 | Y32 | 1920×1080 | `src.SincResize(960,540)` | 1.5931 / 2.0027 / 1.257 | 2.0575 / 2.0393 / 0.991 | False / False | 3 / 3 |
| resize-419 | Y32 | 1920×1080 | `src.SincResize(2880,1620)` | 3.7912 / 4.3563 / 1.149 | 3.6092 / 3.7832 / 1.048 | False / False | 3 / 3 |
| resize-420 | Y32 | 1920×1080 | `src.SincResize(960,1620)` | 3.0939 / 3.5413 / 1.145 | 3.8684 / 3.6142 / 0.934 | False / False | 3 / 3 |
| resize-421 | Y8 | 1920×1080 | `src.SinPowerResize(960,540)` | 0.7288 / 0.8489 / 1.165 | 0.3937 / 0.4605 / 1.170 | True / True | 3 / 3 |
| resize-422 | Y8 | 1920×1080 | `src.SinPowerResize(2880,1620)` | 2.3622 / 1.9242 / 0.815 | 1.3013 / 1.3237 / 1.017 | True / True | 3 / 3 |
| resize-423 | Y8 | 1920×1080 | `src.SinPowerResize(960,1620)` | 1.3208 / 1.6324 / 1.236 | 0.8224 / 0.9496 / 1.155 | True / True | 3 / 3 |
| resize-424 | Y16 | 1920×1080 | `src.SinPowerResize(960,540)` | 0.9394 / 0.8830 / 0.940 | 0.9661 / 0.5227 / 0.541 | True / True | 3 / 3 |
| resize-425 | Y16 | 1920×1080 | `src.SinPowerResize(2880,1620)` | 2.9090 / 2.0482 / 0.704 | 1.9160 / 1.4347 / 0.749 | True / True | 3 / 3 |
| resize-426 | Y16 | 1920×1080 | `src.SinPowerResize(960,1620)` | 1.7081 / 1.7577 / 1.029 | 1.5344 / 1.2491 / 0.814 | True / True | 3 / 3 |
| resize-427 | Y32 | 1920×1080 | `src.SinPowerResize(960,540)` | 1.0324 / 1.2242 / 1.186 | 1.3493 / 1.1703 / 0.867 | False / False | 3 / 3 |
| resize-428 | Y32 | 1920×1080 | `src.SinPowerResize(2880,1620)` | 5.6978 / 3.0255 / 0.531 | 2.9997 / 2.5755 / 0.859 | False / False | 3 / 3 |
| resize-429 | Y32 | 1920×1080 | `src.SinPowerResize(960,1620)` | 2.0579 / 2.3718 / 1.153 | 2.7045 / 2.2770 / 0.842 | False / False | 3 / 3 |
| resize-430 | Y8 | 1920×1080 | `src.SincLin2Resize(960,540)` | 2.3422 / 6.7341 / 2.875 | 2.6686 / 4.7234 / 1.770 | True / True | 3 / 3 |
| resize-431 | Y8 | 1920×1080 | `src.SincLin2Resize(2880,1620)` | 6.1501 / 17.3442 / 2.820 | 7.1435 / 7.1989 / 1.008 | True / True | 3 / 3 |
| resize-432 | Y8 | 1920×1080 | `src.SincLin2Resize(960,1620)` | 4.4813 / 11.0788 / 2.472 | 5.4823 / 8.0799 / 1.474 | True / True | 3 / 3 |
| resize-433 | Y16 | 1920×1080 | `src.SincLin2Resize(960,540)` | 3.1909 / 5.5061 / 1.726 | 3.0745 / 4.8130 / 1.565 | True / True | 3 / 3 |
| resize-434 | Y16 | 1920×1080 | `src.SincLin2Resize(2880,1620)` | 7.0710 / 16.2928 / 2.304 | 7.4358 / 6.7537 / 0.908 | True / True | 3 / 3 |
| resize-435 | Y16 | 1920×1080 | `src.SincLin2Resize(960,1620)` | 5.4226 / 9.4589 / 1.744 | 6.0324 / 8.3846 / 1.390 | True / True | 3 / 3 |
| resize-436 | Y32 | 1920×1080 | `src.SincLin2Resize(960,540)` | 5.3759 / 6.7800 / 1.261 | 5.1562 / 6.2142 / 1.205 | False / False | 3 / 3 |
| resize-437 | Y32 | 1920×1080 | `src.SincLin2Resize(2880,1620)` | 11.0242 / 13.0994 / 1.188 | 10.7055 / 14.8415 / 1.386 | False / False | 3 / 3 |
| resize-438 | Y32 | 1920×1080 | `src.SincLin2Resize(960,1620)` | 9.4673 / 12.1494 / 1.283 | 9.4550 / 11.3549 / 1.201 | False / False | 3 / 3 |
| resize-439 | Y8 | 1920×1080 | `src.UserDefined2Resize(960,540)` | 0.7678 / 0.9106 / 1.186 | 0.5538 / 0.6031 / 1.089 | True / True | 3 / 3 |
| resize-440 | Y8 | 1920×1080 | `src.UserDefined2Resize(2880,1620)` | 2.5748 / 2.4495 / 0.951 | 1.7301 / 1.6652 / 0.962 | True / True | 3 / 3 |
| resize-441 | Y8 | 1920×1080 | `src.UserDefined2Resize(960,1620)` | 1.4414 / 1.7971 / 1.247 | 1.1418 / 1.2092 / 1.059 | True / True | 3 / 3 |
| resize-442 | Y16 | 1920×1080 | `src.UserDefined2Resize(960,540)` | 1.0083 / 0.9552 / 0.947 | 0.6226 / 0.6159 / 0.989 | True / True | 3 / 3 |
| resize-443 | Y16 | 1920×1080 | `src.UserDefined2Resize(2880,1620)` | 3.2584 / 2.5682 / 0.788 | 2.4561 / 1.8347 / 0.747 | True / True | 3 / 3 |
| resize-444 | Y16 | 1920×1080 | `src.UserDefined2Resize(960,1620)` | 1.8377 / 2.0109 / 1.094 | 1.1901 / 1.4980 / 1.259 | True / True | 3 / 3 |
| resize-445 | Y32 | 1920×1080 | `src.UserDefined2Resize(960,540)` | 1.4076 / 1.4447 / 1.026 | 2.1415 / 1.4307 / 0.668 | False / False | 3 / 3 |
| resize-446 | Y32 | 1920×1080 | `src.UserDefined2Resize(2880,1620)` | 3.3421 / 3.4184 / 1.023 | 3.1079 / 2.9929 / 0.963 | False / False | 3 / 3 |
| resize-447 | Y32 | 1920×1080 | `src.UserDefined2Resize(960,1620)` | 2.7393 / 2.7955 / 1.021 | 3.4899 / 2.6251 / 0.752 | False / False | 3 / 3 |
| extra-001 | Y10 | 1920×1080 | `src.BilinearResize(960,540)` | 0.8304 / 0.6911 / 0.832 | 0.4588 / 0.3452 / 0.752 | True / True | 3 / 3 |
| extra-002 | Y10 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.6077 / 1.7320 / 0.664 | 1.3326 / 1.1312 / 0.849 | True / True | 3 / 3 |
| extra-003 | Y10 | 1920×1080 | `src.LanczosResize(960,540)` | 0.9130 / 1.0088 / 1.105 | 0.6888 / 0.7039 / 1.022 | True / True | 3 / 3 |
| extra-004 | Y10 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.0391 / 2.7638 / 0.909 | 2.3024 / 1.8096 / 0.786 | True / True | 3 / 3 |
| extra-005 | Y10 | 1920×1080 | `src.Spline36Resize(960,540)` | 0.9475 / 1.0394 / 1.097 | 0.6758 / 0.7066 / 1.046 | True / True | 3 / 3 |
| extra-006 | Y10 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.0188 / 2.9172 / 0.966 | 2.2654 / 1.7871 / 0.789 | True / True | 3 / 3 |
| extra-007 | Y12 | 1920×1080 | `src.BilinearResize(960,540)` | 0.8246 / 0.7296 / 0.885 | 0.4415 / 0.3489 / 0.790 | True / True | 3 / 3 |
| extra-008 | Y12 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.6357 / 1.6967 / 0.644 | 1.3158 / 1.1380 / 0.865 | True / True | 3 / 3 |
| extra-009 | Y12 | 1920×1080 | `src.LanczosResize(960,540)` | 0.9640 / 1.1595 / 1.203 | 0.6719 / 0.7053 / 1.050 | True / True | 3 / 3 |
| extra-010 | Y12 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.5258 / 2.6675 / 0.757 | 2.3205 / 1.7915 / 0.772 | True / True | 3 / 3 |
| extra-011 | Y12 | 1920×1080 | `src.Spline36Resize(960,540)` | 0.9901 / 1.0348 / 1.045 | 0.7006 / 0.7184 / 1.026 | True / True | 3 / 3 |
| extra-012 | Y12 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.0944 / 2.6539 / 0.858 | 2.2684 / 1.8251 / 0.805 | True / True | 3 / 3 |
| extra-013 | Y14 | 1920×1080 | `src.BilinearResize(960,540)` | 0.8770 / 0.7029 / 0.801 | 0.4418 / 0.3444 / 0.780 | True / True | 3 / 3 |
| extra-014 | Y14 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.7323 / 1.7280 / 0.632 | 1.3907 / 1.1577 / 0.832 | True / True | 3 / 3 |
| extra-015 | Y14 | 1920×1080 | `src.LanczosResize(960,540)` | 0.9693 / 1.0409 / 1.074 | 0.6756 / 0.6957 / 1.030 | True / True | 3 / 3 |
| extra-016 | Y14 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.0997 / 2.5975 / 0.838 | 2.2507 / 1.8966 / 0.843 | True / True | 3 / 3 |
| extra-017 | Y14 | 1920×1080 | `src.Spline36Resize(960,540)` | 0.9902 / 1.0231 / 1.033 | 0.6669 / 0.7281 / 1.092 | True / True | 3 / 3 |
| extra-018 | Y14 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.1997 / 2.8744 / 0.898 | 2.2499 / 1.8215 / 0.810 | True / True | 3 / 3 |

</details>

<details>
<summary>resize-composed — 19 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| resize-composed-448 | RGB24 | 1920×1080 | `src.LanczosResize(960,540)` | 4.8572 / 3.6694 / 0.755 | 5.7176 / 2.8746 / 0.503 | True / True | 3 / 3 |
| resize-composed-449 | RGB24 | 1920×1080 | `src.LanczosResize(2880,1620)` | 9.6009 / 8.9797 / 0.935 | 11.2427 / 6.6495 / 0.591 | True / True | 3 / 3 |
| resize-composed-450 | RGB32 | 1920×1080 | `src.LanczosResize(960,540)` | 3.2803 / 4.9576 / 1.511 | 4.0056 / 3.6486 / 0.911 | True / True | 3 / 3 |
| resize-composed-451 | RGB32 | 1920×1080 | `src.LanczosResize(2880,1620)` | 7.0602 / 11.6073 / 1.644 | 9.0501 / 8.4892 / 0.938 | True / True | 3 / 3 |
| resize-composed-452 | RGB48 | 1920×1080 | `src.LanczosResize(960,540)` | 7.5777 / 4.5369 / 0.599 | 7.6564 / 3.6345 / 0.475 | True / True | 3 / 3 |
| resize-composed-453 | RGB48 | 1920×1080 | `src.LanczosResize(2880,1620)` | 14.1355 / 10.2988 / 0.729 | 14.8278 / 8.0279 / 0.541 | True / True | 3 / 3 |
| resize-composed-454 | RGB64 | 1920×1080 | `src.LanczosResize(960,540)` | 5.2039 / 5.9235 / 1.138 | 5.5536 / 4.8803 / 0.879 | True / True | 3 / 3 |
| resize-composed-455 | RGB64 | 1920×1080 | `src.LanczosResize(2880,1620)` | 11.6854 / 13.0760 / 1.119 | 12.3712 / 10.0611 / 0.813 | True / True | 3 / 3 |
| resize-composed-456 | RGBAP16 | 1920×1080 | `src.LanczosResize(960,540)` | 4.2585 / 4.3415 / 1.019 | 3.7700 / 3.6751 / 0.975 | True / True | 3 / 3 |
| resize-composed-457 | RGBAP16 | 1920×1080 | `src.LanczosResize(2880,1620)` | 13.4831 / 10.4338 / 0.774 | 10.5025 / 9.1850 / 0.875 | True / True | 3 / 3 |
| resize-composed-458 | YV12 | 1920×1080 | `src.LanczosResize(960,540)` | 1.1514 / 1.4018 / 1.218 | 0.9237 / 0.9867 / 1.068 | True / True | 3 / 3 |
| resize-composed-459 | YV12 | 1920×1080 | `src.LanczosResize(2880,1620)` | 4.1591 / 3.8204 / 0.919 | 2.9873 / 2.5974 / 0.869 | True / True | 3 / 3 |
| resize-composed-460 | YUV420P16 | 1920×1080 | `src.LanczosResize(960,540)` | 1.5391 / 1.4776 / 0.960 | 1.1575 / 1.1978 / 1.035 | True / True | 3 / 3 |
| resize-composed-461 | YUV420P16 | 1920×1080 | `src.LanczosResize(2880,1620)` | 4.9817 / 3.8585 / 0.775 | 3.6770 / 2.8991 / 0.788 | True / True | 3 / 3 |
| resize-composed-462 | YUV420PS | 1920×1080 | `src.LanczosResize(960,540)` | 2.3796 / 2.6318 / 1.106 | 4.1050 / 2.5142 / 0.612 | False / False | 3 / 3 |
| resize-composed-463 | YUV420PS | 1920×1080 | `src.LanczosResize(2880,1620)` | 5.2721 / 5.3142 / 1.008 | 5.2061 / 4.7189 / 0.906 | False / False | 3 / 3 |
| resize-composed-464 | YUY2 | 1920×1080 | `src.LanczosResize(960,540)` | 2.1546 / 2.2967 / 1.066 | 1.7961 / 1.6375 / 0.912 | True / True | 3 / 3 |
| resize-composed-465 | YUY2 | 1920×1080 | `src.LanczosResize(2880,1620)` | 6.3326 / 5.7328 / 0.905 | 4.4545 / 4.1784 / 0.938 | True / True | 3 / 3 |
| resize-composed-473 | YUV420P16 | 3840×2160 | `src.LanczosResize(1920,1080)` | 6.1202 / 6.4029 / 1.046 | 5.3792 / 5.5598 / 1.034 | True / True | 3 / 3 |

</details>

<details>
<summary>yuy2 — 11 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| yuy2-029 | YUY2 | 1920×1080 | `src.ConvertToYV16()` | 0.4125 / 0.1029 / 0.249 | 0.4173 / 0.0935 / 0.224 | True / True | 3 / 3 |
| yuy2-032 | YV16 | 1920×1080 | `src.ConvertToYUY2()` | 0.0896 / 0.0801 / 0.894 | 0.0940 / 0.0802 / 0.853 | True / True | 3 / 3 |
| yuy2-138 | YUY2 | 1920×1080 | `src.ConvertToRGB24()` | 5.0136 / 3.7994 / 0.758 | 3.2946 / 3.0376 / 0.922 | False / False | 3 / 3 |
| yuy2-139 | YUY2 | 1920×1080 | `src.ConvertToRGB32()` | 4.8543 / 3.9531 / 0.814 | 3.0963 / 3.1658 / 1.022 | False / False | 3 / 3 |
| yuy2-140 | YUY2 | 1920×1080 | `src.ConvertToYV12()` | 1.5727 / 0.5799 / 0.369 | 1.6913 / 0.5781 / 0.342 | True / True | 3 / 3 |
| yuy2-141 | YUY2 | 1920×1080 | `src.ConvertToYV24()` | 3.9239 / 2.1022 / 0.536 | 2.0798 / 1.4675 / 0.706 | True / True | 3 / 3 |
| yuy2-142 | RGB24 | 1920×1080 | `src.ConvertToYUY2()` | 2.4049 / 3.0062 / 1.250 | 1.8446 / 2.2545 / 1.222 | True / True | 3 / 3 |
| yuy2-143 | RGB32 | 1920×1080 | `src.ConvertToYUY2()` | 2.9987 / 3.1167 / 1.039 | 2.1728 / 2.3642 / 1.088 | True / True | 3 / 3 |
| yuy2-144 | YV12 | 1920×1080 | `src.ConvertToYUY2()` | 0.5258 / 0.6133 / 1.166 | 0.6358 / 0.5649 / 0.889 | True / True | 3 / 3 |
| yuy2-145 | YV24 | 1920×1080 | `src.ConvertToYUY2()` | 1.6180 / 1.9514 / 1.206 | 0.8944 / 1.0543 / 1.179 | True / True | 3 / 3 |
| yuy2-470 | RGB32 | 3840×2160 | `src.ConvertToYUY2()` | 11.6317 / 12.0654 / 1.037 | 10.2158 / 9.0983 / 0.891 | True / True | 3 / 3 |

</details>

## Complete kernel comparisons

Allocation and coefficient construction are outside these timings. Baseline and notes identify the actual upstream implementation. Native resampling uses the upstream AVX512 baseline.

<details>
<summary>depth — 112 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| source_bits=8; destination_bits=8; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.1872 | 0.2056 | 1.098 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=8; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1872 | 0.1794 | 0.958 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=8; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.1845 | 0.2007 | 1.087 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=8; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1845 | 0.1768 | 0.958 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=10; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1624 | 0.1661 | 1.023 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=10; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1624 | 0.1431 | 0.881 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=10; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.0588 | 0.0699 | 1.188 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=10; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.0588 | 0.0635 | 1.080 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=10; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1691 | 0.2001 | 1.184 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=10; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1691 | 0.1645 | 0.973 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=10; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.0592 | 0.0726 | 1.226 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=10; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.0592 | 0.0628 | 1.061 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=16; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.0672 | 0.0747 | 1.112 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=16; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.0672 | 0.0628 | 0.936 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=16; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.0603 | 0.0698 | 1.157 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=16; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.0603 | 0.0728 | 1.207 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=16; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1711 | 0.2081 | 1.216 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=16; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1711 | 0.1648 | 0.963 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=16; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.0599 | 0.0706 | 1.180 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=16; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.0599 | 0.0636 | 1.063 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=32; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1119 | 0.1631 | 1.457 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=32; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1119 | 0.1156 | 1.033 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=32; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.1134 | 0.1736 | 1.531 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=32; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.1134 | 0.1237 | 1.091 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=32; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1125 | 0.1863 | 1.656 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=32; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1125 | 0.1272 | 1.130 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=32; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.1135 | 0.1775 | 1.564 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=8; destination_bits=32; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.1135 | 0.1263 | 1.113 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=8; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1589 | 0.1691 | 1.064 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=8; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1589 | 0.1507 | 0.948 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=8; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.0596 | 0.0820 | 1.377 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=8; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.0596 | 0.0698 | 1.172 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=8; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1744 | 0.2027 | 1.162 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=8; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1744 | 0.1803 | 1.034 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=8; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.0605 | 0.0794 | 1.312 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=8; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.0605 | 0.0694 | 1.147 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=10; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.1538 | 0.2017 | 1.311 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=10; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1538 | 0.1671 | 1.086 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=10; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.1536 | 0.2016 | 1.312 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=10; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1536 | 0.1651 | 1.075 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=16; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1454 | 0.1681 | 1.156 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=16; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1454 | 0.1448 | 0.996 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=16; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.0807 | 0.0840 | 1.041 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=16; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.0807 | 0.0917 | 1.136 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=16; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1547 | 0.2036 | 1.316 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=16; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1547 | 0.1685 | 1.089 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=16; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.0800 | 0.0890 | 1.112 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=16; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.0800 | 0.0973 | 1.216 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=32; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1910 | 0.2061 | 1.079 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=32; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1910 | 0.1835 | 0.961 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=32; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.1663 | 0.2081 | 1.251 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=32; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.1663 | 0.1459 | 0.878 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=32; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1431 | 0.1987 | 1.389 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=32; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1431 | 0.1476 | 1.032 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=32; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.1370 | 0.2046 | 1.493 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=10; destination_bits=32; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.1370 | 0.1504 | 1.098 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=8; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1601 | 0.1689 | 1.055 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=8; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1601 | 0.1503 | 0.939 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=8; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.0593 | 0.0775 | 1.307 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=8; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.0593 | 0.0736 | 1.240 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=8; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1743 | 0.2023 | 1.161 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=8; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1743 | 0.1786 | 1.025 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=8; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.0596 | 0.0805 | 1.351 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=8; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.0596 | 0.0700 | 1.175 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=10; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1527 | 0.1702 | 1.115 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=10; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1527 | 0.1437 | 0.941 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=10; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.0802 | 0.0824 | 1.027 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=10; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.0802 | 0.0871 | 1.085 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=10; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1538 | 0.2019 | 1.312 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=10; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1538 | 0.1663 | 1.081 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=10; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.0795 | 0.0822 | 1.035 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=10; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.0795 | 0.0859 | 1.082 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=16; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.1532 | 0.2021 | 1.319 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=16; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1532 | 0.1666 | 1.088 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=16; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.1527 | 0.2035 | 1.333 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=16; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1527 | 0.1665 | 1.090 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=32; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1500 | 0.1795 | 1.197 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=32; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1500 | 0.1533 | 1.022 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=32; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.1468 | 0.1917 | 1.306 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=32; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.1468 | 0.1694 | 1.154 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=32; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1437 | 0.1958 | 1.362 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=32; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1437 | 0.1442 | 1.003 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=32; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.1446 | 0.2140 | 1.480 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=16; destination_bits=32; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.1446 | 0.1957 | 1.354 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=8; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1542 | 0.1986 | 1.288 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=8; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1542 | 0.1781 | 1.155 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=8; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.1899 | 0.2137 | 1.125 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=8; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.1899 | 0.2041 | 1.075 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=8; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1536 | 0.1802 | 1.174 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=8; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1536 | 0.1685 | 1.097 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=8; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.1572 | 0.1783 | 1.134 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=8; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.1572 | 0.1727 | 1.099 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=10; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1437 | 0.1862 | 1.296 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=10; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1437 | 0.1678 | 1.168 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=10; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.1895 | 0.2220 | 1.171 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=10; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.1895 | 0.2077 | 1.096 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=10; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1435 | 0.1817 | 1.266 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=10; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1435 | 0.1627 | 1.134 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=10; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.1560 | 0.1886 | 1.209 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=10; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.1560 | 0.1849 | 1.185 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=16; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.2029 | 0.1915 | 0.944 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=16; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.2029 | 0.1762 | 0.869 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=16; source_full=0; destination_full=0; chroma=0 | AVX2 | 0.1797 | 0.2127 | 1.184 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=16; source_full=0; destination_full=0; chroma=0 | AVX3_ZEN4 | 0.1797 | 0.1922 | 1.070 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=16; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1500 | 0.1667 | 1.111 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=16; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1500 | 0.1602 | 1.068 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=16; source_full=0; destination_full=0; chroma=1 | AVX2 | 0.1423 | 0.1710 | 1.202 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=16; source_full=0; destination_full=0; chroma=1 | AVX3_ZEN4 | 0.1423 | 0.1639 | 1.152 | AVX2 | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=32; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.3317 | 0.3449 | 1.040 | C | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=32; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.3317 | 0.3251 | 0.980 | C | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=32; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.3447 | 0.3477 | 1.009 | C | Fresh three-round measurement. Old FMA may differ from strict C rounding. |
| source_bits=32; destination_bits=32; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.3447 | 0.3279 | 0.951 | C | Fresh three-round measurement. Old FMA may differ from strict C rounding. |

</details>

<details>
<summary>layout — 108 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| pack_BGR8_noalpha; 256; 32 | AVX2 | unavailable | 0.0007 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR8_noalpha; 256; 32 | native | unavailable | 0.0006 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_noalpha; 256; 32 | AVX2 | unavailable | 0.0014 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_noalpha; 256; 32 | native | unavailable | 0.0010 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| unpack_BGR8_noalpha; 256; 32 | AVX2 | 0.0006 | 0.0005 | 0.901 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR8_noalpha; 256; 32 | native | 0.0006 | 0.0005 | 0.888 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_noalpha; 256; 32 | AVX2 | 0.0011 | 0.0010 | 0.872 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_noalpha; 256; 32 | native | 0.0011 | 0.0010 | 0.921 | AVX2 | Fresh upstream layout comparison. |
| pack_BGR8_alpha; 256; 32 | AVX2 | unavailable | 0.0007 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR8_alpha; 256; 32 | native | unavailable | 0.0006 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_alpha; 256; 32 | AVX2 | unavailable | 0.0013 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_alpha; 256; 32 | native | unavailable | 0.0011 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| unpack_BGR8_alpha; 256; 32 | AVX2 | 0.0006 | 0.0006 | 0.883 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR8_alpha; 256; 32 | native | 0.0006 | 0.0005 | 0.823 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_alpha; 256; 32 | AVX2 | 0.0018 | 0.0010 | 0.556 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_alpha; 256; 32 | native | 0.0018 | 0.0010 | 0.555 | AVX2 | Fresh upstream layout comparison. |
| pack_BGRA8_noalpha; 256; 32 | AVX2 | 0.0009 | 0.0005 | 0.567 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA8_noalpha; 256; 32 | native | 0.0009 | 0.0005 | 0.563 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_noalpha; 256; 32 | AVX2 | 0.0018 | 0.0010 | 0.588 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_noalpha; 256; 32 | native | 0.0018 | 0.0010 | 0.583 | SSE2 | Fresh upstream layout comparison. |
| unpack_BGRA8_noalpha; 256; 32 | AVX2 | 0.0009 | 0.0006 | 0.597 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA8_noalpha; 256; 32 | native | 0.0007 | 0.0005 | 0.713 | AVX512 | Fresh upstream layout comparison. |
| unpack_BGRA16_noalpha; 256; 32 | AVX2 | 0.0019 | 0.0011 | 0.548 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA16_noalpha; 256; 32 | native | 0.0013 | 0.0010 | 0.790 | AVX512 | Fresh upstream layout comparison. |
| pack_BGRA8_alpha; 256; 32 | AVX2 | 0.0014 | 0.0006 | 0.403 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA8_alpha; 256; 32 | native | 0.0014 | 0.0007 | 0.469 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_alpha; 256; 32 | AVX2 | 0.0024 | 0.0012 | 0.514 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_alpha; 256; 32 | native | 0.0024 | 0.0012 | 0.505 | SSE2 | Fresh upstream layout comparison. |
| unpack_BGRA8_alpha; 256; 32 | AVX2 | 0.0012 | 0.0006 | 0.500 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA8_alpha; 256; 32 | native | 0.0007 | 0.0006 | 0.882 | AVX512 | Fresh upstream layout comparison. |
| unpack_BGRA16_alpha; 256; 32 | AVX2 | 0.0027 | 0.0012 | 0.448 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA16_alpha; 256; 32 | native | 0.0017 | 0.0012 | 0.710 | AVX512 | Fresh upstream layout comparison. |
| pack_YUY2; 256; 32 | AVX2 | 0.0004 | 0.0003 | 0.679 | SSE2 | Fresh upstream layout comparison. |
| pack_YUY2; 256; 32 | native | 0.0004 | 0.0003 | 0.634 | SSE2 | Fresh upstream layout comparison. |
| unpack_YUY2; 256; 32 | AVX2 | 0.0017 | 0.0003 | 0.203 | SSE2 | Fresh upstream layout comparison. |
| unpack_YUY2; 256; 32 | native | 0.0017 | 0.0003 | 0.168 | SSE2 | Fresh upstream layout comparison. |
| pack_BGR8_noalpha; 1920; 1080 | AVX2 | unavailable | 0.2201 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR8_noalpha; 1920; 1080 | native | unavailable | 0.2041 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_noalpha; 1920; 1080 | AVX2 | unavailable | 1.0185 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_noalpha; 1920; 1080 | native | unavailable | 1.0077 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| unpack_BGR8_noalpha; 1920; 1080 | AVX2 | 0.6297 | 0.2128 | 0.338 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR8_noalpha; 1920; 1080 | native | 0.6297 | 0.1931 | 0.307 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_noalpha; 1920; 1080 | AVX2 | 1.1985 | 1.2506 | 1.044 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_noalpha; 1920; 1080 | native | 1.1985 | 0.9826 | 0.820 | AVX2 | Fresh upstream layout comparison. |
| pack_BGR8_alpha; 1920; 1080 | AVX2 | unavailable | 0.2307 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR8_alpha; 1920; 1080 | native | unavailable | 0.2230 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_alpha; 1920; 1080 | AVX2 | unavailable | 1.0231 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_alpha; 1920; 1080 | native | unavailable | 0.8841 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| unpack_BGR8_alpha; 1920; 1080 | AVX2 | 0.8184 | 0.3431 | 0.419 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR8_alpha; 1920; 1080 | native | 0.8184 | 0.2691 | 0.329 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_alpha; 1920; 1080 | AVX2 | 1.7089 | 1.3435 | 0.786 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_alpha; 1920; 1080 | native | 1.7089 | 1.0987 | 0.643 | AVX2 | Fresh upstream layout comparison. |
| pack_BGRA8_noalpha; 1920; 1080 | AVX2 | 0.4431 | 0.2949 | 0.666 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA8_noalpha; 1920; 1080 | native | 0.4431 | 0.2838 | 0.641 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_noalpha; 1920; 1080 | AVX2 | 1.5175 | 1.2101 | 0.797 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_noalpha; 1920; 1080 | native | 1.5175 | 1.2265 | 0.808 | SSE2 | Fresh upstream layout comparison. |
| unpack_BGRA8_noalpha; 1920; 1080 | AVX2 | 0.7452 | 0.6999 | 0.939 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA8_noalpha; 1920; 1080 | native | 0.6631 | 0.3052 | 0.460 | AVX512 | Fresh upstream layout comparison. |
| unpack_BGRA16_noalpha; 1920; 1080 | AVX2 | 1.6795 | 1.4477 | 0.862 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA16_noalpha; 1920; 1080 | native | 1.3174 | 1.1291 | 0.857 | AVX512 | Fresh upstream layout comparison. |
| pack_BGRA8_alpha; 1920; 1080 | AVX2 | 0.6979 | 0.5016 | 0.719 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA8_alpha; 1920; 1080 | native | 0.6979 | 0.5550 | 0.795 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_alpha; 1920; 1080 | AVX2 | 1.5234 | 1.3809 | 0.906 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_alpha; 1920; 1080 | native | 1.5234 | 1.3531 | 0.888 | SSE2 | Fresh upstream layout comparison. |
| unpack_BGRA8_alpha; 1920; 1080 | AVX2 | 1.7060 | 0.9054 | 0.531 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA8_alpha; 1920; 1080 | native | 1.6410 | 0.6384 | 0.389 | AVX512 | Fresh upstream layout comparison. |
| unpack_BGRA16_alpha; 1920; 1080 | AVX2 | 2.1445 | 1.6204 | 0.756 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA16_alpha; 1920; 1080 | native | 1.7018 | 1.2347 | 0.726 | AVX512 | Fresh upstream layout comparison. |
| pack_YUY2; 1920; 1080 | AVX2 | 0.1235 | 0.0867 | 0.702 | SSE2 | Fresh upstream layout comparison. |
| pack_YUY2; 1920; 1080 | native | 0.1235 | 0.0892 | 0.722 | SSE2 | Fresh upstream layout comparison. |
| unpack_YUY2; 1920; 1080 | AVX2 | 0.4166 | 0.0989 | 0.237 | SSE2 | Fresh upstream layout comparison. |
| unpack_YUY2; 1920; 1080 | native | 0.4166 | 0.0964 | 0.231 | SSE2 | Fresh upstream layout comparison. |
| pack_BGR8_noalpha; 3840; 2160 | AVX2 | unavailable | 2.2449 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR8_noalpha; 3840; 2160 | native | unavailable | 2.2006 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_noalpha; 3840; 2160 | AVX2 | unavailable | 4.2221 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_noalpha; 3840; 2160 | native | unavailable | 3.9970 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| unpack_BGR8_noalpha; 3840; 2160 | AVX2 | 2.2442 | 2.7802 | 1.239 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR8_noalpha; 3840; 2160 | native | 2.2442 | 2.1287 | 0.949 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_noalpha; 3840; 2160 | AVX2 | 4.3833 | 4.7878 | 1.092 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_noalpha; 3840; 2160 | native | 4.3833 | 3.8715 | 0.883 | AVX2 | Fresh upstream layout comparison. |
| pack_BGR8_alpha; 3840; 2160 | AVX2 | unavailable | 2.1091 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR8_alpha; 3840; 2160 | native | unavailable | 2.1974 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_alpha; 3840; 2160 | AVX2 | unavailable | 4.0830 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| pack_BGR16_alpha; 3840; 2160 | native | unavailable | 3.8075 | unavailable | not available | No raw old SIMD counterpart; corresponding full-filter routes are measured separately. |
| unpack_BGR8_alpha; 3840; 2160 | AVX2 | 3.0801 | 2.8338 | 0.920 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR8_alpha; 3840; 2160 | native | 3.0801 | 2.3420 | 0.760 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_alpha; 3840; 2160 | AVX2 | 6.2528 | 5.2527 | 0.840 | AVX2 | Fresh upstream layout comparison. |
| unpack_BGR16_alpha; 3840; 2160 | native | 6.2528 | 4.4015 | 0.704 | AVX2 | Fresh upstream layout comparison. |
| pack_BGRA8_noalpha; 3840; 2160 | AVX2 | 2.7121 | 2.3381 | 0.862 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA8_noalpha; 3840; 2160 | native | 2.7121 | 2.3921 | 0.882 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_noalpha; 3840; 2160 | AVX2 | 5.1392 | 4.5319 | 0.882 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_noalpha; 3840; 2160 | native | 5.1392 | 4.7293 | 0.920 | SSE2 | Fresh upstream layout comparison. |
| unpack_BGRA8_noalpha; 3840; 2160 | AVX2 | 3.1422 | 2.8139 | 0.896 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA8_noalpha; 3840; 2160 | native | 2.5192 | 2.2507 | 0.893 | AVX512 | Fresh upstream layout comparison. |
| unpack_BGRA16_noalpha; 3840; 2160 | AVX2 | 5.9054 | 5.1942 | 0.880 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA16_noalpha; 3840; 2160 | native | 4.7726 | 4.2624 | 0.893 | AVX512 | Fresh upstream layout comparison. |
| pack_BGRA8_alpha; 3840; 2160 | AVX2 | 2.8371 | 2.7390 | 0.965 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA8_alpha; 3840; 2160 | native | 2.8371 | 2.4842 | 0.876 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_alpha; 3840; 2160 | AVX2 | 5.4764 | 4.9011 | 0.895 | SSE2 | Fresh upstream layout comparison. |
| pack_BGRA16_alpha; 3840; 2160 | native | 5.4764 | 4.8439 | 0.884 | SSE2 | Fresh upstream layout comparison. |
| unpack_BGRA8_alpha; 3840; 2160 | AVX2 | 3.6862 | 3.1193 | 0.846 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA8_alpha; 3840; 2160 | native | 3.0039 | 2.5433 | 0.847 | AVX512 | Fresh upstream layout comparison. |
| unpack_BGRA16_alpha; 3840; 2160 | AVX2 | 7.2521 | 5.9932 | 0.826 | SSSE3 | Fresh upstream layout comparison. |
| unpack_BGRA16_alpha; 3840; 2160 | native | 5.9623 | 4.7878 | 0.803 | AVX512 | Fresh upstream layout comparison. |
| pack_YUY2; 3840; 2160 | AVX2 | 0.7315 | 0.7581 | 1.036 | SSE2 | Fresh upstream layout comparison. |
| pack_YUY2; 3840; 2160 | native | 0.7315 | 0.6603 | 0.903 | SSE2 | Fresh upstream layout comparison. |
| unpack_YUY2; 3840; 2160 | AVX2 | 1.7577 | 1.1735 | 0.668 | SSE2 | Fresh upstream layout comparison. |
| unpack_YUY2; 3840; 2160 | native | 1.7577 | 0.9379 | 0.534 | SSE2 | Fresh upstream layout comparison. |

</details>

<details>
<summary>matrix — 64 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| route=RGB-YUV; bits=8; source_full=1; destination_full=0; width=1920; height=1080 | AVX2 | 0.5195 | 0.4724 | 0.909 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=0; width=1920; height=1080 | AVX3_ZEN4 | 0.5195 | 0.4681 | 0.901 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=0; destination_full=1; width=1920; height=1080 | AVX2 | 0.4994 | 0.5041 | 1.009 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=0; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.4994 | 0.4825 | 0.966 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=0; width=3840; height=2160 | AVX2 | 2.2549 | 2.2320 | 0.990 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=0; width=3840; height=2160 | AVX3_ZEN4 | 2.2549 | 2.1153 | 0.938 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=0; destination_full=1; width=3840; height=2160 | AVX2 | 2.2234 | 2.2336 | 1.005 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=0; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 2.2234 | 2.1088 | 0.948 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.5262 | 0.4948 | 0.940 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.5262 | 0.4678 | 0.889 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.4995 | 0.5071 | 1.015 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.4995 | 0.4710 | 0.943 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 2.2969 | 2.1867 | 0.952 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 2.2969 | 2.1148 | 0.921 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 2.2011 | 2.2221 | 1.010 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 2.2011 | 2.1034 | 0.956 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=0; width=1920; height=1080 | AVX2 | 0.8424 | 0.7868 | 0.934 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=0; width=1920; height=1080 | AVX3_ZEN4 | 0.8424 | 0.7378 | 0.876 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=0; destination_full=1; width=1920; height=1080 | AVX2 | 0.8177 | 0.8137 | 0.995 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=0; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.8177 | 0.7412 | 0.906 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=0; width=3840; height=2160 | AVX2 | 3.7488 | 3.6482 | 0.973 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=0; width=3840; height=2160 | AVX3_ZEN4 | 3.7488 | 3.3982 | 0.906 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=0; destination_full=1; width=3840; height=2160 | AVX2 | 3.4150 | 3.5107 | 1.028 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=0; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.4150 | 3.3591 | 0.984 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.8362 | 0.7868 | 0.941 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.8362 | 0.7287 | 0.872 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.7731 | 0.8364 | 1.082 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.7731 | 0.7342 | 0.950 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 3.5432 | 3.4327 | 0.969 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.5432 | 3.3732 | 0.952 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 3.4300 | 3.4387 | 1.003 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.4300 | 3.3850 | 0.987 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=0; width=1920; height=1080 | AVX2 | 0.9465 | 0.7383 | 0.780 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=0; width=1920; height=1080 | AVX3_ZEN4 | 0.9465 | 0.6911 | 0.730 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=0; destination_full=1; width=1920; height=1080 | AVX2 | 0.7752 | 0.7714 | 0.995 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=0; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.7752 | 0.6823 | 0.880 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=0; width=3840; height=2160 | AVX2 | 3.7026 | 3.3799 | 0.913 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=0; width=3840; height=2160 | AVX3_ZEN4 | 3.7026 | 3.5500 | 0.959 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=0; destination_full=1; width=3840; height=2160 | AVX2 | 3.4183 | 3.3900 | 0.992 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=0; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.4183 | 3.4213 | 1.001 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.9919 | 0.8227 | 0.829 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.9919 | 0.8017 | 0.808 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.8357 | 0.8313 | 0.995 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.8357 | 0.7228 | 0.865 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 3.8395 | 3.4665 | 0.903 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.8395 | 3.4318 | 0.894 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 3.4013 | 3.4395 | 1.011 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.4013 | 3.3725 | 0.992 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=0; width=1920; height=1080 | AVX2 | 1.5241 | 1.5713 | 1.031 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=0; width=1920; height=1080 | AVX3_ZEN4 | 1.5241 | 1.6579 | 1.088 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=0; destination_full=1; width=1920; height=1080 | AVX2 | 1.4873 | 1.5815 | 1.063 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=0; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 1.4873 | 1.6644 | 1.119 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=0; width=3840; height=2160 | AVX2 | 6.7368 | 6.6701 | 0.990 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=0; width=3840; height=2160 | AVX3_ZEN4 | 6.7368 | 6.4249 | 0.954 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=0; destination_full=1; width=3840; height=2160 | AVX2 | 6.1183 | 6.7108 | 1.097 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=0; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 6.1183 | 6.3322 | 1.035 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 1.5194 | 1.5737 | 1.036 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 1.5194 | 1.5282 | 1.006 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 1.4766 | 1.5891 | 1.076 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 1.4766 | 1.6069 | 1.088 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 6.5819 | 6.7352 | 1.023 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 6.5819 | 6.6299 | 1.007 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 6.3283 | 6.6725 | 1.054 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 6.3283 | 6.3360 | 1.001 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |

</details>

<details>
<summary>ordered — 48 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.0887 | 0.0711 | 0.801 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.0887 | 0.0649 | 0.732 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.2482 | 0.3504 | 1.412 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.2482 | 0.3011 | 1.213 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.0929 | 0.0770 | 0.828 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.0929 | 0.0754 | 0.811 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.2480 | 0.3519 | 1.419 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.2480 | 0.2998 | 1.209 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.0926 | 0.0772 | 0.834 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.0926 | 0.0710 | 0.766 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.2482 | 0.3501 | 1.411 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.2482 | 0.2999 | 1.208 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1005 | 0.0807 | 0.803 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1005 | 0.0686 | 0.683 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.2472 | 0.3493 | 1.413 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.2472 | 0.2996 | 1.212 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.0841 | 0.0955 | 1.136 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.0841 | 0.0881 | 1.049 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.2285 | 0.3443 | 1.507 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.2285 | 0.2962 | 1.297 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.0853 | 0.0977 | 1.145 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.0853 | 0.0912 | 1.069 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.2290 | 0.3420 | 1.494 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.2290 | 0.2950 | 1.288 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1017 | 0.0958 | 0.941 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1017 | 0.0891 | 0.876 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.2589 | 0.4321 | 1.669 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.2589 | 0.3583 | 1.384 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.0985 | 0.0967 | 0.982 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.0985 | 0.0876 | 0.890 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.2572 | 0.4338 | 1.687 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.2572 | 0.3589 | 1.396 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.3922 | 0.3975 | 1.013 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.3922 | 0.3261 | 0.831 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.6156 | 0.7117 | 1.156 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.6156 | 0.5501 | 0.894 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.3954 | 0.3966 | 1.003 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.3954 | 0.3243 | 0.820 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.6204 | 0.7062 | 1.138 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.6204 | 0.5436 | 0.876 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.3325 | 0.3922 | 1.180 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.3325 | 0.3198 | 0.962 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.5545 | 0.7074 | 1.276 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.5545 | 0.5358 | 0.966 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.3321 | 0.3917 | 1.179 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.3321 | 0.3199 | 0.963 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.5506 | 0.7078 | 1.285 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.5506 | 0.5382 | 0.977 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |

</details>

<details>
<summary>resample-axis — 198 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| axis=V; filter=triangle; bits=8; width=640; height=360; target=180 | AVX2 | 0.0132 | 0.0183 | 1.383 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=8; width=640; height=360; target=180 | native | 0.0135 | 0.0136 | 1.007 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=16; width=640; height=360; target=180 | AVX2 | 0.0188 | 0.0171 | 0.910 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=16; width=640; height=360; target=180 | native | 0.0132 | 0.0125 | 0.948 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=32; width=640; height=360; target=180 | AVX2 | 0.0147 | 0.0197 | 1.342 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=32; width=640; height=360; target=180 | native | 0.0138 | 0.0158 | 1.148 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=8; width=640; height=360; target=180 | AVX2 | 0.0310 | 0.0428 | 1.382 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=8; width=640; height=360; target=180 | native | 0.0357 | 0.0348 | 0.975 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=16; width=640; height=360; target=180 | AVX2 | 0.0386 | 0.0388 | 1.006 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=16; width=640; height=360; target=180 | native | 0.0328 | 0.0273 | 0.832 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=32; width=640; height=360; target=180 | AVX2 | 0.0352 | 0.0423 | 1.201 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=32; width=640; height=360; target=180 | native | 0.0297 | 0.0348 | 1.171 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=8; width=640; height=360; target=180 | AVX2 | 0.0313 | 0.0430 | 1.374 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=8; width=640; height=360; target=180 | native | 0.0357 | 0.0350 | 0.980 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=16; width=640; height=360; target=180 | AVX2 | 0.0387 | 0.0397 | 1.025 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=16; width=640; height=360; target=180 | native | 0.0328 | 0.0277 | 0.844 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=32; width=640; height=360; target=180 | AVX2 | 0.0373 | 0.0422 | 1.132 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=32; width=640; height=360; target=180 | native | 0.0298 | 0.0350 | 1.176 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=8; width=640; height=360; target=540 | AVX2 | 0.0244 | 0.0141 | 0.578 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=8; width=640; height=360; target=540 | native | 0.0249 | 0.0120 | 0.482 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=16; width=640; height=360; target=540 | AVX2 | 0.0401 | 0.0318 | 0.794 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=16; width=640; height=360; target=540 | native | 0.0246 | 0.0265 | 1.074 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=32; width=640; height=360; target=540 | AVX2 | 0.0244 | 0.0306 | 1.257 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=32; width=640; height=360; target=540 | native | 0.0251 | 0.0280 | 1.116 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=8; width=640; height=360; target=540 | AVX2 | 0.0550 | 0.0773 | 1.406 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=8; width=640; height=360; target=540 | native | 0.0579 | 0.0594 | 1.025 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=16; width=640; height=360; target=540 | AVX2 | 0.0726 | 0.0712 | 0.981 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=16; width=640; height=360; target=540 | native | 0.0547 | 0.0506 | 0.924 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=32; width=640; height=360; target=540 | AVX2 | 0.0497 | 0.0666 | 1.340 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=32; width=640; height=360; target=540 | native | 0.0453 | 0.0533 | 1.177 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=8; width=640; height=360; target=540 | AVX2 | 0.0561 | 0.0770 | 1.374 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=8; width=640; height=360; target=540 | native | 0.0581 | 0.0586 | 1.009 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=16; width=640; height=360; target=540 | AVX2 | 0.0734 | 0.0726 | 0.990 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=16; width=640; height=360; target=540 | native | 0.0549 | 0.0511 | 0.931 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=32; width=640; height=360; target=540 | AVX2 | 0.0502 | 0.0659 | 1.313 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=32; width=640; height=360; target=540 | native | 0.0472 | 0.0538 | 1.141 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=640; height=360; target=320 | AVX2 | 0.0723 | 0.0696 | 0.962 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=640; height=360; target=320 | native | 0.0230 | 0.0246 | 1.068 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=640; height=360; target=320 | AVX2 | 0.0816 | 0.0657 | 0.805 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=640; height=360; target=320 | native | 0.0340 | 0.0267 | 0.786 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=640; height=360; target=320 | AVX2 | 0.1543 | 0.0811 | 0.525 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=640; height=360; target=320 | native | 0.0338 | 0.0480 | 1.421 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=640; height=360; target=320 | AVX2 | 0.0724 | 0.0889 | 1.228 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=640; height=360; target=320 | native | 0.0396 | 0.0431 | 1.088 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=640; height=360; target=320 | AVX2 | 0.0827 | 0.0897 | 1.085 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=640; height=360; target=320 | native | 0.0467 | 0.0463 | 0.991 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=640; height=360; target=320 | AVX2 | 0.1096 | 0.1341 | 1.224 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=640; height=360; target=320 | native | 0.1133 | 0.1169 | 1.032 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=640; height=360; target=320 | AVX2 | 0.0726 | 0.0878 | 1.210 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=640; height=360; target=320 | native | 0.0397 | 0.0428 | 1.076 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=640; height=360; target=320 | AVX2 | 0.0824 | 0.0928 | 1.126 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=640; height=360; target=320 | native | 0.0461 | 0.0499 | 1.084 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=640; height=360; target=320 | AVX2 | 0.1193 | 0.1336 | 1.120 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=640; height=360; target=320 | native | 0.1138 | 0.1203 | 1.057 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=640; height=360; target=960 | AVX2 | 0.2662 | 0.1157 | 0.435 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=640; height=360; target=960 | native | 0.0414 | 0.0600 | 1.450 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=640; height=360; target=960 | AVX2 | 0.2980 | 0.1190 | 0.399 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=640; height=360; target=960 | native | 0.0404 | 0.0645 | 1.598 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=640; height=360; target=960 | AVX2 | 0.4677 | 0.1221 | 0.261 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=640; height=360; target=960 | native | 0.0649 | 0.0844 | 1.300 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=640; height=360; target=960 | AVX2 | 0.2710 | 0.1642 | 0.606 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=640; height=360; target=960 | native | 0.0720 | 0.0850 | 1.180 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=640; height=360; target=960 | AVX2 | 0.2904 | 0.1767 | 0.609 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=640; height=360; target=960 | native | 0.0725 | 0.0954 | 1.316 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=640; height=360; target=960 | AVX2 | 0.2055 | 0.2127 | 1.035 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=640; height=360; target=960 | native | 0.1337 | 0.1418 | 1.061 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=640; height=360; target=960 | AVX2 | 0.2723 | 0.1655 | 0.608 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=640; height=360; target=960 | native | 0.0721 | 0.0871 | 1.208 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=640; height=360; target=960 | AVX2 | 0.2945 | 0.1757 | 0.597 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=640; height=360; target=960 | native | 0.0711 | 0.0925 | 1.300 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=640; height=360; target=960 | AVX2 | 0.2121 | 0.2102 | 0.991 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=640; height=360; target=960 | native | 0.1358 | 0.1472 | 1.084 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=8; width=1920; height=1080; target=540 | AVX2 | 0.1265 | 0.1791 | 1.416 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=8; width=1920; height=1080; target=540 | native | 0.1243 | 0.1261 | 1.015 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=16; width=1920; height=1080; target=540 | AVX2 | 0.1769 | 0.1793 | 1.014 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=16; width=1920; height=1080; target=540 | native | 0.1308 | 0.1340 | 1.024 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=32; width=1920; height=1080; target=540 | AVX2 | 0.1959 | 0.2150 | 1.097 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=32; width=1920; height=1080; target=540 | native | 0.1980 | 0.1955 | 0.987 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=8; width=1920; height=1080; target=540 | AVX2 | 0.2922 | 0.4169 | 1.427 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=8; width=1920; height=1080; target=540 | native | 0.3303 | 0.3234 | 0.979 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=16; width=1920; height=1080; target=540 | AVX2 | 0.3830 | 0.3844 | 1.004 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=16; width=1920; height=1080; target=540 | native | 0.3052 | 0.2679 | 0.878 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=32; width=1920; height=1080; target=540 | AVX2 | 0.6333 | 0.6331 | 1.000 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=32; width=1920; height=1080; target=540 | native | 0.5968 | 0.5384 | 0.902 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=8; width=1920; height=1080; target=540 | AVX2 | 0.2962 | 0.4057 | 1.370 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=8; width=1920; height=1080; target=540 | native | 0.3275 | 0.3281 | 1.002 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=16; width=1920; height=1080; target=540 | AVX2 | 0.3617 | 0.3620 | 1.001 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=16; width=1920; height=1080; target=540 | native | 0.3108 | 0.2615 | 0.841 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=32; width=1920; height=1080; target=540 | AVX2 | 0.5173 | 0.5659 | 1.094 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=32; width=1920; height=1080; target=540 | native | 0.5231 | 0.6002 | 1.147 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=8; width=1920; height=1080; target=1620 | AVX2 | 0.2257 | 0.1188 | 0.526 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=8; width=1920; height=1080; target=1620 | native | 0.2243 | 0.1091 | 0.487 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=16; width=1920; height=1080; target=1620 | AVX2 | 0.3592 | 0.2765 | 0.770 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=16; width=1920; height=1080; target=1620 | native | 0.2267 | 0.2368 | 1.045 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=32; width=1920; height=1080; target=1620 | AVX2 | 0.4117 | 0.3540 | 0.860 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=triangle; bits=32; width=1920; height=1080; target=1620 | native | 0.2748 | 0.4191 | 1.525 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=8; width=1920; height=1080; target=1620 | AVX2 | 0.4863 | 0.6901 | 1.419 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=8; width=1920; height=1080; target=1620 | native | 0.5178 | 0.5322 | 1.028 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=16; width=1920; height=1080; target=1620 | AVX2 | 0.6382 | 0.6416 | 1.005 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=16; width=1920; height=1080; target=1620 | native | 0.5298 | 0.4757 | 0.898 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=32; width=1920; height=1080; target=1620 | AVX2 | 0.7499 | 0.8150 | 1.087 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=lanczos3; bits=32; width=1920; height=1080; target=1620 | native | 0.8355 | 0.8074 | 0.966 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=8; width=1920; height=1080; target=1620 | AVX2 | 0.4784 | 0.6711 | 1.403 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=8; width=1920; height=1080; target=1620 | native | 0.5205 | 0.5293 | 1.017 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=16; width=1920; height=1080; target=1620 | AVX2 | 0.6247 | 0.6133 | 0.982 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=16; width=1920; height=1080; target=1620 | native | 0.5126 | 0.4665 | 0.910 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=32; width=1920; height=1080; target=1620 | AVX2 | 0.7653 | 0.8296 | 1.084 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=V; filter=spline36; bits=32; width=1920; height=1080; target=1620 | native | 0.7160 | 0.8026 | 1.121 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=960 | AVX2 | 0.6411 | 0.5588 | 0.872 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=960 | native | 0.2106 | 0.2037 | 0.968 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=960 | AVX2 | 0.7259 | 0.5676 | 0.782 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=960 | native | 0.3282 | 0.2435 | 0.742 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=960 | AVX2 | 1.5623 | 0.7547 | 0.483 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=960 | native | 0.5650 | 0.5335 | 0.944 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=960 | AVX2 | 0.6762 | 0.7429 | 1.099 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=960 | native | 0.3701 | 0.4953 | 1.338 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=960 | AVX2 | 0.7312 | 0.7358 | 1.006 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=960 | native | 0.4140 | 0.5650 | 1.365 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=960 | AVX2 | 1.0617 | 1.2094 | 1.139 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=960 | native | 1.1187 | 1.1697 | 1.046 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=960 | AVX2 | 0.6445 | 0.7067 | 1.097 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=960 | native | 0.3625 | 0.4822 | 1.330 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=960 | AVX2 | 0.7325 | 0.7370 | 1.006 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=960 | native | 0.4169 | 0.5472 | 1.312 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=960 | AVX2 | 1.0880 | 1.2071 | 1.109 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=960 | native | 1.1598 | 1.1405 | 0.983 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=2880 | AVX2 | 2.0130 | 0.9439 | 0.469 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=2880 | native | 0.4051 | 0.5367 | 1.325 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=2880 | AVX2 | 2.3284 | 1.1094 | 0.476 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=2880 | native | 0.4151 | 0.6789 | 1.635 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=2880 | AVX2 | 4.4401 | 1.1538 | 0.260 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=2880 | native | 1.1459 | 0.9122 | 0.796 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=2880 | AVX2 | 2.1179 | 1.5708 | 0.742 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=2880 | native | 0.6734 | 0.8579 | 1.274 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=2880 | AVX2 | 2.3723 | 1.6246 | 0.685 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=2880 | native | 0.6695 | 0.9525 | 1.423 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=2880 | AVX2 | 1.8975 | 1.9235 | 1.014 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=2880 | native | 1.2347 | 1.4006 | 1.134 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=2880 | AVX2 | 2.0616 | 1.5593 | 0.756 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=2880 | native | 0.6704 | 0.8680 | 1.295 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=2880 | AVX2 | 2.3028 | 1.5914 | 0.691 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=2880 | native | 0.6638 | 0.9477 | 1.428 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=2880 | AVX2 | 1.9772 | 1.9378 | 0.980 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=2880 | native | 1.2377 | 1.4520 | 1.173 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=1152 | AVX2 | 0.7600 | 0.4801 | 0.632 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=1152 | native | 0.1809 | 0.2501 | 1.382 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=1152 | AVX2 | 0.8674 | 0.5089 | 0.587 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=1152 | native | 0.1783 | 0.2963 | 1.662 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=1152 | AVX2 | 1.7638 | 0.6706 | 0.380 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=1152 | native | 0.4703 | 0.4656 | 0.990 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=1152 | AVX2 | 0.7845 | 0.8521 | 1.086 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=1152 | native | 0.4380 | 0.5112 | 1.167 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=1152 | AVX2 | 0.8863 | 0.8790 | 0.992 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=1152 | native | 0.4186 | 0.5338 | 1.275 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=1152 | AVX2 | 1.2367 | 1.2690 | 1.026 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=1152 | native | 1.4467 | 1.1374 | 0.786 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=1152 | AVX2 | 0.8006 | 0.8428 | 1.053 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=1152 | native | 0.4425 | 0.4872 | 1.101 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=1152 | AVX2 | 0.8976 | 0.8940 | 0.996 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=1152 | native | 0.4218 | 0.5499 | 1.304 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=1152 | AVX2 | 1.2243 | 1.2782 | 1.044 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=1152 | native | 1.4649 | 1.1572 | 0.790 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=1280 | AVX2 | 0.9069 | 0.5317 | 0.586 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=1280 | native | 0.1845 | 0.2834 | 1.536 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=1280 | AVX2 | 1.0458 | 0.5775 | 0.552 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=1280 | native | 0.2269 | 0.3508 | 1.546 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=1280 | AVX2 | 2.6713 | 0.6695 | 0.251 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=1280 | native | 0.6762 | 0.4776 | 0.706 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=1280 | AVX2 | 0.8991 | 1.0207 | 1.135 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=1280 | native | 0.4937 | 0.5587 | 1.132 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=1280 | AVX2 | 1.0090 | 1.0174 | 1.008 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=1280 | native | 0.6460 | 0.6286 | 0.973 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=1280 | AVX2 | 1.4019 | 1.3060 | 0.932 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=1280 | native | 1.5126 | 0.8942 | 0.591 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=1280 | AVX2 | 0.8654 | 0.9536 | 1.102 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=1280 | native | 0.4971 | 0.5782 | 1.163 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=1280 | AVX2 | 1.0307 | 1.0095 | 0.979 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=1280 | native | 0.6712 | 0.6305 | 0.939 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=1280 | AVX2 | 1.4520 | 1.3097 | 0.902 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=1280 | native | 1.5604 | 0.8934 | 0.573 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=2400 | AVX2 | 1.7323 | 0.8690 | 0.502 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=2400 | native | 0.3461 | 0.4633 | 1.339 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=2400 | AVX2 | 1.9686 | 0.9292 | 0.472 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=2400 | native | 0.3476 | 0.5554 | 1.598 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=2400 | AVX2 | 3.6982 | 0.9730 | 0.263 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=2400 | native | 0.8287 | 0.7243 | 0.874 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=2400 | AVX2 | 1.7319 | 1.2265 | 0.708 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=2400 | native | 0.5633 | 0.7058 | 1.253 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=2400 | AVX2 | 1.9541 | 1.3027 | 0.667 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=2400 | native | 0.5629 | 0.8021 | 1.425 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=2400 | AVX2 | 1.6125 | 1.6636 | 1.032 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=2400 | native | 1.0626 | 1.2136 | 1.142 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=2400 | AVX2 | 1.6327 | 1.5329 | 0.939 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=2400 | native | 0.5605 | 0.7173 | 1.280 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=2400 | AVX2 | 1.8923 | 1.3433 | 0.710 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=2400 | native | 0.5730 | 0.8310 | 1.450 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=2400 | AVX2 | 1.6593 | 1.6535 | 0.997 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=2400 | native | 1.0530 | 1.2831 | 1.219 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |

</details>

<details>
<summary>resample-pipeline — 54 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| order=HV; filter=triangle; bits=8; dw=960; dh=540 | AVX2 | 0.7141 | 0.6073 | 0.850 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=8; dw=960; dh=540 | native | 0.3077 | 0.2748 | 0.893 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=16; dw=960; dh=540 | AVX2 | 0.9695 | 0.6745 | 0.696 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=16; dw=960; dh=540 | native | 0.6007 | 0.3409 | 0.567 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=32; dw=960; dh=540 | AVX2 | 1.8179 | 1.1669 | 0.642 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=32; dw=960; dh=540 | native | 0.9941 | 0.8359 | 0.841 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=8; dw=960; dh=540 | AVX2 | 0.7855 | 0.9337 | 1.189 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=8; dw=960; dh=540 | native | 0.5284 | 0.6600 | 1.249 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=16; dw=960; dh=540 | AVX2 | 1.0327 | 0.9369 | 0.907 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=16; dw=960; dh=540 | native | 0.6469 | 0.6720 | 1.039 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=32; dw=960; dh=540 | AVX2 | 1.5343 | 1.6447 | 1.072 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=32; dw=960; dh=540 | native | 1.7669 | 1.5701 | 0.889 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=8; dw=960; dh=540 | AVX2 | 0.8051 | 0.9094 | 1.130 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=8; dw=960; dh=540 | native | 0.5408 | 0.6683 | 1.236 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=16; dw=960; dh=540 | AVX2 | 1.0219 | 0.9390 | 0.919 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=16; dw=960; dh=540 | native | 0.6425 | 0.6925 | 1.078 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=32; dw=960; dh=540 | AVX2 | 1.5273 | 1.6732 | 1.096 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=32; dw=960; dh=540 | native | 1.8163 | 1.5595 | 0.859 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=8; dw=2880; dh=1620 | AVX2 | 2.4124 | 1.3592 | 0.563 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=8; dw=2880; dh=1620 | native | 0.8016 | 0.7864 | 0.981 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=16; dw=2880; dh=1620 | AVX2 | 2.9271 | 1.5106 | 0.516 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=16; dw=2880; dh=1620 | native | 1.4000 | 1.0710 | 0.765 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=32; dw=2880; dh=1620 | AVX2 | 5.3231 | 2.3249 | 0.437 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=triangle; bits=32; dw=2880; dh=1620 | native | 2.6377 | 1.9396 | 0.735 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=8; dw=2880; dh=1620 | AVX2 | 2.8720 | 2.5967 | 0.904 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=8; dw=2880; dh=1620 | native | 1.5967 | 1.6675 | 1.044 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=16; dw=2880; dh=1620 | AVX2 | 3.5145 | 2.6407 | 0.751 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=16; dw=2880; dh=1620 | native | 2.3904 | 1.7510 | 0.733 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=32; dw=2880; dh=1620 | AVX2 | 3.6303 | 3.6190 | 0.997 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=lanczos3; bits=32; dw=2880; dh=1620 | native | 3.1588 | 3.0583 | 0.968 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=8; dw=2880; dh=1620 | AVX2 | 2.8370 | 2.6196 | 0.923 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=8; dw=2880; dh=1620 | native | 1.5859 | 1.6948 | 1.069 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=16; dw=2880; dh=1620 | AVX2 | 3.4507 | 2.5715 | 0.745 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=16; dw=2880; dh=1620 | native | 2.4033 | 1.7117 | 0.712 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=32; dw=2880; dh=1620 | AVX2 | 3.5226 | 3.7188 | 1.056 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=HV; filter=spline36; bits=32; dw=2880; dh=1620 | native | 3.0187 | 3.1610 | 1.047 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=triangle; bits=8; dw=960; dh=1620 | AVX2 | 1.3238 | 0.9785 | 0.739 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=triangle; bits=8; dw=960; dh=1620 | native | 0.5484 | 0.4398 | 0.802 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=triangle; bits=16; dw=960; dh=1620 | AVX2 | 1.6185 | 1.2704 | 0.785 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=triangle; bits=16; dw=960; dh=1620 | native | 0.7355 | 0.7092 | 0.964 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=triangle; bits=32; dw=960; dh=1620 | AVX2 | 3.1450 | 2.0321 | 0.646 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=triangle; bits=32; dw=960; dh=1620 | native | 2.2654 | 1.6254 | 0.717 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=lanczos3; bits=8; dw=960; dh=1620 | AVX2 | 1.5331 | 1.7691 | 1.154 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=lanczos3; bits=8; dw=960; dh=1620 | native | 1.0821 | 1.3275 | 1.227 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=lanczos3; bits=16; dw=960; dh=1620 | AVX2 | 1.9065 | 1.9622 | 1.029 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=lanczos3; bits=16; dw=960; dh=1620 | native | 1.2116 | 1.5635 | 1.290 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=lanczos3; bits=32; dw=960; dh=1620 | AVX2 | 2.8961 | 3.0793 | 1.063 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=lanczos3; bits=32; dw=960; dh=1620 | native | 3.6394 | 2.9584 | 0.813 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=spline36; bits=8; dw=960; dh=1620 | AVX2 | 1.5171 | 1.8188 | 1.199 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=spline36; bits=8; dw=960; dh=1620 | native | 1.0804 | 1.3408 | 1.241 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=spline36; bits=16; dw=960; dh=1620 | AVX2 | 1.9092 | 1.9583 | 1.026 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=spline36; bits=16; dw=960; dh=1620 | native | 1.1936 | 1.5272 | 1.280 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=spline36; bits=32; dw=960; dh=1620 | AVX2 | 2.8742 | 3.0927 | 1.076 | AVX2 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |
| order=VH; filter=spline36; bits=32; dw=960; dh=1620 | native | 3.6864 | 2.9572 | 0.802 | AVX512 | Remeasured after long-support fix; three-round medians. New output exactly matches C on timed inputs; upstream float may differ. |

</details>

## Supplementary long-support measurements

These retained measurements use the same current production kernels, at module aab7884 / host f9c088cc. Upstream was retimed alongside that DLL, three observations per side/target, each the median of five calibrated samples. Input is 1920×1080; H output is 960×1080, V is 1920×540, HV is 960×540. The profile number is taps; SincLin2 defaults to 15. Y32 denotes F32. All new outputs match C; old/new F32 can differ. These profiles overlap the main table, and independent measurement sessions need not yield identical times.

| Profile | CPU | Upstream ms | New ms | New / upstream |
|---|---|---|---|---|
| Y8-SincLin2Resize-3-H | avx2 | 0.6314 | 0.7186 | 1.138 |
| Y8-SincLin2Resize-3-H | native | 0.3725 | 0.4784 | 1.284 |
| Y8-SincLin2Resize-3-V | avx2 | 0.2932 | 0.3968 | 1.353 |
| Y8-SincLin2Resize-3-V | native | 0.4309 | 0.3197 | 0.742 |
| Y8-SincLin2Resize-3-HV | avx2 | 0.7660 | 0.9269 | 1.210 |
| Y8-SincLin2Resize-3-HV | native | 0.5823 | 0.6576 | 1.129 |
| Y8-SincLin2Resize-8-H | avx2 | 0.9747 | 2.6109 | 2.679 |
| Y8-SincLin2Resize-8-H | native | 0.8465 | 1.2540 | 1.481 |
| Y8-SincLin2Resize-8-V | avx2 | 0.6917 | 0.9637 | 1.393 |
| Y8-SincLin2Resize-8-V | native | 1.0854 | 0.7654 | 0.705 |
| Y8-SincLin2Resize-8-HV | avx2 | 1.3233 | 3.1180 | 2.356 |
| Y8-SincLin2Resize-8-HV | native | 1.4145 | 1.6215 | 1.146 |
| Y8-SincLin2Resize-15-H | avx2 | 1.5956 | 5.8707 | 3.679 |
| Y8-SincLin2Resize-15-H | native | 1.5523 | 3.6777 | 2.369 |
| Y8-SincLin2Resize-15-V | avx2 | 1.2961 | 1.7166 | 1.324 |
| Y8-SincLin2Resize-15-V | native | 2.0214 | 1.4062 | 0.696 |
| Y8-SincLin2Resize-15-HV | avx2 | 2.2622 | 6.6837 | 2.954 |
| Y8-SincLin2Resize-15-HV | native | 2.5623 | 4.4544 | 1.738 |
| Y8-LanczosResize-3-H | avx2 | 0.6208 | 0.7054 | 1.136 |
| Y8-LanczosResize-3-H | native | 0.3649 | 0.4847 | 1.328 |
| Y8-LanczosResize-3-V | avx2 | 0.2944 | 0.3941 | 1.339 |
| Y8-LanczosResize-3-V | native | 0.4250 | 0.3162 | 0.744 |
| Y8-LanczosResize-3-HV | avx2 | 0.8326 | 0.9143 | 1.098 |
| Y8-LanczosResize-3-HV | native | 0.5821 | 0.6816 | 1.171 |
| Y8-LanczosResize-8-H | avx2 | 0.9732 | 2.6131 | 2.685 |
| Y8-LanczosResize-8-H | native | 0.8645 | 1.2475 | 1.443 |
| Y8-LanczosResize-8-V | avx2 | 0.7154 | 0.9445 | 1.320 |
| Y8-LanczosResize-8-V | native | 1.0852 | 0.7740 | 0.713 |
| Y8-LanczosResize-8-HV | avx2 | 1.3237 | 3.0541 | 2.307 |
| Y8-LanczosResize-8-HV | native | 1.3876 | 1.6294 | 1.174 |
| Y8-LanczosResize-15-H | avx2 | 1.6049 | 5.6726 | 3.535 |
| Y8-LanczosResize-15-H | native | 1.5330 | 3.6708 | 2.395 |
| Y8-LanczosResize-15-V | avx2 | 1.2662 | 1.7102 | 1.351 |
| Y8-LanczosResize-15-V | native | 2.0136 | 1.3977 | 0.694 |
| Y8-LanczosResize-15-HV | avx2 | 2.2671 | 6.7077 | 2.959 |
| Y8-LanczosResize-15-HV | native | 2.5606 | 4.4260 | 1.729 |
| Y16-SincLin2Resize-3-H | avx2 | 0.7194 | 0.7169 | 0.997 |
| Y16-SincLin2Resize-3-H | native | 0.4230 | 0.5206 | 1.231 |
| Y16-SincLin2Resize-3-V | avx2 | 0.3549 | 0.3494 | 0.985 |
| Y16-SincLin2Resize-3-V | native | 0.4028 | 0.2571 | 0.638 |
| Y16-SincLin2Resize-3-HV | avx2 | 0.9732 | 0.9048 | 0.930 |
| Y16-SincLin2Resize-3-HV | native | 0.6942 | 0.6977 | 1.005 |
| Y16-SincLin2Resize-8-H | avx2 | 1.0374 | 2.6627 | 2.567 |
| Y16-SincLin2Resize-8-H | native | 1.0576 | 1.2277 | 1.161 |
| Y16-SincLin2Resize-8-V | avx2 | 0.8906 | 0.8954 | 1.005 |
| Y16-SincLin2Resize-8-V | native | 1.0850 | 0.6545 | 0.603 |
| Y16-SincLin2Resize-8-HV | avx2 | 1.7066 | 3.0728 | 1.800 |
| Y16-SincLin2Resize-8-HV | native | 1.7154 | 1.6152 | 0.942 |
| Y16-SincLin2Resize-15-H | avx2 | 1.8558 | 4.5457 | 2.449 |
| Y16-SincLin2Resize-15-H | native | 1.8629 | 3.8708 | 2.078 |
| Y16-SincLin2Resize-15-V | avx2 | 1.7412 | 1.5927 | 0.915 |
| Y16-SincLin2Resize-15-V | native | 1.8987 | 1.1229 | 0.591 |
| Y16-SincLin2Resize-15-HV | avx2 | 3.1242 | 5.5009 | 1.761 |
| Y16-SincLin2Resize-15-HV | native | 2.9068 | 4.5115 | 1.552 |
| Y16-LanczosResize-3-H | avx2 | 0.7074 | 0.7310 | 1.033 |
| Y16-LanczosResize-3-H | native | 0.4121 | 0.5209 | 1.264 |
| Y16-LanczosResize-3-V | avx2 | 0.3526 | 0.3533 | 1.002 |
| Y16-LanczosResize-3-V | native | 0.4036 | 0.2604 | 0.645 |
| Y16-LanczosResize-3-HV | avx2 | 0.9781 | 0.9012 | 0.921 |
| Y16-LanczosResize-3-HV | native | 0.6879 | 0.6746 | 0.981 |
| Y16-LanczosResize-8-H | avx2 | 1.0487 | 2.6571 | 2.534 |
| Y16-LanczosResize-8-H | native | 1.0621 | 1.2265 | 1.155 |
| Y16-LanczosResize-8-V | avx2 | 0.8713 | 0.8959 | 1.028 |
| Y16-LanczosResize-8-V | native | 1.0199 | 0.6284 | 0.616 |
| Y16-LanczosResize-8-HV | avx2 | 1.6875 | 3.1206 | 1.849 |
| Y16-LanczosResize-8-HV | native | 1.6833 | 1.5987 | 0.950 |
| Y16-LanczosResize-15-H | avx2 | 1.8398 | 5.1937 | 2.823 |
| Y16-LanczosResize-15-H | native | 1.8073 | 3.8348 | 2.122 |
| Y16-LanczosResize-15-V | avx2 | 1.7906 | 1.6314 | 0.911 |
| Y16-LanczosResize-15-V | native | 1.8915 | 1.1199 | 0.592 |
| Y16-LanczosResize-15-HV | avx2 | 3.1242 | 5.3678 | 1.718 |
| Y16-LanczosResize-15-HV | native | 2.9314 | 4.5613 | 1.556 |
| Y32-SincLin2Resize-3-H | avx2 | 1.0542 | 1.2008 | 1.139 |
| Y32-SincLin2Resize-3-H | native | 1.0973 | 1.1235 | 1.024 |
| Y32-SincLin2Resize-3-V | avx2 | 0.5230 | 0.5735 | 1.096 |
| Y32-SincLin2Resize-3-V | native | 0.5013 | 0.5503 | 1.098 |
| Y32-SincLin2Resize-3-HV | avx2 | 1.4804 | 1.6279 | 1.100 |
| Y32-SincLin2Resize-3-HV | native | 1.6985 | 1.6186 | 0.953 |
| Y32-SincLin2Resize-8-H | avx2 | 1.9886 | 2.5908 | 1.303 |
| Y32-SincLin2Resize-8-H | native | 1.7294 | 2.9257 | 1.692 |
| Y32-SincLin2Resize-8-V | avx2 | 1.2897 | 1.3152 | 1.020 |
| Y32-SincLin2Resize-8-V | native | 1.1928 | 1.2278 | 1.029 |
| Y32-SincLin2Resize-8-HV | avx2 | 2.8924 | 3.5662 | 1.233 |
| Y32-SincLin2Resize-8-HV | native | 2.5817 | 3.7573 | 1.455 |
| Y32-SincLin2Resize-15-H | avx2 | 3.9519 | 5.3544 | 1.355 |
| Y32-SincLin2Resize-15-H | native | 3.7489 | 4.7869 | 1.277 |
| Y32-SincLin2Resize-15-V | avx2 | 2.2265 | 2.3243 | 1.044 |
| Y32-SincLin2Resize-15-V | native | 2.2204 | 2.2902 | 1.031 |
| Y32-SincLin2Resize-15-HV | avx2 | 5.4043 | 6.7069 | 1.241 |
| Y32-SincLin2Resize-15-HV | native | 5.1479 | 6.1555 | 1.196 |
| Y32-LanczosResize-3-H | avx2 | 1.0638 | 1.1665 | 1.097 |
| Y32-LanczosResize-3-H | native | 1.1925 | 1.1360 | 0.953 |
| Y32-LanczosResize-3-V | avx2 | 0.4882 | 0.5296 | 1.085 |
| Y32-LanczosResize-3-V | native | 0.5137 | 0.5431 | 1.057 |
| Y32-LanczosResize-3-HV | avx2 | 1.4770 | 1.6482 | 1.116 |
| Y32-LanczosResize-3-HV | native | 1.7757 | 1.5654 | 0.882 |
| Y32-LanczosResize-8-H | avx2 | 1.9824 | 2.5931 | 1.308 |
| Y32-LanczosResize-8-H | native | 1.7725 | 2.9611 | 1.671 |
| Y32-LanczosResize-8-V | avx2 | 1.2387 | 1.2116 | 0.978 |
| Y32-LanczosResize-8-V | native | 1.2592 | 1.3951 | 1.108 |
| Y32-LanczosResize-8-HV | avx2 | 2.8526 | 3.5346 | 1.239 |
| Y32-LanczosResize-8-HV | native | 2.6176 | 3.7839 | 1.446 |
| Y32-LanczosResize-15-H | avx2 | 3.9562 | 5.2908 | 1.337 |
| Y32-LanczosResize-15-H | native | 3.7687 | 4.7452 | 1.259 |
| Y32-LanczosResize-15-V | avx2 | 2.3319 | 2.2582 | 0.968 |
| Y32-LanczosResize-15-V | native | 2.2223 | 2.3269 | 1.047 |
| Y32-LanczosResize-15-HV | avx2 | 5.4205 | 6.7251 | 1.241 |
| Y32-LanczosResize-15-HV | native | 4.9911 | 6.1048 | 1.223 |
