# ORC-HND / ORC-CMD / ORC-INT contracts

Status: **outline; no bootstrap operation implemented**.

Orchestrator owns File Manager's internal handler/type registry and declarative
context-command registry. Plugins may propose bounded declarations; the
frontend renders all UI with house controls.

A command declaration identifies applicability, stable ID, label/localization,
icon reference, selection cardinality, required capability, invocation class,
timeout, and failure behavior. Registration does not grant file mutation.

Invocation classes must distinguish:

- read-only sandbox job;
- host-mediated open-with/external application launch;
- finite first-party File Manager operation;
- independently authorized external shell/application action.

Systemwide file-manager, handler, shell, or desktop integration is implemented
only by first-party platform adapters after explicit user/OS authorization. A
plugin cannot turn a declaration into OS registration or privilege.
