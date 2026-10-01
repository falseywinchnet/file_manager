# Canonical-pair coverage correction — 2026-10-01

## Diagnosis

**MEASURED:** the pinned Carlito font maps U+0061 to glyph 45, has no nominal
U+0301 entry, and maps U+00E1 to glyph 1955. Direct pinned HarfBuzz shaping maps
both `a` + U+0301 and U+00E1 to glyph 1955 at source cluster zero. Isolated U+0301
produces glyph zero. The Arabic, Hebrew and emoji fixtures have none of these
three nominal entries and cannot supply the missing Latin mark cluster.

Raw initial evidence is `COMBINING_ACUTE_cmap_baseline_2026-10-01.txt`.
`COMBINING_ACUTE_cmap_verified_2026-10-01.txt` repeats it with explicit sample
definitions and the additional U+0350 query. The small, named, reproducible
query source is `COMBINING_ACUTE_cmap_query_2026-10-01.cpp`; it adds no CMake
target or general diagnostic framework. It uses the exact FT/HB load flags and
20 DIP/72 DPI profile relevant to the failed fixture.

**OBSERVED:** `HarfBuzzFontEngine::Impl::covers` required nominal glyphs for
every non-ignored original scalar before shaping. It rejected this cluster,
incremented `missing_clusters`, then the normal shaping operation nevertheless
produced the valid composed glyph. This is a demonstrated engine false-negative
coverage report for the canonical pair, not absence of the representable `á`
glyph from the font. It resolves the previously unestablished cause in the
historical paint proof; that old failed record remains unchanged.

## Narrow correction and regressions

**OBSERVED:** the coordinating chat accepted the exact private engine/test/paint
fixture scope before modification. The nominal fast path is preserved. Only
when it fails and the original cluster contains exactly two scalars does the
engine call pinned `hb_unicode_compose` with default Unicode functions. It
accepts that face only if canonical composition succeeds and the composed scalar
has a glyph in that same face. No input normalization, byte replacement, font
expansion, general NFC algorithm or public interface was introduced.

The regression verifies decomposed and precomposed input use glyph 1955 with
identical face, glyph positions/advances and metrics, while preserving the
different original source ends: three bytes for `a` + U+0301, two for U+00E1.
Isolated U+0301, `q` + U+0301 (not canonically composable), and `a` + two U+0301
marks each remain reported as one missing cluster. This is not general Unicode
coverage or a claim that all larger canonically equivalent sequences work.

**REJECTED test assumption:** the initial noncomposable negative candidate
`a` + U+0350 unexpectedly reported complete coverage, failing the first ON/OFF
test runs. The direct cmap query establishes Carlito actually has U+0350 as
glyph 2635. It was an invalid missing-font fixture, not a regression caused by
the correction. The test now uses `q` plus the measured absent U+0301 instead.
The failed initial runs and reason are retained here rather than mislabelled
as successful validation.

## Exact geometry and paint evidence

The immutable compressed pre-fix geometry golden is unchanged. The updated
27-case dump is `.build/text-layout-attribution/geometry-acute.txt`.
`COMBINING_ACUTE_compare_2026-10-01.py` compares explicit byte records, permits
only the three declared coverage changes and rejects every other difference.
It passed: `mixed_controls` configurations 0, 1 and 2 each change missing count
**3 to 2**, while all other serialized bytes remain identical, including line
terminators, exact font bytes, glyphs, faces, source ranges, directions, positions,
advances and metrics. The huge single-grapheme fixture remains missing one
cluster. The retained comparison output is
`COMBINING_ACUTE_geometry_comparison_2026-10-01.txt`.

The private paint fixture now uses the original formerly refused input
`ABC שלום العربية 123 a\u0301 office` as its positive worker/reference case.
Both rasters are identical: 102,400 bytes, 1,443 nonzero pixels, seven runs, zero
missing clusters. Its genuine negative coverage case is now isolated U+0301.
All compatibility/refusal/recovery assertions still pass. Output is
`COMBINING_ACUTE_paint_2026-10-01.txt`. The historical
`TEXT_PAINT_PROBE_initial_refusal_2026-10-01.txt` and previous positive outputs
are preserved as pre-correction evidence, not overwritten.

The ordinary HarfBuzz CTest passed 1/1 in 0.13 seconds; diagnostics-enabled CTest
passed 1/1 in 0.23 seconds, including the prior failure fixtures. The final paint
process exited zero. No performance resampling was performed or speedup claimed.
Frozen SDKs/application stages remain unchanged; this is not deployed behavior.

The parent independently reviewed the changed engine/tests/paint fixture and
the complete query/comparison sources, reran the byte comparison with exactly
the three declared deltas, and reran focused HarfBuzz tests in both projections.
Both passed 1/1 (0.10 seconds ordinary, 0.12 seconds diagnostics-enabled total).

## Reproduction and reviewed scope

After entering the normal Windows toolchain and building the text dependencies,
compile the focused query with:

```powershell
g++ -std=c++20 -O2 -Wall -Wextra -Wconversion -Wsign-conversion -Werror `
  -isystem gui_forms/third_party/freetype/include `
  -isystem gui_forms/third_party/harfbuzz/src `
  gui_forms/experiments/COMBINING_ACUTE_cmap_query_2026-10-01.cpp `
  gui_forms/.build/house-style-text/third_party/harfbuzz/libharfbuzz.a `
  gui_forms/.build/house-style-text/third_party/freetype/libfreetype.a `
  -o gui_forms/.build/house-style-text/acute_cmap.exe
& gui_forms/.build/house-style-text/acute_cmap.exe gui_forms/assets/fonts
```

Generate the geometry dump with the existing trace-enabled layout probe's
`--geometry` mode, redirect it to the candidate path above, and run
`python gui_forms/experiments/COMBINING_ACUTE_compare_2026-10-01.py` from the
repository root. The script checks actual bytes rather than only matching hashes.

Semantic review against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`
covers the changed `covers` function, new named canonical-pair regression and
its invocation, the two paint-input changes, the complete focused C++ cmap query
and typed Python comparison script. Explicit types/conversions, constant input
definitions, named behavior, native owner cleanup, original source lifetime,
conditional work after nominal failure and bounded pair composition were
reviewed. Four-file C++ spelling scan reports zero findings; the focused query
compiled with conversion warnings as errors. Existing unrelated tests/renderer
and vendored code are not globally certified. No remaining violation was
identified in the authored scope.

Source SHA-256:

- Engine: `57c14635eac605058937f8184d03b8413faca9bc3230f739f4816070facb4a05`
- HarfBuzz tests: `ca1130ee4d24155a4c03d678bae834115a7360e8cb74fcc7399b3d6779739a44`
- Paint fixture: `6b646ee70f2746638725f4d41d77aaa12092307b02c903c5ff5419b5722253a2`

The prepared-text registry separately received concrete service/Window/Painter
and retained-command feasibility, including current backend absence, authority
versus shared payload lifetime, and the need for controlled result-allocation
guards. Those proposals do not freeze an API or select a renderer through this
defect correction.
