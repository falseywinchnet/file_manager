# Native preview paint source review

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
