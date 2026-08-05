# Historical Orchestrator task handoff

Status: **superseded by ADR-003**.

This file formerly prohibited implementation until one global Gate O0. The
grand architect replaced that sequencing rule with a negotiated bootstrap:
provider-independent Rust kernel work is open, while each real adapter waits
for its own project-local reply and fixture-backed contract.

Current work must follow:

1. `../AGENTS.md`;
2. `../../decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md`;
3. `DELIVERY_SEQUENCE.md`;
4. `../negotiations/README.md`;
5. the relevant project-local negotiation note.

This historical marker remains so an older task prompt cannot silently restore
the superseded paper-only gate.
