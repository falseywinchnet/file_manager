# Native text shaping: allocation audit and bounded diagnostic

Date: 2026-10-01. **MEASURED negative scheduling evidence**, not D3a API
implementation, native hard-quota capability or final editor acceptance.

## Environment and reproducibility

Windows 11 Home 10.0.22621, AMD EPYC 9354 (4 exposed cores/8 logical processors),
16,757,176 KiB visible memory, Balanced power plan. GCC 16.2.0 MSYS2 Rev4 from
read-only adjacent Plan Paint; CMake Release `-O3 -DNDEBUG`; existing
`gui_forms/.build/house-style-text`, HarfBuzz/FreeType enabled, Skia/native hosts
disabled. Concurrent development activity was not isolated.

HarfBuzz pin: `56feae4035bdd48f62ba2b8d8c16232d4d89b3a4`.
FreeType pin: `f4205da14867c5387cd6a329b90ee10a6df6eeff`.
Vendored SheenBidi 2.9.0 pin: `829d99918adf05761e9f35611dc13e76e6810f80`.
Private engine source unchanged, SHA-256
`a27032aebdee2916d96bc40292b5d3e7fa1e85604c8afbcc33ae395d92637e0b`.
Repository HEAD during reviewed run was
`2756829d4cbd3d55281af3e8f7c7eb8e4f5943d4` plus diagnostic changes.

| Bundled fixed font | SHA-256 |
|---|---|
| Carlito-Regular.ttf | `f6418f708baede9789daef5d458c0f53d2a888af9820e8062934e504fedc6595` |
| NotoSansArabic-Regular.ttf | `ceea25b464a656dc3b26849bab9356740401af62aedf1bfa8b7f0d9b75925b1b` |
| NotoSansHebrew-Regular.ttf | `a7fa16fffb27bedb060a0866267c29e9859aeb9c21cc33f5b3aaf6eb062eca85` |
| NotoEmoji-Regular.ttf | `415dc6290378574135b64c808dc640c1df7531973290c4970c51fdeb849cb0c5` |

The probe admits only these four files, each at most 16 MiB, and uses shared
immutable encoded storage. Their aggregate retained byte capacity is 1,314,192;
font object/cache allocations are not included. Each workload uses a fresh
engine/font registration, content role, size 16 DIP, weight 400, nonitalic.
Font file reads, registration, fixture/oracle generation and timing-vector
allocation occur before timing. First shape follows registration, **not cold
OS-cache preparation**. Thirty-one subsequent shapes are warm-engine samples.

Configure/build and existing focused correctness suite:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake -S gui_forms -B gui_forms/.build/house-style-text -G Ninja `
  -DCMAKE_BUILD_TYPE=Release -DGUI_FORMS_ENABLE_SKIA=OFF `
  -DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=ON -DGUI_FORMS_ENABLE_WINDOWS_HOST=OFF `
  -DGUI_FORMS_BUILD_GALLERY=OFF -DGUI_FORMS_BUILD_TESTS=ON `
  -DGUI_FORMS_BUILD_TEXT_LAYOUT_PROBE=ON
cmake --build gui_forms/.build/house-style-text `
  --target gui_forms_text_layout_probe gui_forms_harfbuzz_font_engine_tests --parallel 2
ctest --test-dir gui_forms/.build/house-style-text `
  -R '^gui_forms_harfbuzz_font_engine_tests$' --output-on-failure
```

The shaping suite passed **1/1 in 0.21 seconds, 0.23 seconds total**. The diagnostic
was launched hidden with redirected stdout/stderr using `Start-Process`, then
`WaitForExit(60000)`. On expiry only that newly launched probe would be stopped
and timeout recorded as incomplete/unknown, never a latency sample. Both runs
finished before the 60-second external deadline with exit zero and empty stderr.
The final harness flushes each completed case so a later timeout preserves it.
This external process deadline is not cooperative shaping cancellation.

The executable takes the absolute bundled font directory as its only argument:
`gui_forms_text_layout_probe.exe C:/Users/Shadow/file_manager/gui_forms/assets/fonts`.
Use the same external deadline when replaying. Ordinary host/runtime/SDK artifacts
were not changed or launched.

## Workloads and measured results

Inputs consist of complete repeated units, never truncated UTF-8. All are at most
16 KiB; units/input byte counts are:

- `office abc `: 5 / 372 / 1,489 repetitions, 55 / 4,092 / 16,379 bytes.
- `[BYTE FF]`: 1,820 literal label repetitions, 16,380 bytes. This probes display
  shaping cost; it is not a source-byte illegal-input projection test.
- `ABC שלום العربية 123 `: 512 repetitions, 16,384 bytes.
- `a👩‍💻b `: 1,170 repetitions, 16,380 bytes.
- `a` followed by 8,191 combining acute marks: 16,383 bytes, one grapheme.

Timing surrounds only `engine.shape(input, font)`, including its internal
temporary destruction before return, excluding destruction of the returned
`ShapedText`, font setup, external TextStore oracle and output inspection. Sorted
percentiles use floor index `(n-1)*percent/100`; with 31 samples tails are sparse.
Results below are milliseconds from the first run; both complete raw CSVs remain.

| Case | First | p50 | p95 | p99 | Worst | Runs / glyphs | Returned vector capacity bytes |
|---|---:|---:|---:|---:|---:|---:|---:|
| ASCII small | 1.5247 | 0.1418 | 0.1620 | 0.1896 | 0.2229 | 1 / 45 | 1,488 |
| ASCII 4 KiB | 1.0866 | 1.0377 | 1.1267 | 1.4724 | 1.5629 | 1 / 3,348 | 107,184 |
| ASCII 16 KiB | 4.0216 | 4.1303 | 4.8897 | 4.9029 | 7.6094 | 1 / 13,401 | 428,880 |
| Labels 16 KiB | 4.2313 | 3.9525 | 4.7142 | 5.1699 | 5.3969 | 1 / 16,380 | 524,208 |
| Mixed bidi 16 KiB | 335.842 | 328.826 | 521.287 | 801.532 | **894.725** | 3,073 / 10,752 | 540,672 |
| Emoji/fallback 16 KiB | 159.180 | 155.825 | 164.787 | 170.581 | **170.971** | 2,341 / 7,020 | 421,248 |
| One enormous grapheme | 2.0726 | 2.1079 | 2.2530 | 2.3038 | 2.3319 | 1 / 8,191 | 262,160 |

Second run after adding output flush, with unchanged shaping/timing algorithm:
mixed bidi p50 312.668 / p99 328.944 / worst 331.370 ms; emoji p50 152.886 /
p99 181.798 / worst 197.012 ms. Preserve the first run's 894.725 ms tail; the
second run does not erase it. Returned capacities and glyph/run counts matched.
The huge-grapheme case reports **one missing cluster in both runs** and is not
complete font coverage. All other tested cases reported zero missing clusters.

This falsifies treating a 16 KiB input cap alone as evidence of acceptable UI
stall for this native path. Moving the call to a worker would not by itself bound
completion or cancellation time. Executor, work subdivision and future optimizations
remain unselected. No end-to-end paint latency or speedup claim is made.

The probe validates finite metrics/positions/advances, source-range bounds and
cluster indices at whole-input grapheme boundaries. It does not require source
cluster monotonicity across visually reordered bidi runs. Repetition checks hash
glyph IDs and cluster offsets, glyph count, total width and missing-cluster count.
**This is limited repeatability evidence, not full geometry or independent shaping
parity.** It does not compare all glyph positions/advances, font IDs, ranges,
directions or extents. Any optimization needs a complete baseline comparator
outside timing before its speed result is accepted.

Raw data: `TEXT_LAYOUT_NATIVE_PROBE_2026-10-01.csv` and
`TEXT_LAYOUT_NATIVE_PROBE_REVIEWED_2026-10-01.csv`.

## Read-only allocation/face/cache audit

**OBSERVED** in `src/render/text/harfbuzz/harfbuzz_font_engine.cpp`:

- `LibraryOwner` calls `FT_Init_FreeType`. `Face` owns the FT face and a shared
  encoded-byte owner. `FT_New_Memory_Face` opens before validating font glyph
  count/scalability. Limits are per-face 64 MiB, at most 64 faces and one million
  glyphs; these are not an aggregate process-memory quota.
- Each shape constructs a whole-input TextStore, candidates, scalar scratch,
  fallback segments, bidi directions and visual segments. Those temporary
  capacities are private and not observable from the returned result here.
- Bidi directions are intersected with every fallback segment; this nested
  traversal is a **HYPOTHESIS** for a cost contributor, not measured attribution.
- Each visual run sets FT character size, creates an HB font referencing the
  face, creates an HB buffer and calls `hb_shape`. Per-run font/buffer and FT work
  are other **HYPOTHESES** pending phase/call-count evidence.
- Returned glyph vectors reserve the library-reported glyph count after shaping.
  The probe reports their capacities and the run-vector capacity, excluding
  allocator headers/rounding and all opaque library/cache/temporary memory.
- `append_run` has early returns for some face/font/buffer failures and does not
  expose an explicit complete-vs-resource-failed result. No allocation-failure
  injection was run, so this experiment does not establish D3a failure atomicity.

Pinned HarfBuzz `src/hb.hh` selects CRT malloc/calloc/realloc/free unless compiled
with its custom allocation hooks; no hook macro was found in this build's Ninja
commands. FreeType `src/base/ftsystem.c` installs malloc/realloc-backed memory
callbacks for its default library. SheenBidi `Source/Object.c` also allocates via
malloc/free. Their internal allocations, peak usage and cache behavior remain
**unknown in this diagnostic**. No generic allocator instrumentation, library
rewrite or hard-quota capability was introduced. An explicitly reported native
measured-capacity experiment remains distinct from a hard-quota guarantee.

## Exact authored review and limitations

Full `planning/PROGRAMMING_HOUSE_STYLE.md` semantic review covers only
`benchmarks/text_layout_probe.cpp` and its narrow OFF-by-default CMake option and
target. Named functions, explicit types, bounded fixed-font input, ownership
transfer of encoded bytes, invariant loop bounds, setup outside measurement,
failure states, conversions and cleanup were reviewed. Parent review corrected
source-size lookup outside the run loop and named glyph count outside its loop.
No remaining violation was identified in this authored scope. The production
engine, public Painter and vendored libraries were inspected, not rewritten or
globally certified.

Spelling scan: one file, zero findings. Strict syntax check initially failed on
an existing sign conversion in included public `Painter::measure_text_utf8`.
That unrelated production header was left unchanged; rechecking the authored
probe with `-isystem gui_forms/include` and `-Wall -Wextra -Wconversion
-Wsign-conversion -Werror` passed. CMake build passed. Final changes only hoist
invariant oracle work outside its loops, beyond timed shape calls; the target
was rebuilt and syntax checked without resampling the unchanged timed algorithm.

The parent independently read the full probe and CMake additions, reran the
one-file spelling scan with zero findings, and reran the existing shaping CTest:
1/1 passed in 0.09 seconds total. This is not native host or rendering validation.

Harness SHA-256: first run
`6cb118990d655bb7bb5f414bc3dc19368b5686567bc84659c576464ef73b9ff3`;
flushed-output second run
`567ab83478625ef79daab2b42392cbef33d277e0597eb78a2a0543747d54f628`;
final reviewed source
`41b1ebfb7b80b93dae5bb668d410cb38e0964efee236c89014b0d0b947788e26`.

Next diagnostic, separately reviewed before private production-file mutation:
bounded compile-time-off phase/call counters to separate TextStore/grapheme,
fallback coverage, bidi/intersection and FT/HB per-run costs. Preserve these
uninstrumented baseline files. No public prepared-layout API, host/input change,
SDK installation or scheduling architecture is authorized by this receipt.

Parent independently reread the full diagnostic and narrow CMake scope, including
ownership, named execution, bounded inputs, timing exclusions and the invariant
hoisting corrections. Its independent HarfBuzz CTest rerun passed 1/1 in 0.09
seconds total; probe spelling scan again reported zero findings. Parent review
retains both runs and does not promote their limited geometry checks to full
optimization parity.
