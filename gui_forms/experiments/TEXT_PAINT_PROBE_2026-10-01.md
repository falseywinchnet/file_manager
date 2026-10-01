# Private prepared-glyph paint proof — 2026-10-01

## Scope and status

**OBSERVED:** after worker lifecycle integration `dcadfe2`, the coordinating
chat authorized a private paint-side ownership/position-transfer fixture.
New files are `src/render/text/paint_probe/paint_probe.hpp`, its `.cpp`, and
`benchmarks/text_paint_probe.cpp`, plus the narrow OFF-by-default CMake option
`GUI_FORMS_BUILD_TEXT_PAINT_PROBE`. It reuses the private worker source without
changing it. No production renderer, host, public Painter, SDK or application
stage was changed.

Inspection found the existing Skia raster path shapes inside `draw_text_utf8`,
maps face IDs through its own registered table, then calls `drawGlyphs`. It does
not expose the required prepared-result transaction and skips unknown faces.
Skia source/archives are absent in this Shadow checkout. This fixture instead
uses the already pinned FreeType CPU rasterizer. That is **not a production
renderer selection, Skia/GDI parity claim, or native-window validation**.

## Fixed diagnostic profile and ownership

The worker owns and destroys its HB/FT engine before the prepared result is
copied out and the Worker itself is destroyed. A separately constructed
synchronous reference shaper is also destroyed before either painter opens
native resources. Each paint call creates its own FT library and four faces
from retained immutable encoded bytes, and destroys the faces before their
library on the painter's construction executor. No worker FT/HB state crosses
to painting. The raster function does not shape text; it consumes the stored
glyph IDs and x/y positions.

The painter pins the exact font table. Validation compares font-set identity
and generation, actual encoded bytes, registration role/weight/italic flag and
face index. Requested effective font, scale, wrap/tab/context and other identity
fields must match the expected authority, as must the private admission epoch.
Live bytes do not restore revoked authority. Local face IDs must be nonzero,
unique and resolve every run. Glyph IDs must fit FT_UInt and be within the
selected face's glyph count; glyph zero is refused in this complete-coverage
fixture. Missing primary/cluster coverage is refused explicitly.

Profile details:

- Fixed 800 by 128 surface, one byte of grayscale coverage per pixel; staged
  destination is 102,400 bytes. A previously published surface can coexist.
- Font size uses the same clamp to 1–4096 and rounded 26.6 sizing as the pinned
  shaper, with 72 by 72 DPI, scale 1, and no additional synthesis or nondefault
  variation-axis selection. Actual selected face bytes/index come from the
  prepared table; requested weight/style cannot silently substitute a face.
- `FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP`, default hinting target, followed by
  `FT_RENDER_MODE_NORMAL`. Only outline glyphs producing 256-level grayscale
  bitmaps are accepted. Bitmap/color output is not a supported capability.
- The fixed origin is (8, 64). Integral positions use floor; fractional positions
  are rounded to 26.6 translation, with screen y converted to FT y direction.
  Each glyph restores its own transform. Advances are validated but never used
  to reshape or recompute the supplied positions.
- Positive pitch only, no row shorter than width; width/rows at most 8192,
  pitch at most 16,384, and rows times pitch at most 64 MiB, checked before
  multiplication/indexing. Negative pitch is refused. FT bitmap allocation has
  already occurred before these checks and is **not** bounded by this guard.
- Finite glyph coordinates have magnitude at most 1,000,000; bitmap offsets
  have the same signed bound. These establish safe signed origin/offset sums.
  Signed clipping precedes conversion to destination indices. Grayscale
  compositing is `coverage + prior * (255 - coverage) / 255` with unsigned
  intermediate values and integer truncation.
- Text admission is 16,384 bytes; run count at most 16,384 and total glyph count
  at most 65,536. These are private diagnostic limits, not a selected public
  layout capacity profile. UTF-8, source extents, glyph clusters and finite
  nonnegative summary metrics are checked. No D1 source mapping or grapheme
  certificate is established here.

All glyph references are validated before staged pixels are produced. A failed
stage leaves the old pixel vector and its actual painted identity unchanged.
Only complete success moves staged storage into the published surface.
The pixel borrow ends on successful paint or destruction. Native face recreation
on every paint and full font-byte comparisons are deliberately unoptimized
fixture operations, not a production hot-path recommendation.

## Executed evidence and retained negative result

**MEASURED:** final Release fixture exited 0 with empty stderr under an external
60-second deadline. Output is `TEXT_PAINT_PROBE_2026-10-01.txt`. Toolchain,
machine, four font assets and pinned FT/HB versions are those recorded in
`TEXT_LAYOUT_NATIVE_PROBE_2026-10-01.md`. There is no paint timing claim.

The positive input is exactly `ABC שלום العربية 123 office`, with requested
content font size 20, weight 400, upright, letter spacing 0.25. Worker-prepared
and independently synchronously shaped reference records produce byte-identical
102,400-byte surfaces through the **same raster implementation**, with 1,369
nonzero pixels and seven runs. This proves ownership and prepared-position
transfer for the fixture; it is not an independent raster-correctness oracle,
visual acceptance or production renderer comparison.

The initial positive expectation failed for the exact input
`ABC שלום العربية 123 a\u0301 office` (the `a` is followed by U+0301 COMBINING
ACUTE ACCENT). The real shaper reported one missing cluster and no missing
primary face; both painters returned missing-font status. The failed output is
retained as `TEXT_PAINT_PROBE_initial_refusal_2026-10-01.txt`. The final fixture
continues to shape that same input as a negative test and verifies refusal
preserves the prior complete raster. This is a material reported coverage limitation,
not evidence of general Unicode coverage. The fully covered positive case does
not erase it. This fixture does not determine whether the missing-cluster report
originates in the bundled fonts or in fallback/cluster handling.

Fourteen executed refusal cases preserve old pixels and painted identity:
real missing cluster, missing lease, provider-generation mismatch, revoked epoch
with still-live bytes, effective-size mismatch, font-generation mismatch, wrong
face index, wrong registered style, wrong encoded font, unknown local face,
out-of-range glyph ID, glyph zero, NaN position, and position conversion bound.
A valid paint succeeds again after all refusals.

Numeric status legend for the fixture log: success 0; stale 1; missing font 2;
incompatible face 3; incompatible glyph 4; invalid geometry 5; unsupported raster
6; native failure 7; resource failure 8. These are private enum values, not a
public protocol. Unsupported negative-pitch/color/bitmap branches and the native
failure/allocation-exception cleanup paths were source-reviewed, **not executed
by the final refusal fixtures**. There is no allocator fault injection or leak
measurement here. Native FT cache/temporary allocation and paint peak memory
remain unknown independently of the controlled destination size.

## Review and reproduction

Configure HarfBuzz ON and `GUI_FORMS_BUILD_TEXT_PAINT_PROBE=ON`; build
`gui_forms_text_paint_probe` and run it with `gui_forms/assets/fonts` as its one
argument. The existing `.build/house-style-text` projection was used with phase
diagnostics and geometry trace OFF. Latest bounds/fixture edits were rebuilt and
the final process rerun before this handoff. Three-file spelling scan reported
zero findings; strict syntax checking of both new `.cpp` files passed with
`-Wall -Wextra -Wconversion -Wsign-conversion -Werror`, using existing public/FT
headers as system includes. `git diff --check` passed.

The parent independently verified the target was up to date, reran the
three-file spelling scan (zero findings), and executed the final binary under
a ten-second external deadline. It exited 0 with empty stderr and reproduced
the fourteen refusals and 102,400-byte/1,369-ink-pixel/seven-run result.

Semantic source review against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`
covers all three new files and the narrow CMake addition: explicit types and
conversions, named execution, single-executor ownership, constant shared font
lifetimes, RAII release order, validation before publication, row/area/index
bounds, finite position conversion, fixed destination allocation and repeated
glyph work. Parent independently read this exact scope and confirmed signed
offset/pitch safety, face-before-library cleanup, staging preservation and load
flag/sizing correspondence with the HB engine. No blocking finding or remaining
house-style violation was identified in this scope. Unchanged production paths
and vendor code are not certified by this review.

Source SHA-256:

- Header: `e4314cdd510ecbdebf2118ea8c58fb9093243c69e1ae9cd023466681b69165c2`
- Painter: `65e431fc45b8bf0b2a2aec0a4b73bcf034263c7c5578612943b7eafbb7986967`
- Probe: `8890ae6bdeabf38dc4737f7ebada0bcb8cc27321dfe8fcbad8976eed08d4cb17`

Registry was notified of remaining public semantics: variation/load/hinting/
raster-profile identity, lease versus revocation authority, face/glyph
compatibility guarantees, exact outcome vocabulary and native memory budgets.
Production prepared-layout integration, host publication, native UI testing and
matched SDK adoption remain separate gates; this fixture advertises none of them.
