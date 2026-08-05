# M4b grapheme segmentation and shaping seam

Date: 2026-08-04

## Scope and status

- **GIVEN:** user-visible navigation and shaping ranges may not split an
  extended grapheme cluster.
- **GIVEN:** the portable core does not expose AppKit, CoreText, DirectWrite,
  Uniscribe, Skia, Wayland, X11, or another platform font object.
- **OBSERVED:** Unicode Standard Annex #29 revision 47 defines the Unicode
  17.0.0 extended grapheme rules, including Indic conjunct rule GB9c and emoji
  ZWJ rule GB11.
- **CANDIDATE:** `TextShaper` and `FontFallbackResolver` are a reversible
  service boundary. They do not select HarfBuzz, a platform shaper, font
  discovery policy, or glyph cache.

## Pinned normative inputs

Production properties and the test oracle come from the official Unicode
17.0.0 files recorded in
[`third_party/unicode/README.md`](../third_party/unicode/README.md). The
generator verifies four fixed SHA-256 digests before producing output.

Normal builds are offline. They compile the checked-in generated tables;
regeneration requires the operator to supply all four downloaded source paths.
The generated production table is 145,422 source bytes and the generated test
oracle is 43,637 source bytes. A second generation was byte-identical under
`cmp`.

## Implemented grapheme contract

`TextStore` now adds a distinct `GraphemeIndex` and provides:

- a grapheme count and snapshot count;
- checked UTF-8/grapheme position conversion;
- boundary membership;
- next/previous cluster navigation; and
- exact cluster ranges.

The segmenter applies UAX #29 GB3 through GB999 in declared order. Its tables
cover grapheme-break properties, `Indic_Conjunct_Break`, and
`Extended_Pictographic`. Edits rebuild the boundary vector atomically with the
existing scalar, UTF-16, line, and style metadata. Contiguous UTF-8 and full
metadata rebuilds remain a **CANDIDATE** baseline, not a storage decision.

## Renderer-neutral shaping/fallback seam

`include/gui_forms/text_shaping.hpp` exposes only portable value types:

- opaque `FontFaceId` and `GlyphId` values;
- OpenType tags, direction, language, feature ranges, and positive finite font
  size;
- absolute UTF-8 source/cluster positions;
- finite glyph advances and offsets; and
- fallback requests containing the complete grapheme cluster and ordered list
  of already attempted faces.

Request validation rejects malformed UTF-8, invalid faces/sizes, scalar or
grapheme splits, and invalid feature ranges. Result validation requires the
same face/range/direction, grapheme-aligned clusters, nondecreasing LTR or
nonincreasing RTL cluster order, and finite placement values.

This is deliberately not a shaper. `TextShaper` and `FontFallbackResolver` are
abstract synchronous services so backend experiments can be compared without
changing controls or public position types.

## Measured gates

- **MEASURED:** `gui_forms_grapheme_conformance_tests` passes all 766 cases in
  `GraphemeBreakTest-17.0.0.txt`, retaining each official source line in failure
  diagnostics.
- **MEASURED:** direct store tests cover combining marks, emoji ZWJ sequences,
  regional-indicator flags, typed round trips, navigation, mutation rebuilds,
  and split-cluster rejection.
- **MEASURED:** shaping seam tests cover absolute cluster maps, LTR/RTL order,
  grapheme and scalar split rejection, nonfinite placement rejection, and
  ordered complete-cluster fallback input.
- **MEASURED:** Apple Clang 16 with `-Wall -Wextra -Wpedantic -Werror`, Skia and
  native hosts disabled: 19/19 tests pass.
- **MEASURED:** the complete native Release suite with the Gallery, AppKit host,
  Skia, CoreGraphics, archive/font/boundary audits, and close regression passes
  24/24.
- **MEASURED:** ASan plus UBSan focused text/grapheme/shaping tests: 3/3 pass.
- **MEASURED:** MinGW-w64 GCC 15.2.0 emits static PE32+ x86-64 executables under
  the repository's existing GCC aggregate-warning exception; Wine devel 11.10
  runs all three focused executables successfully.
- **MEASURED:** macOS grapheme and shaping test executables import only `libc++`
  and `libSystem`; the portable core has no unresolved Skia, AppKit, CoreText,
  CoreGraphics, DirectWrite, or Win32 font/window symbol.

**OBSERVED negative:** macOS AppleClang AddressSanitizer aborts when
`detect_leaks=1` is requested because leak detection is unsupported on this
platform. The same ASan/UBSan binaries pass with `halt_on_error=1`; this is a
harness capability limit, not a suppressed product failure.

## Explicitly open

- **OPEN:** shaping backend comparison, script-run analysis, bidi, line
  breaking, font discovery, real per-cluster fallback, glyph run/cache policy,
  and text measurement integration.
- **OPEN:** selection, caret affinity, clipboard commands, undo/redo, password
  masking, read-only behavior, multiline viewport/reflow, IME/preedit state,
  candidate geometry, accessible text ranges, and `TextBox` integration.
- **OPEN:** a shaping corpus must compare HarfBuzz and admitted platform
  adapters before Gate T1 can select a stack.
- **OPEN:** the existing contiguous-store benchmark must compare gap and piece
  candidates before large-document storage is selected.

M4b closes complete extended-grapheme segmentation and the backend-neutral
shaping/fallback interface. It does not close M4 or the row-7 editor gate.
