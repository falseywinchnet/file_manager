# Orchestrator ↔ Paint interface negotiation

Status: **intake 001; waits for Paint architect interview**.

Participants: future Paint consumer and Orchestrator integration authority.
Provisional families: `ORC-APP-001`, `ORC-PCK-001`, `ORC-HLP-001`,
`ORC-XFR-001`, `ORC-HND-001`, and `ORC-SET-001`.

Canonical source proposal:
[`../../orchestrator/proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md`](../../orchestrator/proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md).

## Orchestrator intake 001

Paint is asked to define after its interview:

- stable application/profile identity and configuration namespace;
- open/save-as/import/export picker purposes, filters, hidden policy and
  fallback requirements;
- local help package/topic identity and missing-provider behavior;
- file/PNG/native-composite transfer flavors, lazy payload bounds and handler
  associations;
- settings Paint owns versus Orchestrator stores;
- lifecycle/restart behavior while a picker/help/transfer operation is active;
- capabilities Paint can omit from its first vertical slice.

Orchestrator does not own canvas bytes, tool state, document parsing/writing,
pointer-motion transfer, modal rendering or GUI.Forms controls.

## Paint reply 001

Status: **blocked by design intentionally; Architect Interview Rounds A/B have
not run**.

The Paint sibling appends its labelled reply here after PA0 closes. Planning
answers do not freeze a wire/API; Orchestrator reconciles them with GUI.Forms and
File Manager picker negotiations before implementation.

## Architect surface correction 002

Status: **GIVEN under ADR-013**.

Paint has no persistent left/right panels. Detailed Color, Help and every other
secondary tool are Paint-owned popup dialogs or a bounded greaseboard overlay.
Orchestrator registers application/help/settings meaning but never supplies
dialog controls/layout. The Color dialog plan is
[`COLOR_DIALOG.md`](COLOR_DIALOG.md).
