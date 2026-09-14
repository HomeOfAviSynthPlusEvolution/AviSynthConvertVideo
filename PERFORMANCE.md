# Video conversion benchmark results

Current performance comparisons: **522 full-filter cases**, **592 kernel rows**, and **108 supplementary long-support resampling rows** (54 profiles × two targets). The supplementary set overlaps the filter audit. The tables include refreshed AMD measurements for **Floyd including cached U8 range conversion, U16 low-bit row conversion and integer/F32 horizontal resampling, including aligned F32 coefficient vectors and aligned/shared exact integer coefficient blocks, uniform short integer supports and long regular pair loads, paired F32 loads, single-vector float windows, long integer sliding pairs and fixed short F32 supports, Ordered quantization and range mapping, and U8 integer matrix accumulation, U8 depth range mapping, direct packed RGB repacking, YUY2 chroma neutralization, Zen4 luma extraction and additional high-precision integer matrix coverage**. Unaffected paths retain their existing measurements. Missing upstream counterparts are explicitly marked, not counted as wins. Coverage is the measured workload set, not every possible parameter combination. Times are milliseconds.

## Reference and method

- AMD refresh: 2026-09-14, Ryzen 9 7940H, Windows x64, clang-cl 22.1.3 Release, pinned to logical CPU 12 (0x1000). Each refresh uses one optimized module build for its affected paths. No concurrent build or test work during timing.
- Affected timings are replaced in their existing rows using the original harness, deterministic input, pitches and statistics; unchanged paths retain their valid measurements. Full-filter observations use the original GetFrame harness and three rounds. Matrix, Depth and Ordered kernel observations retain three warmups and seven samples of ten calls; resampling uses its original axis, ratio and pipeline harnesses. The fixed host source is `bda0aab8bd9b366da41e946d41236735bae839af`. Source identities, binary hashes, raw observations, scope checks and reproduction scripts are recorded in untracked `docs/PERFORMANCE-CURRENT-2026-09-14/`, `docs/PMU-NEXT-2026-09-14/`, and `docs/PMU-INTERNAL-2026-09-14/`.
- Final benchmark artifacts and timing logs are checked for diagnostic markers before publication. Restored sources are rebuilt with fresh timestamps; measured full-filter paths pass a clean native smoke run and exact C output checks.
- The U16 BGRA unpack alignment optimization applies only to AVX3_SPR. AMD AVX2/AVX3_ZEN4 unpack instructions are unchanged, so their existing rows remain current. Controlled-address AMD checks and the separate SPR PMU evidence are recorded in `docs/PMU-NEXT-2026-09-14/02-packed-alpha/`; server pipeline timings are not substituted into these AMD tables.
- The F32 three-plane matrix output alignment optimization also applies only to AVX3_SPR. It aligns the row body with a bounded partial first vector when all output planes have the same address phase. AMD AVX2/AVX3_ZEN4 retain their existing kernel loops and measurements; controlled-address comparisons and SPR PMU evidence are in `docs/PMU-INTERNAL-2026-09-14/02-MATRIX-REPORT.md`.
- Upstream reference: `5c82777b374bdef16e13007a11e77d735ac1e4eb`. Its existing measurements are retained unchanged; no upstream code was timed during this refresh. Module and upstream values therefore come from separate sessions on the same AMD machine. Small differences are not established gains or regressions.
- AVX2 uses SetMaxCPU("avx2") on the module DLL. Native measures the highest available production target, Highway **AVX3_ZEN4**, rather than choosing the fastest measured target. The upstream column uses its saved AVX512 result where implemented. Floyd remains the shared C implementation for both CPU settings.
- Full-filter timing includes GetFrame processing and output allocation, excluding source generation, construction and output hashing. The original deterministic source and exact expressions are reused; increasing output frame numbers avoid output-cache hits. No Prefetch. Each observation is the median of five calibrated samples of approximately 15 ms, with 2–100 calls per sample. Three observations per module target are reduced to a median; AVX2/native observations are measured separately. These are warm single-thread measurements.
- Resampling kernel timing retains the original buffer geometry, deterministic input, coefficient preparation, call boundary, warmup and five-sample median procedure. Allocation, coefficient construction, C reference execution and output checking are outside timing. Three observations per module target are reduced to a median. The kernel harness runs AVX2 then native and does not execute upstream. Kernel and full-filter scopes must not be mixed.
- Every refreshed module full-filter hash matches its none=C baseline in all rounds. Every refreshed kernel output matches C exactly, including F32. Equality flags compare refreshed module hashes with saved upstream hashes; upstream can differ due to rounding/clipping and corrected semantics. Hashes cover active pixels, excluding padding and frame properties.
- Unaffected full-filter observations retain their recorded one or three rounds. Unaffected matrix U8 kernels retain five rounds; other matrix/depth/ordered kernels retain three, layout retains its five-sample harness, and vertical resampling retains its three observations. Unchanged supplementary paths retain three observations. Per-row notes identify their baseline and scope.
- Additional precision=20 matrix API rows use deterministic LCG input, 64-byte pitches, three warmups, seven calibrated continuous samples per observation (approximately 30 ms, 3–500 calls), and the median of three observations. Allocation, initialization, plan creation and exact C checks are outside timing. These rows have no saved upstream counterpart; their absolute times are not compared with the other matrix harness. The 522 original full-filter profiles do not select the changed wide accumulator, so their existing data remain current.
- Cases with either side below 0.005 ms are retained but excluded from aggregate ratios. Ratios are module/upstream elapsed time; below 1 is faster. Medians are medians of case ratios, not a total-runtime speedup. Differences within 5%, or small absolute differences, are treated as measurement noise rather than reasons to disable a target.

## Full-filter summary

| Family | Nontrivial cases | AVX2 median | AVX2 range | Native median | Native range |
|---|---|---|---|---|---|
| chroma | 25 | 0.923 | 0.542–1.304 | 0.822 | 0.612–1.101 |
| depth | 132 | 1.141 | 0.379–1.506 | 0.994 | 0.334–1.204 |
| depth-alpha | 13 | 1.067 | 0.976–3.307 | 1.028 | 0.904–2.917 |
| floyd | 17 | 1.049 | 0.844–1.262 | 1.025 | 0.830–1.209 |
| greyscale | 21 | 0.900 | 0.293–1.297 | 0.902 | 0.257–1.304 |
| interlaced | 9 | 0.992 | 0.206–1.132 | 0.975 | 0.211–1.137 |
| layout | 23 | 0.955 | 0.126–1.820 | 1.052 | 0.108–1.578 |
| luma | 15 | 0.906 | 0.598–1.047 | 0.867 | 0.512–1.009 |
| matrix-filter | 54 | 1.011 | 0.666–1.825 | 0.891 | 0.539–1.423 |
| ordered | 16 | 0.970 | 0.306–1.330 | 0.876 | 0.249–1.097 |
| resize | 144 | 0.996 | 0.316–1.609 | 0.819 | 0.412–1.097 |
| resize-composed | 19 | 0.992 | 0.559–1.605 | 0.787 | 0.404–0.952 |
| yuy2 | 11 | 0.847 | 0.202–1.218 | 0.829 | 0.211–1.131 |

## Complete full-filter comparisons

Expand each family. CPU cells are **upstream ms / new ms / ratio**; low-work ratios are omitted. Equality is **AVX2 / native** versus upstream. All new outputs match C. Observation counts are for the module.

<details>
<summary>chroma — 36 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| chroma-108 | YV24 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-109 | YV24 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 1.4085 / 1.5152 / 1.076 | 1.0270 / 0.9198 / 0.896 | True / True | 3 / 3 |
| chroma-110 | YV24 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 1.6652 / 1.8906 / 1.135 | 1.4305 / 1.1765 / 0.822 | True / True | 3 / 3 |
| chroma-111 | YV16 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.0660 / 1.8459 / 0.602 | 1.2413 / 0.9725 / 0.783 | True / True | 3 / 3 |
| chroma-112 | YV16 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-113 | YV16 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.3727 / 0.4630 / 1.242 | 0.5286 / 0.4165 / 0.788 | True / True | 3 / 3 |
| chroma-114 | YV12 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 2.2820 / 1.9686 / 0.863 | 1.5901 / 1.4027 / 0.882 | True / True | 3 / 3 |
| chroma-115 | YV12 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.4294 / 0.5601 / 1.304 | 0.5857 / 0.4959 / 0.847 | True / True | 3 / 3 |
| chroma-116 | YV12 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-117 | YV411 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.3337 / 2.2472 / 0.674 | 1.2527 / 0.9778 / 0.781 | True / True | 3 / 3 |
| chroma-118 | YV411 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 1.7955 / 0.9731 / 0.542 | 0.6343 / 0.5557 / 0.876 | True / True | 3 / 3 |
| chroma-119 | YV411 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 2.0725 / 1.3805 / 0.666 | 1.0809 / 0.9005 / 0.833 | True / True | 3 / 3 |
| chroma-120 | YUV444P16 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-121 | YUV444P16 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 1.8694 / 1.8408 / 0.985 | 1.7320 / 1.3045 / 0.753 | True / True | 3 / 3 |
| chroma-122 | YUV444P16 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 2.3940 / 2.2096 / 0.923 | 2.2615 / 1.5334 / 0.678 | True / True | 3 / 3 |
| chroma-123 | YUV422P16 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.5540 / 2.2340 / 0.629 | 1.4265 / 1.3127 / 0.920 | True / True | 3 / 3 |
| chroma-124 | YUV422P16 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-125 | YUV422P16 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.6135 / 0.5689 / 0.927 | 0.6715 / 0.4561 / 0.679 | True / True | 3 / 3 |
| chroma-126 | YUV420P16 | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 2.9570 / 2.3251 / 0.786 | 1.9219 / 1.5613 / 0.812 | True / True | 3 / 3 |
| chroma-127 | YUV420P16 | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.7285 / 0.6562 / 0.901 | 0.6793 / 0.5726 / 0.843 | True / True | 3 / 3 |
| chroma-128 | YUV420P16 | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-129 | YUV444PS | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-130 | YUV444PS | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 3.0745 / 3.0677 / 0.998 | 4.4746 / 2.7407 / 0.612 | False / False | 3 / 3 |
| chroma-131 | YUV444PS | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 3.6483 / 3.8947 / 1.068 | 5.0919 / 3.3416 / 0.656 | False / False | 3 / 3 |
| chroma-132 | YUV422PS | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.4198 / 3.8475 / 1.125 | 4.2501 / 2.9042 / 0.683 | False / False | 3 / 3 |
| chroma-133 | YUV422PS | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| chroma-134 | YUV422PS | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 1.2223 / 1.4399 / 1.178 | 1.2954 / 1.3583 / 1.049 | False / False | 3 / 3 |
| chroma-135 | YUV420PS | 1920×1080 | `src.ConvertToYUV444(chromaresample="spline36")` | 3.4057 / 3.5481 / 1.042 | 3.9281 / 3.2769 / 0.834 | False / False | 3 / 3 |
| chroma-136 | YUV420PS | 1920×1080 | `src.ConvertToYUV422(chromaresample="spline36")` | 1.1525 / 1.3109 / 1.137 | 1.1508 / 1.2665 / 1.101 | False / False | 3 / 3 |
| chroma-137 | YUV420PS | 1920×1080 | `src.ConvertToYUV420(chromaresample="spline36")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| extra-044 | YUV420P10 | 1920×1080 | `src.ConvertToYUV444(chromaresample="lanczos")` | 2.8778 / 2.4110 / 0.838 | 1.7372 / 1.6691 / 0.961 | True / True | 3 / 3 |
| extra-045 | YUV420P10 | 1920×1080 | `src.ConvertToYUV420(chromaresample="bilinear")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| extra-046 | YUV422P12 | 1920×1080 | `src.ConvertToYUV444(chromaresample="lanczos")` | 3.3453 / 2.3003 / 0.688 | 1.2608 / 1.3277 / 1.053 | True / True | 3 / 3 |
| extra-047 | YUV422P12 | 1920×1080 | `src.ConvertToYUV420(chromaresample="bilinear")` | 0.4055 / 0.3662 / 0.903 | 0.5115 / 0.3224 / 0.630 | True / True | 3 / 3 |
| extra-048 | YUV444P14 | 1920×1080 | `src.ConvertToYUV444(chromaresample="lanczos")` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 3 / 3 |
| extra-049 | YUV444P14 | 1920×1080 | `src.ConvertToYUV420(chromaresample="bilinear")` | 2.0774 / 1.4108 / 0.679 | 1.3794 / 1.0721 / 0.777 | True / True | 3 / 3 |

</details>

<details>
<summary>depth — 132 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| depth-146 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.1876 / 0.0712 / 0.379 | 0.1904 / 0.0636 / 0.334 | True / True | 3 / 3 |
| depth-147 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=false,dither=-1)` | 0.1773 / 0.0761 / 0.429 | 0.1784 / 0.0648 / 0.363 | True / True | 3 / 3 |
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
| depth-alpha-278 | RGB32 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=-1)` | 0.7562 / 1.1140 / 1.473 | 0.7961 / 1.0536 / 1.324 | False / False | 3 / 3 |
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
| floyd-306 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=8)` | 5.6087 / 5.8813 / 1.049 | 5.6990 / 5.8277 / 1.023 | True / True | 3 / 3 |
| floyd-307 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=8)` | 6.1734 / 7.6466 / 1.239 | 6.2729 / 7.5853 / 1.209 | True / True | 3 / 3 |
| floyd-308 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=8)` | 5.5793 / 5.7588 / 1.032 | 5.6631 / 5.7995 / 1.024 | True / True | 3 / 3 |
| floyd-309 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=8)` | 6.1718 / 7.4475 / 1.207 | 6.4427 / 7.4325 / 1.154 | True / True | 3 / 3 |
| floyd-310 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=1,dither_bits=10)` | 5.4962 / 5.8682 / 1.068 | 5.6238 / 5.8232 / 1.035 | True / True | 3 / 3 |
| floyd-311 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=1,dither_bits=10)` | 6.0377 / 7.4473 / 1.233 | 6.3431 / 7.4487 / 1.174 | True / True | 3 / 3 |
| floyd-312 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=true,dither=1,dither_bits=10)` | 5.9955 / 5.7655 / 0.962 | 6.1339 / 5.8110 / 0.947 | True / True | 3 / 3 |
| floyd-313 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=1,dither_bits=10)` | 5.8914 / 7.4334 / 1.262 | 6.1708 / 7.4127 / 1.201 | True / True | 3 / 3 |
| floyd-314 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=5)` | 6.0797 / 6.0136 / 0.989 | 6.2431 / 6.0812 / 0.974 | True / True | 3 / 3 |
| floyd-315 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=5)` | 8.1021 / 6.8373 / 0.844 | 8.1141 / 6.9820 / 0.860 | True / True | 3 / 3 |
| floyd-316 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=1,dither_bits=3)` | 6.0278 / 6.1430 / 1.019 | 6.2032 / 6.1654 / 0.994 | True / True | 3 / 3 |
| floyd-317 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=1,dither_bits=3)` | 7.7917 / 6.7433 / 0.865 | 8.1557 / 6.7732 / 0.830 | True / True | 3 / 3 |
| floyd-318 | YUV444P16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=8)` | 16.6596 / 17.4370 / 1.047 | 17.0280 / 17.3366 / 1.018 | True / True | 3 / 3 |
| floyd-319 | YUV444P16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=8)` | 18.5018 / 22.5866 / 1.221 | 19.2703 / 22.6047 / 1.173 | True / True | 3 / 3 |
| floyd-320 | RGBAP16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=1,dither_bits=8)` | 16.8681 / 17.7663 / 1.053 | 17.2342 / 17.7138 / 1.028 | True / True | 3 / 3 |
| floyd-321 | RGBAP16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=1,dither_bits=8)` | 18.6826 / 22.6129 / 1.210 | 19.6817 / 22.5448 / 1.145 | True / True | 3 / 3 |
| floyd-471 | Y16 | 3840×2160 | `src.ConvertBits(8,dither=1)` | 21.8065 / 22.8627 / 1.048 | 22.4093 / 22.9646 / 1.025 | True / True | 3 / 3 |

</details>

<details>
<summary>greyscale — 21 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| greyscale-031 | YUY2 | 1920×1080 | `src.Greyscale()` | 0.1855 / 0.1612 / 0.869 | 0.1762 / 0.1579 / 0.896 | True / True | 3 / 3 |
| greyscale-034 | YV16 | 1920×1080 | `src.Greyscale()` | 0.1144 / 0.1126 / 0.984 | 0.1202 / 0.1084 / 0.902 | True / True | 1 / 1 |
| greyscale-036 | RGB24 | 1920×1080 | `src.Greyscale()` | 2.9452 / 0.8669 / 0.294 | 2.9049 / 0.8223 / 0.283 | True / True | 3 / 3 |
| greyscale-038 | RGB32 | 1920×1080 | `src.Greyscale()` | 1.0374 / 1.0222 / 0.985 | 0.9629 / 0.9000 / 0.935 | True / True | 1 / 3 |
| greyscale-040 | RGB48 | 1920×1080 | `src.Greyscale()` | 3.2569 / 1.9939 / 0.612 | 3.2463 / 1.9195 / 0.591 | True / True | 1 / 1 |
| greyscale-042 | RGB64 | 1920×1080 | `src.Greyscale()` | 2.1675 / 2.3243 / 1.072 | 2.1951 / 2.1122 / 0.962 | True / True | 1 / 1 |
| greyscale-044 | RGBP8 | 1920×1080 | `src.Greyscale()` | 1.6155 / 0.4735 / 0.293 | 1.6153 / 0.4156 / 0.257 | True / True | 3 / 3 |
| greyscale-046 | RGBP16 | 1920×1080 | `src.Greyscale()` | 1.7692 / 1.0498 / 0.593 | 1.6700 / 1.0050 / 0.602 | True / True | 1 / 1 |
| greyscale-048 | RGBPS | 1920×1080 | `src.Greyscale()` | 1.9216 / 2.4920 / 1.297 | 1.9675 / 2.4933 / 1.267 | True / True | 3 / 3 |
| greyscale-050 | YV12 | 1920×1080 | `src.Greyscale()` | 0.0700 / 0.0713 / 1.019 | 0.0673 / 0.0689 / 1.023 | True / True | 1 / 1 |
| greyscale-052 | YUV420P16 | 1920×1080 | `src.Greyscale()` | 0.1682 / 0.1605 / 0.954 | 0.1625 / 0.1749 / 1.076 | True / True | 1 / 1 |
| greyscale-054 | YUV420PS | 1920×1080 | `src.Greyscale()` | 0.5696 / 0.5356 / 0.940 | 0.5553 / 0.5781 / 1.041 | True / True | 1 / 1 |
| greyscale-468 | YUY2 | 3840×2160 | `src.Greyscale()` | 1.2583 / 1.1328 / 0.900 | 1.2363 / 1.1912 / 0.964 | True / True | 3 / 3 |
| extra-029 | RGBP10 | 1920×1080 | `src.Greyscale()` | 1.7962 / 1.1236 / 0.626 | 1.9924 / 1.0291 / 0.517 | True / True | 1 / 1 |
| extra-031 | RGBP12 | 1920×1080 | `src.Greyscale()` | 1.9275 / 1.2652 / 0.656 | 1.7628 / 1.0925 / 0.620 | True / True | 1 / 1 |
| extra-033 | RGBP14 | 1920×1080 | `src.Greyscale()` | 1.7066 / 1.0629 / 0.623 | 1.7807 / 1.1423 / 0.642 | True / True | 1 / 1 |
| extra-035 | RGBAP8 | 1920×1080 | `src.Greyscale()` | 1.7611 / 0.5378 / 0.305 | 1.7685 / 0.4709 / 0.266 | True / True | 1 / 3 |
| extra-037 | RGBAP16 | 1920×1080 | `src.Greyscale()` | 1.8680 / 1.4529 / 0.778 | 1.9119 / 1.2368 / 0.647 | True / True | 1 / 1 |
| extra-039 | RGBAPS | 1920×1080 | `src.Greyscale()` | 2.2839 / 2.8387 / 1.243 | 2.3906 / 3.1169 / 1.304 | True / True | 3 / 3 |
| extra-041 | YUVA444P16 | 1920×1080 | `src.Greyscale()` | 0.9061 / 0.9330 / 1.030 | 0.9054 / 0.9499 / 1.049 | True / True | 1 / 1 |
| extra-043 | YUVA444PS | 1920×1080 | `src.Greyscale()` | 1.6466 / 1.6214 / 0.985 | 1.6042 / 1.8745 / 1.168 | True / True | 3 / 3 |

</details>

<details>
<summary>interlaced — 9 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| extra-019 | YUY2 | 1920×1080 | `src.ConvertToYV12(interlaced=true)` | 2.6480 / 0.7098 / 0.268 | 2.6670 / 0.6776 / 0.254 | True / True | 3 / 3 |
| extra-020 | YUY2 | 1920×1080 | `src.ConvertToYV16(interlaced=true)` | 0.4243 / 0.0875 / 0.206 | 0.4174 / 0.0881 / 0.211 | True / True | 3 / 3 |
| extra-021 | YUY2 | 1920×1080 | `src.ConvertToRGB32(interlaced=true)` | 5.4152 / 3.7794 / 0.698 | 3.0095 / 2.9335 / 0.975 | False / False | 3 / 3 |
| extra-022 | YV12 | 1920×1080 | `src.ConvertToYV16(interlaced=true)` | 0.4757 / 0.4871 / 1.024 | 0.5115 / 0.4386 / 0.858 | True / True | 3 / 3 |
| extra-023 | YV12 | 1920×1080 | `src.ConvertToRGB32(interlaced=true)` | 3.3230 / 3.4536 / 1.039 | 2.4924 / 2.8327 / 1.137 | False / False | 3 / 3 |
| extra-024 | YV16 | 1920×1080 | `src.ConvertToYV12(interlaced=true)` | 0.3428 / 0.3880 / 1.132 | 0.4399 / 0.3518 / 0.800 | True / True | 3 / 3 |
| extra-025 | YV16 | 1920×1080 | `src.ConvertToRGB32(interlaced=true)` | 3.6618 / 3.0619 / 0.836 | 1.8858 / 2.0656 / 1.095 | False / False | 3 / 3 |
| extra-026 | RGB32 | 1920×1080 | `src.ConvertToYV12(interlaced=true)` | 3.3151 / 3.2873 / 0.992 | 2.3597 / 2.4496 / 1.038 | True / True | 3 / 3 |
| extra-027 | RGB32 | 1920×1080 | `src.ConvertToYV16(interlaced=true)` | 2.8323 / 2.9047 / 1.026 | 1.9842 / 1.9873 / 1.002 | True / True | 3 / 3 |

</details>

<details>
<summary>layout — 29 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| layout-001 | RGB24 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.1734 / 0.3108 / 1.792 | 0.1930 / 0.1949 / 1.010 | True / True | 3 / 3 |
| layout-002 | RGB24 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.2111 / 0.3722 / 1.763 | 0.2303 / 0.2873 / 1.248 | True / True | 3 / 3 |
| layout-003 | RGB24 | 1920×1080 | `src.ConvertToRGB32()` | 0.1731 / 0.2592 / 1.498 | 0.2126 / 0.2281 / 1.073 | True / True | 3 / 3 |
| layout-004 | RGB32 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.5575 / 0.3492 / 0.626 | 0.4042 / 0.4577 / 1.132 | True / True | 1 / 1 |
| layout-005 | RGB32 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.8304 / 0.6989 / 0.842 | 0.7914 / 0.5733 / 0.724 | True / True | 1 / 1 |
| layout-006 | RGB32 | 1920×1080 | `src.ConvertToRGB24()` | 0.1524 / 0.2716 / 1.782 | 0.1785 / 0.1857 / 1.041 | True / True | 3 / 3 |
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
| layout-017 | RGB48 | 1920×1080 | `src.ConvertToRGB64()` | 0.4729 / 0.7580 / 1.603 | 0.4295 / 0.6779 / 1.578 | True / True | 3 / 3 |
| layout-018 | RGB64 | 1920×1080 | `src.ConvertToPlanarRGB()` | 1.5071 / 1.4133 / 0.938 | 1.2303 / 1.0863 / 0.883 | True / True | 1 / 1 |
| layout-019 | RGB64 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 1.6310 / 1.5166 / 0.930 | 1.4925 / 1.2045 / 0.807 | True / True | 1 / 1 |
| layout-020 | RGB64 | 1920×1080 | `src.ConvertToRGB48()` | 0.5851 / 0.8624 / 1.474 | 0.5943 / 0.7335 / 1.234 | True / True | 3 / 3 |
| layout-021 | RGBP16 | 1920×1080 | `src.ConvertToRGB48()` | 2.0026 / 1.0104 / 0.505 | 1.9881 / 0.9013 / 0.453 | True / True | 1 / 1 |
| layout-022 | RGBP16 | 1920×1080 | `src.ConvertToRGB64()` | 1.1720 / 1.1193 / 0.955 | 1.0611 / 1.1857 / 1.117 | True / True | 1 / 1 |
| layout-023 | RGBP16 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 1 / 1 |
| layout-024 | RGBP16 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.5524 / 0.5476 / 0.991 | 0.5434 / 0.6110 / 1.124 | True / True | 1 / 1 |
| layout-025 | RGBAP16 | 1920×1080 | `src.ConvertToRGB48()` | 2.0013 / 0.9674 / 0.483 | 1.9672 / 0.8971 / 0.456 | True / True | 1 / 1 |
| layout-026 | RGBAP16 | 1920×1080 | `src.ConvertToRGB64()` | 1.2382 / 1.1531 / 0.931 | 1.1526 / 1.2477 / 1.083 | True / True | 1 / 1 |
| layout-027 | RGBAP16 | 1920×1080 | `src.ConvertToPlanarRGB()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| layout-028 | RGBAP16 | 1920×1080 | `src.ConvertToPlanarRGBA()` | 0.0000 / 0.0001 / low-work | 0.0000 / 0.0001 / low-work | True / True | 1 / 1 |
| layout-466 | RGB32 | 3840×2160 | `src.ConvertToRGB24()` | 1.2357 / 1.7546 / 1.420 | 1.2510 / 1.6567 / 1.324 | True / True | 3 / 3 |

</details>

<details>
<summary>luma — 21 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| luma-030 | YUY2 | 1920×1080 | `src.ConvertToY8()` | 0.0739 / 0.0626 / 0.847 | 0.0743 / 0.0675 / 0.909 | True / True | 1 / 3 |
| luma-033 | YV16 | 1920×1080 | `src.ConvertToY8()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| luma-035 | RGB24 | 1920×1080 | `src.ConvertToY()` | 0.4474 / 0.4636 / 1.036 | 0.4386 / 0.4326 / 0.986 | False / False | 3 / 3 |
| luma-037 | RGB32 | 1920×1080 | `src.ConvertToY()` | 1.0472 / 0.6261 / 0.598 | 0.8328 / 0.4265 / 0.512 | False / False | 1 / 3 |
| luma-039 | RGB48 | 1920×1080 | `src.ConvertToY()` | 1.3337 / 1.2800 / 0.960 | 1.1899 / 1.2010 / 1.009 | False / False | 1 / 1 |
| luma-041 | RGB64 | 1920×1080 | `src.ConvertToY()` | 1.9320 / 1.4489 / 0.750 | 1.7956 / 1.3066 / 0.728 | False / False | 3 / 3 |
| luma-043 | RGBP8 | 1920×1080 | `src.ConvertToY()` | 0.2122 / 0.2060 / 0.971 | 0.2115 / 0.1920 / 0.908 | False / False | 1 / 3 |
| luma-045 | RGBP16 | 1920×1080 | `src.ConvertToY()` | 0.4301 / 0.3282 / 0.763 | 0.4031 / 0.3417 / 0.848 | False / False | 1 / 1 |
| luma-047 | RGBPS | 1920×1080 | `src.ConvertToY()` | 0.7560 / 0.7918 / 1.047 | 0.7563 / 0.7504 / 0.992 | False / False | 1 / 1 |
| luma-049 | YV12 | 1920×1080 | `src.ConvertToY()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| luma-051 | YUV420P16 | 1920×1080 | `src.ConvertToY()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| luma-053 | YUV420PS | 1920×1080 | `src.ConvertToY()` | 0.0003 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| luma-467 | YUY2 | 3840×2160 | `src.ConvertToY8()` | 0.6492 / 0.5927 / 0.913 | 0.6792 / 0.5886 / 0.867 | True / True | 1 / 3 |
| extra-028 | RGBP10 | 1920×1080 | `src.ConvertToY()` | 0.3641 / 0.3394 / 0.932 | 0.3680 / 0.3110 / 0.845 | False / False | 1 / 1 |
| extra-030 | RGBP12 | 1920×1080 | `src.ConvertToY()` | 0.6561 / 0.4361 / 0.665 | 0.3409 / 0.3309 / 0.971 | False / False | 1 / 1 |
| extra-032 | RGBP14 | 1920×1080 | `src.ConvertToY()` | 0.4130 / 0.3695 / 0.895 | 0.3591 / 0.3119 / 0.869 | False / False | 1 / 1 |
| extra-034 | RGBAP8 | 1920×1080 | `src.ConvertToY()` | 0.2369 / 0.2147 / 0.906 | 0.2674 / 0.1922 / 0.719 | False / False | 1 / 3 |
| extra-036 | RGBAP16 | 1920×1080 | `src.ConvertToY()` | 0.5192 / 0.4217 / 0.812 | 0.5235 / 0.3101 / 0.592 | False / False | 1 / 1 |
| extra-038 | RGBAPS | 1920×1080 | `src.ConvertToY()` | 0.9739 / 0.9555 / 0.981 | 1.0173 / 0.8709 / 0.856 | False / False | 1 / 1 |
| extra-040 | YUVA444P16 | 1920×1080 | `src.ConvertToY()` | 0.0005 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |
| extra-042 | YUVA444PS | 1920×1080 | `src.ConvertToY()` | 0.0005 / 0.0003 / low-work | 0.0003 / 0.0003 / low-work | True / True | 1 / 1 |

</details>

<details>
<summary>matrix-filter — 54 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| matrix-filter-055 | RGB24 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.7446 / 0.8952 / 1.202 | 0.7461 / 0.6966 / 0.934 | True / True | 3 / 3 |
| matrix-filter-056 | RGB24 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.2282 / 2.7471 / 1.233 | 1.6689 / 1.8549 / 1.111 | True / True | 3 / 3 |
| matrix-filter-057 | RGB24 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 2.4129 / 3.0182 / 1.251 | 1.9184 / 2.0083 / 1.047 | True / True | 3 / 3 |
| matrix-filter-058 | RGB32 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 1.3379 / 1.0579 / 0.791 | 1.2510 / 0.8798 / 0.703 | True / True | 3 / 3 |
| matrix-filter-059 | RGB32 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.8288 / 2.9020 / 1.026 | 2.0732 / 2.0149 / 0.972 | True / True | 3 / 3 |
| matrix-filter-060 | RGB32 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 2.9937 / 3.1589 / 1.055 | 2.4539 / 2.1712 / 0.885 | True / True | 3 / 3 |
| matrix-filter-061 | RGB48 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 1.7886 / 2.0014 / 1.119 | 1.7760 / 1.8215 / 1.026 | False / False | 3 / 3 |
| matrix-filter-062 | RGB48 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 3.5707 / 4.0334 / 1.130 | 4.0377 / 3.0712 / 0.761 | False / False | 3 / 3 |
| matrix-filter-063 | RGB48 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.9283 / 4.3152 / 1.098 | 4.4995 / 3.4103 / 0.758 | False / False | 3 / 3 |
| matrix-filter-064 | RGB64 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 2.4155 / 2.2851 / 0.946 | 2.2413 / 1.9031 / 0.849 | False / False | 3 / 3 |
| matrix-filter-065 | RGB64 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 4.2674 / 4.1538 / 0.973 | 4.5671 / 3.1067 / 0.680 | False / False | 3 / 3 |
| matrix-filter-066 | RGB64 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 4.7095 / 4.5070 / 0.957 | 5.0869 / 3.3210 / 0.653 | False / False | 3 / 3 |
| matrix-filter-067 | RGBP8 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.4927 / 0.4560 / 0.926 | 0.5091 / 0.3951 / 0.776 | True / True | 3 / 3 |
| matrix-filter-068 | RGBP8 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 1.9739 / 2.0945 / 1.061 | 1.3964 / 1.2700 / 0.909 | True / True | 3 / 3 |
| matrix-filter-069 | RGBP8 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 2.2058 / 2.3589 / 1.069 | 1.6958 / 1.4508 / 0.856 | True / True | 3 / 3 |
| matrix-filter-070 | RGBP10 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.7785 / 0.7932 / 1.019 | 0.8383 / 0.7377 / 0.880 | True / True | 3 / 3 |
| matrix-filter-071 | RGBP10 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.6662 / 2.8375 / 1.064 | 2.0176 / 1.9599 / 0.971 | True / True | 3 / 3 |
| matrix-filter-072 | RGBP10 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.0362 / 3.1939 / 1.052 | 2.4305 / 2.2186 / 0.913 | True / True | 3 / 3 |
| matrix-filter-073 | RGBP12 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.7899 / 0.7692 / 0.974 | 0.7947 / 0.6985 / 0.879 | False / False | 3 / 3 |
| matrix-filter-074 | RGBP12 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.6362 / 2.7944 / 1.060 | 1.9698 / 1.9325 / 0.981 | False / False | 3 / 3 |
| matrix-filter-075 | RGBP12 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.0954 / 3.1049 / 1.003 | 2.4391 / 2.1686 / 0.889 | False / False | 3 / 3 |
| matrix-filter-076 | RGBP14 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.7645 / 0.7501 / 0.981 | 0.8089 / 0.7091 / 0.877 | False / False | 3 / 3 |
| matrix-filter-077 | RGBP14 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 2.6003 / 2.8330 / 1.089 | 2.0159 / 1.9721 / 0.978 | False / False | 3 / 3 |
| matrix-filter-078 | RGBP14 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 2.9920 / 3.1179 / 1.042 | 2.4101 / 2.1690 / 0.900 | False / False | 3 / 3 |
| matrix-filter-079 | RGBP16 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 0.8890 / 0.7154 / 0.805 | 0.9765 / 0.6646 / 0.681 | False / False | 3 / 3 |
| matrix-filter-080 | RGBP16 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 3.3444 / 2.6636 / 0.796 | 3.3373 / 1.8996 / 0.569 | False / False | 3 / 3 |
| matrix-filter-081 | RGBP16 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.4758 / 2.9587 / 0.851 | 3.9150 / 2.1117 / 0.539 | False / False | 3 / 3 |
| matrix-filter-082 | RGBPS | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 1.4842 / 1.6482 / 1.110 | 1.5691 / 1.5142 / 0.965 | False / False | 3 / 3 |
| matrix-filter-083 | RGBPS | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 4.0324 / 4.7104 / 1.168 | 5.0861 / 4.0973 / 0.806 | False / False | 3 / 3 |
| matrix-filter-084 | RGBPS | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 4.4318 / 5.1330 / 1.158 | 5.2850 / 4.6047 / 0.871 | False / False | 3 / 3 |
| matrix-filter-085 | RGBAP16 | 1920×1080 | `src.ConvertToYUV444(matrix="Rec709")` | 1.0853 / 0.8701 / 0.802 | 1.1031 / 0.8521 / 0.772 | False / False | 3 / 3 |
| matrix-filter-086 | RGBAP16 | 1920×1080 | `src.ConvertToYUV422(matrix="Rec709")` | 3.2149 / 3.0386 / 0.945 | 3.6654 / 2.2427 / 0.612 | False / False | 3 / 3 |
| matrix-filter-087 | RGBAP16 | 1920×1080 | `src.ConvertToYUV420(matrix="Rec709")` | 3.5749 / 3.2706 / 0.915 | 4.2971 / 2.6401 / 0.614 | False / False | 3 / 3 |
| matrix-filter-088 | YV24 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.4726 / 0.4548 / 0.962 | 0.5029 / 0.3929 / 0.781 | False / False | 3 / 3 |
| matrix-filter-089 | YV24 | 1920×1080 | `src.ConvertToRGB32(matrix="Rec709")` | 0.8788 / 1.1174 / 1.271 | 0.8875 / 0.7839 / 0.883 | False / False | 3 / 3 |
| matrix-filter-090 | YV16 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 3.1763 / 2.1147 / 0.666 | 1.4663 / 1.3081 / 0.892 | False / False | 3 / 3 |
| matrix-filter-091 | YV16 | 1920×1080 | `src.ConvertToRGB32(matrix="Rec709")` | 3.6093 / 2.9314 / 0.812 | 1.8460 / 2.1544 / 1.167 | False / False | 3 / 3 |
| matrix-filter-092 | YV12 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 2.4820 / 2.1918 / 0.883 | 1.7954 / 1.5427 / 0.859 | False / False | 3 / 3 |
| matrix-filter-093 | YV12 | 1920×1080 | `src.ConvertToRGB32(matrix="Rec709")` | 2.8661 / 2.9520 / 1.030 | 2.1703 / 2.3713 / 1.093 | False / False | 3 / 3 |
| matrix-filter-094 | YUV444P10 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.7423 / 0.7319 / 0.986 | 0.7860 / 0.6845 / 0.871 | False / False | 3 / 3 |
| matrix-filter-095 | YUV444P10 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 2.4399 / 2.9178 / 1.196 | 2.4365 / 3.0416 / 1.248 | False / False | 3 / 3 |
| matrix-filter-096 | YUV444P12 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.7524 / 0.7196 / 0.956 | 0.7875 / 0.7127 / 0.905 | False / False | 3 / 3 |
| matrix-filter-097 | YUV444P12 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 2.2828 / 2.8882 / 1.265 | 2.3204 / 2.9461 / 1.270 | False / False | 3 / 3 |
| matrix-filter-098 | YUV444P14 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.7352 / 0.7256 / 0.987 | 0.7728 / 0.6949 / 0.899 | False / False | 3 / 3 |
| matrix-filter-099 | YUV444P14 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 2.3179 / 2.8954 / 1.249 | 2.3859 / 2.9652 / 1.243 | False / False | 3 / 3 |
| matrix-filter-100 | YUV444P16 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 0.7419 / 0.7077 / 0.954 | 0.7778 / 0.6573 / 0.845 | False / False | 3 / 3 |
| matrix-filter-101 | YUV444P16 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 1.9779 / 1.9253 / 0.973 | 2.0263 / 1.8620 / 0.919 | False / False | 3 / 3 |
| matrix-filter-102 | YUV444PS | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 1.4314 / 1.6506 / 1.153 | 1.5718 / 1.5149 / 0.964 | False / False | 3 / 3 |
| matrix-filter-103 | YUV444PS | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 2.6645 / 4.8633 / 1.825 | 3.2910 / 4.6822 / 1.423 | False / False | 3 / 3 |
| matrix-filter-104 | YUV420P16 | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 3.4421 / 2.8835 / 0.838 | 2.5429 / 2.2872 / 0.899 | False / False | 3 / 3 |
| matrix-filter-105 | YUV420P16 | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 4.6571 / 4.0766 / 0.875 | 3.8199 / 3.7809 / 0.990 | False / False | 3 / 3 |
| matrix-filter-106 | YUV420PS | 1920×1080 | `src.ConvertToPlanarRGB(matrix="Rec709")` | 6.5137 / 5.3500 / 0.821 | 4.5857 / 4.7235 / 1.030 | False / False | 3 / 3 |
| matrix-filter-107 | YUV420PS | 1920×1080 | `src.ConvertToRGB64(matrix="Rec709")` | 7.6010 / 8.6730 / 1.141 | 6.3003 / 7.8299 / 1.243 | False / False | 3 / 3 |
| matrix-filter-469 | RGB32 | 3840×2160 | `src.ConvertToYUV420()` | 12.0970 / 12.0585 / 0.997 | 10.8731 / 8.2049 / 0.755 | True / True | 3 / 3 |

</details>

<details>
<summary>ordered — 16 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| ordered-290 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=8)` | 0.0949 / 0.0724 / 0.763 | 0.0979 / 0.0692 / 0.706 | True / True | 1 / 1 |
| ordered-291 | Y10 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=8)` | 0.2442 / 0.3249 / 1.330 | 0.2608 / 0.2711 / 1.039 | True / True | 3 / 3 |
| ordered-292 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=8)` | 0.0934 / 0.0686 / 0.735 | 0.1124 / 0.0708 / 0.630 | True / True | 1 / 1 |
| ordered-293 | Y16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=8)` | 0.2454 / 0.3238 / 1.320 | 0.2708 / 0.2705 / 0.999 | True / True | 3 / 3 |
| ordered-294 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=0,dither_bits=10)` | 0.0919 / 0.0973 / 1.058 | 0.1026 / 0.1126 / 1.097 | True / True | 1 / 1 |
| ordered-295 | Y16 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=0,dither_bits=10)` | 0.2310 / 0.2944 / 1.275 | 0.2469 / 0.2595 / 1.051 | True / True | 3 / 3 |
| ordered-296 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=true,fulld=true,dither=0,dither_bits=10)` | 0.1002 / 0.0984 / 0.982 | 0.1163 / 0.1137 / 0.978 | True / True | 1 / 1 |
| ordered-297 | Y16 | 1920×1080 | `src.ConvertBits(16,fulls=false,fulld=true,dither=0,dither_bits=10)` | 0.2582 / 0.2946 / 1.141 | 0.2749 / 0.2582 / 0.939 | True / True | 3 / 3 |
| ordered-298 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=5)` | 0.3963 / 0.1243 / 0.314 | 0.4126 / 0.1029 / 0.249 | True / True | 3 / 3 |
| ordered-299 | Y8 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=5)` | 0.6280 / 0.3822 / 0.609 | 0.6410 / 0.3140 / 0.490 | True / True | 3 / 3 |
| ordered-300 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=true,fulld=true,dither=0,dither_bits=3)` | 0.3399 / 0.1041 / 0.306 | 0.3581 / 0.1076 / 0.300 | False / False | 3 / 3 |
| ordered-301 | Y10 | 1920×1080 | `src.ConvertBits(10,fulls=false,fulld=true,dither=0,dither_bits=3)` | 0.5561 / 0.3702 / 0.666 | 0.5750 / 0.3049 / 0.530 | False / False | 3 / 3 |
| ordered-302 | YUV444P16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=8)` | 0.4895 / 0.4657 / 0.951 | 0.5284 / 0.4293 / 0.812 | True / True | 1 / 1 |
| ordered-303 | YUV444P16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=8)` | 0.7690 / 0.9787 / 1.273 | 0.8219 / 0.8287 / 1.008 | True / True | 3 / 3 |
| ordered-304 | RGBAP16 | 1920×1080 | `src.ConvertBits(8,fulls=true,fulld=true,dither=0,dither_bits=8)` | 0.7251 / 0.6937 / 0.957 | 0.8044 / 0.6384 / 0.794 | True / True | 1 / 1 |
| ordered-305 | RGBAP16 | 1920×1080 | `src.ConvertBits(8,fulls=false,fulld=true,dither=0,dither_bits=8)` | 0.9657 / 1.1805 / 1.222 | 1.0209 / 1.0108 / 0.990 | True / True | 3 / 3 |

</details>

<details>
<summary>resize — 144 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| resize-322 | Y8 | 1920×1080 | `src.PointResize(960,540)` | 0.6798 / 0.3589 / 0.528 | 0.2951 / 0.2395 / 0.812 | True / True | 3 / 3 |
| resize-323 | Y8 | 1920×1080 | `src.PointResize(2880,1620)` | 2.2321 / 1.1475 / 0.514 | 0.7303 / 0.7130 / 0.976 | True / True | 3 / 3 |
| resize-324 | Y8 | 1920×1080 | `src.PointResize(960,1620)` | 1.1824 / 0.6840 / 0.578 | 0.5030 / 0.5175 / 1.029 | True / True | 3 / 3 |
| resize-325 | Y16 | 1920×1080 | `src.PointResize(960,540)` | 0.8958 / 0.3331 / 0.372 | 0.4511 / 0.2448 / 0.543 | True / True | 3 / 3 |
| resize-326 | Y16 | 1920×1080 | `src.PointResize(2880,1620)` | 2.5130 / 1.1753 / 0.468 | 1.7048 / 0.7654 / 0.449 | True / True | 3 / 3 |
| resize-327 | Y16 | 1920×1080 | `src.PointResize(960,1620)` | 1.3797 / 0.7612 / 0.552 | 0.7212 / 0.5739 / 0.796 | True / True | 3 / 3 |
| resize-328 | Y32 | 1920×1080 | `src.PointResize(960,540)` | 1.7089 / 0.5403 / 0.316 | 1.0368 / 0.4273 / 0.412 | True / True | 3 / 3 |
| resize-329 | Y32 | 1920×1080 | `src.PointResize(2880,1620)` | 5.1167 / 2.0647 / 0.404 | 2.4421 / 1.7136 / 0.702 | True / True | 3 / 3 |
| resize-330 | Y32 | 1920×1080 | `src.PointResize(960,1620)` | 2.8796 / 0.9343 / 0.324 | 2.1093 / 0.9005 / 0.427 | True / True | 3 / 3 |
| resize-331 | Y8 | 1920×1080 | `src.BilinearResize(960,540)` | 0.6852 / 0.4846 / 0.707 | 0.3395 / 0.3025 / 0.891 | True / True | 3 / 3 |
| resize-332 | Y8 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.2803 / 0.9622 / 0.422 | 0.9274 / 0.5080 / 0.548 | True / True | 3 / 3 |
| resize-333 | Y8 | 1920×1080 | `src.BilinearResize(960,1620)` | 1.1799 / 0.7183 / 0.609 | 0.6050 / 0.4853 / 0.802 | True / True | 3 / 3 |
| resize-334 | Y16 | 1920×1080 | `src.BilinearResize(960,540)` | 0.9131 / 0.4794 / 0.525 | 0.8397 / 0.3512 / 0.418 | True / True | 3 / 3 |
| resize-335 | Y16 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.8190 / 1.3900 / 0.493 | 1.3946 / 0.8498 / 0.609 | True / True | 3 / 3 |
| resize-336 | Y16 | 1920×1080 | `src.BilinearResize(960,1620)` | 1.5274 / 1.0458 / 0.685 | 1.1709 / 0.7288 / 0.622 | True / True | 3 / 3 |
| resize-337 | Y32 | 1920×1080 | `src.BilinearResize(960,540)` | 1.7943 / 1.2960 / 0.722 | 1.1074 / 0.9070 / 0.819 | False / False | 3 / 3 |
| resize-338 | Y32 | 1920×1080 | `src.BilinearResize(2880,1620)` | 5.3794 / 2.2908 / 0.426 | 2.7359 / 1.8879 / 0.690 | False / False | 3 / 3 |
| resize-339 | Y32 | 1920×1080 | `src.BilinearResize(960,1620)` | 3.0312 / 2.1338 / 0.704 | 2.1713 / 1.9168 / 0.883 | False / False | 3 / 3 |
| resize-340 | Y8 | 1920×1080 | `src.BicubicResize(960,540)` | 0.7206 / 0.8395 / 1.165 | 0.4024 / 0.4138 / 1.028 | True / True | 3 / 3 |
| resize-341 | Y8 | 1920×1080 | `src.BicubicResize(2880,1620)` | 2.3343 / 1.8104 / 0.776 | 1.2788 / 1.0630 / 0.831 | True / True | 3 / 3 |
| resize-342 | Y8 | 1920×1080 | `src.BicubicResize(960,1620)` | 1.3089 / 1.5547 / 1.188 | 0.8102 / 0.8150 / 1.006 | True / True | 3 / 3 |
| resize-343 | Y16 | 1920×1080 | `src.BicubicResize(960,540)` | 0.9362 / 0.8765 / 0.936 | 0.7138 / 0.4760 / 0.667 | True / True | 3 / 3 |
| resize-344 | Y16 | 1920×1080 | `src.BicubicResize(2880,1620)` | 2.9383 / 1.9232 / 0.655 | 1.9144 / 1.1564 / 0.604 | True / True | 3 / 3 |
| resize-345 | Y16 | 1920×1080 | `src.BicubicResize(960,1620)` | 1.6911 / 1.6872 / 0.998 | 1.3518 / 1.0014 / 0.741 | True / True | 3 / 3 |
| resize-346 | Y32 | 1920×1080 | `src.BicubicResize(960,540)` | 1.0781 / 1.2428 / 1.153 | 1.3239 / 1.0875 / 0.821 | False / False | 3 / 3 |
| resize-347 | Y32 | 1920×1080 | `src.BicubicResize(2880,1620)` | 5.6237 / 3.2940 / 0.586 | 2.8737 / 2.6526 / 0.923 | False / False | 3 / 3 |
| resize-348 | Y32 | 1920×1080 | `src.BicubicResize(960,1620)` | 2.1103 / 2.3169 / 1.098 | 2.7281 / 2.0768 / 0.761 | False / False | 3 / 3 |
| resize-349 | Y8 | 1920×1080 | `src.LanczosResize(960,540)` | 0.7702 / 0.8993 / 1.168 | 0.5860 / 0.5717 / 0.976 | True / True | 3 / 3 |
| resize-350 | Y8 | 1920×1080 | `src.LanczosResize(2880,1620)` | 2.6613 / 2.3623 / 0.888 | 1.8927 / 1.4299 / 0.755 | True / True | 3 / 3 |
| resize-351 | Y8 | 1920×1080 | `src.LanczosResize(960,1620)` | 1.4337 / 1.7210 / 1.200 | 1.2367 / 1.1143 / 0.901 | True / True | 3 / 3 |
| resize-352 | Y16 | 1920×1080 | `src.LanczosResize(960,540)` | 0.9902 / 0.9060 / 0.915 | 0.7162 / 0.6548 / 0.914 | True / True | 3 / 3 |
| resize-353 | Y16 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.3197 / 2.3749 / 0.715 | 2.3961 / 1.4676 / 0.613 | True / True | 3 / 3 |
| resize-354 | Y16 | 1920×1080 | `src.LanczosResize(960,1620)` | 1.8410 / 1.8360 / 0.997 | 1.3737 / 1.3199 / 0.961 | True / True | 3 / 3 |
| resize-355 | Y32 | 1920×1080 | `src.LanczosResize(960,540)` | 1.4957 / 1.5817 / 1.058 | 1.8703 / 1.3482 / 0.721 | False / False | 3 / 3 |
| resize-356 | Y32 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.4958 / 3.8586 / 1.104 | 3.4024 / 3.2955 / 0.969 | False / False | 3 / 3 |
| resize-357 | Y32 | 1920×1080 | `src.LanczosResize(960,1620)` | 2.7844 / 2.8875 / 1.037 | 3.6374 / 2.6580 / 0.731 | False / False | 3 / 3 |
| resize-358 | Y8 | 1920×1080 | `src.Lanczos4Resize(960,540)` | 0.8675 / 0.9410 / 1.085 | 0.6547 / 0.6716 / 1.026 | True / True | 3 / 3 |
| resize-359 | Y8 | 1920×1080 | `src.Lanczos4Resize(2880,1620)` | 3.2955 / 3.3134 / 1.005 | 2.2657 / 1.7250 / 0.761 | True / True | 3 / 3 |
| resize-360 | Y8 | 1920×1080 | `src.Lanczos4Resize(960,1620)` | 1.5631 / 1.8910 / 1.210 | 1.4571 / 1.3625 / 0.935 | True / True | 3 / 3 |
| resize-361 | Y16 | 1920×1080 | `src.Lanczos4Resize(960,540)` | 1.0659 / 0.9438 / 0.885 | 0.9042 / 0.7893 / 0.873 | True / True | 3 / 3 |
| resize-362 | Y16 | 1920×1080 | `src.Lanczos4Resize(2880,1620)` | 3.3931 / 3.2599 / 0.961 | 2.6651 / 1.7403 / 0.653 | True / True | 3 / 3 |
| resize-363 | Y16 | 1920×1080 | `src.Lanczos4Resize(960,1620)` | 1.9666 / 1.9752 / 1.004 | 1.7817 / 1.5471 / 0.868 | True / True | 3 / 3 |
| resize-364 | Y32 | 1920×1080 | `src.Lanczos4Resize(960,540)` | 1.6435 / 2.0630 / 1.255 | 2.0522 / 1.6792 / 0.818 | False / False | 3 / 3 |
| resize-365 | Y32 | 1920×1080 | `src.Lanczos4Resize(2880,1620)` | 3.8317 / 4.7884 / 1.250 | 3.8644 / 3.8935 / 1.008 | False / False | 3 / 3 |
| resize-366 | Y32 | 1920×1080 | `src.Lanczos4Resize(960,1620)` | 3.0492 / 3.5737 / 1.172 | 3.9017 / 3.0676 / 0.786 | False / False | 3 / 3 |
| resize-367 | Y8 | 1920×1080 | `src.BlackmanResize(960,540)` | 0.7988 / 0.9442 / 1.182 | 0.6543 / 0.6496 / 0.993 | True / True | 3 / 3 |
| resize-368 | Y8 | 1920×1080 | `src.BlackmanResize(2880,1620)` | 2.6851 / 3.2808 / 1.222 | 2.2287 / 1.7782 / 0.798 | True / True | 3 / 3 |
| resize-369 | Y8 | 1920×1080 | `src.BlackmanResize(960,1620)` | 1.5314 / 1.8810 / 1.228 | 1.4558 / 1.3469 / 0.925 | True / True | 3 / 3 |
| resize-370 | Y16 | 1920×1080 | `src.BlackmanResize(960,540)` | 1.0585 / 0.9302 / 0.879 | 0.9184 / 0.7540 / 0.821 | True / True | 3 / 3 |
| resize-371 | Y16 | 1920×1080 | `src.BlackmanResize(2880,1620)` | 3.2790 / 3.2113 / 0.979 | 2.6843 / 1.6978 / 0.632 | True / True | 3 / 3 |
| resize-372 | Y16 | 1920×1080 | `src.BlackmanResize(960,1620)` | 1.9318 / 1.9232 / 0.996 | 1.7237 / 1.5561 / 0.903 | True / True | 3 / 3 |
| resize-373 | Y32 | 1920×1080 | `src.BlackmanResize(960,540)` | 1.6204 / 2.0380 / 1.258 | 2.0081 / 1.6372 / 0.815 | False / False | 3 / 3 |
| resize-374 | Y32 | 1920×1080 | `src.BlackmanResize(2880,1620)` | 3.7956 / 4.8562 / 1.279 | 3.6420 / 3.8680 / 1.062 | False / False | 3 / 3 |
| resize-375 | Y32 | 1920×1080 | `src.BlackmanResize(960,1620)` | 3.0324 / 4.1938 / 1.383 | 3.8746 / 3.0771 / 0.794 | False / False | 3 / 3 |
| resize-376 | Y8 | 1920×1080 | `src.Spline16Resize(960,540)` | 0.7178 / 0.8403 / 1.171 | 0.3961 / 0.3998 / 1.009 | True / True | 3 / 3 |
| resize-377 | Y8 | 1920×1080 | `src.Spline16Resize(2880,1620)` | 2.3308 / 2.1861 / 0.938 | 1.3922 / 1.0680 / 0.767 | True / True | 3 / 3 |
| resize-378 | Y8 | 1920×1080 | `src.Spline16Resize(960,1620)` | 1.3169 / 1.5596 / 1.184 | 0.8203 / 0.8334 / 1.016 | True / True | 3 / 3 |
| resize-379 | Y16 | 1920×1080 | `src.Spline16Resize(960,540)` | 0.9440 / 0.8522 / 0.903 | 0.6090 / 0.4775 / 0.784 | True / True | 3 / 3 |
| resize-380 | Y16 | 1920×1080 | `src.Spline16Resize(2880,1620)` | 2.9790 / 1.8178 / 0.610 | 1.9962 / 1.1458 / 0.574 | True / True | 3 / 3 |
| resize-381 | Y16 | 1920×1080 | `src.Spline16Resize(960,1620)` | 1.8173 / 1.7216 / 0.947 | 1.5854 / 0.9870 / 0.623 | True / True | 3 / 3 |
| resize-382 | Y32 | 1920×1080 | `src.Spline16Resize(960,540)` | 1.3098 / 1.3208 / 1.008 | 1.1952 / 1.0148 / 0.849 | False / False | 3 / 3 |
| resize-383 | Y32 | 1920×1080 | `src.Spline16Resize(2880,1620)` | 5.7237 / 3.2180 / 0.562 | 3.0358 / 2.7740 / 0.914 | False / False | 3 / 3 |
| resize-384 | Y32 | 1920×1080 | `src.Spline16Resize(960,1620)` | 2.1304 / 2.3367 / 1.097 | 2.7151 / 2.1216 / 0.781 | False / False | 3 / 3 |
| resize-385 | Y8 | 1920×1080 | `src.Spline36Resize(960,540)` | 0.7855 / 0.8957 / 1.140 | 0.5871 / 0.5597 / 0.953 | True / True | 3 / 3 |
| resize-386 | Y8 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 2.7129 / 2.3731 / 0.875 | 1.8550 / 1.4308 / 0.771 | True / True | 3 / 3 |
| resize-387 | Y8 | 1920×1080 | `src.Spline36Resize(960,1620)` | 1.5344 / 1.7277 / 1.126 | 1.2480 / 1.0929 / 0.876 | True / True | 3 / 3 |
| resize-388 | Y16 | 1920×1080 | `src.Spline36Resize(960,540)` | 1.0232 / 0.9255 / 0.905 | 0.7260 / 0.6450 / 0.888 | True / True | 3 / 3 |
| resize-389 | Y16 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.2689 / 2.3721 / 0.726 | 2.4113 / 1.4263 / 0.592 | True / True | 3 / 3 |
| resize-390 | Y16 | 1920×1080 | `src.Spline36Resize(960,1620)` | 1.9002 / 1.8037 / 0.949 | 1.3576 / 1.3007 / 0.958 | True / True | 3 / 3 |
| resize-391 | Y32 | 1920×1080 | `src.Spline36Resize(960,540)` | 1.5437 / 1.5481 / 1.003 | 1.9536 / 1.4199 / 0.727 | False / False | 3 / 3 |
| resize-392 | Y32 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.5722 / 3.8249 / 1.071 | 3.2024 / 3.3316 / 1.040 | False / False | 3 / 3 |
| resize-393 | Y32 | 1920×1080 | `src.Spline36Resize(960,1620)` | 2.8494 / 3.0593 / 1.074 | 3.7067 / 2.5952 / 0.700 | False / False | 3 / 3 |
| resize-394 | Y8 | 1920×1080 | `src.Spline64Resize(960,540)` | 0.8251 / 0.9429 / 1.143 | 0.6457 / 0.6637 / 1.028 | True / True | 3 / 3 |
| resize-395 | Y8 | 1920×1080 | `src.Spline64Resize(2880,1620)` | 2.7744 / 3.2935 / 1.187 | 2.2239 / 1.7508 / 0.787 | True / True | 3 / 3 |
| resize-396 | Y8 | 1920×1080 | `src.Spline64Resize(960,1620)` | 1.5412 / 1.8788 / 1.219 | 1.4401 / 1.3593 / 0.944 | True / True | 3 / 3 |
| resize-397 | Y16 | 1920×1080 | `src.Spline64Resize(960,540)` | 1.0724 / 0.9379 / 0.875 | 0.9127 / 0.7595 / 0.832 | True / True | 3 / 3 |
| resize-398 | Y16 | 1920×1080 | `src.Spline64Resize(2880,1620)` | 3.4433 / 3.1894 / 0.926 | 2.6949 / 1.7717 / 0.657 | True / True | 3 / 3 |
| resize-399 | Y16 | 1920×1080 | `src.Spline64Resize(960,1620)` | 2.0697 / 1.9264 / 0.931 | 1.7929 / 1.5520 / 0.866 | True / True | 3 / 3 |
| resize-400 | Y32 | 1920×1080 | `src.Spline64Resize(960,540)` | 1.7629 / 2.1052 / 1.194 | 2.0882 / 1.6341 / 0.783 | False / False | 3 / 3 |
| resize-401 | Y32 | 1920×1080 | `src.Spline64Resize(2880,1620)` | 3.8120 / 4.8290 / 1.267 | 3.5669 / 3.7317 / 1.046 | False / False | 3 / 3 |
| resize-402 | Y32 | 1920×1080 | `src.Spline64Resize(960,1620)` | 3.0772 / 3.4901 / 1.134 | 3.8508 / 3.1830 / 0.827 | False / False | 3 / 3 |
| resize-403 | Y8 | 1920×1080 | `src.GaussResize(960,540)` | 0.8175 / 0.9498 / 1.162 | 0.6590 / 0.6659 / 1.010 | True / True | 3 / 3 |
| resize-404 | Y8 | 1920×1080 | `src.GaussResize(2880,1620)` | 2.7161 / 3.3141 / 1.220 | 2.2148 / 1.7721 / 0.800 | True / True | 3 / 3 |
| resize-405 | Y8 | 1920×1080 | `src.GaussResize(960,1620)` | 1.5451 / 1.9325 / 1.251 | 1.4352 / 1.3450 / 0.937 | True / True | 3 / 3 |
| resize-406 | Y16 | 1920×1080 | `src.GaussResize(960,540)` | 1.0839 / 0.9474 / 0.874 | 0.9097 / 0.7790 / 0.856 | True / True | 3 / 3 |
| resize-407 | Y16 | 1920×1080 | `src.GaussResize(2880,1620)` | 3.7006 / 3.3390 / 0.902 | 2.6110 / 1.7477 / 0.669 | True / True | 3 / 3 |
| resize-408 | Y16 | 1920×1080 | `src.GaussResize(960,1620)` | 2.1267 / 1.9631 / 0.923 | 1.7633 / 1.5406 / 0.874 | True / True | 3 / 3 |
| resize-409 | Y32 | 1920×1080 | `src.GaussResize(960,540)` | 1.7128 / 1.9768 / 1.154 | 2.0014 / 1.7181 / 0.858 | False / False | 3 / 3 |
| resize-410 | Y32 | 1920×1080 | `src.GaussResize(2880,1620)` | 4.0593 / 4.8062 / 1.184 | 3.6274 / 3.7647 / 1.038 | False / False | 3 / 3 |
| resize-411 | Y32 | 1920×1080 | `src.GaussResize(960,1620)` | 3.0458 / 3.5643 / 1.170 | 3.7938 / 3.1379 / 0.827 | False / False | 3 / 3 |
| resize-412 | Y8 | 1920×1080 | `src.SincResize(960,540)` | 0.8025 / 0.9425 / 1.174 | 0.6536 / 0.6758 / 1.034 | True / True | 3 / 3 |
| resize-413 | Y8 | 1920×1080 | `src.SincResize(2880,1620)` | 2.7375 / 3.3194 / 1.213 | 2.1922 / 1.7805 / 0.812 | True / True | 3 / 3 |
| resize-414 | Y8 | 1920×1080 | `src.SincResize(960,1620)` | 1.5236 / 1.8833 / 1.236 | 1.4538 / 1.3613 / 0.936 | True / True | 3 / 3 |
| resize-415 | Y16 | 1920×1080 | `src.SincResize(960,540)` | 1.0501 / 0.9368 / 0.892 | 0.9107 / 0.7737 / 0.850 | True / True | 3 / 3 |
| resize-416 | Y16 | 1920×1080 | `src.SincResize(2880,1620)` | 3.2920 / 3.2343 / 0.982 | 2.6467 / 1.7799 / 0.673 | True / True | 3 / 3 |
| resize-417 | Y16 | 1920×1080 | `src.SincResize(960,1620)` | 1.9782 / 1.9562 / 0.989 | 1.8187 / 1.5256 / 0.839 | True / True | 3 / 3 |
| resize-418 | Y32 | 1920×1080 | `src.SincResize(960,540)` | 1.5931 / 1.9500 / 1.224 | 2.0575 / 1.6370 / 0.796 | False / False | 3 / 3 |
| resize-419 | Y32 | 1920×1080 | `src.SincResize(2880,1620)` | 3.7912 / 4.6249 / 1.220 | 3.6092 / 3.9578 / 1.097 | False / False | 3 / 3 |
| resize-420 | Y32 | 1920×1080 | `src.SincResize(960,1620)` | 3.0939 / 3.5175 / 1.137 | 3.8684 / 2.9907 / 0.773 | False / False | 3 / 3 |
| resize-421 | Y8 | 1920×1080 | `src.SinPowerResize(960,540)` | 0.7288 / 0.8438 / 1.158 | 0.3937 / 0.3822 / 0.971 | True / True | 3 / 3 |
| resize-422 | Y8 | 1920×1080 | `src.SinPowerResize(2880,1620)` | 2.3622 / 1.7894 / 0.757 | 1.3013 / 1.0179 / 0.782 | True / True | 3 / 3 |
| resize-423 | Y8 | 1920×1080 | `src.SinPowerResize(960,1620)` | 1.3208 / 1.5528 / 1.176 | 0.8224 / 0.8311 / 1.011 | True / True | 3 / 3 |
| resize-424 | Y16 | 1920×1080 | `src.SinPowerResize(960,540)` | 0.9394 / 0.8560 / 0.911 | 0.9661 / 0.4762 / 0.493 | True / True | 3 / 3 |
| resize-425 | Y16 | 1920×1080 | `src.SinPowerResize(2880,1620)` | 2.9090 / 1.8974 / 0.652 | 1.9160 / 1.2022 / 0.627 | True / True | 3 / 3 |
| resize-426 | Y16 | 1920×1080 | `src.SinPowerResize(960,1620)` | 1.7081 / 1.7352 / 1.016 | 1.5344 / 1.0678 / 0.696 | True / True | 3 / 3 |
| resize-427 | Y32 | 1920×1080 | `src.SinPowerResize(960,540)` | 1.0324 / 1.1741 / 1.137 | 1.3493 / 1.0733 / 0.795 | False / False | 3 / 3 |
| resize-428 | Y32 | 1920×1080 | `src.SinPowerResize(2880,1620)` | 5.6978 / 3.2598 / 0.572 | 2.9997 / 2.8268 / 0.942 | False / False | 3 / 3 |
| resize-429 | Y32 | 1920×1080 | `src.SinPowerResize(960,1620)` | 2.0579 / 2.4290 / 1.180 | 2.7045 / 2.1140 / 0.782 | False / False | 3 / 3 |
| resize-430 | Y8 | 1920×1080 | `src.SincLin2Resize(960,540)` | 2.3422 / 2.7787 / 1.186 | 2.6686 / 2.0572 / 0.771 | True / True | 3 / 3 |
| resize-431 | Y8 | 1920×1080 | `src.SincLin2Resize(2880,1620)` | 6.1501 / 9.8984 / 1.609 | 7.1435 / 5.8707 / 0.822 | True / True | 3 / 3 |
| resize-432 | Y8 | 1920×1080 | `src.SincLin2Resize(960,1620)` | 4.4813 / 5.5690 / 1.243 | 5.4823 / 4.1198 / 0.751 | True / True | 3 / 3 |
| resize-433 | Y16 | 1920×1080 | `src.SincLin2Resize(960,540)` | 3.1909 / 2.8171 / 0.883 | 3.0745 / 2.3889 / 0.777 | True / True | 3 / 3 |
| resize-434 | Y16 | 1920×1080 | `src.SincLin2Resize(2880,1620)` | 7.0710 / 9.0391 / 1.278 | 7.4358 / 5.7535 / 0.774 | True / True | 3 / 3 |
| resize-435 | Y16 | 1920×1080 | `src.SincLin2Resize(960,1620)` | 5.4226 / 5.6296 / 1.038 | 6.0324 / 4.3720 / 0.725 | True / True | 3 / 3 |
| resize-436 | Y32 | 1920×1080 | `src.SincLin2Resize(960,540)` | 5.3759 / 6.7278 / 1.251 | 5.1562 / 4.8855 / 0.948 | False / False | 3 / 3 |
| resize-437 | Y32 | 1920×1080 | `src.SincLin2Resize(2880,1620)` | 11.0242 / 11.9442 / 1.083 | 10.7055 / 11.3707 / 1.062 | False / False | 3 / 3 |
| resize-438 | Y32 | 1920×1080 | `src.SincLin2Resize(960,1620)` | 9.4673 / 12.1497 / 1.283 | 9.4550 / 9.1855 / 0.972 | False / False | 3 / 3 |
| resize-439 | Y8 | 1920×1080 | `src.UserDefined2Resize(960,540)` | 0.7678 / 0.8714 / 1.135 | 0.5538 / 0.4887 / 0.882 | True / True | 3 / 3 |
| resize-440 | Y8 | 1920×1080 | `src.UserDefined2Resize(2880,1620)` | 2.5748 / 2.3847 / 0.926 | 1.7301 / 1.4259 / 0.824 | True / True | 3 / 3 |
| resize-441 | Y8 | 1920×1080 | `src.UserDefined2Resize(960,1620)` | 1.4414 / 1.6865 / 1.170 | 1.1418 / 1.0298 / 0.902 | True / True | 3 / 3 |
| resize-442 | Y16 | 1920×1080 | `src.UserDefined2Resize(960,540)` | 1.0083 / 0.8791 / 0.872 | 0.6226 / 0.5462 / 0.877 | True / True | 3 / 3 |
| resize-443 | Y16 | 1920×1080 | `src.UserDefined2Resize(2880,1620)` | 3.2584 / 2.4372 / 0.748 | 2.4561 / 1.4563 / 0.593 | True / True | 3 / 3 |
| resize-444 | Y16 | 1920×1080 | `src.UserDefined2Resize(960,1620)` | 1.8377 / 1.8893 / 1.028 | 1.1901 / 1.3055 / 1.097 | True / True | 3 / 3 |
| resize-445 | Y32 | 1920×1080 | `src.UserDefined2Resize(960,540)` | 1.4076 / 1.3222 / 0.939 | 2.1415 / 1.1774 / 0.550 | False / False | 3 / 3 |
| resize-446 | Y32 | 1920×1080 | `src.UserDefined2Resize(2880,1620)` | 3.3421 / 3.5229 / 1.054 | 3.1079 / 3.0045 / 0.967 | False / False | 3 / 3 |
| resize-447 | Y32 | 1920×1080 | `src.UserDefined2Resize(960,1620)` | 2.7393 / 2.6119 / 0.954 | 3.4899 / 2.3075 / 0.661 | False / False | 3 / 3 |
| extra-001 | Y10 | 1920×1080 | `src.BilinearResize(960,540)` | 0.8304 / 0.5296 / 0.638 | 0.4588 / 0.3520 / 0.767 | True / True | 3 / 3 |
| extra-002 | Y10 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.6077 / 1.4513 / 0.557 | 1.3326 / 0.9641 / 0.724 | True / True | 3 / 3 |
| extra-003 | Y10 | 1920×1080 | `src.LanczosResize(960,540)` | 0.9130 / 0.9673 / 1.059 | 0.6888 / 0.7136 / 1.036 | True / True | 3 / 3 |
| extra-004 | Y10 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.0391 / 2.5360 / 0.834 | 2.3024 / 1.5785 / 0.686 | True / True | 3 / 3 |
| extra-005 | Y10 | 1920×1080 | `src.Spline36Resize(960,540)` | 0.9475 / 0.9854 / 1.040 | 0.6758 / 0.6944 / 1.028 | True / True | 3 / 3 |
| extra-006 | Y10 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.0188 / 2.4341 / 0.806 | 2.2654 / 1.5439 / 0.682 | True / True | 3 / 3 |
| extra-007 | Y12 | 1920×1080 | `src.BilinearResize(960,540)` | 0.8246 / 0.5524 / 0.670 | 0.4415 / 0.3652 / 0.827 | True / True | 3 / 3 |
| extra-008 | Y12 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.6357 / 1.3740 / 0.521 | 1.3158 / 0.9137 / 0.694 | True / True | 3 / 3 |
| extra-009 | Y12 | 1920×1080 | `src.LanczosResize(960,540)` | 0.9640 / 0.9724 / 1.009 | 0.6719 / 0.6572 / 0.978 | True / True | 3 / 3 |
| extra-010 | Y12 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.5258 / 2.4667 / 0.700 | 2.3205 / 1.5625 / 0.673 | True / True | 3 / 3 |
| extra-011 | Y12 | 1920×1080 | `src.Spline36Resize(960,540)` | 0.9901 / 0.9628 / 0.972 | 0.7006 / 0.6430 / 0.918 | True / True | 3 / 3 |
| extra-012 | Y12 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.0944 / 2.4699 / 0.798 | 2.2684 / 1.5831 / 0.698 | True / True | 3 / 3 |
| extra-013 | Y14 | 1920×1080 | `src.BilinearResize(960,540)` | 0.8770 / 0.5344 / 0.609 | 0.4418 / 0.3599 / 0.815 | True / True | 3 / 3 |
| extra-014 | Y14 | 1920×1080 | `src.BilinearResize(2880,1620)` | 2.7323 / 1.4423 / 0.528 | 1.3907 / 0.8954 / 0.644 | True / True | 3 / 3 |
| extra-015 | Y14 | 1920×1080 | `src.LanczosResize(960,540)` | 0.9693 / 0.9650 / 0.996 | 0.6756 / 0.7008 / 1.037 | True / True | 3 / 3 |
| extra-016 | Y14 | 1920×1080 | `src.LanczosResize(2880,1620)` | 3.0997 / 2.4876 / 0.803 | 2.2507 / 1.5034 / 0.668 | True / True | 3 / 3 |
| extra-017 | Y14 | 1920×1080 | `src.Spline36Resize(960,540)` | 0.9902 / 0.9672 / 0.977 | 0.6669 / 0.6644 / 0.996 | True / True | 3 / 3 |
| extra-018 | Y14 | 1920×1080 | `src.Spline36Resize(2880,1620)` | 3.1997 / 2.5576 / 0.799 | 2.2499 / 1.5093 / 0.671 | True / True | 3 / 3 |

</details>

<details>
<summary>resize-composed — 19 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| resize-composed-448 | RGB24 | 1920×1080 | `src.LanczosResize(960,540)` | 4.8572 / 3.3844 / 0.697 | 5.7176 / 2.3112 / 0.404 | True / True | 3 / 3 |
| resize-composed-449 | RGB24 | 1920×1080 | `src.LanczosResize(2880,1620)` | 9.6009 / 8.6542 / 0.901 | 11.2427 / 5.6444 / 0.502 | True / True | 3 / 3 |
| resize-composed-450 | RGB32 | 1920×1080 | `src.LanczosResize(960,540)` | 3.2803 / 4.5293 / 1.381 | 4.0056 / 3.1887 / 0.796 | True / True | 3 / 3 |
| resize-composed-451 | RGB32 | 1920×1080 | `src.LanczosResize(2880,1620)` | 7.0602 / 11.3349 / 1.605 | 9.0501 / 7.3843 / 0.816 | True / True | 3 / 3 |
| resize-composed-452 | RGB48 | 1920×1080 | `src.LanczosResize(960,540)` | 7.5777 / 4.2371 / 0.559 | 7.6564 / 3.4853 / 0.455 | True / True | 3 / 3 |
| resize-composed-453 | RGB48 | 1920×1080 | `src.LanczosResize(2880,1620)` | 14.1355 / 9.8847 / 0.699 | 14.8278 / 7.0522 / 0.476 | True / True | 3 / 3 |
| resize-composed-454 | RGB64 | 1920×1080 | `src.LanczosResize(960,540)` | 5.2039 / 5.5468 / 1.066 | 5.5536 / 4.6530 / 0.838 | True / True | 3 / 3 |
| resize-composed-455 | RGB64 | 1920×1080 | `src.LanczosResize(2880,1620)` | 11.6854 / 12.9070 / 1.105 | 12.3712 / 9.2516 / 0.748 | True / True | 3 / 3 |
| resize-composed-456 | RGBAP16 | 1920×1080 | `src.LanczosResize(960,540)` | 4.2585 / 4.2254 / 0.992 | 3.7700 / 2.9660 / 0.787 | True / True | 3 / 3 |
| resize-composed-457 | RGBAP16 | 1920×1080 | `src.LanczosResize(2880,1620)` | 13.4831 / 9.8113 / 0.728 | 10.5025 / 6.3896 / 0.608 | True / True | 3 / 3 |
| resize-composed-458 | YV12 | 1920×1080 | `src.LanczosResize(960,540)` | 1.1514 / 1.3698 / 1.190 | 0.9237 / 0.8794 / 0.952 | True / True | 3 / 3 |
| resize-composed-459 | YV12 | 1920×1080 | `src.LanczosResize(2880,1620)` | 4.1591 / 3.7044 / 0.891 | 2.9873 / 2.2632 / 0.758 | True / True | 3 / 3 |
| resize-composed-460 | YUV420P16 | 1920×1080 | `src.LanczosResize(960,540)` | 1.5391 / 1.3972 / 0.908 | 1.1575 / 1.0395 / 0.898 | True / True | 3 / 3 |
| resize-composed-461 | YUV420P16 | 1920×1080 | `src.LanczosResize(2880,1620)` | 4.9817 / 3.6860 / 0.740 | 3.6770 / 2.5017 / 0.680 | True / True | 3 / 3 |
| resize-composed-462 | YUV420PS | 1920×1080 | `src.LanczosResize(960,540)` | 2.3796 / 2.4835 / 1.044 | 4.1050 / 2.2253 / 0.542 | False / False | 3 / 3 |
| resize-composed-463 | YUV420PS | 1920×1080 | `src.LanczosResize(2880,1620)` | 5.2721 / 6.3318 / 1.201 | 5.2061 / 4.7031 / 0.903 | False / False | 3 / 3 |
| resize-composed-464 | YUY2 | 1920×1080 | `src.LanczosResize(960,540)` | 2.1546 / 2.2028 / 1.022 | 1.7961 / 1.5360 / 0.855 | True / True | 3 / 3 |
| resize-composed-465 | YUY2 | 1920×1080 | `src.LanczosResize(2880,1620)` | 6.3326 / 5.9264 / 0.936 | 4.4545 / 3.6207 / 0.813 | True / True | 3 / 3 |
| resize-composed-473 | YUV420P16 | 3840×2160 | `src.LanczosResize(1920,1080)` | 6.1202 / 6.1492 / 1.005 | 5.3792 / 4.4352 / 0.825 | True / True | 3 / 3 |

</details>

<details>
<summary>yuy2 — 11 cases</summary>

| Case | Input | Size | Expression | AVX2: old / new / ratio | Native: old / new / ratio | Equal output | Observations AVX2 / native |
|---|---|---|---|---|---|---|---|
| yuy2-029 | YUY2 | 1920×1080 | `src.ConvertToYV16()` | 0.4125 / 0.0832 / 0.202 | 0.4173 / 0.0882 / 0.211 | True / True | 3 / 3 |
| yuy2-032 | YV16 | 1920×1080 | `src.ConvertToYUY2()` | 0.0896 / 0.0759 / 0.847 | 0.0940 / 0.0803 / 0.854 | True / True | 3 / 3 |
| yuy2-138 | YUY2 | 1920×1080 | `src.ConvertToRGB24()` | 5.0136 / 3.5896 / 0.716 | 3.2946 / 2.6273 / 0.797 | False / False | 3 / 3 |
| yuy2-139 | YUY2 | 1920×1080 | `src.ConvertToRGB32()` | 4.8543 / 3.6945 / 0.761 | 3.0963 / 2.7367 / 0.884 | False / False | 3 / 3 |
| yuy2-140 | YUY2 | 1920×1080 | `src.ConvertToYV12()` | 1.5727 / 0.5750 / 0.366 | 1.6913 / 0.5073 / 0.300 | True / True | 3 / 3 |
| yuy2-141 | YUY2 | 1920×1080 | `src.ConvertToYV24()` | 3.9239 / 1.9066 / 0.486 | 2.0798 / 1.0578 / 0.509 | True / True | 3 / 3 |
| yuy2-142 | RGB24 | 1920×1080 | `src.ConvertToYUY2()` | 2.4049 / 2.9282 / 1.218 | 1.8446 / 2.0871 / 1.131 | True / True | 3 / 3 |
| yuy2-143 | RGB32 | 1920×1080 | `src.ConvertToYUY2()` | 2.9987 / 3.0709 / 1.024 | 2.1728 / 2.2674 / 1.044 | True / True | 3 / 3 |
| yuy2-144 | YV12 | 1920×1080 | `src.ConvertToYUY2()` | 0.5258 / 0.6223 / 1.184 | 0.6358 / 0.5273 / 0.829 | True / True | 3 / 3 |
| yuy2-145 | YV24 | 1920×1080 | `src.ConvertToYUY2()` | 1.6180 / 1.6786 / 1.037 | 0.8944 / 0.8743 / 0.978 | True / True | 3 / 3 |
| yuy2-470 | RGB32 | 3840×2160 | `src.ConvertToYUY2()` | 11.6317 / 11.6994 / 1.006 | 10.2158 / 8.2307 / 0.806 | True / True | 3 / 3 |

</details>

## Complete kernel comparisons

Allocation and coefficient construction are outside these timings. Baseline and notes identify the actual upstream implementation. Native resampling uses the saved upstream AVX512 baseline.

<details>
<summary>depth — 112 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| source_bits=8; destination_bits=8; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.1872 | 0.0722 | 0.386 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained; upstream FMA rounding may differ. |
| source_bits=8; destination_bits=8; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1872 | 0.0611 | 0.326 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained; upstream FMA rounding may differ. |
| source_bits=8; destination_bits=8; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.1845 | 0.0726 | 0.394 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained; upstream FMA rounding may differ. |
| source_bits=8; destination_bits=8; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1845 | 0.0609 | 0.330 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained; upstream FMA rounding may differ. |
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
<summary>matrix — 72 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| route=RGB-YUV; bits=8; source_full=1; destination_full=0; width=1920; height=1080 | AVX2 | 0.5195 | 0.4724 | 0.909 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=0; width=1920; height=1080 | AVX3_ZEN4 | 0.5195 | 0.3958 | 0.762 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=0; width=3840; height=2160 | AVX2 | 2.2549 | 2.2320 | 0.990 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=0; width=3840; height=2160 | AVX3_ZEN4 | 2.2549 | 1.8479 | 0.820 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.5262 | 0.4948 | 0.940 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.5262 | 0.3994 | 0.759 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 2.2969 | 2.1867 | 0.952 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=RGB-YUV; bits=8; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 2.2969 | 1.8722 | 0.815 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| route=YUV-RGB; bits=8; source_full=0; destination_full=1; width=1920; height=1080 | AVX2 | 0.4994 | 0.5041 | 1.009 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=0; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.4994 | 0.3906 | 0.782 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| route=YUV-RGB; bits=8; source_full=0; destination_full=1; width=3840; height=2160 | AVX2 | 2.2234 | 2.2336 | 1.005 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=0; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 2.2234 | 1.8579 | 0.836 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| route=YUV-RGB; bits=8; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.4995 | 0.5071 | 1.015 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.4995 | 0.3878 | 0.776 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| route=YUV-RGB; bits=8; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 2.2011 | 2.2221 | 1.010 | AVX2 | Retained accepted U8 paired-batch measurements, five rounds. |
| route=YUV-RGB; bits=8; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 2.2011 | 1.8503 | 0.841 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=0; width=1920; height=1080 | AVX2 | 0.8424 | 0.7868 | 0.934 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=0; width=1920; height=1080 | AVX3_ZEN4 | 0.8424 | 0.7378 | 0.876 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=0; width=3840; height=2160 | AVX2 | 3.7488 | 3.6482 | 0.973 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=0; width=3840; height=2160 | AVX3_ZEN4 | 3.7488 | 3.3982 | 0.906 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.8362 | 0.7868 | 0.941 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.8362 | 0.7287 | 0.872 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 3.5432 | 3.4327 | 0.969 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=10; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.5432 | 3.3732 | 0.952 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=0; destination_full=1; width=1920; height=1080 | AVX2 | 0.8177 | 0.8137 | 0.995 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=0; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.8177 | 0.7412 | 0.906 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=0; destination_full=1; width=3840; height=2160 | AVX2 | 3.4150 | 3.5107 | 1.028 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=0; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.4150 | 3.3591 | 0.984 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.7731 | 0.8364 | 1.082 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.7731 | 0.7342 | 0.950 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 3.4300 | 3.4387 | 1.003 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=YUV-RGB; bits=10; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.4300 | 3.3850 | 0.987 | AVX2 | Retained accepted 10-bit measurements, three rounds. Upstream high-depth arithmetic may differ. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=0; width=1920; height=1080 | AVX2 | 0.9465 | 0.7383 | 0.780 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=0; width=1920; height=1080 | AVX3_ZEN4 | 0.9465 | 0.6911 | 0.730 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=0; width=3840; height=2160 | AVX2 | 3.7026 | 3.3799 | 0.913 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=0; width=3840; height=2160 | AVX3_ZEN4 | 3.7026 | 3.5500 | 0.959 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.9919 | 0.8227 | 0.829 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.9919 | 0.8017 | 0.808 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 3.8395 | 3.4665 | 0.903 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.8395 | 3.4318 | 0.894 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=16; precision=20; source_full=1; destination_full=1; width=960; height=540 | AVX2 | unavailable | 0.8846 | unavailable | unavailable | High-precision API; three rounds of continuous batches, exact C. No saved upstream counterpart. |
| route=RGB-YUV; bits=16; precision=20; source_full=1; destination_full=1; width=960; height=540 | AVX3_ZEN4 | unavailable | 0.4645 | unavailable | unavailable | High-precision API; three rounds of continuous batches, exact C. No saved upstream counterpart. |
| route=RGB-YUV; bits=16; precision=20; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | unavailable | 3.5837 | unavailable | unavailable | High-precision API; three rounds of continuous batches, exact C. No saved upstream counterpart. |
| route=RGB-YUV; bits=16; precision=20; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | unavailable | 1.8791 | unavailable | unavailable | High-precision API; three rounds of continuous batches, exact C. No saved upstream counterpart. |
| route=YUV-RGB; bits=16; source_full=0; destination_full=1; width=1920; height=1080 | AVX2 | 0.7752 | 0.7714 | 0.995 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=0; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.7752 | 0.6823 | 0.880 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=0; destination_full=1; width=3840; height=2160 | AVX2 | 3.4183 | 3.3900 | 0.992 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=0; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.4183 | 3.4213 | 1.001 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 0.8357 | 0.8313 | 0.995 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 0.8357 | 0.7228 | 0.865 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 3.4013 | 3.4395 | 1.011 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 3.4013 | 3.3725 | 0.992 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=16; precision=20; source_full=1; destination_full=1; width=960; height=540 | AVX2 | unavailable | 0.8978 | unavailable | unavailable | High-precision API; three rounds of continuous batches, exact C. No saved upstream counterpart. |
| route=YUV-RGB; bits=16; precision=20; source_full=1; destination_full=1; width=960; height=540 | AVX3_ZEN4 | unavailable | 0.4654 | unavailable | unavailable | High-precision API; three rounds of continuous batches, exact C. No saved upstream counterpart. |
| route=YUV-RGB; bits=16; precision=20; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | unavailable | 3.6145 | unavailable | unavailable | High-precision API; three rounds of continuous batches, exact C. No saved upstream counterpart. |
| route=YUV-RGB; bits=16; precision=20; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | unavailable | 1.8821 | unavailable | unavailable | High-precision API; three rounds of continuous batches, exact C. No saved upstream counterpart. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=0; width=1920; height=1080 | AVX2 | 1.5241 | 1.5713 | 1.031 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=0; width=1920; height=1080 | AVX3_ZEN4 | 1.5241 | 1.6579 | 1.088 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=0; width=3840; height=2160 | AVX2 | 6.7368 | 6.6701 | 0.990 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=0; width=3840; height=2160 | AVX3_ZEN4 | 6.7368 | 6.4249 | 0.954 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 1.5194 | 1.5737 | 1.036 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 1.5194 | 1.5282 | 1.006 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 6.5819 | 6.7352 | 1.023 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=RGB-YUV; bits=32; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 6.5819 | 6.6299 | 1.007 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=0; destination_full=1; width=1920; height=1080 | AVX2 | 1.4873 | 1.5815 | 1.063 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=0; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 1.4873 | 1.6644 | 1.119 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=0; destination_full=1; width=3840; height=2160 | AVX2 | 6.1183 | 6.7108 | 1.097 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=0; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 6.1183 | 6.3322 | 1.035 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=1; destination_full=1; width=1920; height=1080 | AVX2 | 1.4766 | 1.5891 | 1.076 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=1; destination_full=1; width=1920; height=1080 | AVX3_ZEN4 | 1.4766 | 1.6069 | 1.088 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=1; destination_full=1; width=3840; height=2160 | AVX2 | 6.3283 | 6.6725 | 1.054 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |
| route=YUV-RGB; bits=32; source_full=1; destination_full=1; width=3840; height=2160 | AVX3_ZEN4 | 6.3283 | 6.3360 | 1.001 | AVX2 | Fresh three-round measurement. Upstream high-depth arithmetic differs; reverse F32 omits clipping. |

</details>

<details>
<summary>ordered — 48 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.0887 | 0.0711 | 0.801 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.0887 | 0.0649 | 0.732 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.2482 | 0.3178 | 1.281 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.2482 | 0.2648 | 1.067 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.0929 | 0.0770 | 0.828 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.0929 | 0.0754 | 0.811 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.2480 | 0.3208 | 1.294 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.2480 | 0.2652 | 1.069 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.0926 | 0.0772 | 0.834 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.0926 | 0.0710 | 0.766 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.2482 | 0.3214 | 1.295 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.2482 | 0.2650 | 1.068 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.1005 | 0.0807 | 0.803 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.1005 | 0.0686 | 0.683 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.2472 | 0.3205 | 1.296 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=8; quantization_bits=8; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.2472 | 0.2649 | 1.071 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.0841 | 0.0955 | 1.136 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.0841 | 0.0881 | 1.049 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.2285 | 0.2912 | 1.274 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.2285 | 0.2543 | 1.113 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.0853 | 0.0977 | 1.145 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.0853 | 0.0912 | 1.069 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.2290 | 0.2909 | 1.270 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=10; quantization_bits=10; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.2290 | 0.2546 | 1.112 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.1017 | 0.0958 | 0.941 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.1017 | 0.0891 | 0.876 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.2589 | 0.2901 | 1.121 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.2589 | 0.2539 | 0.981 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.0985 | 0.0967 | 0.982 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.0985 | 0.0876 | 0.890 | AVX2 | Fresh three-round measurement. Corrected Bayer entries can differ from upstream. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.2572 | 0.2907 | 1.130 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=16; destination_bits=16; quantization_bits=10; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.2572 | 0.2544 | 0.989 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.3922 | 0.1205 | 0.307 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.3922 | 0.1003 | 0.256 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.6156 | 0.3775 | 0.613 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.6156 | 0.3069 | 0.499 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.3954 | 0.1184 | 0.300 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.3954 | 0.1000 | 0.253 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.6204 | 0.3794 | 0.612 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=8; destination_bits=8; quantization_bits=5; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.6204 | 0.3076 | 0.496 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=1; destination_full=1; chroma=0 | AVX2 | 0.3325 | 0.1002 | 0.301 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=1; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.3325 | 0.1008 | 0.303 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=0; destination_full=1; chroma=0 | AVX2 | 0.5545 | 0.3640 | 0.656 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=0; destination_full=1; chroma=0 | AVX3_ZEN4 | 0.5545 | 0.3003 | 0.542 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=1; destination_full=1; chroma=1 | AVX2 | 0.3321 | 0.0994 | 0.299 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=1; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.3321 | 0.1011 | 0.304 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=0; destination_full=1; chroma=1 | AVX2 | 0.5506 | 0.3650 | 0.663 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| source_bits=10; destination_bits=10; quantization_bits=3; source_full=0; destination_full=1; chroma=1 | AVX3_ZEN4 | 0.5506 | 0.2996 | 0.544 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |

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
| axis=H; filter=triangle; bits=8; width=640; height=360; target=320 | AVX2 | 0.0723 | 0.0366 | 0.506 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=640; height=360; target=320 | native | 0.0230 | 0.0225 | 0.975 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=640; height=360; target=320 | AVX2 | 0.0816 | 0.0392 | 0.480 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=640; height=360; target=320 | native | 0.0340 | 0.0258 | 0.758 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=640; height=360; target=320 | AVX2 | 0.1543 | 0.0684 | 0.443 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=640; height=360; target=320 | native | 0.0338 | 0.0445 | 1.316 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=640; height=360; target=320 | AVX2 | 0.0724 | 0.0843 | 1.164 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=640; height=360; target=320 | native | 0.0396 | 0.0368 | 0.930 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=640; height=360; target=320 | AVX2 | 0.0827 | 0.0847 | 1.024 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=640; height=360; target=320 | native | 0.0467 | 0.0495 | 1.061 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=640; height=360; target=320 | AVX2 | 0.1096 | 0.1031 | 0.941 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=640; height=360; target=320 | native | 0.1133 | 0.0771 | 0.680 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=640; height=360; target=320 | AVX2 | 0.0726 | 0.0848 | 1.168 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=640; height=360; target=320 | native | 0.0397 | 0.0368 | 0.925 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=640; height=360; target=320 | AVX2 | 0.0824 | 0.0863 | 1.048 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=640; height=360; target=320 | native | 0.0461 | 0.0499 | 1.083 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=640; height=360; target=320 | AVX2 | 0.1193 | 0.1020 | 0.855 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=640; height=360; target=320 | native | 0.1138 | 0.0761 | 0.668 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=640; height=360; target=960 | AVX2 | 0.2662 | 0.0729 | 0.274 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=640; height=360; target=960 | native | 0.0414 | 0.0333 | 0.804 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=640; height=360; target=960 | AVX2 | 0.2980 | 0.0697 | 0.234 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=640; height=360; target=960 | native | 0.0404 | 0.0397 | 0.984 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=640; height=360; target=960 | AVX2 | 0.4677 | 0.0850 | 0.182 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=640; height=360; target=960 | native | 0.0649 | 0.0550 | 0.848 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=640; height=360; target=960 | AVX2 | 0.2710 | 0.1413 | 0.521 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=640; height=360; target=960 | native | 0.0720 | 0.0608 | 0.845 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=640; height=360; target=960 | AVX2 | 0.2904 | 0.1386 | 0.477 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=640; height=360; target=960 | native | 0.0725 | 0.0639 | 0.882 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=640; height=360; target=960 | AVX2 | 0.2055 | 0.2001 | 0.974 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=640; height=360; target=960 | native | 0.1337 | 0.0997 | 0.745 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=640; height=360; target=960 | AVX2 | 0.2723 | 0.1410 | 0.518 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=640; height=360; target=960 | native | 0.0721 | 0.0597 | 0.829 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=640; height=360; target=960 | AVX2 | 0.2945 | 0.1389 | 0.472 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=640; height=360; target=960 | native | 0.0711 | 0.0650 | 0.914 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=640; height=360; target=960 | AVX2 | 0.2121 | 0.1989 | 0.938 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=640; height=360; target=960 | native | 0.1358 | 0.0974 | 0.717 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
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
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=960 | AVX2 | 0.6411 | 0.3113 | 0.486 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=960 | native | 0.2106 | 0.2054 | 0.976 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=960 | AVX2 | 0.7259 | 0.3290 | 0.453 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=960 | native | 0.3282 | 0.2395 | 0.730 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=960 | AVX2 | 1.5623 | 0.7591 | 0.486 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=960 | native | 0.5650 | 0.5966 | 1.056 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=960 | AVX2 | 0.6762 | 0.7148 | 1.057 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=960 | native | 0.3701 | 0.3366 | 0.909 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=960 | AVX2 | 0.7312 | 0.7466 | 1.021 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=960 | native | 0.4140 | 0.4396 | 1.062 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=960 | AVX2 | 1.0617 | 1.0059 | 0.947 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=960 | native | 1.1187 | 0.7867 | 0.703 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=960 | AVX2 | 0.6445 | 0.7148 | 1.109 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=960 | native | 0.3625 | 0.3359 | 0.927 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=960 | AVX2 | 0.7325 | 0.7412 | 1.012 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=960 | native | 0.4169 | 0.4384 | 1.052 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=960 | AVX2 | 1.0880 | 0.9680 | 0.890 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=960 | native | 1.1598 | 0.8129 | 0.701 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=2880 | AVX2 | 2.0130 | 0.6597 | 0.328 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=2880 | native | 0.4051 | 0.3002 | 0.741 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=2880 | AVX2 | 2.3284 | 0.6347 | 0.273 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=2880 | native | 0.4151 | 0.3626 | 0.874 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=2880 | AVX2 | 4.4401 | 0.9654 | 0.217 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=2880 | native | 1.1459 | 0.7586 | 0.662 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=2880 | AVX2 | 2.1179 | 1.2811 | 0.605 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=2880 | native | 0.6734 | 0.5393 | 0.801 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=2880 | AVX2 | 2.3723 | 1.2450 | 0.525 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=2880 | native | 0.6695 | 0.5924 | 0.885 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=2880 | AVX2 | 1.8975 | 1.8219 | 0.960 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=2880 | native | 1.2347 | 1.2391 | 1.004 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=2880 | AVX2 | 2.0616 | 1.2773 | 0.620 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=2880 | native | 0.6704 | 0.5391 | 0.804 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=2880 | AVX2 | 2.3028 | 1.2662 | 0.550 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=2880 | native | 0.6638 | 0.5987 | 0.902 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=2880 | AVX2 | 1.9772 | 1.8041 | 0.912 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=2880 | native | 1.2377 | 1.2431 | 1.004 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=1152 | AVX2 | 0.7600 | 0.3722 | 0.490 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=1152 | native | 0.1809 | 0.1669 | 0.922 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=1152 | AVX2 | 0.8674 | 0.3826 | 0.441 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=1152 | native | 0.1783 | 0.2022 | 1.134 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=1152 | AVX2 | 1.7638 | 0.7187 | 0.408 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=1152 | native | 0.4703 | 0.5115 | 1.088 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=1152 | AVX2 | 0.7845 | 0.8405 | 1.071 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=1152 | native | 0.4380 | 0.3953 | 0.903 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=1152 | AVX2 | 0.8863 | 0.8640 | 0.975 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=1152 | native | 0.4186 | 0.4548 | 1.087 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=1152 | AVX2 | 1.2367 | 1.3307 | 1.076 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=1152 | native | 1.4467 | 1.1988 | 0.829 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=1152 | AVX2 | 0.8006 | 0.8419 | 1.052 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=1152 | native | 0.4425 | 0.3977 | 0.899 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=1152 | AVX2 | 0.8976 | 0.8588 | 0.957 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=1152 | native | 0.4218 | 0.4487 | 1.064 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=1152 | AVX2 | 1.2243 | 1.2667 | 1.035 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=1152 | native | 1.4649 | 1.1837 | 0.808 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=1280 | AVX2 | 0.9069 | 0.4349 | 0.480 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=1280 | native | 0.1845 | 0.1867 | 1.012 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=1280 | AVX2 | 1.0458 | 0.4218 | 0.403 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=1280 | native | 0.2269 | 0.2211 | 0.974 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=1280 | AVX2 | 2.6713 | 0.6788 | 0.254 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=1280 | native | 0.6762 | 0.4760 | 0.704 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=1280 | AVX2 | 0.8991 | 0.9641 | 1.072 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=1280 | native | 0.4937 | 0.3517 | 0.713 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=1280 | AVX2 | 1.0090 | 0.9692 | 0.961 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=1280 | native | 0.6460 | 0.3857 | 0.597 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=1280 | AVX2 | 1.4019 | 1.2574 | 0.897 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=1280 | native | 1.5126 | 0.8390 | 0.555 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=1280 | AVX2 | 0.8654 | 0.9280 | 1.072 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=1280 | native | 0.4971 | 0.3535 | 0.711 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=1280 | AVX2 | 1.0307 | 0.9740 | 0.945 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=1280 | native | 0.6712 | 0.3839 | 0.572 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=1280 | AVX2 | 1.4520 | 1.2496 | 0.861 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=1280 | native | 1.5604 | 0.8409 | 0.539 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=2400 | AVX2 | 1.7323 | 0.5505 | 0.318 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=8; width=1920; height=1080; target=2400 | native | 0.3461 | 0.2520 | 0.728 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=2400 | AVX2 | 1.9686 | 0.5287 | 0.269 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=16; width=1920; height=1080; target=2400 | native | 0.3476 | 0.3059 | 0.880 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=2400 | AVX2 | 3.6982 | 0.8174 | 0.221 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=triangle; bits=32; width=1920; height=1080; target=2400 | native | 0.8287 | 0.5952 | 0.718 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=2400 | AVX2 | 1.7319 | 1.0725 | 0.619 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=8; width=1920; height=1080; target=2400 | native | 0.5633 | 0.4566 | 0.811 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=2400 | AVX2 | 1.9541 | 1.0538 | 0.539 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=16; width=1920; height=1080; target=2400 | native | 0.5629 | 0.4978 | 0.884 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=2400 | AVX2 | 1.6125 | 1.5413 | 0.956 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=lanczos3; bits=32; width=1920; height=1080; target=2400 | native | 1.0626 | 1.2201 | 1.148 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=2400 | AVX2 | 1.6327 | 1.0736 | 0.658 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=8; width=1920; height=1080; target=2400 | native | 0.5605 | 0.4651 | 0.830 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=2400 | AVX2 | 1.8923 | 1.0641 | 0.562 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=16; width=1920; height=1080; target=2400 | native | 0.5730 | 0.5096 | 0.889 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=2400 | AVX2 | 1.6593 | 1.6622 | 1.002 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| axis=H; filter=spline36; bits=32; width=1920; height=1080; target=2400 | native | 1.0530 | 1.1312 | 1.074 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |

</details>

<details>
<summary>resample-pipeline — 54 rows</summary>

| Route | Target | Upstream ms | New ms | New / upstream | Baseline | Notes |
|---|---|---|---|---|---|---|
| order=HV; filter=triangle; bits=8; dw=960; dh=540 | AVX2 | 0.7141 | 0.3985 | 0.558 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=8; dw=960; dh=540 | native | 0.3077 | 0.2749 | 0.894 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=16; dw=960; dh=540 | AVX2 | 0.9695 | 0.4205 | 0.434 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=16; dw=960; dh=540 | native | 0.6007 | 0.3111 | 0.518 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=32; dw=960; dh=540 | AVX2 | 1.8179 | 1.0907 | 0.600 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=32; dw=960; dh=540 | native | 0.9941 | 0.8075 | 0.812 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=8; dw=960; dh=540 | AVX2 | 0.7855 | 0.9116 | 1.161 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=8; dw=960; dh=540 | native | 0.5284 | 0.5185 | 0.981 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=16; dw=960; dh=540 | AVX2 | 1.0327 | 0.9180 | 0.889 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=16; dw=960; dh=540 | native | 0.6469 | 0.5880 | 0.909 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=32; dw=960; dh=540 | AVX2 | 1.5343 | 1.4348 | 0.935 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=32; dw=960; dh=540 | native | 1.7669 | 1.2496 | 0.707 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=8; dw=960; dh=540 | AVX2 | 0.8051 | 0.9061 | 1.126 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=8; dw=960; dh=540 | native | 0.5408 | 0.5126 | 0.948 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=16; dw=960; dh=540 | AVX2 | 1.0219 | 0.9250 | 0.905 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=16; dw=960; dh=540 | native | 0.6425 | 0.5866 | 0.913 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=32; dw=960; dh=540 | AVX2 | 1.5273 | 1.4361 | 0.940 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=32; dw=960; dh=540 | native | 1.8163 | 1.2508 | 0.689 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=8; dw=2880; dh=1620 | AVX2 | 2.4124 | 0.8597 | 0.356 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=8; dw=2880; dh=1620 | native | 0.8016 | 0.4646 | 0.580 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=16; dw=2880; dh=1620 | AVX2 | 2.9271 | 1.1568 | 0.395 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=16; dw=2880; dh=1620 | native | 1.4000 | 0.8188 | 0.585 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=32; dw=2880; dh=1620 | AVX2 | 5.3231 | 2.1471 | 0.403 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=triangle; bits=32; dw=2880; dh=1620 | native | 2.6377 | 1.8594 | 0.705 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=8; dw=2880; dh=1620 | AVX2 | 2.8720 | 2.2689 | 0.790 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=8; dw=2880; dh=1620 | native | 1.5967 | 1.3412 | 0.840 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=16; dw=2880; dh=1620 | AVX2 | 3.5145 | 2.2374 | 0.637 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=16; dw=2880; dh=1620 | native | 2.3904 | 1.3097 | 0.548 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=32; dw=2880; dh=1620 | AVX2 | 3.6303 | 3.5577 | 0.980 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=lanczos3; bits=32; dw=2880; dh=1620 | native | 3.1588 | 3.0409 | 0.963 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=8; dw=2880; dh=1620 | AVX2 | 2.8370 | 2.2884 | 0.807 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=8; dw=2880; dh=1620 | native | 1.5859 | 1.3469 | 0.849 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=16; dw=2880; dh=1620 | AVX2 | 3.4507 | 2.1974 | 0.637 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=16; dw=2880; dh=1620 | native | 2.4033 | 1.3213 | 0.550 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=32; dw=2880; dh=1620 | AVX2 | 3.5226 | 3.6400 | 1.033 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=HV; filter=spline36; bits=32; dw=2880; dh=1620 | native | 3.0187 | 3.1152 | 1.032 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=triangle; bits=8; dw=960; dh=1620 | AVX2 | 1.3238 | 0.6108 | 0.461 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=triangle; bits=8; dw=960; dh=1620 | native | 0.5484 | 0.4189 | 0.764 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=triangle; bits=16; dw=960; dh=1620 | AVX2 | 1.6185 | 0.8429 | 0.521 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=triangle; bits=16; dw=960; dh=1620 | native | 0.7355 | 0.6890 | 0.937 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=triangle; bits=32; dw=960; dh=1620 | AVX2 | 3.1450 | 1.9482 | 0.619 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=triangle; bits=32; dw=960; dh=1620 | native | 2.2654 | 1.5560 | 0.687 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=lanczos3; bits=8; dw=960; dh=1620 | AVX2 | 1.5331 | 1.7785 | 1.160 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=lanczos3; bits=8; dw=960; dh=1620 | native | 1.0821 | 1.0334 | 0.955 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=lanczos3; bits=16; dw=960; dh=1620 | AVX2 | 1.9065 | 1.9062 | 1.000 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=lanczos3; bits=16; dw=960; dh=1620 | native | 1.2116 | 1.2691 | 1.047 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=lanczos3; bits=32; dw=960; dh=1620 | AVX2 | 2.8961 | 2.7348 | 0.944 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=lanczos3; bits=32; dw=960; dh=1620 | native | 3.6394 | 2.4399 | 0.670 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=spline36; bits=8; dw=960; dh=1620 | AVX2 | 1.5171 | 1.8059 | 1.190 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=spline36; bits=8; dw=960; dh=1620 | native | 1.0804 | 1.0348 | 0.958 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=spline36; bits=16; dw=960; dh=1620 | AVX2 | 1.9092 | 2.0742 | 1.086 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=spline36; bits=16; dw=960; dh=1620 | native | 1.1936 | 1.2406 | 1.039 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=spline36; bits=32; dw=960; dh=1620 | AVX2 | 2.8742 | 2.6916 | 0.936 | AVX2 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |
| order=VH; filter=spline36; bits=32; dw=960; dh=1620 | native | 3.6864 | 2.4285 | 0.659 | AVX512 | AMD current-code refresh, three rounds; exact C output. Existing upstream baseline retained. |

</details>

## Supplementary long-support measurements

Current module timings for these full-filter profiles are compared with their saved upstream baselines. Refreshed horizontal paths use the same five-sample harness and three module observations per target; unchanged vertical paths retain their measurements. Input is 1920×1080; H output is 960×1080, V is 1920×540, HV is 960×540. The profile number is taps; SincLin2 defaults to 15. Y32 denotes F32. All new outputs match C; upstream F32 can differ. These profiles overlap the main table, and independent measurement sessions need not yield identical times.

| Profile | CPU | Upstream ms | New ms | New / upstream |
|---|---|---|---|---|
| Y8-SincLin2Resize-3-H | avx2 | 0.6314 | 0.6995 | 1.108 |
| Y8-SincLin2Resize-3-H | native | 0.3725 | 0.3412 | 0.916 |
| Y8-SincLin2Resize-3-V | avx2 | 0.2932 | 0.3968 | 1.353 |
| Y8-SincLin2Resize-3-V | native | 0.4309 | 0.3197 | 0.742 |
| Y8-SincLin2Resize-3-HV | avx2 | 0.7660 | 0.9007 | 1.176 |
| Y8-SincLin2Resize-3-HV | native | 0.5823 | 0.5281 | 0.907 |
| Y8-SincLin2Resize-8-H | avx2 | 0.9747 | 1.0162 | 1.043 |
| Y8-SincLin2Resize-8-H | native | 0.8465 | 0.7002 | 0.827 |
| Y8-SincLin2Resize-8-V | avx2 | 0.6917 | 0.9637 | 1.393 |
| Y8-SincLin2Resize-8-V | native | 1.0854 | 0.7654 | 0.705 |
| Y8-SincLin2Resize-8-HV | avx2 | 1.3233 | 1.4869 | 1.124 |
| Y8-SincLin2Resize-8-HV | native | 1.4145 | 1.2390 | 0.876 |
| Y8-SincLin2Resize-15-H | avx2 | 1.5956 | 1.6728 | 1.048 |
| Y8-SincLin2Resize-15-H | native | 1.5523 | 1.2118 | 0.781 |
| Y8-SincLin2Resize-15-V | avx2 | 1.2961 | 1.7166 | 1.324 |
| Y8-SincLin2Resize-15-V | native | 2.0214 | 1.4062 | 0.696 |
| Y8-SincLin2Resize-15-HV | avx2 | 2.2622 | 2.5534 | 1.129 |
| Y8-SincLin2Resize-15-HV | native | 2.5623 | 1.9952 | 0.779 |
| Y8-LanczosResize-3-H | avx2 | 0.6208 | 0.7030 | 1.132 |
| Y8-LanczosResize-3-H | native | 0.3649 | 0.3416 | 0.936 |
| Y8-LanczosResize-3-V | avx2 | 0.2944 | 0.3941 | 1.339 |
| Y8-LanczosResize-3-V | native | 0.4250 | 0.3162 | 0.744 |
| Y8-LanczosResize-3-HV | avx2 | 0.8326 | 0.8955 | 1.075 |
| Y8-LanczosResize-3-HV | native | 0.5821 | 0.5197 | 0.893 |
| Y8-LanczosResize-8-H | avx2 | 0.9732 | 1.0199 | 1.048 |
| Y8-LanczosResize-8-H | native | 0.8645 | 0.6943 | 0.803 |
| Y8-LanczosResize-8-V | avx2 | 0.7154 | 0.9445 | 1.320 |
| Y8-LanczosResize-8-V | native | 1.0852 | 0.7740 | 0.713 |
| Y8-LanczosResize-8-HV | avx2 | 1.3237 | 1.5328 | 1.158 |
| Y8-LanczosResize-8-HV | native | 1.3876 | 1.1381 | 0.820 |
| Y8-LanczosResize-15-H | avx2 | 1.6049 | 1.6754 | 1.044 |
| Y8-LanczosResize-15-H | native | 1.5330 | 1.2216 | 0.797 |
| Y8-LanczosResize-15-V | avx2 | 1.2662 | 1.7102 | 1.351 |
| Y8-LanczosResize-15-V | native | 2.0136 | 1.3977 | 0.694 |
| Y8-LanczosResize-15-HV | avx2 | 2.2671 | 2.5482 | 1.124 |
| Y8-LanczosResize-15-HV | native | 2.5606 | 1.9941 | 0.779 |
| Y16-SincLin2Resize-3-H | avx2 | 0.7194 | 0.7158 | 0.995 |
| Y16-SincLin2Resize-3-H | native | 0.4230 | 0.4427 | 1.047 |
| Y16-SincLin2Resize-3-V | avx2 | 0.3549 | 0.3494 | 0.985 |
| Y16-SincLin2Resize-3-V | native | 0.4028 | 0.2571 | 0.638 |
| Y16-SincLin2Resize-3-HV | avx2 | 0.9732 | 0.9053 | 0.930 |
| Y16-SincLin2Resize-3-HV | native | 0.6942 | 0.5999 | 0.864 |
| Y16-SincLin2Resize-8-H | avx2 | 1.0374 | 1.1109 | 1.071 |
| Y16-SincLin2Resize-8-H | native | 1.0576 | 0.9516 | 0.900 |
| Y16-SincLin2Resize-8-V | avx2 | 0.8906 | 0.8954 | 1.005 |
| Y16-SincLin2Resize-8-V | native | 1.0850 | 0.6545 | 0.603 |
| Y16-SincLin2Resize-8-HV | avx2 | 1.7066 | 1.5694 | 0.920 |
| Y16-SincLin2Resize-8-HV | native | 1.7154 | 1.3357 | 0.779 |
| Y16-SincLin2Resize-15-H | avx2 | 1.8558 | 1.8472 | 0.995 |
| Y16-SincLin2Resize-15-H | native | 1.8629 | 1.5906 | 0.854 |
| Y16-SincLin2Resize-15-V | avx2 | 1.7412 | 1.5927 | 0.915 |
| Y16-SincLin2Resize-15-V | native | 1.8987 | 1.1229 | 0.591 |
| Y16-SincLin2Resize-15-HV | avx2 | 3.1242 | 2.6588 | 0.851 |
| Y16-SincLin2Resize-15-HV | native | 2.9068 | 2.2580 | 0.777 |
| Y16-LanczosResize-3-H | avx2 | 0.7074 | 0.7128 | 1.008 |
| Y16-LanczosResize-3-H | native | 0.4121 | 0.4426 | 1.074 |
| Y16-LanczosResize-3-V | avx2 | 0.3526 | 0.3533 | 1.002 |
| Y16-LanczosResize-3-V | native | 0.4036 | 0.2604 | 0.645 |
| Y16-LanczosResize-3-HV | avx2 | 0.9781 | 0.9219 | 0.943 |
| Y16-LanczosResize-3-HV | native | 0.6879 | 0.6010 | 0.874 |
| Y16-LanczosResize-8-H | avx2 | 1.0487 | 1.1054 | 1.054 |
| Y16-LanczosResize-8-H | native | 1.0621 | 0.9411 | 0.886 |
| Y16-LanczosResize-8-V | avx2 | 0.8713 | 0.8959 | 1.028 |
| Y16-LanczosResize-8-V | native | 1.0199 | 0.6284 | 0.616 |
| Y16-LanczosResize-8-HV | avx2 | 1.6875 | 1.5717 | 0.931 |
| Y16-LanczosResize-8-HV | native | 1.6833 | 1.3035 | 0.774 |
| Y16-LanczosResize-15-H | avx2 | 1.8398 | 1.8472 | 1.004 |
| Y16-LanczosResize-15-H | native | 1.8073 | 1.5868 | 0.878 |
| Y16-LanczosResize-15-V | avx2 | 1.7906 | 1.6314 | 0.911 |
| Y16-LanczosResize-15-V | native | 1.8915 | 1.1199 | 0.592 |
| Y16-LanczosResize-15-HV | avx2 | 3.1242 | 2.6449 | 0.847 |
| Y16-LanczosResize-15-HV | native | 2.9314 | 2.2737 | 0.776 |
| Y32-SincLin2Resize-3-H | avx2 | 1.0542 | 0.9740 | 0.924 |
| Y32-SincLin2Resize-3-H | native | 1.0973 | 0.8799 | 0.802 |
| Y32-SincLin2Resize-3-V | avx2 | 0.5230 | 0.5735 | 1.096 |
| Y32-SincLin2Resize-3-V | native | 0.5013 | 0.5503 | 1.098 |
| Y32-SincLin2Resize-3-HV | avx2 | 1.4804 | 1.5342 | 1.036 |
| Y32-SincLin2Resize-3-HV | native | 1.6985 | 1.3773 | 0.811 |
| Y32-SincLin2Resize-8-H | avx2 | 1.9886 | 2.4720 | 1.243 |
| Y32-SincLin2Resize-8-H | native | 1.7294 | 1.9118 | 1.105 |
| Y32-SincLin2Resize-8-V | avx2 | 1.2897 | 1.3152 | 1.020 |
| Y32-SincLin2Resize-8-V | native | 1.1928 | 1.2278 | 1.029 |
| Y32-SincLin2Resize-8-HV | avx2 | 2.8924 | 3.4600 | 1.196 |
| Y32-SincLin2Resize-8-HV | native | 2.5817 | 2.8974 | 1.122 |
| Y32-SincLin2Resize-15-H | avx2 | 3.9519 | 5.2141 | 1.319 |
| Y32-SincLin2Resize-15-H | native | 3.7489 | 3.3312 | 0.889 |
| Y32-SincLin2Resize-15-V | avx2 | 2.2265 | 2.3243 | 1.044 |
| Y32-SincLin2Resize-15-V | native | 2.2204 | 2.2902 | 1.031 |
| Y32-SincLin2Resize-15-HV | avx2 | 5.4043 | 6.5225 | 1.207 |
| Y32-SincLin2Resize-15-HV | native | 5.1479 | 4.7154 | 0.916 |
| Y32-LanczosResize-3-H | avx2 | 1.0638 | 1.0358 | 0.974 |
| Y32-LanczosResize-3-H | native | 1.1925 | 0.8531 | 0.715 |
| Y32-LanczosResize-3-V | avx2 | 0.4882 | 0.5296 | 1.085 |
| Y32-LanczosResize-3-V | native | 0.5137 | 0.5431 | 1.057 |
| Y32-LanczosResize-3-HV | avx2 | 1.4770 | 1.4950 | 1.012 |
| Y32-LanczosResize-3-HV | native | 1.7757 | 1.3108 | 0.738 |
| Y32-LanczosResize-8-H | avx2 | 1.9824 | 2.5323 | 1.277 |
| Y32-LanczosResize-8-H | native | 1.7725 | 1.8957 | 1.070 |
| Y32-LanczosResize-8-V | avx2 | 1.2387 | 1.2116 | 0.978 |
| Y32-LanczosResize-8-V | native | 1.2592 | 1.3951 | 1.108 |
| Y32-LanczosResize-8-HV | avx2 | 2.8526 | 3.4327 | 1.203 |
| Y32-LanczosResize-8-HV | native | 2.6176 | 2.7807 | 1.062 |
| Y32-LanczosResize-15-H | avx2 | 3.9562 | 5.1941 | 1.313 |
| Y32-LanczosResize-15-H | native | 3.7687 | 3.4300 | 0.910 |
| Y32-LanczosResize-15-V | avx2 | 2.3319 | 2.2582 | 0.968 |
| Y32-LanczosResize-15-V | native | 2.2223 | 2.3269 | 1.047 |
| Y32-LanczosResize-15-HV | avx2 | 5.4205 | 6.6340 | 1.224 |
| Y32-LanczosResize-15-HV | native | 4.9911 | 4.6749 | 0.937 |
