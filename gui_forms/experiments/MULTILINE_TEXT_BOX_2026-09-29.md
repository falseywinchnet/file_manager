# Retained multiline TextBox development evidence

Date: 2026-09-29 (Shadow Windows local date).

**GIVEN:** the current owner requests a traditional Notepad consumer of public
GUI.Forms. This prerequisite belongs in the toolkit; no native OS edit control,
browser runtime, or private Notepad editing engine is admitted.

**OBSERVED:** TextBox already provides TextStore grapheme boundaries, bounded
whole-text undo/redo, host clipboard commands, and portable text/selection
events. Its prior renderer lays out a single line and measures every prefix on
each paint. The public text metric seam exposes text width and font metrics,
but no cluster positions from the shaper.

The additive C++ implementation opts into multiline and wrap independently;
existing defaults remain single-line. It preserves exact UTF-8 and all stored
line endings, supports LF/CR/CRLF Enter insertion, optional literal Tab, visual
row navigation and selection, pointer hit testing, caret reveal and wheel
scrolling. Layout caches document revision, effective font, provider pointer,
and viewport width. Painting visits only the visible rows. Complete shaping
runs between tabs reach the renderer intact, including long joining-script
text. Four-space tab stops affect presentation only.

**REJECTED:** bounding prefix work by splitting arbitrary 128/256-grapheme
shaping chunks. Such cuts can alter Arabic/Indic joining or ligatures. No such
chunks remain in the implementation.

**CANDIDATE development limits:** 1 MiB total UTF-8, 4096 UTF-8 bytes per logical
line excluding its terminator. These are provisional limits for the existing
prefix metrics and contiguous storage baseline, not an accepted final product
constitution. `validate_multiline_text` returns a precise result before document
replacement. Rejected loads, edits, paste, or semantic set-value preserve the
current value, selection and undo history. Masked text cannot enable multiline.
Undo remains capped at 128 snapshots and 8 MiB. A future cluster-position
metric projection and storage/history comparison are the reversal path.

## Test scope and environment

- Shadow Windows x64, AMD EPYC 9354 processor; GCC 16.2.0, CMake 4.4.3,
  Ninja 1.13.2; Release configuration.
- Read-only adjacent Plan Paint MinGW toolchain selected by the repository
  Windows toolchain script. Outputs remain in `gui_forms/.build/shadow-windows`.
- Initial base was `7ce5cf4`; parent made baseline commit `ddade5b` while this
  work was in progress. This record describes the final local TextBox source,
  header and `tests/multiline_text_box_tests.cpp`, not an immutable release.
- Build concurrency: 2. No worker agents or desktop input automation.

**MEASURED focused tests:** existing `gui_forms_input_controls_tests` and new
`gui_forms_multiline_text_box_tests` passed on Windows (initial combined CTest
elapsed 0.33 seconds). The corpus covers:

- exact mixed CRLF/CR/LF preservation, Unicode line separators, configured
  Enter, literal Tab, read-only commands, undo/redo and portable clipboard;
- short-row preferred horizontal position, visual/document Home/End,
  PageUp/Down, Shift range selection and soft-wrap upstream caret affinity;
- combining sequences, supplementary emoji and atomic CRLF grapheme deletion,
  including caret repair when an edit joins a combining sequence;
- two-dimensional pointer selection, wrap/font/width invalidation, horizontal
  reveal and persistent wheel scrolling independent of an offscreen caret;
- invalid UTF-8, document/line bound refusal and atomic preservation; and
- 10,000 logical lines, viewport-only painting, zero new metric calls on warm
  repaint, maximum admitted long-line layout, and unsplit shaping input.

The initial test compile failed because the fixture called protected
`Panel::local_bounds`; the fixture now uses its known local damage rectangle.
No production visibility was widened to accommodate the test.

**MEASURED final verification:** the final Release rebuild completed with two
jobs. `ctest --test-dir gui_forms/.build/shadow-windows --output-on-failure
--parallel 2` passed **65/65 tests in 9.17 seconds**, including native application,
text, accessibility, C ABI, boundary audits and the multiline fixture. The
matching development SDK was installed with `cmake --install` from that build.
Source/installed TextBox header SHA-256 matched:
`ec7c7327a4f6499e454aca05255de38a5541e1d1643158638479f1235674c8d8`.
Build/installed application DLL SHA-256 matched:
`aafc07514503f1e86ca2ef27687bddf8f121996831f21975a81d1b053ef7fd72`.
The native application input regression includes the separately parent-owned
Windows control-character filter; this chat did not edit the Windows host.

## Diagnostic workload observations

One focused run printed 97.6907 ms for setting/layout/painting 10,000 CRLF
lines of `line fixture` (140,000 bytes) with 120,002 cold metric calls; the next
paint made zero metric calls. Only visible text rows were emitted. A 4096-byte
ASCII logical line took 275.42 ms for cold layout with the same synthetic metric
provider. That provider itself constructs TextStore per metric call, so these
numbers include test-oracle cost and are **not native rendering performance**.

The prior single-line painter's source provides the structural baseline:
every paint measures every grapheme prefix. The multiline cache eliminates that
work on unchanged repaints; it does not eliminate quadratic cold prefix cost.
No latency-distribution or native large-document responsiveness claim follows
from these one-run diagnostics. Full-size native workloads and p50/p95/p99
measurements remain necessary before lifting the provisional limits.

## Remaining edges

Visual bidi caret/range geometry, full IME composition, native accessible text
ranges, visible scrollbars and native cross-platform acceptance are unverified
or absent. Logical grapheme-safe storage does not imply those capabilities.
The viewport supports wheel and keyboard caret reveal. The metrics provider
must be installed on the Window for terminal metrics; a provider-free window
uses the existing explicitly estimated typography fallback. Cached geometry
tracks provider identity, effective font and width, not an unexposed internal
revision of a provider mutated in place.

The provider negotiation entry records the public edge. Parent coordination
owns canonical registry reconciliation and the shared development SDK manifest.
