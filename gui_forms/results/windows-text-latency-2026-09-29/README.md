# Windows retained text latency — 2026-09-29

**GIVEN:** File Manager's interface is unacceptably slow. This investigation
measures the Windows painter actually consumed by File Manager.

## Reproduced bottleneck

**MEASURED:** Shadow Windows x64, GCC 16.2.0 Release, GDI CPU renderer, shipped
fonts, 1340×850 DIB at 1× scale. Each frame resolves and draws 100 distinct
ordinary File Manager captions using content font 13. Four passes of five
frames include native GDI completion. No filesystem work or desktop automation
is involved. Exact outputs are beside this record.

| Implementation | First five-frame average | Later five-frame averages |
|---|---:|---:|
| Baseline | 3578.63 ms/frame | 3595.83–3760.28 ms/frame |
| Bounded font handles and text-run cache | 520.852 ms/frame | 168.265–226.219 ms/frame |
| Also retain Uniscribe state per native font | 20.2375 ms/frame | 8.459–8.891 ms/frame |

The final warm workload is over 400× faster than the baseline. This is a
specified renderer workload, not a claim that every File Manager operation is
400× faster. The first pass is an average containing cold work, not an isolated
first-frame measurement.

**OBSERVED cause:** every measure/draw previously created/deleted native fonts,
checked fallback coverage per grapheme, and discarded Uniscribe font state.
Even unchanged retained labels paid the same expensive setup repeatedly.

## Change and limits

The painter owns at most 64 cached native fonts, each with its own Uniscribe
cache. Additional fonts use the existing immediate ownership path. Fonts remain
selected only during the corresponding operation; native handles and shaping
state are released at painter destruction. Font keys include pixel height,
weight, italic and requested family. Text runs key the full FontSpec and text;
DPI changes invalidate them. At most 512 texts of at most 2048 UTF-8 bytes each
are retained; overflow clears the run cache. Longer text takes the uncached
coverage route. Nothing caches file contents or disables accessibility.

The portable retained API, glyph/fallback selection, shaping algorithm, damage
model and visual design are unchanged. This repair is private Windows backend
work; macOS/Linux are not claimed as measured here.

## Verification

**MEASURED:** all 64 GUI.Forms CTest suites passed in 6.89 seconds.
Windows text tests compare cached and fresh-renderer pixels across 15 multilingual
captions, three scales and two weights, including Arabic, Hebrew, Indic scripts,
CJK and Cyrillic. Additional tests exercise 520 unique labels, more than 64 font
sizes, repeated DPI changes, metric agreement and native GDI handle cleanup.

Reproduce after configuring the Windows toolkit build:

```powershell
cmake --build gui_forms/.build/shadow-windows --target gui_forms_windows_text_tests --parallel 2
& gui_forms/.build/shadow-windows/gui_forms_windows_text_tests.exe gui_forms/assets/fonts --benchmark
```

Full File Manager model/native-window measurements are recorded separately in
`frontend/results/2026-09-29-shadow-windows/LATENCY.md`; they distinguish this
renderer bottleneck from directory observation and application state updates.

## Full File Manager follow-up: pixel work

The text-only improvement was not sufficient: the actual 1340×850 File Manager
window at the host's 2× DPI scale still took roughly 0.4 seconds per full repaint.
Opt-in primitive timing (`GUI_FORMS_PROFILE_PAINTER=1`) identified solid fills,
rounded outlines and repeated gradient sampling as the next costs. Across five
frames, the initial profile spent 497 ms in solid fills, 361 ms in linear
gradients, and 182–200 ms in radial gradients. See `native-profile.txt`.

The renderer now computes convex rounded-rectangle spans per scanline. Opaque
interiors use contiguous fills, and borders subtract the inner interval rather
than testing every interior pixel. Vertical gradients compute one color per
row, horizontal gradients one per column. Non-axis linear and radial brushes
retain exact sampled colors in a FIFO cache bounded to 32 entries and 16 MiB
of pixel storage per painter. Keys include geometry, scale, stops and spread;
current clipping and destination alpha composition are applied on every replay.
Oversized brushes retain the scalar path. This caches material colors, not a
window screenshot or a stale background.

**MEASURED:** final non-profiling File Manager native runs show ready in
187–229 ms, the first presented frame observed in 350–400 ms, steady full-window
render/present in 28.794–32.951 ms, and steady toolbar-control repaint in
0.410–0.699 ms. Full repaint improved from approximately 1.2 seconds. The
1,080-logical-pixel partial samples follow initial pending startup damage;
that larger sample is retained and explained in the frontend record.
The full-window workload renders 2680×1700 physical pixels. These observations
are not a claim of universal input latency or 60 Hz full-window rendering.

A scalar reference test compares every color channel against the former
per-pixel shape definitions, for solid/rounded fills, rounded strokes, three
linear-gradient directions and radial gradients. It includes fractional
translation and clipping, rounded clipping, opaque/translucent colors, changed
gradient stops, cache reuse, and 1×/1.5×/2× scales. This accompanies the unchanged
canvas sampling and multilingual-text suites.

Final combined verification after the pixel/material changes: **64/64 suites
passed in 6.99 seconds** (`performance-final-ctest.txt`). The reference workload
renders 336 small images, including both cold and cached cases and changed
material colors. No performance threshold is smuggled into correctness tests.

The final staged DLL was rerun after the last build: native ready 218–270 ms,
first observed presentation 393–444 ms, steady full redraw 26.638–29.243 ms,
steady toolbar redraw 0.325–0.522 ms. All 11 frontend suites passed in 2.31 s.
Those exact-deliverable results supersede the intermediate candidate ranges
above; full raw logs and hashes are in the frontend latency record.
