# Native preview paint source review

## Explicit sRGB destination followup

**Source acceptance for this diagnostic follow-up. No correctness, ownership or house-style blockers found in the reviewed changes.** The change preserves the test’s integrity as a new, explicitly configured snapshot control; it does not prove the cause of the earlier failure.

The operation order matters here and is correct: the probe obtains a compatible bitmap, assigns the proposed sRGB destination profile, checks for failure and verifies the profile, **then draws the view into that destination**. It does not relabel already captured failing samples. Apple documents both configuring the destination before `cacheDisplayInRect` and the possibility that retagging returns nil or the original representation. The code handles both outcomes appropriately. [AppKit snapshot documentation](https://developer.apple.com/documentation/appkit/nsview/cachedisplay%28in%3Ato%3A%29?language=objc), [bitmap retagging documentation](https://developer.apple.com/documentation/appkit/nsbitmapimagerep/retagging%28with%3A%29?language=objc).

The source retains the important controls:

- Expected RGB values, channel tolerance, opacity threshold, nine sample positions, and presence/retirement predicates are unchanged.
- Snapshot creation and sampling remain outside timed spans.
- A missing compatible bitmap, failed retagging, unexpected destination profile or failed sample conversion rejects the run.
- Under ARC, the compatible bitmap, returned destination, color space and sampled colors remain owned for the synchronous operation. Returning the same bitmap from retagging creates no ownership problem. No foreign borrow escapes the existing autorelease scope.
- Separating `sample` from its color-space conversion makes the operation order and intermediate objects explicit.

The retained log corroborates the README’s account that both processes failed their first BGRA warmup with 640×480 arranged/image bounds and the reported `(0, 0.898072, 0.818158, 1)` sample. The README correctly labels the display-profile explanation **HYPOTHESIS**, preserves the failed control, and requires native success before accepting timing results. A future pass would validate this explicit-sRGB snapshot control; it would not, by itself, establish normal display-path color fidelity.

Exact reviewed scope:

- Snapshot destination setup and sample-conversion changes in [probe.mm](C:/Users/Shadow/file_manager/frontend/experiments/preview_paint/probe.mm).
- The workload clarification and diagnostic account in [README.md](C:/Users/Shadow/file_manager/frontend/experiments/preview_paint/README.md).
- [MacDisplayProfileRejectedLastTest.log](C:/Users/Shadow/file_manager/frontend/results/2026-10-04-preview-paint/MacDisplayProfileRejectedLastTest.log).
- The narrowly scoped log-preservation rules in [.gitattributes](C:/Users/Shadow/file_manager/frontend/results/2026-10-04-preview-paint/.gitattributes).

I confirmed the retained PNG is present, but did **not** independently decode its ICC profile or reproduce the System.Drawing sample; those details remain root-reported observations. The `.gitattributes` rules apply only to logs in this results scope and preserve their original line endings.

No edits, builds, native execution or Git mutations were performed.

---

## Diagnostic followup after the first native pixel rejection

**Source acceptance for the diagnostic-only changes. No correctness or house-style blockers found in this diff.**

The original pixel predicate, tolerance and failure condition are unchanged. A mismatch still throws and prevents measurement completion.

The added diagnostics are appropriately scoped:

- Warmup logging identifies the geometry and format before the control runs.
- Failure logging records image-versus-retirement phase, sample position, observed color, bitmap extent, arranged PictureBox bounds and image bounds.
- The failure PNG is encoded from the **already captured bitmap**. It adds no second snapshot or native draw.
- Snapshot-write failure is reported through `snapshot_saved`; it does not suppress the original pixel failure.
- `State` is borrowed synchronously. The bitmap, color and encoded data remain within the existing per-warmup autorelease lifetime, with no retained foreign borrow.
- Logging, encoding and disk writing remain outside the timed spans and occur only during warmup diagnostics. The first mismatch throws, so failure capture does not repeat through the remaining sample positions.

The distinct forward/reverse filenames avoid the two CTest processes overwriting each other’s evidence. The added workflow glob matches their location in the paint build directory.

Exact reviewed scope: the `check_pixels` signature, mismatch diagnostics and two call-site updates, plus warmup logging in [probe.mm](C:/Users/Shadow/file_manager/frontend/experiments/preview_paint/probe.mm); and the failure-PNG artifact pattern in [native-builds.yml](C:/Users/Shadow/file_manager/.github/workflows/native-builds.yml).

This accepts the diagnostics under the complete house style; it does **not** accept the failing paint control or establish a rendering explanation. I performed no edits, builds or tests, and did not inspect the retained failed log in this review.

---

Read-only sibling review of the complete authored probe, CMake, protocol and
workflow additions. Latest correction review first. Source-equivalent rebases
do not widen this scope. Native compilation and execution remain pending.

**Final scoped acceptance: the corrections resolve the remaining style finding and accurately document the measurement boundaries. No new findings.**

Reviewed against `77572097052f3dbdadb6ac8bda4bae560867f572`:

- In [probe.mm](C:/Users/Shadow/file_manager/frontend/experiments/preview_paint/probe.mm), `milliseconds` now computes a named `double` before returning it.
- Each format’s warmup and each individual measured format invocation now has its own autorelease pool. The pool drains after `measure` completes all five timed spans and after the scalar durations are copied into persistent sample arrays. No temporary Objective-C borrow from that invocation escapes the pool; the owning window/view, source buffers and measurement state remain outside it.
- In [README.md](C:/Users/Shadow/file_manager/frontend/experiments/preview_paint/README.md), visibility and client-extent validation are correctly described as initial checks. The document also states that pool drainage is outside the timed intervals and does not prove physical memory release.

The prior source acceptance for the probe, CMake and workflow scope remains valid, with **no remaining house-style violations identified in that reviewed scope**. Per-sample pool drainage improves temporary-object lifetime discipline without turning the measurements into RSS or complete cleanup-cost evidence.

This was a read-only diff review. No edits, builds or tests were performed; native compilation and CI results remain separate from this acceptance.

---

**No measurement-logic or ownership blocker found in the reviewed paint probe. One small house-style correction remains:** in `milliseconds`, assign `elapsed.count()` to a named `double` and return that value, following the standard’s “calculate, then return” rule.

The measurement design is internally consistent:

- **Forced native draw:** the probe marks the AppKit view dirty and calls `displayIfNeeded`. Each measured display must advance both public retained-paint and presentation counters by exactly one. This guards against silently timing a no-op or accepting an unexpected additional retained paint. Supporting inspection of the host draw path confirms image synchronization occurs within native drawing.
- **Timing boundaries:** admission, bind-through-display, combined admission-through-display, cached display, and clear/remove/display are measured separately. The combined interval is measured directly. Metric snapshots, post-display assertions, pixel snapshots, sorting and reporting are outside the timers. The README accurately identifies the admission-success check inside the bind/display interval.
- **Controls and ordering:** every format/geometry receives a warmup with nine native color samples and a cleared-image control. Thirty samples exercise all six format permutations five times, and the second process reverses geometry order. All source buffers and sample arrays are prepared before repeated measurement.
- **Retirement:** each measurement clears the PictureBox, removes the image, forces another native display, then checks zero logical resource/byte counts and rejection of the stale ID. A failure stops the run; window ownership handles eventual resource teardown.
- **Callback lifetime:** the delayed continuation explicitly owns a retained `shared_ptr<State>` through a heap context reclaimed on callback entry. `State` owns the source buffers and PictureBox; its model pointer is observational. The continuation checks closed/null state before using that borrow, and the main path clears it after `run_macos` returns. Execution and close notification stay on the owning thread.
- **Build/workflow scope:** the standalone bundle consumes the installed Application SDK, copies SDK fonts into its own resources, and introduces no provider edits or installation rules. Forward/reverse CTests are serial with 60-second process limits. The workflow preserves failure through `pipefail` and retains the measurement log.

The README appropriately limits interpretation. In particular, native snapshot samples do not prove complete pixel equivalence or physical scan-out; presentation counters do not measure desktop composition; and clear/remove plus a draw does not prove allocator release or RSS reduction. The entire batch also runs within one autorelease pool, so native temporary-object drainage is not established per sample. That is compatible with the explicit absence of residency conclusions.

Two details should remain precise when recording results:

- The nine-pixel and cleared-pixel assertions run during **warmup**, not on every timed sample. Every timed sample still checks draw counters, retirement accounting and stale-ID rejection.
- Visibility, occlusion and client extent are checked at the start of the measurement callback. The README’s statement that geometry changes or occlusion reject the run describes that initial check; it is not continuous monitoring throughout collection.

Exact reviewed scope:

- Full [probe.mm](C:/Users/Shadow/file_manager/frontend/experiments/preview_paint/probe.mm).
- Full [CMakeLists.txt](C:/Users/Shadow/file_manager/frontend/experiments/preview_paint/CMakeLists.txt).
- Full [README.md](C:/Users/Shadow/file_manager/frontend/experiments/preview_paint/README.md).
- The native Mac paint measurement step and log artifact path in [native-builds.yml](C:/Users/Shadow/file_manager/.github/workflows/native-builds.yml).

I applied the complete [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md), including named retained callback state, explicit types, initialization, borrows, conversions, failure handling and repeated-loop storage. Supporting reads of the public Mac host options and host initialization/draw paths were for lifetime and measurement interpretation, not blanket acceptance of unchanged GUI.Forms code.

No edits, builds, tests or Git mutations were performed. Native compilation, successful execution, and any resulting performance conclusions remain unverified by this review.
