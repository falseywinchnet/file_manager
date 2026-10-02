# Focused CPU causal-trace result at 520f31b

Date: 2026-10-01. Status: **rejected comparison, usable raw observations**.
This record adds no host/fixture changes, accepted performance result or product
availability claim.

## Evidence and independent parsing

**OBSERVED coordinator report:** all three native jobs in
[run 36956152036](https://github.com/falseywinchnet/file_manager/actions/runs/36956152036)
passed. Their success does not accept the separately reported CPU experiment.
The downloaded receipt identifies source
`520f31b8d5d248c16ad0112cc48fc359027b6565`, exit 1, status `rejected`, and
`accepted_marker: false`. Interval 0 rejected with
`normal ten-second caret counts diverged`; later comparison intervals did not run.

The recording chat independently read these local bytes:

| Artifact under `.build/provider-520f31b-evidence/focused-cpu/` | SHA-256 |
|---|---|
| `experiment.log` | `56b6dfe1917f09c1b9242c6fc561d9f760b0aab0b730f1682650575af7a77398` |
| `receipt.json` | `b7fcaa38916389cd8a7f3153ae73e99aa466389b29b872f06639578a7ba0ba2e` |

**MEASURED independent parse:** named PowerShell helpers split key/value records
without treating every field as numeric. Only arithmetic operands were converted
explicitly to decimal integers; `comparison_accepted=false` remained text. Maps
reject duplicate wake, poll, entry and exit identities. Every draw entry was
joined to its same-ID exit, the same linked poll at both boundaries, exactly one
fired deadline and exactly one unique wake. All 17 associations were complete
and unique. This reproduces the coordinator's corrected results; it does not
reuse the coordinator's initially failed generic integer cast.

The primary trace has 136 rows: 51 arms, 17 wakes, 34 polls, 17 draw entries and
17 exits. Its overflow, ambiguity, clock-failure, serial-exhaustion, unpaired,
unfinished-draw and pending-deadline header fields are zero. All eight hidden
windows have empty traces and independently checked zero interval draw, wake,
collection and attributed CPU activity. A clear header is evidence integrity,
not acceptance of the comparison.

## Timing observations

The interval lasted 10.001287125 seconds, with 17 draws, 17 wakes and 34
collections. Draw-completion gaps ranged from 531.257334 to 670.082833 ms.
The 17-draw count fails the unchanged 18–19 gate; the maximum gap also exceeds
630 ms. Neither failure is waived because native correctness jobs passed.

| Difference across 17 unique associations | Minimum ms | Mean ms | Maximum ms |
|---|---:|---:|---:|
| Desired model deadline to timer callback entry | 5.732750 | 77.626145 | 152.769750 |
| Scheduler poll timestamp to draw entry | 0.246000 | 5.979652 | 18.198500 |
| Draw entry to draw exit | 0.303292 | 0.533970 | 0.780458 |

**OBSERVED interpretation:** the largest measured delay component is before the
timer callback. The raw mean deadline-to-wake delay is materially larger than
either subsequent component in this particular interval. Rendering duration
alone does not explain these gaps. TextBox's existing callback rearms from its
current time plus 530 ms, so late polling can shift later deadlines.

These are wall-time differences, not CPU costs. The desired deadline is the
model deadline associated with the latest host-observed arm; libdispatch supplies
no arm-completion token. The first arm may predate capture, as represented by the
inherited-arm flag. Poll time precedes its callback/invalidation work, so
poll-to-draw includes that work as well as deferred native drawing. The trace
cannot separate OS timer delivery, main-queue scheduling, blocking or contention.
It does not establish which of those mechanisms caused the lateness. The enabled
diagnostic's own overhead is included. No user-machine or macOS-version inference
is drawn from this CI sample.

## CPU observation, separate from wall latency

| Clock/scope | Interval CPU ms |
|---|---:|
| Process | 62.024000 |
| Main thread | 35.428666 |
| Draw scope | 9.106125 |
| Collection scope | 2.020999 |
| Remaining main thread: main minus those two scopes | 24.301542 |

The remaining share is 68.592879% of main-thread CPU. Process and main-thread
totals correspond to 0.620160% and 0.354241% of one core over the observed wall
interval. The process-minus-main difference is 26.595334 ms, but the clocks use
sequential, slightly skewed boundaries; that difference is not exact other-thread
CPU attribution.

The residual means only CPU outside the two recorded scopes, including any
diagnostic work outside them. It is not evidence that Core Animation consumed
24.301542 ms. Wall-time phase durations cannot be subtracted from CPU totals.
The rejected, instrumented blank-textbox observation neither reproduces nor
explains the user's reported 7% CPU workload. There is no accepted focused versus
cleared comparison from this run, and no justified performance trend against the
earlier rejected 16-draw run.

## Candidate next CPU investigation

**CANDIDATE, not implemented by this record:** run a separate profiling workload
to identify executing main-thread stacks outside `drawRetainedRect` and
`collectDamage`. Preserve this comparison executable's four-interval protocol
and gates. A profiling-only run should hold the same blank primary/hidden-window
configuration for 120 seconds focused and 120 seconds cleared, after settling,
then repeat in reverse order if the initial stack sample count supports analysis.
Record source, symbols, OS/hardware, fonts, bounds/scale, focus/key/visibility,
actual elapsed time, CPU clocks and actual caret/draw counts for each span. These
long holds need separately assigned fixture scope; they are not a request to
extend the current acceptance interval or synthesize extra caret work.

Use Instruments CPU Profiler where supported by the selected host/toolchain;
record instrument/version availability before running. Time Profiler is a
fallback with its sampling limitations stated. Apple recommends CPU Profiler for
CPU optimization and explains periodic sampling aliasing in Time Profiler.
[Apple: Optimize CPU performance with Instruments](https://developer.apple.com/videos/play/wwdc2025/308/).

Capture symbolicated main-thread executing stacks with system libraries visible,
retain the full profile, and report sample count, self/inclusive weight and
unresolved symbols. Classify stacks by whether they are inside either measured
scope or outside both; do not label all outside stacks as CA. Correlate spans
using explicit clock/marker mapping rather than visually guessing the profiler
time range. Low CPU activity may yield too few samples: preserve that result and
increase separately declared recording duration if needed, without forced work.
Sampler weights are statistical evidence, not an exact reconstruction of this
run's 24.301542 ms residual. Compare profiling-enabled and profiling-disabled CPU
totals on the same workload to expose perturbation.

If the separate question is why wake delivery was late, use a separate Thread
State Trace/System Trace investigation of runnable/blocked intervals and queue
activity; an on-CPU stack profile cannot measure waiting as consumed CPU.
[Apple: Getting started with hang analysis](https://developer.apple.com/tutorials/instruments/getting-started-with-hang-analysis).
Keep that latency investigation distinct from CPU attribution. No OS-versus-queue
cause, CA attribution or explanation of the user's 7% follows before such evidence.

## Review and scope

Only this new Markdown receipt was authored. No host, fixture, runner, tool or
build source changed. The ad hoc read-only parser used named helpers, explicit
types/conversions, initialized maps/lists, checked unique joins and decimal
arithmetic; it is not a newly installed analysis tool. Manual review against
`planning/PROGRAMMING_HOUSE_STYLE.md` covers that parser and the receipt's units,
source attribution, failure status and candidate-versus-measured distinctions.
No remaining violations were identified in this authored scope. `git diff
--check` passes. No native rerun or profiler capture was performed by this chat.
