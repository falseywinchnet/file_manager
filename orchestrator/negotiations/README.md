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
| Engine | [`../../engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 001-007 | semantic v0 and live query reconciled; contained M4 installed query/admin transport measured; native NTFS/ext4, million-entry, and other-platform evidence open |
| GUI.Forms | [`../../gui_forms/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../gui_forms/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 001–002 | named FM0 consumption manifest ready; application-backbone modal/help/transfer reply remains independent |
| SwiftEdit / GUI.Forms document view | [`SWIFTEDIT_DOCUMENT_VIEW_2026-10-01.md`](SWIFTEDIT_DOCUMENT_VIEW_2026-10-01.md) | D1 round 001; prepared-paragraph candidate 001 | bounded D1 model reviewed; private reuse/worker/paint proofs recorded; visual-only consumer accepts candidate public seam with provider source feasibility; byte budgets/retained replay/production backend gates open, installed/native availability pending |
| SwiftEdit / GUI.Forms dynamic document windows | [`SWIFTEDIT_DYNAMIC_WINDOWS_2026-10-01.md`](SWIFTEDIT_DYNAMIC_WINDOWS_2026-10-01.md) | intake/reply 001 | provider source audit confirms current public-route gap within audited scope; W1 lifecycle proposal awaits reconciliation; no runtime availability or implementation authorization |
| Kolmogrov | [`../../kolmogrov/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../kolmogrov/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 001 | awaiting Kolmogrov reply |
| File Manager | [`../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | 003–010 | all Frontend 001 opening predicates satisfied; settings/service v1 and contained installed Engine route measured; picker and later-provider rounds remain independent |
| Paint | [`../../paint/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../paint/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | intake 001 | waits for Paint architect interview; no runtime contract admitted |
| Text Editor | [`../../text_editor/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../text_editor/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | intake 001 | waits for Text Editor architect interview; no runtime contract admitted |
| Games | [`../../games/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../games/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | intake 001 | waits for Games architect interview; existing app/settings/help families only |
| External Games / portable GUI.Forms capabilities | [`GAMES_PORTABLE_CAPABILITIES_2026-10-01.md`](GAMES_PORTABLE_CAPABILITIES_2026-10-01.md) | intake 001 | concrete image/text-mask/PCM/LiveSurface needs; consumer owns asset migration, dedicated shared provider scope being coordinated; no new runtime availability |
| External Games / window cursor interaction | [`GAMES_WINDOW_CURSOR_2026-10-01.md`](GAMES_WINDOW_CURSOR_2026-10-01.md) | proposal 001; reconciled reply 003 | Games owns assigned Windows cursor-only development; precise status/window identity/lease lifecycle reconciled; implementation/native/SDK evidence pending, no runtime availability |
| Lexicon | [`../../lexicon/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../lexicon/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | intake 001 | proposed lexical-provider family; source/data/runtime gates open only on paper |
| Plugin Runtime research | [`../../plugin_runtime/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../plugin_runtime/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md) | import 001 | frozen input; factual corrections only |

## Ownership

The ledgers are dialogue records. `../spec/CONTRACT_REGISTRY.md` and accepted
contract files are canonical. Executable code may implement a fixture-draft
contract experimentally, but it may not promote its own shape to stable.
