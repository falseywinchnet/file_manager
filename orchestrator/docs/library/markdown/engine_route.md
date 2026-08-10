# Engine search route

Status: **OBSERVED development adapter; installed Engine transport remains open**.

The frontend submits one orchestrator.search request without selecting catalogue or live lanes. The broker preserves the accepted fallback law and projects one result shape while keeping source and completeness explicit.

## Routing law

- A usable catalogue is preferred.
- Authoritative catalogue no-match is terminal.
- Unsupported, unavailable, stale, or quarantined catalogue outcomes may fall back to live traversal.
- Denied, invalid, budget-exceeded, timeout, and cancelled outcomes never route around authority or cost limits.
- A continuation cursor remains bound to its selected source lane.

## Current implementation claim

The typed Rust adapter supervises a separately built Go Engine over bounded JSONL for development conformance. Availability becomes available only when the connected peer advertises engine.live.query and contract.ORC-ENG-004. This does not establish installed Engine discovery, query/admin authentication, or platform service supervision.

## Authority and evidence locators

- [../decisions/ADR-007-ENGINE-ORCHESTRATOR-SEMANTIC-V0-AND-CAPABILITY-GATES.md](../../../../decisions/ADR-007-ENGINE-ORCHESTRATOR-SEMANTIC-V0-AND-CAPABILITY-GATES.md)
- [../decisions/ADR-008-CATALOGUE-INDEPENDENT-LIVE-SEARCH.md](../../../../decisions/ADR-008-CATALOGUE-INDEPENDENT-LIVE-SEARCH.md)
- [spec/contracts/ENGINE_AND_KOLMOGROV.md](../../../spec/contracts/ENGINE_AND_KOLMOGROV.md)
- [src/engine_port.rs](../../../src/engine_port.rs)
- [src/engine_jsonl.rs](../../../src/engine_jsonl.rs)
- [tests/live_search.rs](../../../tests/live_search.rs)
