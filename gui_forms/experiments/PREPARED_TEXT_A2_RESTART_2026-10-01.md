# A2 shutdown checkpoint — 2026-10-01

**OBSERVED:** coordinator relayed owner's five-minute shutdown direction during
A2 work. Feature expansion stopped. Root owns shared-repository checkpoint and
push; this provider has not committed independently.

## Already integrated

A1 Windows candidate/front frame transaction is committed/pushed at `f26b143`.
Its independent coordinator review and final 2/2 focused tests are recorded in
`WINDOWS_DIB_TRANSACTION_2026-10-01.md` and the adjacent raw test log.

## Saved, compilable A2 dependency work

- `src/core/text/unicode/unicode_grapheme.hpp/.cpp`: private requirement query,
  typed failure status and uniquely owned bounded boundary output. Preflight
  checks actual Scalar/boundary payload plus old output before allocation;
  exact arrays eliminate implicit growth. Invalid input/budget failure preserves
  the old owner; moves empty the source. Existing vector entry retains behavior
  and shares the break kernel through a read-only span.
- `tests/bounded_grapheme_tests.cpp`: existing-output equivalence for empty,
  ASCII, CRLF, combining, Hebrew, emoji, Indic and 8191-mark input; exact budget,
  one byte below, old/new overlap, malformed input, and moved-from tests.
- `src/render/text/harfbuzz/harfbuzz_font_engine.hpp/.cpp`: private
  `shape_bounded` returns flat uniquely owned run/glyph arrays. Its fixed
  scratch arrays cover scalars, font segments, directions and visual segments;
  bounded Unicode segmentation replaces TextStore only in this route. A fixed
  64-entry candidate table uses stable insertion, avoiding hidden sort scratch.
  Count/product/aggregate requests are checked before controlled allocation;
  native glyph counts are checked before copying. `append_run<true/false>`
  shares native shaping while selecting bounded versus legacy output at compile
  time. Existing vector-output callers remain on the original route.
- `tests/bounded_shape_tests.cpp`: 27 exact successful comparisons against the
  existing shaper plus a long combining input; metric/coverage/face/source-range/
  glyph identity and position comparisons; input/run/glyph/output/workspace
  budget failures, overflowing count, invalid UTF-8, caller-old-owner retention,
  and exact-limit acceptance.

**MEASURED on current saved source:** `.build/house-style-text` native GCC 16.2
Release builds `gui_forms_text_engine` and its existing HarfBuzz test. CTest
text-store + bounded-grapheme + HarfBuzz suites passed **3/3 in 0.28 s**.
The new bounded-shape test was compiled/linked directly against that build and
passed. Its first attempted CMake build failed only because its new target had
not yet been wired by root. Root then wired the target. Final CMake build and
CTest passed **4/4 in 0.67 s**, including bounded-shape 0.27 s, bounded-grapheme
0.05 s, text-store 0.03 s and existing HarfBuzz 0.29 s. These are functional test
durations, not latency benchmarks. All processes completed before handoff.

The Unicode helper and its test passed strict C++20 `-Wall -Wextra -Wconversion
-Wsign-conversion -Werror` syntax checking. Full strict/style review of the new
bounded shaper and its test is **not complete**. Functional success does not
establish house-style acceptance. Review against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md` before promoting this checkpoint, including
all typed ownership, conversion/order, result bounds and failure paths. Check
the preflight record accounting against actual arrays and service reservations.
Opaque HB/FT/SheenBidi allocations and allocator bookkeeping are not capped by
the reported controlled array payload. Resource exhaustion has not been injected.

Direct bounded-shape validation command (toolchain already entered):

```powershell
g++ -std=c++20 -O2 -I gui_forms/include -I gui_forms/src/render/text/harfbuzz `
  gui_forms/tests/bounded_shape_tests.cpp `
  gui_forms/.build/house-style-text/libgui_forms_text_engine.a `
  gui_forms/.build/house-style-text/libgui_forms_core.a `
  gui_forms/.build/house-style-text/third_party/harfbuzz/libharfbuzz.a `
  gui_forms/.build/house-style-text/third_party/freetype/libfreetype.a `
  gui_forms/.build/house-style-text/libgui_forms_bidi.a `
  -o gui_forms/.build/house-style-text/bounded_shape_checkpoint.exe
& gui_forms/.build/house-style-text/bounded_shape_checkpoint.exe gui_forms/assets/fonts
```

## Next work, not yet implemented

Registry agreed exact A2 service/session/layout/GrayTextMask names and finite
development profile 003 in `orchestrator/spec/contracts/GUI_PREPARED_TEXT_DEVELOPMENT.md`.
Public hierarchical prepared-text headers, production storage/service/raster,
worker readiness and revocation, shared generation/font/mask budget ledger,
typed retained commands/Painter/Window attachment and installed consumer SDK
are **not implemented**. Only the private bounded shaping dependencies above
have been written. No new SDK or capability availability follows.

Keep the agreed service ledger alive across retired sessions/layouts/commands:
8 MiB reservation per payload, max three distinct generations / 24 MiB;
metadata 1 MiB included, transferred input separately charged until moved;
encoded fonts aggregate 8 MiB across at most two banks, never 8 MiB per
generation; controlled shaping arrays at most 16 MiB; mask max 4096 per axis /
4 MiB pixels and old+candidate 8 MiB. Exact device-size rounding and pixel/DIP
units are in the reconciled registry. Native allocations remain opaque.

Games requires newline/word wrapping, six roles and monochrome options beyond
the first single-paragraph grayscale profile. SwiftEdit now has an owner request
for a usable MacBook build; provider relayed public macOS SDK/Application/picker
packaging assessment to root. No macOS prepared-text backend is implemented or
advertised. Preserve those consumer needs without inventing current support.
