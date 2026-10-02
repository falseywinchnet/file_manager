# Mac exposure fixture startup readiness

**OBSERVED:** native run 36947418184 at `7b260cf` compiled the Mac host and
diagnostics. The ordinary GUI.Forms suite passed 78/79 tests; the exposure test
failed after 0.26 seconds with `initial native frame was not painted`. The
idle/visibility fixture subsequently passed in 4.81 seconds. The build stopped
before SDK export, development text tests and optional CPU diagnostics.

Evidence: GitHub job 110652756898. The coordinator retained its raw log locally
under `.build/provider-7b260cf-evidence/macos-job.log`. This failure is not a
text-mask result or a CPU measurement.

**OBSERVED source ordering:** `run_macos` invokes `host_ready` before
`makeKeyAndOrderFront` and before entering the application run loop. The fixture
previously scheduled its first snapshot exactly 100 ms after that service
callback. Neither event establishes that a first application frame has painted.

**HYPOTHESIS:** shared-runner startup scheduling exposed that readiness
assumption. The failed log alone does not prove the cause or exclude a host bug.

The fixture now checks native visibility and an observed application paint before
the initial pixel snapshot. It schedules one named check every 50 ms, with a
five-second startup failure deadline. The snapshot cannot manufacture the first
application paint. Once ready, all existing occlusion/pixel-preservation checks
and minimize/restore/hide/show timings remain unchanged. The startup budget is
fixture setup, not a product latency acceptance threshold. Persistent missing
paint still fails and reports check and application-paint counts.

The coordinator reviewed the authored scope against
`planning/PROGRAMMING_HOUSE_STYLE.md`: two new initialized state fields, the
named deferred-check ownership helpers, the initial readiness/failure branch,
and the ready callback's scheduling replacement. Each queued check owns a
shared state reference, releases its heap context on entry, and uses native
window/view borrows only within the callback. Types, operation order and bounded
retry work are explicit. Unchanged legacy Objective-C blocks and other fixture
code are not newly certified. No product host or renderer source changes.

`git diff --check` passes. Objective-C++ compilation and native execution of this
correction remain pending CI; the Windows coordinator cannot claim a local Mac
test pass. The original failed run remains evidence.

The optional focused-CPU workflow step now records an attempt even when another
native test failed. Its runner reports a missing executable if compilation did
not finish. A diagnostic result cannot turn a failed SDK job into a passing one;
this separates measurement availability from unrelated regression acceptance.

## Deferred continuation lifetime correction

**OBSERVED:** run 36948765463 at `be0ed41`, Mac job 110656819770,
compiled the exposure fixture but aborted its execution after 1.95 seconds with
uncaught `std::bad_function_call`. The retained log is
`.build/provider-be0ed41-evidence/macos-job.log`, lines 1358–1360. The log has no
stack trace; it does not by itself identify the throwing call.

**OBSERVED source defect:** `check_initial_frame` owned a heap `shared_ptr<State>`
holder and passed a reference to it into `exercise`. Nested Objective-C blocks
captured that C++ reference, not an independently owned shared pointer. The holder
was deleted when the initial callback returned. The later restore/hide/show
blocks therefore borrowed a dead holder even though `main` still owned `State`.
The earlier startup review covered the initial callback but missed that its
reference escaped into the unchanged block chain. This corrects that review's
scope limitation; a surviving pointee does not make a destroyed owning handle a
valid borrow.

**MEASURED language-mechanism check:** local MSYS2 Clang 22.1.8 compiled a minimal
`const State&` parameter captured by a block with
`-x c++ -std=c++20 -fblocks -S -emit-llvm -o - -`. The emitted block stores the
referent pointer and invokes through that pointer, with no referent copy. This
confirms the reference-capture mechanism, not an AppKit execution result.
**HYPOTHESIS:** the dangling handle caused the observed callback exception. Native
rerun remains necessary to confirm that this fix resolves the CI failure.

The correction replaces all four nested blocks with named `dispatch_after_f`
stages. Each `ExposureStep` owns its own `shared_ptr<State>` and stage value;
the callback adopts the heap context immediately, and any successor copies its
state owner before the current context dies. Only one successor is queued.
Window/view borrows are reacquired by the existing fixture title at each native
stage and do not cross dispatch boundaries. A missing native window/view fails
the fixture. The five-second actual-paint readiness deadline, 50 ms readiness
poll, 150/250/50/100 ms transition delays, pixel comparisons and no-repaint
assertion remain intact. No product host, CMake, font, CPU or renderer changes
belong to this correction.

**MEASURED portable ownership validation:** on Shadow Windows, GCC 16.2.0 built
`.build/macos-exposure-context-check.cpp` with C++20 and
`-Wall -Wextra -Werror`; its executable returned 0. The harness extracts the
actual stage/context declarations, queue helper and dispatch callback from the
fixture, substitutes a deterministic queue/native-stage stand-in, drops the
original owner before dispatch, and checks all five invocations survive, one
successor owns state between invocations, close runs once and final ownership
expires. It does not compile Objective-C++ or validate AppKit pixels/timing.

Manual source review against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`
covers the added stage/context declarations, queue helper, extracted window
lookup, changed scheduling sites and named continuation/dispatch functions.
Types and read-only inputs are explicit; state fields initialize before dispatch;
shared ownership transfers/copies and native borrow extents are visible; no
anonymous executable blocks remain in this fixture. Allocation occurs once per
scheduled stage, outside pixel work, with at most the executing context and one
successor live. Existing exception handling records failure and invokes the
host-supplied close callable. No remaining violations were identified in this
authored scope; unchanged paint/pixel helpers and unrelated legacy source are not
certified. `git diff --check` passes. Full native compilation and execution remain
pending coordinator CI, separately from root's font-bundle provisioning work.
