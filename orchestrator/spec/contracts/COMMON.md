# ORC-COM-001: common contract vocabulary

Status: **paper outline**.

This contract will define canonical identifier equality, generation and snapshot
semantics, availability/staleness, provenance chains, typed errors, deadlines,
cancellation, streaming sequence numbers, quotas, and partial results.

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

Open paper work: binary/text normalization, timestamp vocabulary, capability
context encoding, field-presence rules, canonical fixture notation, and the
complete error taxonomy.
