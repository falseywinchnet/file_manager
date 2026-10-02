# Mac focused-caret CPU attribution diagnostic

Status: **OBSERVED authored instrumentation; native compile/run pending root CI**.
The coordinator assigned this bounded diagnostic after the current blank-editor
probe left process CPU unexplained by synchronous presentation wall timing.
No performance improvement or Core Animation causal attribution is claimed.

## Clock and private host boundary

Apple's [libc clock implementation](https://github.com/apple-oss-distributions/Libc/blob/main/gen/clock_gettime.c)
implements `CLOCK_PROCESS_CPUTIME_ID` with process user/system usage and
`CLOCK_THREAD_CPUTIME_ID` with calling-thread usage. The diagnostic calls the
checked `clock_gettime` interface, rejects failed/negative/invalid readings and
checks seconds-to-nanoseconds overflow. Both native scopes and the main-thread
interval use `CLOCK_THREAD_CPUTIME_ID`; elapsed wall time uses steady_clock.
No wall duration is subtracted from CPU time.

Two private Objective-C selectors enable/reset per-view attribution and return a
fixed thirteen-integer snapshot. They are not a public GUI.Forms API. Enable is
main-executor-only and refuses while a measured scope is active; disable retains
the snapshot. Fields are draw calls/CPU ns, collect calls/CPU ns, clock failures,
scope overlaps, saturation, extent mismatches, first/last draw wall ns, minimum/
maximum draw gap ns, and completed extent observations. Four fixed extent values
and source byte count detect per-draw variation. Counters saturate explicitly.

The default is disabled. `drawRetainedRect` and `collectDamage` have conditional
stack scope owners; disabled paths make **zero CPU-clock calls** and allocate no
diagnostic storage. Enabled scopes count CPU through cleanup/return, reject clock
regression and mark overlapping scopes. Any overlap invalidates the experiment,
so summing accepted draw and collection scopes does not double-count nested work.
Hidden windows must have no such work. Same-clock interval subtraction then
reports remaining main-thread CPU, including diagnostic clock overhead. It does
not name that remainder AppKit/CA CPU or measure physical presentation.

Process CPU is reported separately. Sequential process/thread boundary readings
have small skew, so process-minus-thread is not reported as exact other-thread
CPU. Deferred AppKit/CA work may occur outside the scopes, but current symbolized
running-thread stacks are required to attribute that remainder. No private Apple
API, forced transaction flush, image/presentation strategy change or new timer
frequency is introduced.

## Predeclared native protocol

Run the existing `gui_forms_macos_idle_visibility_tests` executable with
`--focused-cpu-attribution`. Root owns runner wiring; the existing default
visibility test and `--paint-cost-experiment` source/bounds are unchanged.

- Nine owned windows: one visible 640 x 480 blank multiline TextBox and eight
  hidden children. This is a provider fixture, not the full SwiftEdit control tree.
- Four ten-second intervals in focused/cleared/cleared/focused order, each after
  three seconds settling. Only model focus changes between modes. Native key
  window/application activation must remain present at interval boundaries.
- Actual TextBox 530 ms caret scheduling and native deferred painting run
  normally. No `displayIfNeeded`, manual caret ticks, `CATransaction` flush or
  in-interval snapshot/logging is used. A single queued boundary callback and
  constant opt-in attribution overhead are included in the interval.
- Accepted duration is 10.0–10.1 seconds. Focused intervals need 18–19 draws,
  equal draw counts across both focused intervals, matching paint/deadline counts,
  and successive completed draw gaps of 430–630 ms. Native timer wakes must be
  between draws and twice draws plus one: the existing host floors deadline delay
  to milliseconds, permitting an early wake/rearm rather than another paint.
- Bounds and backing scale must match across intervals; every focused draw must
  keep the same dirty/damage/destination/clip areas and source byte extent. Both
  focused intervals must match those extents, and accumulated model damage must
  equal the observed per-draw area times draw count.
- Cleared primary and hidden children must have zero draws, collections, native
  timer wakes, paint/deadline counts and attributed CPU. Layout, input and model
  focus/occlusion changes during intervals, display-link ticks, native faults,
  clock errors, overlapping scopes and saturated counters reject the experiment.
- Reports and validation run after both CPU end readings. First-party scopes
  cannot run while the main callback collects snapshots. Diagnostics are disabled
  outside measured phases. Failure disables all views and closes children before
  the primary; successful shutdown requires all nine close notifications.

The protocol is roughly 52 seconds plus reporting/setup. Root must provide a
sufficient command timeout. A rejected run is retained, not repaired by widening
bounds or quoting only its favorable intervals. One complete run has only two
focused and two cleared observations; repeat whole comparable runs before drawing
performance conclusions. There is no CPU performance acceptance threshold.

## Verification and exact house-style review

Read the complete `planning/PROGRAMMING_HOUSE_STYLE.md`. Exact authored scope:
new CPU aggregate/clock/scope helpers, private ivar/selectors and two opt-in scope
sites in `src/host/macos/application/macos_host.mm`; added includes/private
declarations/argument route in `tests/macos_idle_visibility_tests.mm`; and all of
`tests/macos_focused_cpu_experiment.inc`. The three source hashes are in
`MACOS_FOCUSED_CPU_ATTRIBUTION_HASHES_2026-10-01.csv`.

Reviewed explicit types/const inputs, initialized fixed storage, checked time
conversion and saturation, same-clock/nonoverlap accounting, main-executor
ownership, scope destruction and borrow lifetime, named callbacks, one pending
callback, report boundaries and failure cleanup. C++ scope owners cannot be
copied; their view field outlives the stack guard. No worker, lambda, retained
anonymous callback, hot-loop allocation or public ABI was added. Existing host
blocks and previous fixtures are not newly certified. No remaining violation was
identified in this authored scope; native compiler evidence remains pending.

**MEASURED portable check:** extracted the exact aggregate, clock conversion and
scope helpers into `gui_forms/.build/mac-cpu-attribution-check.cpp`, substituting
only a deterministic named clock at the foreign-call boundary. Shadow MinGW
GCC 16.2 C++20 compilation with `-Wall -Wextra -Wconversion -Wsign-conversion
-Werror` and execution passed. Cases cover disabled zero clock calls, CPU delta,
failed read, regressing clock, overlap detection, scope release, saturation,
extent mismatch/gap tracking, invalid nanoseconds and conversion overflow with
unchanged output. This does not compile Objective-C++, validate Apple clock
behavior, or execute the native experiment. Command after toolchain selection:

```text
g++ -std=c++20 -Wall -Wextra -Wconversion -Wsign-conversion -Werror gui_forms/.build/mac-cpu-attribution-check.cpp -o gui_forms/.build/mac-cpu-attribution-check.exe
gui_forms/.build/mac-cpu-attribution-check.exe
```

Source review corrected an initial NSString/std::string title assignment before
handoff. Native build/default regression/focused protocol execution are pending
coordinator CI. No Mac performance measurement was made on this Windows host.

## Independent integration review

The coordinator reviewed the three manifested source files against the house
style and verified all three SHA-256 values on 2026-10-01. Review covered scope
ownership and destruction, disabled-path behavior, checked clock arithmetic,
nonoverlapping attribution, callback lifetime within the synchronous native run,
interval ordering, failure cleanup, geometry/cadence gates and explicit limits on
interpretation. `Window::request_focus` returns success for an unchanged focus,
so the consecutive cleared intervals do not incorrectly fail setup. No remaining
authored-scope violation was identified. Existing host behavior is not certified
by this review.

The separately reviewed `tools/run_macos_focused_cpu.py` preserves raw output and
a revision-tagged receipt, requires both successful exit and the acceptance
marker, and retains rejection/timeout results. Python compilation passed on
Shadow. The workflow runs this optional diagnostic after the ordinary native
tests and preserves its evidence without treating rejection as SDK failure.
Objective-C++ compilation and native execution remain pending CI.
