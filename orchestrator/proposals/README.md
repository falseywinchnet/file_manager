# Cross-project contract proposals

Subprojects place paper proposals under a directory named for the source project,
for example `engine/`, `gui_forms/`, `file_manager/`, `kolmogrov/`, or
`plugin_supervisor/`.

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

Proposal acceptance is not automatic. The canonical result is integrated into
`../spec/` and receives a contract ID/status update.
