# Preview work follows visible demand

**OBSERVED baseline:** at main `e7bd9326`, selecting an ordinary file starts
`PreviewWork` even when the preview is collapsed, the inspector pane is hidden,
or Settings hides the workspace. A matching completion can then call
`Window::load_png` on the UI thread for a hidden preview. Existing generation
cancellation handles selection supersession but does not express visibility.

The frontend now keeps a UI-owned empty/deferred/pending/ready state. Hidden
selection retains intent without scheduling a read. Hiding a pending preview
advances the existing atomic cancellation generation. Showing deferred content
requests the current single selection once. Completed content survives collapse
and reopening without another request or image registration. Selection,
settings, and explicit refresh still invalidate it through the existing paths.
This does not add a filesystem watcher or deduplicate unrelated settings refreshes.

The named availability callback uses the existing public Window event. It
queries current effective visibility, including ancestor layout collapse;
deferred event payloads are not treated as current state. The application owns
its subscription token and revokes the borrowed target during destruction.
Workers observe only the existing atomic generation/stop token. Selection and
request setup enter empty before control changes; completion enters ready
before publication, preventing availability callbacks from restarting their own
work. Stable states return without walking ancestry.

**REJECTED:** the first implementation subscribed to ancestor
`visible_changed` events. The added layout-only collapse fixture failed because
ResponsiveTrackPanel preserves authored visibility while changing effective
availability. `WindowsRejectedLayoutLastTest.log` retains that failure. The
window availability subscription corrects it without a new toolkit API.

**MEASURED correctness:** Windows Release, GCC 16.2.0, the current GUI.Forms
application library built with HarfBuzz/Skia/transactional DIB disabled, at most
two compiler jobs. All 15 frontend CTest suites passed in 7.40 seconds, including
the interaction suite in 3.58 seconds. `WindowsLastTest.log` records the complete
run. Fixtures own generated temporary files. Windows reported error 1314 for
symlink fixture creation; those assertions were skipped, not passed.

The new test holds the ordinary worker after startup and counts queued jobs:
two hidden selections enqueue zero requests; each restored demand enqueues one.
It exercises automatic short-window collapse/growth, whole-pane collapse,
explicit toggles, Settings, disable/re-enable, multiple/empty selection, and
layout-only collapse/restoration. A valid obsolete PNG completion cannot publish
while hidden or after reopening the same file. Reopening completed content
preserves its ImageId and generation. Existing tests retain same-identity F5
refresh, native retained paint order, literal UTF-8 text, and unsupported-format
explanations. Unsupported content now waits for expansion before loading its
explanation.

This is deterministic scheduling/correctness evidence, not a latency benchmark.
Cancellation remains cooperative and cannot interrupt an already-entered OS
read or UI image admission. The fixture holds work before its read; it does not
inject a blocked OS read or count decoder invocations. Source review verifies
generation/visibility rejection precedes PNG admission. Large visible PNG
admission, thumbnails, broader formats, and fuller document viewing remain open.
Native macOS/Linux checks and a package containing this change are pending.

The complete house style was applied to the new state, subscription, demand
transitions, selection ordering, probes, and regression. Independent scoped
source acceptance and exact reviewed file hashes are in `SOURCE_REVIEW.md`.
Root verified those hashes and the three-file spelling scan (zero candidates).
No unchanged legacy-code compliance is claimed. The earlier test helper typo
(`PngPreviewReady` instead of the existing `ImagePreviewReady`) was corrected
before the accepted build.
