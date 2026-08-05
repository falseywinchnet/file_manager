# Accepted architecture decisions

Decision records are authoritative only when their status is **accepted** and
their owner approval names the grand architect's direction. Candidate research
and existing implementation do not silently create decisions.

| Record | Status | Scope |
|---|---|---|
| [`ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md`](ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md) | accepted | Go engine object/binding model, root/volume topology, immutable-generation storage spine, observation/publication policy, structures, ranking baseline, migration, API projections, external/network placement, large-directory mode, and initial performance constitution |
| [`ADR-002-ORACLE-CONTRACT-AUTHORITY-AND-PROGRAM-SEQUENCE.md`](ADR-002-ORACLE-CONTRACT-AUTHORITY-AND-PROGRAM-SEQUENCE.md) | superseded | Historical paper-first Oracle name and global dependency gate |
| [`ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md`](ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md) | accepted | Orchestrator naming, integration authority, project-local ABI negotiation, user-scoped bootstrap kernel, CLI authority, fallbacks, and stub boundaries |
| [`ADR-004-FRONTEND-001-LOCATION-AND-OPENING-GATE.md`](ADR-004-FRONTEND-001-LOCATION-AND-OPENING-GATE.md) | accepted | `frontend/` ownership, Design DNA 006 authority for Frontend 001, GUI.Forms go-ahead plus architect start gate, and fixture-backed service boundaries |

Each record retains its alternatives, consequences, reversal path, and
unresolved implementation edges. Engine workers consume ADR-001 through
`../engine/docs/ARCHITECT_HANDOFF_001.md`, the retained Kolmogrov/semantic
boundary through `../engine/docs/ARCHITECT_HANDOFF_002.md`, and current
integration authority through ADR-003 and the engine-local negotiation ledger.
