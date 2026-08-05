# ORC-COM-001: common contract vocabulary

Status: **fixture-draft bootstrap subset; full shared vocabulary remains open**.

The executable bootstrap defines terminal status, typed errors, a bounded JSONL
request/reply shape, optional contract/version reference, deadline,
cancellation identity, and response budget. It now rejects incompatible
declared contract ranges, expired deadlines, and oversized requested responses.
Canonical cross-project identifiers,
generation/snapshot semantics, provenance chains, capability contexts, stream
sequence numbers, and partial-result detail remain negotiated work.

Current source projection: `../../src/common.rs`.
Current fixtures: `../../conformance/fixtures/bootstrap/`.

## Core 1.0 release projection

`orchestrator.release` is the executable `ORC-LIF-001` readiness projection for
the Core 1.0 profile. It returns target and build versions, a readiness Boolean,
and every named unsatisfied requirement. It reports `development` until the
contract, discovery, authentication, transport, independent-client,
cross-version, hostile-fixture, and provenance gates in
[`../../planning/CORE_1_0_RELEASE.md`](../../planning/CORE_1_0_RELEASE.md) pass.
A running lifecycle is not evidence that Core 1.0 is ready.

Requirements:

- paths are addresses, not file identity;
- exact, derived, inferred, user-authored, unavailable, and stale facts remain
  distinguishable;
- process-local ordinals never leave their generation without a durable mapping;
- every derived value identifies producer, version, source anchor, and source
  generation;
- unknown critical semantics fail closed;
- all collections are bounded, paged, streamed with backpressure, or explicitly
  rejected.

The bootstrap terminal vocabulary is `success`, `partial`, `invalid`, `denied`,
`unsupported`, `unavailable`, `stale`, `version_mismatch`, `budget_exceeded`,
`timeout`, `cancelled`, `quarantined`, and `internal_fault`. The JSONL laboratory
caps one frame at 1 MiB. This is a fixture projection, not the production local
daemon codec. `max_response_bytes` bounds the successful response; the bounded
terminal error envelope remains returnable when that requested success budget
is too small to contain an error.

ADR-009's installed-wire draft is separately versioned as
`orchestrator.local` 0.1. Each message uses `ORC1`, a four-byte unsigned
big-endian payload length, and a nonempty UTF-8 JSON payload capped at 1 MiB.
The first frame is a credential/version hello; only an authenticated connection
may submit ORC-COM requests. Unix discovery and framing are executable, but the
wire remains pre-stable pending concurrency, hostile, independent-client, and
Windows projection evidence.

Open work: identifier equality/lifetime, binary/text normalization, timestamp
vocabulary, capability context encoding, field-presence rules, provenance,
stream backpressure, and the complete cross-provider error detail taxonomy.
