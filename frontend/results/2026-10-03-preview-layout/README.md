# Selected preview uses the available interior

**OBSERVED baseline:** at `cda9917`, the selected text control has fixed
190x108 bounds and PictureBox has 194x112 bounds, inside a 266x174 preview
surface in the default retained-window fixture. Widening the inspector leaves
those content boxes unchanged. The text's explicit content-font override also
overrides its nominal monospace style role.

The content dimensions now follow the committed preview surface minus its
authored eight-unit padding. Both axes stay positive, preserving Label's
fixed-size measurement path rather than wrapping file contents during layout.
PNG uses the existing zoom policy, so it fills one axis while preserving aspect
ratio. House icons remain centered at their existing size. The excerpt now uses
the explicit monospace font role at the unchanged authored size of 13.

On surface layout or presentation changes, one primary-font sample establishes
a line-height estimate. The row allowance is floored and clamped to 1..32 before
conversion to size_t. It does not measure the file's full text. Fallback glyphs
can have taller metrics; the existing content clip remains authoritative. These
are bounded excerpts, not a scrolling full-document viewer. The PropertyList
retains the single scroll plane and its 260-unit expanded/81-unit collapsed
header behavior.

**REJECTED:** the new extent assertion fails against the old fixed boxes;
`WindowsRejectedExtentLastTest.log` records text 190x108 versus surface 266x174.
The first adaptive implementation exposed feedback through the generated
auto-sized parent, producing surface 240/body 194 heights. That failure is retained
in `WindowsRejectedGrowthLastTest.log`. The authored surface now declares its
existing 174-unit height as both minimum and maximum, so content fills that
fixed header region without pushing properties downward. The duplicate runtime
minimum assignment was removed. Eight-unit padding and the row ceiling are
reversible composition choices, not a new provider or text-layout contract.

**MEASURED Windows correctness:** Release, GCC 16.2, existing normal GUI.Forms
application SDK with HarfBuzz/Skia/transactional DIB disabled; build commands use
two compiler jobs. All 15 frontend suites passed in 7.16 s, including interaction
in 3.58 s. The accepted fixture records text 250x158 and image 250x158, with seven
rows at its normal text scale. Text scale 2 reduces the row budget while preserving
the authored font; widening the inspector enlarges the body without changing
selection. Ordinary 1024/800-wide windows and expanded unsupported-format
explanations retain padded bounds. Portrait/landscape eight-pixel BGRA fixtures
exercise the same PictureBox geometry without admitting additional file formats.
Selection generation, F5 revision refresh and collapse/reopening tests remain.

The generated fixtures include non-ASCII UTF-8 and a long unbroken paragraph at
the existing 64 KiB byte bound. Windows symlink fixture assertions report error 1314
and skip; they are not counted as passed. The CTest log is correctness evidence,
not a quiet-host performance or perceived-smoothness measurement.

Native Mac fixture changes require the same padded extent after actual text/PNG
pixels are observed, and capture both native-text-preview.png and
native-image-preview.png. CI preserves the new image capture alongside existing
diagnostics. These native changes have not yet run on this Windows host;
macOS/Linux checks and visual inspection of those artifacts remain pending.

House-style review scope is the content-fitting method, its named retained
subscriptions and presentation callback, initial dimensions/font change,
authored CSS constraints, new/changed interaction checks, native extent/capture
helpers and their call sites, and the single CI artifact path. The four C++/ObjC++
files pass the spelling scanner with zero candidates. Independent semantic
review is recorded separately; unchanged toolkit and generated code are not
newly certified.
