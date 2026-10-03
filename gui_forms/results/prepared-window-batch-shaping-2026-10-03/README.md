# Private prepared-window batch shaping

**OBSERVED implementation:** one worker-confined `PreparedWindowShaper` retains
the exact immutable font bank and one reusable native shaping workspace. It
shapes the admitted complete paragraphs sequentially, preserving their separate
source mappings, under the batch's existing single 8 MiB payload reservation.
No public session, worker thread, host wake or frontend activation is added.
The entire component remains behind `GUI_FORMS_BUILD_PREPARED_TEXT`.

The row table is charged before allocation. Each native call receives the
remaining aggregate output, run and glyph allowances; its temporary output
owner is charged while live and retires after the arrays transfer. Input,
paragraph descriptors, row geometry and retained arrays share the same payload
accounting. Output arrays become deeply const, rather than merely making their
owning pointer const. The font bank outlives the native engine and retained
results. The constructing executor must also initialize, shape and destroy the
shaper; it creates no hidden executor or callback.

Worker observation checks the complete desired identity without granting the
worker admission or desire rights. Authority then ledger lock order is used at
final publication. Closing/revocation is checked around each native call and
again before transferring the entire row table. Refusal preserves the batch's
input, committed fields and reservation; partial geometry retires locally.
Native shaping is not interruptible and no shutdown deadline or RSS bound is
claimed. Initialization rechecks ledger closing before publishing its engine.

An empty paragraph may use zero remaining runs/glyphs in an already prepared
workspace. Its temporary owner still needs output capacity. Nonempty input,
workspace preparation and the older `shape_bounded` API retain their previous
zero-count refusal. Empty rows currently preserve the engine's existing private
metrics: requested device-font-size height, zero ascent/descent and no glyphs.
This is not a decided public baseline, caret or line-box policy.

## Windows correctness evidence

**MEASURED:** Release MSYS2 GCC 16.2.0 on Shadow Windows; at most two compiler
jobs, HarfBuzz enabled, Skia disabled. Eight focused suites passed in 1.89 s:
prepared input, batch, shape, bounded shape, bounded workspace, prepared service,
prepared raster and prepared display. `WindowsLastTest.log` preserves the output.
After adding foreign-executor and repeated-initialization/output checks, the
shape target was rebuilt and its suite passed in 0.62 s total;
`WindowsFinalShapeLastTest.log` preserves that final run. These durations are
correctness evidence, not performance measurements.

The fixtures cover 512 mixed-script rows including empty/EOF rows, CR/LF/CRLF,
nonzero source/display bases, independent per-paragraph run/glyph/metric
comparison, atomic control projection versus literal lookalikes, exact retained
byte accounting, final-owner reservation retirement, six allocation failure
points, late revocation, late missing coverage and closing during initialization.
Foreign calls refuse before initialization and after publication; repeated
initialization and populated-batch shaping return busy without changing input,
geometry identity or accounting. Worker observation cannot mutate authority.

The first build exposed a test-only use of a nonexistent `SourceByteRange`
equality operator. Explicit comparisons of the begin/end values fixed that
diagnostic without changing production APIs. No failed runtime test was hidden.
The aggregate fixed 8 MiB/run/glyph ceilings are **not reached by these fixtures**;
their remaining-capacity arithmetic was source reviewed. Lower-level zero-count
and exact temporary-owner-byte boundaries are directly tested. Artificial
mutable certified input or test-only production budget knobs were not added.

An independent OFF configure in `.build/prepared-window-off` produced a build
graph containing neither the new source nor test target. Normal SDK export
therefore remains separate. Native macOS/Linux integration is pending.

## House-style review

Root and the visible sibling “Audit File Manager Details against interviews”
reviewed the authored scope against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`, separately from functional tests and the
supplemental spelling scan. That scope is the new shaper header/implementation
and shape tests; batch geometry and observation additions in the batch header,
implementation and tests; the narrow empty-input condition in the existing
HarfBuzz implementation; and CMake/workflow registration. Review included
explicit types and conversions, initialization, operation order, deep
immutability, executor/borrow lifetimes, font and native-engine destruction,
authority/ledger lock order, failure preservation and repeated-work storage.
The final foreign-executor/busy additions received a separate read-only review
with no finding. The eight-file spelling scan also reported no violation.

`reviewed-source-sha256.json` records UTF-8 LF-normalized hashes of the final
files. Whole-file hashes identify bytes, not a claim that unchanged legacy
source or dependencies were fully reviewed. No remaining concrete violation
was reported in this authored scope. The geometry and budget test limitations
above remain; passing tests do not certify the complete product or house style.

## Remaining work

The earlier reusable-workspace measurements are in
`../reusable-shaping-workspace-2026-10-03/README.md`; this batch stage adds no
new timing claim. Session delivery, a reliable readiness channel outside the
ordinary callback queue, whole-batch controller adoption and immutable retained
rendering still need implementation and evidence. Wrapping, tabs, overlong
paragraphs, interior anchors, editing and accessibility remain requirements.
The existing File Manager preview does not yet consume this development path.
