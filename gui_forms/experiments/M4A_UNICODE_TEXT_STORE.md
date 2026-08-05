# M4a Unicode text-store foundation

Date: 2026-08-04

## Scope and status

- **GIVEN:** GUI.Forms text state is renderer- and host-neutral. AppKit,
  CoreText, DirectWrite, Uniscribe, Skia, Wayland, X11, and native edit controls
  do not define the portable storage or position contract.
- **GIVEN:** editable text must eventually support grapheme navigation, shaping,
  fallback, bidi, selection, clipboard, undo, composition, candidate geometry,
  password handling, and accessible ranges.
- **CANDIDATE:** M4a uses contiguous UTF-8 as the first measured storage
  baseline behind `TextStore`. This representation is not selected for large
  documents; rebuild counters deliberately expose its cost for the later
  gap/piece/rope comparison.
- **OBSERVED:** prior GUI.Forms code carried UTF-8 strings for painting and text
  input but had no reusable validation, typed positions, line index, atomic edit
  model, or style-span transformation.

## Implemented contract

`include/gui_forms/text.hpp` now provides distinct strong types for:

- UTF-8 byte offsets;
- UTF-16 code-unit offsets;
- Unicode scalar indexes; and
- line indexes.

`validate_utf8` rejects unexpected/invalid continuation bytes, truncation,
overlong encodings, surrogate code points, and values beyond U+10FFFF. Invalid
input is never silently replaced.

`TextStore` provides:

- bounded construction and replacement with a 16 MiB default byte limit;
- exact UTF-8, UTF-16, and scalar sizes and checked conversions;
- rejection of byte offsets inside a scalar and UTF-16 offsets inside a
  surrogate pair;
- next/previous scalar boundaries and scalar lookup;
- atomic range replacement with removed/inserted scalar counts;
- Unicode line indexing for CRLF, CR, LF, NEL, line separator, and paragraph
  separator;
- sorted, nonoverlapping opaque style spans with adjacent-span normalization;
- deterministic span splitting/shifting across edits and explicit inserted-text
  style; and
- structured revision, edit, style-update, metadata-rebuild, and rejection
  counters.

Failed mutations preserve text, spans, revision, edit count, and metadata. They
increment only the declared rejection diagnostic.

## Measured tests

Target:

```text
gui_forms_text_store_tests
```

The deterministic corpus includes ASCII, precomposed Latin, a combining mark,
supplementary-plane emoji, CJK, Hebrew, LF, and zero-width joiner scalars.

**MEASURED:**

- strict malformed-sequence classes are distinguished;
- every valid UTF-8/UTF-16/scalar test boundary round-trips;
- surrogate-pair and UTF-8 scalar splits are rejected;
- invalid UTF-8, split edits, overlapping/split style spans, and byte-limit
  overflow leave committed state unchanged;
- all six declared line-break forms produce deterministic content ranges;
- 2,000 seeded mixed-script insert/replace/delete operations match a scalar
  reference model at every surviving boundary; and
- successful baseline edits report exactly one full metadata rebuild apiece.

Build and regression measurements:

- **MEASURED:** focused ordinary-build target: PASS;
- **MEASURED:** Apple Clang 16 strict `-Wall -Wextra -Wpedantic -Werror`,
  Skia/host-disabled suite: 17/17 PASS;
- **MEASURED:** the strict `gui_forms_text_store_tests` executable imports only
  `libc++` and `libSystem`; no renderer, AppKit, CoreGraphics, or Skia symbol is
  unresolved by it;
- **MEASURED:** AddressSanitizer plus UndefinedBehaviorSanitizer focused target:
  PASS; and
- **MEASURED:** complete ordinary native suite: 23/23 PASS, including host,
  renderer, archive, font, and boundary audits.
- **MEASURED:** MinGW-w64 GCC 15.2.0 emits a static PE32+ x86-64 text-test
  executable under strict warnings, and Wine devel 11.10 executes the same
  2,000-edit corpus: PASS.

**OBSERVED negative:** the first raw MinGW `-Werror` build stopped in the
pre-existing `display_chunk.cpp` aggregate initializers on GCC's
`-Wmissing-field-initializers`. The rerun used the repository's already-recorded
strict Windows exception, `-Wno-missing-field-initializers`; no M4a warning was
suppressed and no unrelated renderer source was changed.

## Explicitly open

- **MEASURED:** M4b Unicode 17.0.0 extended grapheme boundaries and typed
  cluster navigation now pass the official conformance corpus. See
  `M4B_GRAPHEME_SHAPING_SEAM.md`.
- **OPEN:** normalization is preserved exactly as authored; no NFC/NFD rewrite
  is performed.
- **OBSERVED:** the M4b renderer-neutral shaping/fallback service and glyph-run
  validation vocabulary exist. **OPEN:** actual shaping, font fallback, cache,
  bidi, script/language analysis, and line breaking.
- **OPEN:** selection, clipboard commands, undo/redo, password masking,
  read-only policy, multiline viewport behavior, IME/preedit state, accessible
  ranges, and `TextBox` integration.
- **OPEN:** contiguous UTF-8 rebuild performance must be compared against at
  least gap and piece candidates with named short-field, multiline, and
  high-churn workloads before a storage ADR is accepted.

M4a therefore closes the storage/position prerequisite, not the M4 text or row-7
editor exit gate.
