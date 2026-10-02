# Bounded macOS CPU profiling workload

Date: 2026-10-01. Development experiment only; native capture and sample analysis
remain pending. This record does not accept a performance comparison or change
an architecture decision.

## Scope and protocol

**GIVEN:** the coordinator assigned the new workload include, its parent include
and CLI branch, this receipt, and the profiling runner. The coordinator owns
native build flags, matching dSYM production, workflow integration and artifact
upload. Host, scheduler, renderer and the existing focused-CPU comparator are
unchanged by this assignment.

**OBSERVED, source:** `tests/macos_cpu_profile_workload.inc` adds
`--cpu-profile-hold focused|cleared` to the idle visibility executable. Each
invocation constructs nine blank multiline TextBoxes in nine 640 by 480 windows,
with one visible primary and eight hidden owned windows. Boundary validation
requires bundled-font renderer readiness in every window, primary native
activation, exact requested focus/deadline state, stable dimensions and scale,
and no input, layout, focus or occlusion transitions during the hold. Focused
holds require caret paint/deadline activity; cleared and hidden windows must
remain quiet. The private scalar/causal diagnostic is never enabled.

Readiness is bounded to 15 seconds, followed by three seconds of settling and a
120-second hold. The duration validity bound is 120 through 130 seconds; there
is no comparator cadence/count relaxation or performance acceptance threshold.
Close has a five-second watchdog. The normal idle-visibility CTest path and
existing focused-CPU comparator are unchanged.

Process and main-thread CPU clocks bracket the hold. Raw clock reads, monotonic
wall ticks and ratio, and all nine before/after metrics and native states are
printed before completed-hold validation. Startup failures retain their failure
record and final invalid result; they do not fabricate completed boundaries.
Output consistently declares `comparison_accepted=false`.

The `CpuProfileHold` signpost uses subsystem `local.guiforms.cpu-profile` and
the Points of Interest category. Its begin precedes the CPU start reads; its
end follows the CPU end reads. One dispatch callback is scheduled at the hold
boundary; no polling or fixture output occurs inside the steady hold. Named
deferred contexts retain shared state. Native/model pointers remain borrowed
only while the synchronous native run is active. Returning from that run revokes
queued callbacks' model access.

## Runner and evidence boundaries

**OBSERVED, source:** `tools/run_macos_cpu_profile.py` attempts, in order, fresh
unprofiled focused, profiled focused, profiled cleared and unprofiled cleared
processes. It supports explicit `--build-dir`, `--executable`, `--evidence-dir`
and `--template`. It records the executable SHA-256 before the series and checks
it before and after each launched hold. Raw evidence is stored below a unique
attempt directory and is never deleted by this runner.

Preflight saves source revision/status, OS, architecture, hardware, selected
developer directory, Xcode and xctrace versions, actual template listing, and
actual record/export help. Exact listed `CPU Profiler` is preferred, with exact
`Time Profiler` fallback. An explicitly supplied existing template file is
hashed; an explicit named template must match a complete listing line. Required
command flags must appear in installed help. Discovery does not establish
recording permissions, CPU-counter support or useful signpost/sample capture.

The runner launches the application in an owned process group, then attaches
xctrace to that exact PID in its own group. Profiled readiness additionally
requires `os_signpost_enabled`; this is only a preliminary guard, not proof
that exported trace data contains a mapped hold. A missing recorder/template
does not silently turn a profiled case into a control. No privilege changes,
security-setting changes, global process termination or automatic retries occur.
Each hold/recorder pair is bounded to 180 seconds, xctrace recording to 150
seconds, TOC export to 60 seconds, and the overall attempt to 1080 seconds.
Termination uses three seconds after TERM, then three after KILL per live owned
group; cleanup may extend an expired work deadline by those bounded waits.

Artifacts include application and recorder streams, command/exit receipts,
recursive raw `.trace` bundles, TOC XML, observed TOC table attributes and the
original clock/window records. The coordinator must retain the exact executable
and matching UUID-verified dSYM with these artifacts. No generic parser guesses
sample columns, interval mapping or executing-thread semantics from unknown
schemas. Export failure is `export_failed`; recorder/signpost failure can be
`unavailable` or `unmapped`. Otherwise sample analysis remains `analysis_pending`.

The explicit analysis-state helper returns `too_sparse` only when an executing
main-thread sample count has actually been supplied and is below 200. This is
an inspectability guard, not statistical confidence. The current TOC-only export
does not supply that count and must not infer zero. Even a larger supplied count
leaves analysis pending. Waiting stacks do not establish executing CPU cost.
No OS-versus-queue, Core Animation residual or user-workload CPU attribution is
authorized by the boundary clocks alone. Successful workload execution is not
accepted sample attribution. Any incomplete workload makes the runner exit 1.

## Local validation and source review

**MEASURED, portable checks on Shadow Windows:** nine Python unittest methods
passed in `.build/test_macos_cpu_profile.py`. Cases cover exact template-name
discovery, raw CPU arithmetic, duplicate/missing/malformed boundary evidence,
invalid clocks/durations/modes/exits, unknown versus sparse sample states,
changed-binary and missing-template launch refusal, unknown/malformed TOC,
owned-group TERM/KILL escalation and stream cleanup, and four-run ordering with
an incomplete overall exit. POSIX signalling is mocked on Windows; this does
not prove Darwin permissions or xctrace attachment behavior.

Twelve extracted C++ validation cases passed in
`.build/macos-profile-validation.cpp`, built with the read-only adjacent MinGW
toolchain using `g++ -std=c++20 -Wall -Wextra -Werror -I gui_forms/include`.
The actual source records and validation functions are extracted by
`.build/build_profile_validation.py`; only AppKit rectangle/log types are
stubbed. Cases include both valid modes, early/late hold, CPU regression,
geometry/focus/input change, hidden/cleared painting, missing bundled fonts and
inactive focused caret. This does not compile or execute the Objective-C++
callback/host/signpost path.

A direct Windows runner invocation preserved an `unavailable` receipt with zero
runs at `.build/profile-portable-platform-check/attempt-b1c6b27f2e4445d5b06a8f87584801a6/receipt.json`.
No Mac processes were launched. The repository C++ spelling scanner found zero
candidates in an exact `.cpp` copy of the new include and in the extracted
validation harness. It does not accept `.inc` or Python paths. `git diff --check`
passed; the parent diff contains only the include and CLI dispatch branch.

**OBSERVED, manual house-style review:** reviewed the complete new include and
runner, the parent additions, and authored portable checks against
`planning/PROGRAMMING_HOUSE_STYLE.md`. Review covered explicit types, named
callbacks and retained state, raw-pointer borrow revocation, process/stream
ownership and cleanup, capture-before-serialization order, initialization,
clock validity and conversions, failure states, bounded/reused hash storage,
and absence of per-frame diagnostic allocation/output. No known remaining
house-style violations were identified in the authored scope. This is not a
compliance claim for inherited comparator/helper or host implementations.

Native compilation, GUI readiness, recorder permissions, trace span mapping,
sample schema, executing main-thread sample count and symbolication remain
unverified until the coordinator's macOS run. Retain failed attempts unchanged.

## Integration invocation and freeze

After the coordinator builds the dedicated Release executable with debug
information and verifies its dSYM UUID, the exact runner invocation is:

```sh
python3 -B tools/run_macos_cpu_profile.py \
  --build-dir .build/native-macos-arm64/cpu-profile-build \
  --evidence-dir .build/native-macos-arm64/cpu-profile
```

The target to build is `gui_forms_macos_idle_visibility_tests`; the default
executable is that target's `.app/Contents/MacOS/gui_forms_macos_idle_visibility_tests`
under the supplied build directory. An alternate executable can be passed
explicitly. Archive the entire evidence directory recursively, including hidden
trace contents, and the coordinator's executable/dSYM evidence.

Working-byte SHA-256 at handoff (line-ending conversion may change these):

| File | SHA-256 |
|---|---|
| `tests/macos_cpu_profile_workload.inc` | `ad2807b570ae4a7e7e9985cd3da16550a409ec92c5f9226b1950760e377f9be2` |
| `tests/macos_idle_visibility_tests.mm` | `8d47b3fb80cd5581065d3591fef8c844722c1f2bfd6372b5b07ededaf1c0d364` |
| `../tools/run_macos_cpu_profile.py` | `121c6744b6367c7bbb7c94aad2781939763d6085270f6cdad364106e00ed0f7f` |

Coordinator review matched those three hashes, reviewed the complete runner and
fixture plus the parent dispatch additions, and independently reran all nine
Python tests and twelve extracted C++ validation cases successfully. The optional
workflow builds a separate Release target with `-O3 -DNDEBUG -g`, prepared-text
and text-mask development options off, then checks executable/dSYM UUID equality
and preserves both with recursive trace evidence. Native profiling remains
unverified. Workflow configuration is opt-in through `mac_cpu_profile`; ordinary
push builds do not run the long holds.
