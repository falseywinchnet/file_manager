# Orchestrator conformance corpus

Status: **bootstrap fixtures executable; cross-language peers pending**.

`fixtures/bootstrap/` contains canonical JSON request/response examples for
version, Core 1.0 release readiness, status, incompatible contract versions,
expired deadlines, response budgets, semantic-fact stub state, and unknown
methods. Rust integration tests execute the same requests against the kernel
and compare semantic JSON equality.

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

`fixtures/local-wire-v0/` contains ADR-009 client/server hello and request
payloads. Tests add and inspect the fixed `ORC1`/big-endian-length header and
round-trip the typed messages. The all-zero credential is public fixture data,
not authentication material.

Later contract families add every terminal error, unknown critical fields,
version downgrade, fragmentation/coalescing, EOF, cancellation, backpressure,
quota, stale generation, capability denial, and partial-provider case required
by `../planning/CONFORMANCE_AND_VERSIONING.md`.

The JSONL fixture layer is inspectable laboratory interchange. It is not a
decision that bulk plugin results or the final local daemon transport use JSON.
