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
