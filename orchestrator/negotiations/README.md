# Cross-project interface negotiations

Status: **active under ADR-003**.

Orchestrator places a negotiation note in each affected project's own
documentation tree. That location lets the producer or consumer answer beside
its implementation evidence without granting it authority to freeze the
program ABI.

## Round protocol

1. Orchestrator writes a numbered proposal with contract IDs, operations,
   semantics, limits, failures, cancellation, transport/ABI needs, and fixtures.
2. The project appends its reply in the reserved section. Each item is accepted,
   rejected, or counterproposed with a code/evidence locator.
3. Orchestrator records a reconciliation in the same file.
4. Agreed semantics and fixtures move into `../spec/` and `../conformance/`.
5. The next round addresses only unresolved or newly discovered edges.

Never rewrite an earlier round after another project has replied. Corrections
are appended with provenance.

## Active ledgers

| Project | Note | Round | State |
|---|---|---:|---|
| Engine | [`../../engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 001-006 | semantic v0 reconciled; required catalogue-independent fallback round 006 awaiting Engine reply |
| GUI.Forms | [`../../gui_forms/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../gui_forms/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 001 | awaiting GUI.Forms reply |
| Kolmogrov | [`../../kolmogrov/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../kolmogrov/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 001 | awaiting Kolmogrov reply |
| File Manager | [`../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 002 | Core 1.0 bootstrap direction accepted; transport/auth fixtures pending |
| Plugin Runtime research | [`../../plugin_runtime/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../plugin_runtime/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | import 001 | frozen input; factual corrections only |

## Ownership

The ledgers are dialogue records. `../spec/CONTRACT_REGISTRY.md` and accepted
contract files are canonical. Executable code may implement a fixture-draft
contract experimentally, but it may not promote its own shape to stable.
