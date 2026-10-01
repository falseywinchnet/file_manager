# Bounded shaping dependency source review

**OBSERVED:** resumed A2 after the owner revoked the historical shutdown pause.
Reviewed the complete `planning/PROGRAMMING_HOUSE_STYLE.md` against the bounded
Unicode and shaping checkpoint before starting public service integration.

Exact reviewed scope: new declarations and definitions in
`src/core/text/unicode/unicode_grapheme.hpp/.cpp` (storage requirement, bounded
owner/function, span-based break-kernel parameter); new bounded declarations,
array accounting, scalar decoding, `shape_bounded` and changed shared
`append_run` hunks in `src/render/text/harfbuzz/harfbuzz_font_engine.hpp/.cpp`;
complete `tests/bounded_grapheme_tests.cpp` and `tests/bounded_shape_tests.cpp`.
Unchanged legacy engine registration/resolution, original TextStore internals,
other tests and vendored libraries are not globally certified.

Types, source-byte versus glyph indices, fixed capacities/live counts, named
execution, operation order, native borrows, uniquely owned arrays, cleanup and
failure publication were reviewed. The public service must separately enforce
session confinement and aggregate reservations; these private synchronous
functions do not establish that service lifecycle themselves.

All bounded output/workspace array products and sums are checked before their
allocations. Unicode replacement includes old output plus new scalar/boundary
arrays. Shaping counts segmentation peak separately from the later simultaneous
scratch-array peak. Fixed face candidates use stable insertion; the bounded
route has no implicit vector growth or standard-library sort scratch. Native
glyph counts are checked before copying into flat fixed-capacity arrays. Native
HB/FT/SheenBidi storage and allocation bookkeeping remain outside the reported
controlled array payload; this is not a process hard quota. Persistent encoded
font owners are a separate service-ledger category, not shape scratch.

Strict checking found four inherited implicit int-to-float conversions in the
shared glyph kernel. They are now explicit; Y is widened to signed 64-bit before
integer negation and conversion to float, avoiding signed 32-bit INT_MIN negation
while preserving positive zero. Float geometry storage and calculations remain
the established private engine precision. Added a separate position count and
non-null record check after HB output acquisition, before any glyph copy. No
remaining house-style violation was identified in the reviewed authored scope.
Allocation failure injection and whole-native-memory quotas remain unproved.

**MEASURED:** GCC 16.2 C++20 strict syntax check passes for the engine plus
bounded-shape test using `-Wall -Wextra -Wconversion -Wsign-conversion -Werror`,
with existing public/vendor headers treated as system includes. The earlier
bounded-Unicode/helper test strict check also passed. New reviewed glyph checks
build in `.build/house-style-text`; the four focused CTest suites pass 4/4 in
0.43 s. Bounded versus established shaping compares exact metric, coverage,
face, source-range and glyph fields for 27 configurations plus the long-mark
case. These are correctness checks, not speed claims or an independent shaping
oracle. The public service, aggregate budget accounting and portable raster
remain separate implementation work.

The requested golden comparison caught a rejected intermediate conversion:
negating after conversion to float changed zero vertical advances to negative
zero. The ordinary equality tests passed but serialization differed. Failed
`geometry-bounded-review.txt` remains in the build evidence. After the widened
integer correction, `geometry-bounded-review-fixed.txt` is byte-identical to
the acute-corrected checkpoint: **12,043,216 bytes**. Diagnostic HarfBuzz CTest
also passes 1/1 in 0.16 s. A focused positive-zero assertion now guards the
horizontal bounded output. The immutable earlier golden remains untouched;
the already accepted acute coverage-counter correction is not reversed.
