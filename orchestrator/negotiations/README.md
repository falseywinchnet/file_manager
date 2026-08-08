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
| Engine | [`../../engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 001-006 | semantic v0 reconciled; live-query reply 006 implemented on the development adapter with native NTFS/ext4, million-entry, and installed-transport evidence open |
| GUI.Forms | [`../../gui_forms/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../gui_forms/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 001–002 | consumption-manifest reply and application-backbone modal/help/transfer reply pending |
| Kolmogrov | [`../../kolmogrov/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../kolmogrov/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 001 | awaiting Kolmogrov reply |
| File Manager | [`../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 003–007 | live macOS Core 1.0 bootstrap ready and architect direction recorded; GUI.Forms FM0 remains the opening gate; picker and later-provider rounds remain independent |
| Paint | [`../../paint/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../paint/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | intake 001 | waits for Paint architect interview; no runtime contract admitted |
| Text Editor | [`../../text_editor/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../text_editor/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | intake 001 | waits for Text Editor architect interview; no runtime contract admitted |
| Games | [`../../games/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../games/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | intake 001 | waits for Games architect interview; existing app/settings/help families only |
| Lexicon | [`../../lexicon/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../lexicon/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | intake 001 | proposed lexical-provider family; source/data/runtime gates open only on paper |
| Plugin Runtime research | [`../../plugin_runtime/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../plugin_runtime/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | import 001 | frozen input; factual corrections only |

## Ownership

The ledgers are dialogue records. `../spec/CONTRACT_REGISTRY.md` and accepted
contract files are canonical. Executable code may implement a fixture-draft
contract experimentally, but it may not promote its own shape to stable.
