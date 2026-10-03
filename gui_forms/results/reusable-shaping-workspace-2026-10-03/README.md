# Reusable bounded shaping workspace: correctness checkpoint

Status: **private candidate; Windows correctness measured; native matrix and
performance pending**. No File Manager renderer, text service, public SDK, or
consumer feature selects this path yet.

## Problem and implementation

**OBSERVED:** the existing bounded shaper reserves the declared run/glyph
ceilings for each returned paragraph. Repeating that ownership shape is not a
practical way to retain a window of short rows under the existing 8 MiB aggregate.
This candidate prepares a separate serial workspace, shapes once per native run,
then copies only active run/glyph records into an independently owned result.
The 16 MiB controlled workspace and 8 MiB output ceilings remain separate.

The workspace contains reusable Unicode scalar/boundary storage, font/direction/
visual-run records, and staging geometry. Preparation checks arithmetic and the
simultaneous old/new requested storage before allocation. Failed preparation
preserves the old workspace. Calls validate input and prepared capacity before
traversal, never grow scratch, and restore the prepared capacities after a
smaller per-call limit or native failure. Failed output allocation cannot replace
the caller's previous owner. No input/native pointer is retained in output.

The existing bounded API shares the extracted traversal and keeps its allocation
and output contract. Face-table indices replace transient segment pointers;
registration remains unchanged during a call. Unicode break rules are unchanged.

## Evidence

The visible sibling implementation ran the existing Windows MinGW Release build
at `.build/prepared-window-input`, one compiler job, with
`GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS=ON`. `WindowsLastTest.log` preserves the final
six passing suites: bounded grapheme, bounded shape, bounded shape workspace,
prepared text service, prepared text raster, and prepared display. Reported
combined duration was 1.25 seconds; this is test duration, not a speed benchmark.

The new fixtures compare nine inputs at three font settings against the existing
bounded shaper, including all glyphs, clusters, runs, metrics, and coverage. They
exercise eight workspace-preparation allocation failures, three result allocation
failures, exact and one-less storage limits, invalid UTF-8, unprepared requests,
missing primary fonts, 512 independently retained mixed-script results, six
native failure hooks, a later-run failure, and subsequent successful reuse.
Grapheme fixtures retain an independent existing segmentation control, including
CRLF, combining marks, Indic conjuncts, emoji, and 8,191 combining marks.

Sufficient preparation and grapheme fill are checked for zero C++ allocations.
A nonempty prepared shape allocates one result owner plus its two active arrays;
an empty result allocates one owner with no run/glyph arrays. This instrumentation
does not count opaque native-library allocation and is not a zero-allocation
whole-shape claim.

## Independent source review and limits

Root reviewed the authored changes in the eight source/test files named by
`reviewed-source-sha256.json`, plus the five CMake registration lines. The review
applied the complete `planning/PROGRAMMING_HOUSE_STYLE.md`: explicit initialized
types, named traversal, owner/move/borrow lifetimes, prevalidated capacity,
replacement order, complete failure paths, checked conversions, and repeated
storage reuse. The extracted traversal was compared with its previous bounded
path. No actionable new correctness or style defect was found in that scope.
The spelling scan separately reported eight files and zero candidates. The
first scan invocation hit Windows' Python alias; rerunning through the configured
Windows toolchain succeeded. Untouched legacy helpers are not certified.

Controlled accounting describes requested first-party owner/array storage, not
allocator overhead, complete stack frames, encoded font owners, or opaque
FreeType/HarfBuzz/SheenBidi memory. The 512-row test charges its owner array and
result allocations; it does not certify window input/batch/controller accounting.
Window cancellation, complete retained-batch admission, raster activation,
end-to-end input latency, native macOS/Linux correctness, and performance remain
unmeasured by this checkpoint. A separate benchmark must compare geometry before
timing and calibrate its clock. No product-speed claim follows from this change.
