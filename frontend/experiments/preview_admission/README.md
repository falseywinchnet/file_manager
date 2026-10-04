# Consumer image admission laboratory

This standalone installed-Core consumer measures the synchronous, owning-thread
`Window::load_png` and `Window::load_bgra32_premultiplied` calls. It changes no
GUI.Forms implementation, application preview format or runtime contract. It
does not attach a native renderer or measure decoding, painting, transport, RSS,
selection latency or complete preview latency.

## Workload and measurement

Generated opaque RGBA `(12,226,198,255)` controls have width 1024 and heights 64,
256 and 1024. Raw BGRA uses the corresponding channel order. Each PNG has zero
row filters, with either zlib level 0 (stored) or level 6 (deflated) compression.
The highly compressible solid image is an explicit control, not a photo corpus.
The probe validates PNG structure/dimensions; same-pixel content follows from
the generator and raw initializer, not an independent runtime PNG decode.

| Height | Raw BGRA bytes | Stored PNG bytes | Deflated PNG bytes |
|---:|---:|---:|---:|
| 64 | 262,144 | 262,296 | 492 |
| 256 | 1,048,576 | 1,048,980 | 1,692 |
| 1024 | 4,194,304 | 4,195,716 | 6,504 |

All inputs/sample arrays are prepared before collection. Each path is warmed.
Sixty-one iterations record all three paths, rotating order: raw/store/deflate
21 times, store/deflate/raw 20 times, and deflate/raw/store 20 times. A second
process reverses geometry order. This reduces some order bias but does not cover
all permutations or produce independent statistical trials. Complete samples
and nearest-rank p50/p95/p99 are printed; p99 is the maximum of 61 observations.
There is no timing threshold assertion.

Timers include allocation, validation, copy/hash and other work inside the
public admission call. They exclude source reads, fixture generation, warmup,
retirement, assertions, reporting and sorting. Both paths require retirement
to restore zero registry accounting after each sample. The raw ownership test
also changes caller bytes after admission, checks a distinct retained buffer
with its original first byte, removes it and rejects its stale ID. Borrowed
views end before removal. These are logical ownership/accounting checks, not
allocator decommit, backend retirement or physical-memory measurements.

## Running

Use a fresh fixture directory; generation refuses to overwrite existing files.
The probe links only the public installed `GUIForms::Core` target.

```text
python frontend/experiments/preview_admission/fixture.py frontend/.build/admission-data
cmake -S frontend/experiments/preview_admission -B frontend/.build/admission -G Ninja -DCMAKE_BUILD_TYPE=Release -DGUIForms_DIR=PATH_TO_SDK/lib/cmake/GUIForms
cmake --build frontend/.build/admission --parallel 1
frontend/.build/admission/preview_admission frontend/.build/admission-data
frontend/.build/admission/preview_admission frontend/.build/admission-data --reverse
```

Append `.exe` on Windows. Native CI uses each platform's normal exported SDK
before enabling unrelated development renderer options, and retains both logs.

## Shadow Windows observations

**MEASURED:** GCC 16.2.0, C++20, Release `-O3 -DNDEBUG`, Shadow Windows x64,
installed `gui_forms/.build/shadow-sdk` Core. Provider archive/source hashes and
raw measurements are in `../../results/2026-10-04-preview-admission/`.
The final forward/reverse medians, in milliseconds:

| Height | Raw BGRA p50 | Stored PNG p50 | Deflated PNG p50 |
|---:|---:|---:|---:|
| 64 | 0.2962 / 0.3925 | 0.8634 / 0.9323 | 0.0021 / 0.0023 |
| 256 | 1.3741 / 1.3520 | 3.6195 / 3.5595 | 0.0072 / 0.0069 |
| 1024 | 5.4200 / 5.4479 | 14.3518 / 14.3166 | 0.0248 / 0.0245 |

The compressed control reverses any claim that raw admission is universally
cheaper. Encoded extent materially affects PNG admission; the compressed path
defers decoding to a later renderer stage excluded here. At 4 MiB, raw admission
still takes roughly 5.4 ms in this local workload. No new provider, transport or
image ownership strategy is selected by these observations. Next measurement
must include actual renderer synchronization/first paint, replacement overlap
and resource retirement before end-to-end or residency conclusions.

Initial uncompressed-only runs are retained as exploratory evidence, including
their slower tails. The final three-way controls supersede that narrow comparison.
Initial compilation mistakes (implicit StableId and a Panel outside the Core
package) were corrected to an explicit StableId and plain retained Control.

The full programming house style was independently reviewed for all authored
C++, Python, CMake and workflow additions. That review does not certify the
unchanged GUI.Forms implementation or vendor dependencies. Native platform
measurements beyond this Windows run remain pending.
