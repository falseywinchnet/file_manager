# Orchestrator conformance corpus

Status: **macOS Core 1.0 bootstrap/wire corpus executable; other platforms and
installed launchd lifecycle pending**.

`fixtures/bootstrap/` contains canonical JSON request/response examples for
version, Core 1.0 release readiness, status, incompatible contract versions,
expired deadlines, response budgets, semantic-fact stub state, and unknown
methods. They also pin explicit unsupported cancellation for atomic bootstrap
operations, must-understand extension rejection, and ordinary unknown-field
tolerance. The `frontend_bootstrap` pair pins the single-read `ORC-FE-001`
release/contract/availability/routing/control snapshot and its attributed
opening-gate projection. Rust integration tests execute the same requests
against the kernel and compare semantic JSON equality.

`fixtures/engine/semantic-v0/` contains the ADR-007 cross-project Engine
fixtures. Rust tests decode the frozen status/query vocabulary and prove that a
coverage-limited cached result, an authoritative no-match within a checked
generation, provider absence, and unsupported live traversal remain distinct.
These fixtures open fake-provider and adapter work; they do not claim a
production local transport or a connected Engine runtime.

`fixtures/engine/live-query-v0-draft/` contains the negotiating ADR-008
`ORC-ENG-004` projection. It exercises source-explicit progressive results,
permission-partial traversal, and fallback routing without claiming that the
Engine provider or contract version is frozen. Promotion waits for Engine reply
006 and the native containment/resource evidence named there.

`fixtures/local-wire-v1/` contains the current ADR-010 client/server hello and
request payloads. Tests add and inspect the fixed `ORC1`/big-endian-length header
and round-trip the typed messages. `fixtures/local-wire-v0/` retains the
pre-release 0.1 hello as an incompatible-major rejection corpus. All-zero
credentials are public fixture data, not authentication material.

`clients/cpp/` is the independent C++17 Unix consumer for O1.5. The Rust test
suite invokes CMake as a separate build, then uses the resulting executable to
read the atomic typed bootstrap state and its golden provenance digest, shut down, and
reconnect across two real daemon instances. It shares neither Rust types nor a
native ABI. Windows named pipes remain separate promotion work.

The cross-process CLI suite fills all four workers with unauthenticated sockets
while an authenticated client reads status and requests shutdown. This proves
control recovers within the one-second hello bound and shutdown interrupts
active peers. A separate saturation case fills all four workers and
eight pending slots, observes excess peers close, releases the held peers, and
then reads the non-authoritative rejection counters and verifies clean
authenticated shutdown. Long-run resource measurements and
later-family cases remain follow-on work, not unreported Core release gates.

`tests/local_hostile.rs` drives incompatible-major authentication, bad frame
magic, oversized declarations, abrupt payload EOF, and a valid request written
one byte at a time through the running daemon. It then proves the daemon still
answers status and shuts down. Bootstrap fixtures separately cover cancellation,
unknown-critical-field rejection, and optional-field tolerance. Long-run and
other-platform cases remain open rather than being implied by these results.

The adopted-listener and activation fixtures exercise ADR-011's persistent
supervisor socket, stale discovery, credential rotation, and bounded
rediscovery without mutating launchd state. `launchd-plist` output passes
`plutil`; a real installed LaunchAgent lifecycle remains the macOS release gate.

Endpoint fixtures additionally reject symlink publication files and
noncanonical modes. Stdio fixtures send a record larger than one MiB and invalid
UTF-8 before valid status/shutdown records, proving bounded drain and preserved
record alignment. A contained panic fixture proves any worker panic requests
whole-daemon termination rather than silent capacity loss.

Later contract families add every terminal error, unknown critical fields,
version downgrade, fragmentation/coalescing, EOF, cancellation, backpressure,
quota, stale generation, capability denial, and partial-provider case required
by `../planning/CONFORMANCE_AND_VERSIONING.md`.

The JSONL fixture layer is inspectable laboratory interchange. It is not a
decision that bulk plugin results or the final local daemon transport use JSON.
