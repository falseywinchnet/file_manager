# Prepared-row shaping comparison

Status: **MEASURED Windows-only private shaping experiment; no consumer activation**.
Date: 2026-10-03. Implementation baseline `6e224d0` (PR #22), subsequently
rebase-merged as `0f77d9f` with the same tree
`196bd8fe3527088018b559f199ea0d7a088a1584` after both native matrices passed.

## Workload and controls

The new `tests/prepared_window_shape_bench.cpp` compares the existing bounded
shaper against the reusable workspace, with exact geometry/coverage checks
before timing. The nominal profile is content font 13, scale 1, width 190;
Carlito, Noto Arabic, Noto Hebrew and Noto Emoji are registered from the pinned
GUI.Forms fixtures. This is not File Manager's actual monospace preview profile.

Seven rows include Latin ligatures, combining marks, Arabic and Hebrew mixed
with digits/Latin, an empty row, emoji, and a literal `[U+0000]` label. The larger
scene repeats those categories with row numbers: 512 rows, 7,441 display bytes
including conceptual LF separators. The label is not certified source/control
projection. Correctness mode separately exercises 512 empty rows; it does not
exercise a window controller or establish source-EOF semantics.

Each scene ran in one process: ten discarded warmup pairs and 101 measured
pairs for A/A (legacy/legacy), followed by the same counts for A/B
(legacy/workspace). Pair order alternates. A/A controls expose ordinary process
noise and drift; this is not repeated-process or cross-platform performance
evidence. Source/test/compiler activity in the collaborating threads was held
during measurement. This remains an ordinary shared Shadow desktop.

The Windows QPC clock reported 10 MHz and a minimum observed interval of 0.1 us
in 10,000 calibration reads. Release uses GCC 16.2, `-O3 -DNDEBUG`, HarfBuzz text,
prepared-text development support and native layout diagnostics ON. The machine
reports AMD EPYC 9354, four virtual cores/eight logical processors. Raw console
output preserves registration/preparation times separately. Font-file IO,
native C allocations, allocator overhead, certified input/batch metadata and UI
paint are excluded. A non-diagnostic build still needs its own measurements.

## Results

These are full-cycle durations, including result retirement in both arms.
Nearest-rank percentiles are recomputed from preserved CSV, not fitted estimates.

| Scene / arm | p50 ms | p95 ms | p99 ms | max ms |
|---|---:|---:|---:|---:|
| Seven rows, A/B legacy | 4.1419 | 4.7965 | 5.1431 | 5.2634 |
| Seven rows, A/B workspace | 0.6817 | 0.9613 | 1.0574 | 1.1431 |
| 512 rows, A/B legacy | 250.0924 | 285.4216 | 307.8072 | 326.9506 |
| 512 rows, A/B workspace | 54.9201 | 70.5001 | 78.7275 | 79.7380 |

A/A full-cycle medians were 4.3970/4.3700 ms for seven rows and
263.6409/264.0368 ms for 512 rows. Tails remain noisy: the 512-row A/A maxima
were 347.2884/416.9639 ms. The experiment supports proceeding with bounded
workspace integration; it does not establish application responsiveness.
Even the improved full 512-row preparation is too long for synchronous UI work.

| Scene / arm | C++ allocation calls per cycle | Cumulative requested bytes per cycle | Peak controlled output bytes | Controlled scratch bytes |
|---|---:|---:|---:|---:|
| Seven rows, legacy | 57 | 19,276,256 | 2,752,624 | 3,692 |
| Seven rows, workspace | 19 | 2,888 | 6,984 | 5,049,144 |
| 512 rows, legacy | 4,170 | 1,410,153,680 | 2,752,624 | 4,188 |
| 512 rows, workspace | 1,390 | 273,240 | 277,336 | 5,049,144 |

The workspace is prepared once before repeated shaping; its allocation is not
part of per-cycle requested bytes. Legacy allocates ceiling-sized output on
every row and releases that row before the next. Workspace retains every row
until the window completes, and its output figure includes the 512-owner array.
These are different retention laws. `window_ready` therefore includes row
destruction only in the legacy arm; use `full_cycle` for lifetime-cost comparison.
Do not add the scratch figure to cumulative requested bytes or describe the
experiment as reduced total process memory. The workspace deliberately spends
more reusable scratch to avoid repeated ceiling-sized allocation and clearing.
Completed runs match between arms: 11 for seven rows and 1,023 for 512 rows.

## Verification, review and reproduction

Both registered correctness tests passed on Windows (0.93 s combined test
duration, not a benchmark). They compare every glyph/run/metric, retained output
and coverage, and check width before admitting timing. CSV acquisition is atomic
and exclusive; existing files and failed-run partial evidence are preserved.
Flush and close errors are checked. The correctness tests exercise duplicate
refusal and failed-run evidence retention. CTest only selects `--verify-only`.

```powershell
. ./tools/Enter-WindowsToolchain.ps1
ctest --test-dir .build/prepared-window-input -R '^gui_forms_prepared_window_shape_(preview|rows)_verify$' --output-on-failure
./.build/prepared-window-input/gui_forms_prepared_window_shape_bench.exe --fonts gui_forms/assets/fonts --scene preview-seven --measure --csv NEW-preview.csv
./.build/prepared-window-input/gui_forms_prepared_window_shape_bench.exe --fonts gui_forms/assets/fonts --scene window-512 --measure --csv NEW-window.csv
```

Root reviewed the entire new benchmark and seven CMake registration lines
against the complete house style: explicit initialized types, named behavior,
bounded counts/conversions, separate owners, stream/descriptor destruction,
failure preservation, per-row versus retained lifetimes, mode selection outside
the row kernel, and allocations outside timed storage collection. The spelling
scan reports no candidates. No remaining authored-scope violation was found;
unchanged library/tooling code is not certified. Metrics extraction is ordinary
CSV parsing; all 4,040/208,060 data rows and phase counts were checked. Compressed
raw bytes were decompressed and compared exactly; digests and aggregate metrics
are in `verified-metrics.json`.

Native macOS/Linux correctness of this new harness remains pending. Activation
still needs cancellation between bounded work units, complete aggregate storage
accounting, retained source/context lifetime, raster integration, and real
native input-to-paint measurements. This change does not open a public API.
