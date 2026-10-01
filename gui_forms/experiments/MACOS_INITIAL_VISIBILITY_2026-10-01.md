# Mac host initial visibility correction and idle probe

Status: authored candidate; native compilation and execution pending.

## Evidence and bounded correction

**OBSERVED:** `src/host/macos/application/macos_host.mm` orders initially hidden
owned windows out before attaching their host sessions. `notifyOcclusion:`
discards events before attachment. `initializeHost` previously collected damage
and armed scheduling without first reconciling native visibility; the host and
model initially assumed unoccluded state.

The correction samples both native `isVisible` and the visible occlusion bit
immediately after accepted attachment, then dispatches the initial occlusion
state before the first post-attachment collection. Ordinary native show/hide
notifications remain responsible for subsequent transitions. No new blanket
timer suppression was introduced. Explicit UI timers retain the existing
portable rule allowing execution while occluded.

Private `hostJSON` diagnostics now include initial/current host occlusion and
counts of scheduled wakes, damage collection, native draws and display ticks.
These are main-thread scalar counters; they do not instrument every glyph or
allocate in the counted callbacks. Snapshot serialization remains explicit.

**HYPOTHESIS:** inconsistent initial hidden state contributes to SwiftEdit idle
CPU. This is not a measured causal result. Consumer native CI observed 2.375%
and 1.192% of one core in different runs against the same provider SDK; these
are not a before/after comparison. The latter native stack sample includes
TextBox paint/shaping and native image conversion, with most samples waiting.
It does not identify the responsible window or establish the owner's 7% cause.

**MEASURED follow-up:** SwiftEdit CI 36865627733, unchanged provider SDK, ran
the production nine-window topology. The focused interval used 0.142749 CPU
seconds over 5.01702 seconds (2.84529% of one core); clearing main-editor focus
used 0.0328 CPU seconds over 5.01246 seconds (0.654369%). Main-window counters
were nine paints/nine deadlines when focused and zero/zero when cleared. The
hidden Save picker remained model-unoccluded and focused, with nine paints/nine
deadlines in the first interval and nine paints/eight deadlines in the second.
Its painted area was 115776 in each interval. Other hidden windows had no
paint/deadline activity. Thus hidden work is observed, but its CPU contribution
and the correction's effect still require a controlled candidate comparison.

The initial sibling summary incorrectly reported all hidden windows idle after
reading truncated output; that summary is retracted. Complete evidence is
`C:/Users/Shadow/notepad/docs/performance/2026-10-01-mac-idle/controlled-first-attempt.txt`.
The primary-hide phase was refused by the public handle, so no hidden-primary
CPU comparison or complete probe pass is claimed.

## Native regression fixture

`tests/macos_idle_visibility_tests.mm` owns a primary window and an initially
hidden child with a focused TextBox. Named main-queue callbacks check:

- The hidden model is occluded immediately after attachment.
- An explicit UI timer still fires while hidden.
- Disconnecting that timer leaves no hidden caret wake deadline.
- Showing the child permits focused caret wake and drawing.
- Hiding the child suppresses wakes and draws over a quiet interval after
  native ordering callbacks have settled.
- Showing again restores visibility, and shutdown closes both owned windows
  without recorded native callback faults.

The fixture uses existing private Objective-C diagnostics and public host
callbacks; it adds no public ABI. It prints each stage's host snapshot. It is
a scheduling regression, not a CPU benchmark or proof about all nine SwiftEdit
windows. Native run duration is approximately 4.5 seconds; requested CTest
timeout is 15 seconds. The coordinator owns CMake and native CI wiring.

## Source review and verification

Reviewed authored scope against `planning/PROGRAMMING_HOUSE_STYLE.md`: the
initialization and diagnostic additions in `macos_host.mm`, and the complete new
fixture. Explicit types, named executable callbacks, initialized retained state,
non-owning model lifetime, one outstanding queued stage, separate observable
operations/assertions, and shutdown ownership were reviewed. No new anonymous
blocks, lambdas, `auto`, or arrow member access occur in the fixture. Existing
host blocks and unrelated legacy implementation were not audited or claimed
compliant. The JSON assembly follows the existing private diagnostics seam.

`git diff --check` passed. Native Objective-C++ compilation and execution are
unverified on the Windows authoring host. No SDK or release artifact was
published. All 21 frozen Windows A2 integration files still match their saved
SHA-256 manifest. Runtime show/hide behavior remains an explicit native test
condition; no speculative second correction was applied.
