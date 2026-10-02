# Private focused-CPU deadline-to-draw trace

Date: 2026-10-01. Development diagnostic only. Assigned scope is diagnostic
helpers/fields and instrumentation seams in `src/host/macos/application/macos_host.mm`,
`tests/macos_focused_cpu_experiment.inc`, and this receipt. No timer, caret,
presentation, CMake or runner behavior change is intended. Native execution is
pending coordinator integration; portable checks do not establish native results.

## Retained negative evidence

**OBSERVED:** provider `5e553b8`, native run 36953730211, Mac raw evidence at
`C:/Users/Shadow/notepad/.build/provider-5e553b8-evidence/mac/focused-cpu/experiment.log`.
Interval 0 lasted 10.002803084 seconds and recorded 16 draws, 16 scheduled wakes,
16 fired deadlines and 32 collections. The completion gaps were
531.436333–678.841834 ms; their 15-gap mean was 607.882922 ms. The count gate
rejected first, and the maximum gap independently exceeds the existing bound.
Eight hidden windows had zero interval draws/wakes/collections/attributed CPU.
All nine scalar clock/overlap/saturation/extent-failure fields were zero.

**MEASURED arithmetic from the raw receipt:** process CPU 57.417000 ms;
main-thread CPU 32.991417 ms; draw scope 8.000584 ms; collection scope 2.653296 ms;
remaining main-thread CPU 22.337537 ms, or 67.7071% of the main-thread total.
The process total corresponds to 0.574009% of one core. This rejected interval
is not an accepted four-interval comparison. Remaining CPU is outside the two
scopes, not attributed to Core Animation. Process-minus-main is a difference
between separate clocks with sequential boundary reads, not exact other-thread
CPU attribution. No unavailable prior timestamps are reconstructed.

**OBSERVED source ordering:** TextBox rearms at `FrameClock::now() + 530 ms` in
its frame callback; late polling can shift subsequent deadlines. Mac timer arms
use remaining time to the model deadline, floored milliseconds with a 1 ms
minimum and 250 microseconds of dispatch leeway. Collection polls the scheduler
before taking damage; paint invalidation can also post a collection callback.
The 32/16 collection/wake ratio is consistent with that route, but the old
receipt does not identify collection origins. Drawing rearms the remaining model
deadline; it does not independently schedule another 530 ms from presentation.

## Trace ownership and interpretation

The existing private 13-field CPU snapshot method and layout are unchanged.
A separate `cpuCausalTraceSnapshot` returns an owning, fixed wire image: one
13-value header followed by space for 256 13-value rows. This capacity is a
fixture diagnostic bound, not a product limit or architecture decision.

The view owns a `unique_ptr<MacCausalTrace>`, allocated/reset only when the
existing private CPU diagnostic is enabled, before measured clocks begin.
Disabled seams do not allocate, log or sample any additional clock. Enabled
events use fixed named records with no per-event allocation or I/O. Main-executor
access needs no new locks or callbacks. Only scalar values cross the snapshot
boundary; no native pointer, model pointer or callback borrow is retained.

Events are timer arm, timer callback entry, scheduler poll, draw entry and draw
exit. Rows carry event time, arm serial, desired model deadline, requested relative
timer delay, timer callback serial, poll serial, fired-deadline count, returned
next deadline, linked poll serial, draw serial, pending count and flags. The
protocol line in each log defines event kinds and flag bits. Header fields name
overflow, ambiguity, invalid clocks, serial exhaustion, unpaired/incomplete draw
and pending deadlines. Overflow preserves the first 256 rows without overwriting
them and prevents accepting a complete causal trace.

The first timer may have been armed before capture: enabling seeds the current
model deadline without rearming or inventing an arm timestamp. Arm serial zero
and the inherited-arm flag make that boundary explicit. A wake's arm serial is
the latest arm observed by the host, not a completion token supplied by
libdispatch. The requested relative delay is not an exact kernel due timestamp.
Repeated arms and early callbacks are recorded without changing the timer.

A timer callback scope associates its synchronous poll with that wake. Other
polls explicitly carry the untimed-poll flag and wake zero. Polls with zero fired
deadlines preserve any existing pending draw association, including early wakes.
One fired deadline links the next draw to that poll. Multiple fired deadlines
before a draw mark ambiguity and clear the single-poll link instead of selecting
one arbitrarily. Draw entry transfers the pending association to the in-progress
draw; any later poll remains pending after that draw exits. Nested timer scopes
restore the outer wake identity and mark ambiguity. Unpaired/nested/incomplete
draws cannot establish an accepted causal pairing.

Time conversion enforces the native FrameClock nanosecond period at compile time,
rejects negative ticks, checks event-time regression, checks sequence exhaustion
and checks pending-count addition. No unsigned subtraction is used to hide an
early wake. Analysis can compare wake time with the desired deadline, poll time
with draw entry, and entry with exit only where the corresponding validity and
association fields permit it. Poll time is the existing scheduler `now` argument;
it precedes callback work, not a timestamp of completed invalidation. Arm, poll
and draw-entry records reuse existing clock samples; wake entry and draw exit
add opt-in wall-clock samples. That overhead remains part of this diagnostic
workload. It cannot establish OS-versus-main-queue causation or CA CPU usage.

## Capture and unchanged acceptance gates

The fixture reads traces after end clocks and after disabling all nine views,
alongside the existing attribution/model snapshots. Raw traces, CPU clocks and
all nine window records are emitted before validation. Every raw row remains
`comparison_accepted=false`; a clear trace header is not comparison acceptance.
Incomplete/ambiguous traces refuse diagnostic acceptance. Event-kind counts must
also match scalar wakes, collections and draw entry/exit counts.

The existing duration, cadence, draw/wake count, equal-focused-count, quiet-window,
geometry, damage, saturation and same-clock CPU-accounting gates are unchanged.
No forced painting, catch-up blink, relaxed threshold or interval extension was
introduced. Only the existing terminal marker after all four intervals and nine
closes accepts the comparison. Trace failures retain raw evidence first.

## Validation and source review

**MEASURED portable validation:** MSYS2 GCC 16.2.0 on Shadow Windows, C++20,
`-Wall -Wextra -Werror`. `.build/macos-causal-trace-check.cpp` extracts the actual
host trace declarations/helpers and substitutes a deterministic FrameClock.
Six named test groups pass: disabled/no events and zero clock reads; repeated
arms/early wakes/ordering and stable poll-to-draw links; untimed polls and multiple
pending deadlines; capacity overflow/negative time/regression/serial exhaustion;
unpaired or unfinished draws and new pending work during a draw; nested wake
scope ownership. Early zero-fire polls also preserve an already pending link.

`.build/focused-cpu-causal-evidence-check.cpp` extracts the actual fixture
emitters and saved-data validation, uses the real MetricsSnapshot serializer and
synthetic native values, and passes ten cases. They cover a valid interval plus
cadence, duration, hidden activity, clock failure, incomplete snapshot, missing
counter, trace overflow, trace ambiguity and trace/scalar count disagreement.
Every case retains four clocks, nine windows, nine metric records and nine trace
headers, and none emits the final comparison marker. Output is retained at
`.build/focused-cpu-causal-evidence-check.log`.

Clang 22.1.8 checks the matching Objective-C++ snapshot/category declarations
and fixed-array return with `-fobjc-runtime=macosx-10.13 -fobjc-arc -fsyntax-only
-Wall -Wextra -Werror` in `.build/macos-causal-snapshot-syntax.mm`. This is a
minimal declaration check on Windows, not native AppKit compilation or execution.
The initial shell invocation split an unquoted runtime argument; quoting that
argument produced the successful syntax check. `git diff --check` passes.

Manual review against the complete `planning/PROGRAMMING_HOUSE_STYLE.md` covers
the new host trace types, named bookkeeping functions, wake scope, view-owned
pointer, enable/disable/snapshot methods and the poll/arm/wake/draw seams; and
the fixture's private category, copied trace storage, capture, raw emitter,
trace-status/count validation and protocol line. Explicit types, initialization,
const inputs, named ownership, checked conversions, record publication order,
failure flags and fixed repeated-event storage were reviewed separately from
tests. Snapshot packing uses the documented private boundary layout; ordinary
bookkeeping uses named fields. No remaining violations were identified in that
authored scope. Unchanged timer expressions, native blocks, core scheduler,
TextBox and other legacy code are not certified by this review.

Native Objective-C++ build, the actual trace sequence and the next accepted or
rejected interval remain pending coordinator macOS CI. The prior negative result
and its unattributed CPU share remain observations, not a diagnosed cause.
