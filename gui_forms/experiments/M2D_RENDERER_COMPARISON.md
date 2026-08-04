# M2d renderer comparison and replaceability evidence

Date: 2026-08-04

Status: **MEASURED evidence for the bounded M2d proving slice**. The Skia CPU
adapter and the CoreGraphics CPU reference lane are still **CANDIDATE** renderer
implementations. Gate R1 remains open; this record is not a renderer ADR.

## Scope and evidence labels

- **GIVEN**: rendering remains CPU-only; renderer APIs stay private; the core and
  public headers remain renderer-free; AppKit remains the macOS host; PNG remains
  the only admitted image resource; retained display chunks remain authoritative.
- **OBSERVED**: both private adapters consume the same `Painter` display-chunk
  vocabulary and the same validated `ImageRegistry`. The CoreGraphics lane adds
  no renderer type to a public header.
- **MEASURED**: the Release benchmark ran the Gallery at 1%, 10%, and 100%
  damage at 1x, 2x, and 3x with 24 warmups and 240 samples per case, plus 300
  scheduled 30 Hz bitmap-band frames at 2x.
- **CANDIDATE**: CoreGraphics is a credible lower-linked-surface macOS reference
  lane. It is not a cross-platform recommendation and is not selected.

## Implementation delivered

- `src/render/coregraphics/coregraphics_raster.*`: private CPU RGBA8 adapter for
  rectangle, line, text, PNG, clipping, translation, and damage-clipped frames.
  PNG bytes arrive only through the bounded registry and are eagerly decoded.
- `benchmarks/renderer_benchmark.cpp`: identical retained Gallery and active-band
  workloads; structured p50/p95/p99/worst/mean timing, CPU time, RSS snapshots,
  exact copied/damaged pixels, chunk metrics, checksums, and artifacts.
- `src/core/device_damage.hpp`: one internal device-pixel alignment contract used
  by the macOS host, comparison raster, and benchmark. Fractional logical damage
  is expanded outward before both tree traversal and raster clipping.
- `cmake/check_renderer_boundaries.cmake`: build-enforced core/public-header
  symbol boundary, adapter isolation, dependency denylist, and linked-size audit.
- CoreGraphics smoke and fractional-damage regression coverage.

## Named measurement

Environment record: Apple M3 `Mac15,5`, 8 GiB, Darwin 23.6.0, Apple LLVM
16.0.0, AC power without independent pinning, Release build, sRGB premultiplied
RGBA8. The Gallery process was also running, so these numbers are comparison
evidence, not an approved performance budget.

Representative 2x render latency from the final r4 record:

| Workload | Skia p50 / p95 | CoreGraphics p50 / p95 |
|---|---:|---:|
| 1% Gallery damage | 0.011 / 0.013 ms | 0.044 / 0.057 ms |
| 10% Gallery damage | 0.068 / 0.075 ms | 0.135 / 0.138 ms |
| 100% Gallery damage | 0.913 / 0.934 ms | 1.909 / 1.936 ms |
| 900x10 30 Hz band | 0.025 / 0.028 ms | 0.014 / 0.015 ms |

The band emitted exactly 300 scheduler wakes and 300 active-surface ticks per
renderer, rebuilt 300 band chunks, repainted zero surrounding controls, damaged
10,800,000 physical pixels in total, and missed zero 33.333 ms frame budgets.

All 18 Gallery damage cases were stable across a repeated final paint and were
byte-identical to a clean full-repaint reference for the same renderer and scale:
zero differing pixels and zero maximum channel delta. Checksums intentionally
differ between renderers because their text rasterization and pixel output are
not specified as cross-renderer identical.

The recorded `damage_copy_ns` value is a CPU row-copy presentation surrogate.
It is not AppKit compositor latency. The recorded RSS is in-process and
order-dependent. The first-frame values are likewise in-process and order-biased:
Skia ran first and CoreGraphics second. These fields are retained so they cannot
be mistaken for cold-launch or independent steady-state evidence.

## Linked surface and replacement boundary

The Release audit passed with:

- renderer-free core archive: 241,808 bytes;
- private Skia adapter archive: 34,360 bytes;
- pinned Skia archive: 5,758,736 bytes;
- private CoreGraphics adapter archive: 30,384 bytes;
- combined benchmark executable: 3,393,296 bytes.

The combined comparison binary links CoreFoundation, CoreGraphics, CoreText,
ImageIO, libc++, and libSystem. It links no Metal, MetalKit, OpenGL, or WebKit.
The renderer-free core exports no Skia, CoreGraphics, CoreText, AppKit, Metal, or
OpenGL symbol. Public headers expose none of those renderer/platform tokens.

Line count is only a review-surface proxy, not a complexity score: the Skia
adapter is 391 lines and the CoreGraphics reference adapter is 512 lines.

## Visual inspection

Both final 2x artifacts are upright, complete, and preserve the professional
Windows 7/10-inspired Gallery direction. Portsmouth Rapids is used for titles
and controls; Lucida Grande remains the content/field fallback. CoreGraphics and
Skia have visibly different glyph metrics and antialiasing, but neither final
artifact shows missing text, inverted coordinates, damaged-image seams, or an
incorrectly oriented PNG. No objective shaping or text-quality corpus exists yet.

Artifacts:

- `results/m2d-renderer-2026-08-04-r4/renderer-results.jsonl`, SHA-256
  `5df4510dee051305a154863e449a4bd28bf1f6b9742af9d6b6ed90e2ee28d739`;
- `results/m2d-renderer-2026-08-04-r4/renderer-boundary-audit.txt`, SHA-256
  `394095393e6a33ba77f3c9f17f4a18e6d3a9c9e9a5330020ad14e850daba1b7e`;
- final Skia PNG, SHA-256
  `f45f070e497537a856653709390e206e3ccd450b9cbce80e0cc7a1b5055e902a`;
- final CoreGraphics PNG, SHA-256
  `dcb5548345a52453e39eae52d13ce2148ccfbf94bc91053a31e29e3722bd2411`.

## Negative results retained

- Initial run: the CoreGraphics artifact was vertically inverted, text was
  absent, and partial repaint checksums diverged. The raw directory is retained.
- r2: the surface orientation was corrected, but the CoreText baseline was still
  outside the visible surface and fractional partial damage still diverged.
- r3: text was restored and raster clipping was rounded, but 1% damage still
  differed from a clean full repaint by exactly one physical row: 900, 1,800,
  and 2,700 pixels at 1x, 2x, and 3x. The retained-tree clip still used the
  fractional logical rectangle.
- r4: host/tree and raster now share outward device-pixel alignment; all damage
  equivalence checks pass.
- Full ASan+UBSan instrumentation failed to link the no-RTTI pinned Skia archive
  because UBSan vptr checks require Skia RTTI symbols. ASan was run on the full
  build; UBSan was run separately on the renderer-free core and CoreGraphics
  surface. This is a sanitizer/toolchain limitation, not a passing product claim.

## Verification

- Normal Debug build: 17/17 tests passed, including both renderer smokes,
  native close, font/archive policy, and renderer boundary audit.
- Release strict diagnostics (`-Wall -Wextra -Wpedantic -Werror`): 17/17 passed.
  The pinned Skia include root is marked `SYSTEM`; all GUI.Forms sources remain
  first-party warning targets.
- Full AddressSanitizer build: 17/17 passed.
- Renderer-free core plus CoreGraphics UndefinedBehaviorSanitizer build: 11/11
  passed with Skia, the AppKit host, Gallery, and benchmarks disabled.
- Canonical lifecycle trace test remains passing; M2d does not change its retained
  event contract.
- The Gallery demonstration remains running from the normal build.

## Gate R1 remains open

M2d supplies the required chunk-native measurements, exact 1/10/100% damage
records, 30 Hz bitmap-band record, symbol/dependency audit, and one credible
comparison lane. It does not justify a renderer ADR. Gate R1 still lacks:

- isolated out-of-process cold/warm launch and independent RSS;
- allocator instrumentation and actual AppKit present/compositor latency;
- repeated quiet-system trials with an approved budget and variance policy;
- identical dedicated text-heavy, large-PNG, vector-icon, and translucent-
  backplane fixtures;
- objective text/shaping quality and cross-platform comparison evidence;
- owner approval in a numbered decision record.

Skia therefore remains replaceable, CoreGraphics remains a reference candidate,
and new behavior continues to target the renderer-neutral vocabulary.
