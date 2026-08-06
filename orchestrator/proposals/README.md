# Cross-project contract proposals

Subprojects may place unsolicited proposals under a directory named for the
source project. Orchestrator then routes the proposal into the affected
project-local `ORCHESTRATOR_INTERFACE_NEGOTIATION.md` round. Orchestrator-originated
requirements begin directly in that project-local note.

A proposal must state:

- user operation and why a boundary is required;
- provider and every consumer;
- authority/capability and privacy scope;
- request, response, event, and bulk-data shapes;
- identity and generation semantics;
- deadline, cancellation, backpressure, and quotas;
- errors, partial results, restart, and version mismatch;
- performance reason for the proposed projection;
- alternatives and reversal path;
- required golden and hostile fixtures.

Proposal acceptance is not automatic. The project replies in its local ledger;
Orchestrator records the reconciliation. Only the integrated result under
`../spec/` is canonical and receives a contract ID/status update.

Current proposal families include:

- [`application_backbone/`](application_backbone/) for first-party application,
  Document Picker, help, administration projection and transfer semantics;
- [`first_party_extensions/`](first_party_extensions/) for host-mediated file
  capabilities, Archive Viewer and Image Converter plugin proofs.
