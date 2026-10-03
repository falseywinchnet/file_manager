# Private prepared-window session correctness

**OBSERVED implementation:** one explicitly owned worker constructs, uses and
destroys a `PreparedWindowShaper`. One admitted unique batch moves through the
queued/running/ready slot without an additional payload reservation. New desired
metadata revokes old authority but does not retire an occupied slot. Owner-side
adoption transfers an entire immutable batch; allocation failure, stale identity
and other refusals preserve the ready/input/output owners as documented.

The ledger now arbitrates one workspace/session claim across old A2 and private
batch sessions. Failed startup releases it; cancellation and worker exit alone
do not. Confirmed join releases it independently of old geometry retained by
commands or consumers. Session mutex, authority mutex, then ledger mutex is the
nested lock order. Shaping and notification execute outside those locks, and
the worker's temporary batch borrow ends before ready publication.

The session records startup, completion, failed/closed notification, exit and
join separately. Its shared named notification target survives a concurrent
close through the final worker join. This private lifetime rule does not change
public A2 callbacks. A typed failed notification closes further admission; it
does not prove eventual native UI delivery. Public service close still needs
explicit application-owned private-session close/join. Neither the native host
readiness bridge nor frontend adoption is enabled.

## Identity and resource review corrections

Review identified that a fresh authority could previously accept caller-reused
numeric identities. The private session now requires a fresh authority, obtains
session IDs from the same nonreusing atomic source as A2, and obtains a distinct
nonreused controller ID. Both are bound before thread creation and exposed in the
snapshot for constructing input keys. Failed starts burn assigned IDs. Wrong
session/controller keys refuse before advancing authority. Exhaustion neither
wraps nor changes output; local-counter fixtures test this without resetting
process-wide counters.

The lower-level manual batch-authority primitive still accepts caller-supplied
identities. This correction enforces the session path, not a general sealed
authority factory. Direct A2-versus-private issued-ID comparison is not a fixture;
their common allocator is established by source review.

**CANDIDATE scope:** this private stage owns one projection-controller lifetime
per session lifetime. A controller that survives session replacement still needs
its own negotiated factory/ownership contract; no public architecture or ABI is
selected by this finite stage.

The shaper's private owner-byte parameter now charges `sizeof(PreparedWindowSession)`
against the same 16 MiB controlled workspace allowance. It validates before
addition/subtraction and includes the fixed context in the reported peak. A
4096-byte owner fixture proves the exact added charge; a maximum-size request
refuses before allocation or overflow. Connection implementation allocations,
shared control blocks, thread stacks, native caches and RSS remain separate
unknown/measured categories, not silently included in the controlled tally.

## Validation

**MEASURED:** Release MSYS2 GCC 16.2.0, Shadow Windows, HarfBuzz enabled, Skia
disabled, at most two compiler jobs. The initial nine-suite integration passed
in 2.02 s. After the identity corrections, all nine suites passed again in
1.99 s; `WindowsFinalLastTest.log` preserves the final evidence. The suites are
prepared input, batch, shape, session, A2 service, raster, display, bounded shape
and bounded workspace. The final session suite took 0.11 s. These are correctness
durations, not preview or shutdown latency measurements.

Tests cover transferred ownership, full-key and foreign-ledger refusal, stale
completion, busy/latest desire, cancellation, three retained generations and
fourth refusal, allocation failure during shared adoption, injected startup,
initialization, worker and notification failures, and all owner-executor checks.
Deterministic gates exercise close during initialization, shaping and an
in-flight callback. The deliberately blocked fake callback is a lifetime test;
a native signal must remain nonblocking. Start-failure injection occurs before
the actual `std::thread` constructor, not inside the OS thread allocator.

Replacement fixtures use the same service and font bank while retaining old
geometry. Both IDs differ, old complete keys cannot become desired or adopt new
results, and both generations remain charged. A nonfresh authority refuses
without rewriting or revoking its existing certified batch. Separate A2 tests
exercise cross-family claim exclusion, close versus join, failed-setup cleanup,
idempotent retirement and retained geometry after worker retirement.

The session CTest has a 60-second timeout, verified in the generated test file.
Windows CI now includes the A2 service suite alongside the new session suite;
POSIX already includes A2. An independent OFF configure contained neither the
session nor batch-shaper source/test target. The normal SDK stays separate from
development-only source. Native acceptance of this session stage is pending.

## House style and remaining work

Source review is independent of tests and the supplemental ten-file spelling
scan. Root reviewed all three new session files, the ledger/session-claim and
identity allocator additions, A2 integration, shaper owner-context accounting,
new test hunks, and CMake/workflow registration against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`. The implementation sibling reviewed its
three session files and independently reviewed the root's owner-byte arithmetic
and registration. The Details-review sibling reviewed claim lifecycle and the
later identity additions. Reviewed source hashes are UTF-8 LF-normalized.
Whole-file hashes identify bytes; unchanged legacy implementations and shared
test support are not certified wholesale.
No concrete remaining house-style or correctness finding was reported in the
authored scope. Coverage and integration limits above remain explicit.

The native readiness investigation is in
`../../experiments/PREPARED_WINDOW_SESSION_2026-10-03.md`. A reliable UI drain,
D1/controller dispatch, immutable batch rendering and native/installed evidence
remain required before visible activation. Wrapping, tabs, long paragraphs,
interior anchors, caret/editing and accessibility remain open requirements.
