# ORC-COM-001: common contract vocabulary

Status: **fixture-draft bootstrap subset; full shared vocabulary remains open**.

The executable bootstrap defines terminal status, typed errors, a bounded JSONL
request/reply shape, optional contract/version reference, deadline,
cancellation identity, and response budget. Canonical cross-project identifiers,
generation/snapshot semantics, provenance chains, capability contexts, stream
sequence numbers, and partial-result detail remain negotiated work.

Current source projection: `../../src/common.rs`.
Current fixtures: `../../conformance/fixtures/bootstrap/`.

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
daemon codec.

Open work: identifier equality/lifetime, binary/text normalization, timestamp
vocabulary, capability context encoding, field-presence rules, provenance,
stream backpressure, and the complete cross-provider error detail taxonomy.
