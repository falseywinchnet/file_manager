# Opt-in Mac paint cost surrogate

Status: independently reviewed; native compilation and experiment
execution pending. This is a diagnostic candidate, not an architecture decision
or a claim that focused SwiftEdit CPU is solved.

## Scope and execution

The existing `gui_forms_macos_idle_visibility_tests` executable accepts
`--paint-cost-experiment`. With no argument it retains the default short native
visibility regression. No CMake or rendering source change is needed.

The experiment creates one visible blank, unfocused multiline TextBox and eight
hidden owned TextBox windows using provider controls only. It does not read or
compile SwiftEdit source, expose a public host API, suppress application timers,
or change production rendering. There is no autonomous caret deadline in this
unfocused surrogate. The native application owns all nine model lifetimes;
probe pointers are non-owning, callbacks name their stack-owned context, and
one callback is outstanding at a time. The main native-window observation is
weak. Shutdown must report all nine windows closed before acceptance.

After three seconds of settling, four ten-second intervals run in ABBA order:

- A: invalidate a 2 by 20 logical rectangle in the blank editor, forcing command
  rebuilding, then request native presentation of that rectangle.
- B: request native exposure of the same rectangle with unchanged cached
  commands.

Both paths call the same damage collection and synchronous `displayIfNeeded`
seam. Identical pixels are inferred from unchanged static, unfocused control
content; this experiment performs no native pixel capture or comparison. Its
direct checks establish equal counts and extents and the intended chunk-rebuild
difference, not pixel equivalence. There are eighteen absolute-deadline
ticks per interval at 530 ms spacing, followed by the remainder of the ten-second
interval. Two seconds separate intervals. Expected duration is approximately
49 seconds plus setup and output; coordinator CI should impose a 90-second
process limit, separately from the unchanged default CTest timeout.

Suggested native command:

```text
gui_forms_macos_idle_visibility_tests --paint-cost-experiment
```

## Measurements and rejection rules

Each interval records steady-clock wall duration and `std::clock` process CPU
seconds, expressed as a percentage of one core. CPU includes the same per-tick
private host JSON serialization/parsing and validation overhead in both modes;
it is not reported as application-only CPU. Interval setup and final report
serialization occur outside the timed interval. Native asynchronous work that
runs during the interval remains included in process CPU, but need not occur
inside a synchronous phase timer.

Every tick verifies exactly one additional native draw and the same native
dirty area, frame-damage bounding area, CG destination and clip bounding areas,
and declared source-buffer byte extent across all ticks and modes. These are
extents submitted by the host, not bytes copied. Cadence more than 100 ms late,
interval duration outside 10.0–10.1 seconds, focus/occlusion changes, autonomous
deadlines, missing extents, or extra native draws reject the run.

At each interval boundary all nine windows report complete public metrics and
private host JSON. Separate `cost-phase` rows contain JSON deltas for calls and
nanoseconds in all six instrumented phases. Saturated or decreasing counters
reject subtraction. The main window must have eighteen paints, eighteen calls
in each synchronous phase, matching painted area, zero layout/deadline activity,
and eighteen rebuilt chunks in A versus zero in B. Hidden windows must have
zero paints, deadlines, rebuilds and synchronous phase activity. Native faults
also reject the run.

Only a final `cost-experiment=accepted-comparable-intervals` row establishes
that all four intervals and shutdown passed these checks. Earlier printed rows
may belong to a subsequently rejected run and must not be presented as a valid
comparison. Partial/rejected output should be retained with its rejection
reason. Two observations per mode are not a broad performance distribution;
repeat complete runs before attributing a small CPU difference to rebuilding.

## Review and current verification

Exact authored source scope:

- `tests/macos_idle_visibility_tests.mm`: direct standard includes, private
  existing-host method declaration, include and command-line mode dispatch.
- `tests/macos_paint_cost_experiment.inc`: complete diagnostic implementation.

Reviewed against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`: explicit
types, const inputs, named callbacks, fixed probe storage, model/native owner
lifetimes, allocation at reporting boundaries, deadline conversions, counter
subtraction guards, CPU-clock failure/ordering, rejection status, and shutdown
ordering. No lambdas, anonymous blocks, `auto`, arrow member access, or worker
creation was added. Per-tick JSON allocation is intentional measurement
overhead, disclosed and present in both paths; no per-pixel work is added.
No known house-style violation remains in this authored scope. Existing native
host and unrelated test code are not newly certified by this review.

`git diff --check` passed. Source was traced against Window paint, localized
invalidation and native exposure behavior. No full Objective-C++ build or native
experiment result is claimed on the Windows authoring host. Coordinator owns
native CI wiring, review, commit and push.

The coordinator independently verified both source hashes and reviewed the full
diagnostic source, private selector declaration, and mode dispatch. Review traced
explicit-damage painting with clean retained chunks, callback/model lifetimes,
matched tick scheduling and observation overhead, counter validation, and final
acceptance conditions. No defect was found in that authored scope.

Mac CI invokes `tools/run_macos_paint_cost.py` after normal build/tests and SDK
export. The wrapper imposes a 90-second process timeout and preserves raw output
plus a revision-stamped JSON receipt under `.build/native-macos-arm64/paint-cost`.
Acceptance requires both exit zero and the final comparable-interval marker.
The separate diagnostic step may fail without invalidating the already-tested
SDK; a green build therefore does not imply an accepted experiment. Consumers
must inspect its receipt. Timeout, launch failure and rejected comparisons remain
explicit results in uploaded evidence. The wrapper passed Python compilation and
language-neutral house-style source review; native execution is pending.

## First native attempt: rejected, retained

Native run `36881268179` at `4448dde` passed the ordinary build/test suites on
all three platforms. Its optional Mac experiment exited 1 after 10.749817833
seconds with `cadence exceeded declared lateness bound`; the receipt records
`status=rejected` and `accepted_marker=false`. No interval comparison was
accepted. This is negative experiment evidence, not a CPU result.

The coordinator added rejection-only interval, mode, completed-tick, elapsed-time,
maximum-lateness and host-snapshot diagnostics. The wrapper copies the rejection
reason into its receipt, and the initial header is flushed before the run loop.
All timing limits and acceptance checks remain unchanged. These authored hunks
were source-reviewed for explicit types, failure ordering and diagnostic failure
containment; the wrapper passes Python compilation. Native re-execution remains
required. The final source manifest includes these diagnostics.

## Second native attempt: precise rejection retained

Run `36883661867` at `275a133` completed the ordinary Mac job successfully.
The optional experiment again rejected the comparison: exit 1, 18.17112675
seconds of process wall time, no accepted marker. The new diagnostic identifies
interval 1 (exposure), three completed ticks, elapsed 2.25212375 seconds and
maximum lateness 131,070,458 ns, beyond the unchanged 100 ms limit. Interval 0
had completed, but its printed values are not an accepted ABBA comparison and
do not establish the cause of consumer idle CPU. Raw log and receipt are retained
in the run's `build-evidence-macos-arm64` artifact and locally under
`.build/provider-275a133-evidence/paint-cost/`. Two cadence failures on hosted CI
are a limitation of this experiment's usable evidence, not a reason to loosen
the declared gate or claim the blank-document CPU problem is solved.
