# Focused CPU rejected-interval evidence retention

Date: 2026-10-01. Scope: `tests/macos_focused_cpu_experiment.inc` only;
no native host, CMake, runner or renderer changes.

## Prior negative result

**OBSERVED:** the downloaded `b5eeb04` evidence at
`.build/provider-b5eeb04-evidence/focused-cpu/experiment.log` contains interval 0
rejection, `caret draw cadence outside predeclared bounds`, and the protocol
line. It contains no measured gaps, CPU totals or window snapshots. The verifier
printed its window record only after the cadence and other throwing assertions;
the interval duration gate could also throw before any window was inspected.
The rejected run establishes neither comparable CPU intervals nor the actual
gap that failed. This correction cannot reconstruct missing prior observations.

## Capture and acceptance order

The fixture now owns four raw CPU clock-read records and a fixed nine-element
array of window observations. Each clock record preserves attempted status,
native return status/error, raw seconds/nanoseconds, checked-conversion validity
and converted nanoseconds. No clock error throws before the other boundary clock
can be read. Boundary order remains process/thread/wall at start and
wall/thread/process at end. Raw wall ticks, tick ratio and boundary-presence flags
are emitted; a missing boundary is not a valid elapsed interval.

After the end clocks, one synchronous main-queue callback disables attribution
on all nine views before collecting their attribution, model metrics, geometry
and state. Host JSON serialization follows those nine captures; metrics JSON and
all diagnostic output follow capture of every window. No event pumping, forced
painting, scheduling or console output is introduced between window captures.
The snapshots and diagnostic allocation/output are outside the measured clocks.
Native pointers are local callback borrows; the probe owns copied observations.

Raw output includes all thirteen attribution values, including first/last draw
wall times and min/max gaps; full model metrics; before/after host snapshots;
visibility, focus, deadlines, activation, bounds and scale; and the established
cross-interval count/extent references. Every raw record explicitly carries
`comparison_accepted=false`. Missing native views/capture failures are represented
explicitly, and metric serialization failures produce a failure record rather
than silently ending the remaining output. Incomplete evidence or stream failure
cannot produce an accepted interval.

All raw records are flushed before validation. The verifier consumes the saved
observations, not new live reads after logging. Setup/start-clock exceptions also
capture the available nine-window state and emit attempted/unavailable boundary
flags; a validation exception does not overwrite the original observation with
a later recapture. Interval summaries indicate that interval gates passed but
still carry `comparison_accepted=false`. Only the existing terminal marker after
all four intervals, successful native run and nine closes accepts the comparison.

No cadence, duration, count, quiet-window, extent, CPU-accounting, native-state
or saturation gate is relaxed. The 10–10.1 second interval, focused 18–19 draws
equal across focused intervals, wake bound, 430–630 ms draw gaps, hidden/cleared
quiet requirements and three-second settling remain unchanged. There is no
performance acceptance threshold or Core Animation attribution claim.

## Validation and exact review scope

**MEASURED portable validation:** Shadow Windows, MSYS2 GCC 16.2.0, C++20,
`-Wall -Wextra -Werror`. The generated local harness
`.build/focused-cpu-evidence-check.cpp` extracts the new records, clock conversion,
emitters, saved-window verifier and interval validation arithmetic. It uses a
synthetic native rectangle and clock, and the real `MetricsSnapshot` type and
serializer. Build command:

```text
g++ -std=c++20 -Wall -Wextra -Werror -I gui_forms/include .build/focused-cpu-evidence-check.cpp gui_forms/src/core/metrics/snapshot/metrics_snapshot.cpp -o .build/focused-cpu-evidence-check.exe
```

Execution returned 0. Seven cases cover a valid interval and rejection for
429,999,999 ns minimum gap, 10.101 second duration, hidden-window painting,
invalid end clock, incomplete capture and missing host counter. Every case's
output retained four raw clock records, nine raw window records and nine metric
records. The six failures retained their original or newly explicit failure
reasons; no case emitted the final comparison marker. Clock checks additionally
cover native failure/error preservation, invalid nanoseconds and conversion
overflow. The harness log is `.build/focused-cpu-evidence-check.log`.
An intermediate harness compile caught misleading same-line indentation in its
test setup; that harness formatting was corrected before the successful run.

Manual source review against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md` covers the two observation record types,
added probe storage/flags, clock reader, interval reset and clock boundaries,
three-pass snapshot capture, raw emitters, saved-state verifier changes,
failure-path evidence emission and protocol flushing. Types, initialization,
checked clock conversion, failure status and const borrows are explicit; behavior
uses named functions. Fixed observation storage is on the probe and is reset
outside measurement. String/JSON storage and copies occur only at diagnostic
boundaries; no logging or new allocation is added to a paint loop. The existing
deferred callback still borrows the synchronous run's stack probe with no new
callback or native-resource lifetime. No remaining house-style violations were
identified in the authored fixture scope. Unchanged host/runner code is not
newly certified by this review.

`git diff --check` passes. The portable harness does not compile Objective-C++ or
exercise native capture. Native compilation, all-nine snapshot observation and
another accepted-or-rejected interval remain pending coordinator macOS CI. The
prior cadence rejection remains negative evidence without invented missing data.
