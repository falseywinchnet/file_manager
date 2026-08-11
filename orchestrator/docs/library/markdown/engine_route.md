# Engine search route

Status: **OBSERVED development and contained M4 installed adapters; other-platform promotion remains open**.

The frontend submits one orchestrator.search request without selecting catalogue or live lanes. The broker preserves the accepted fallback law and projects one result shape while keeping source and completeness explicit.

## Routing law

- A usable catalogue is preferred.
- Authoritative catalogue no-match is terminal.
- Unsupported, unavailable, stale, or quarantined catalogue outcomes may fall back to live traversal.
- Denied, invalid, budget-exceeded, timeout, and cancelled outcomes never route around authority or cost limits.
- A continuation cursor remains bound to its selected source lane.

## Current implementation claim

The typed Rust development adapter supervises a separately built Go Engine over bounded JSONL for conformance. The contained M4 product route instead validates the Engine's private host-bound ENG1 discovery, same-user peer, query/admin authority split and instance identity. Availability becomes available only when the connected peer advertises the required capability. This named installed route does not promote Windows/Linux transport or distribution signing.

## Authority and evidence locators

- [../decisions/ADR-007-ENGINE-ORCHESTRATOR-SEMANTIC-V0-AND-CAPABILITY-GATES.md](../../../../decisions/ADR-007-ENGINE-ORCHESTRATOR-SEMANTIC-V0-AND-CAPABILITY-GATES.md)
- [../decisions/ADR-008-CATALOGUE-INDEPENDENT-LIVE-SEARCH.md](../../../../decisions/ADR-008-CATALOGUE-INDEPENDENT-LIVE-SEARCH.md)
- [../decisions/ADR-019-CONTAINED-ENGINE-ADAPTER-AND-IDENTITY-BOUND-SERVICE-CONTROLS.md](../../../../decisions/ADR-019-CONTAINED-ENGINE-ADAPTER-AND-IDENTITY-BOUND-SERVICE-CONTROLS.md)
- [spec/contracts/ENGINE_AND_KOLMOGROV.md](../../../spec/contracts/ENGINE_AND_KOLMOGROV.md)
- [src/engine_port.rs](../../../src/engine_port.rs)
- [src/engine_jsonl.rs](../../../src/engine_jsonl.rs)
- [src/engine_local.rs](../../../src/engine_local.rs)
- [tests/live_search.rs](../../../tests/live_search.rs)
