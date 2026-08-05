# Accepted architecture decisions

Decision records are authoritative only when their status is **accepted** and
their owner approval names the grand architect's direction. Candidate research
and existing implementation do not silently create decisions.

| Record | Status | Scope |
|---|---|---|
| [`ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md`](ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md) | accepted | Go engine object/binding model, root/volume topology, immutable-generation storage spine, observation/publication policy, structures, ranking baseline, migration, API projections, external/network placement, large-directory mode, and initial performance constitution |
| [`ADR-002-ORACLE-CONTRACT-AUTHORITY-AND-PROGRAM-SEQUENCE.md`](ADR-002-ORACLE-CONTRACT-AUTHORITY-AND-PROGRAM-SEQUENCE.md) | accepted | Oracle master contract authority, runtime ownership, legacy plugin-runtime disposition, waiting frontend, and GUI.Forms → engine/Oracle → frontend delivery order |

Each record retains its alternatives, consequences, reversal path, and
unresolved implementation edges. Engine workers consume ADR-001 through
`../engine/docs/ARCHITECT_HANDOFF_001.md` and consume the ADR-002
Kolmogrov/semantic/Oracle boundary through
`../engine/docs/ARCHITECT_HANDOFF_002.md`.
