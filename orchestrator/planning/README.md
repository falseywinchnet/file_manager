# Orchestrator planning index

Status: **active master specification alongside the bootstrap kernel**.

| File | Purpose |
|---|---|
| `MASTER_SPECIFICATION.md` | Contract doctrine and contribution workflow |
| `AUTHORITY_AND_PROCESS_MODEL.md` | Runtime topology, trust boundaries, and failure behavior |
| `HIVE_AND_SETTINGS_MODEL.md` | Semantic memory, provider projections, registry/configuration ownership |
| `STORAGE_REFERENCE_WINDOWS_REGISTRY.md` | Microsoft Registry hive backing/log comparison and its deliberately limited implication |
| `CONFORMANCE_AND_VERSIONING.md` | Compatibility vocabulary, fixtures, generators, and gates |
| `DELIVERY_SEQUENCE.md` | Parallel bootstrap, per-edge integration gates, and the GUI.Forms-triggered Frontend 001 opening |
| `FUTURE_THREAD_HANDOFF.md` | Superseded historical handoff retained for provenance |

Canonical contract semantics live under `../spec/`, not in implementation notes
or generated language bindings.

Runtime bootstrap source lives under `../src/`. Project-to-project dialogue is
indexed by `../negotiations/README.md`.
