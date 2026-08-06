# Orchestrator ↔ Text Editor interface negotiation

Status: **intake 001; waits for Text Editor architect interview**.

Participants: future Text Editor consumer and Orchestrator integration
authority. Provisional families: `ORC-APP-001`, `ORC-PCK-001`, `ORC-HLP-001`,
`ORC-HND-001`, `ORC-SET-001`, and a possible future bounded hint-provider
declaration.

Canonical source proposal:
[`../../orchestrator/proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md`](../../orchestrator/proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md).

## Orchestrator intake 001

Text Editor is asked to define after its interview:

- stable application/profile identity and settings namespace;
- open/open-many/save-as picker purposes, hidden-file default/session behavior,
  extensionless filters and fallback requirements;
- local help topic/provider identity;
- handler declarations and any explicit OS-default-registration request;
- which editor preferences are durable application settings;
- whether color hints are built-in or use a provider declaration, and the
  capability/provenance/quota required if external;
- lifecycle/restart behavior during picker/help/settings operations.

Orchestrator does not own document bytes, encoding/newline interpretation,
editing, search/replace, decoration spans, external-change handling or file
writes.

## Text Editor reply 001

Status: **blocked by design intentionally; Architect Interview Rounds A/B have
not run**.

The Text Editor sibling appends its labelled reply here after TE0 closes.
Planning answers do not freeze a wire/API; Orchestrator reconciles them before
implementation.

## Architect surface correction 002

Status: **GIVEN under ADR-013**.

Text Editor has no persistent left/right panels. Find/Replace, Characters, Help
and every other secondary tool are Text Editor-owned popup dialogs or a bounded
greaseboard overlay. Orchestrator registers application/help/settings meaning
but never supplies dialog controls/layout. The character utility plan is
[`CHARACTERS_DIALOG.md`](CHARACTERS_DIALOG.md).
