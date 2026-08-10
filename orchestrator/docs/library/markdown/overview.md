# Service system overview

Status: **DECIDED topology; OBSERVED macOS Core 1.0 implementation**.

Orchestrator is the user-scoped, headless integration authority. The refactored service system separates command presentation, runtime hosts, transport/authentication, semantic dispatch, provider ports, and release evidence without changing the stable Core 1.0 contract meanings.

## Layer map

- main.rs is only the process exit boundary.
- cli.rs parses commands and renders human or structured projections.
- service/ owns stdio, bounded Unix hosting, and launchd activation.
- local_endpoint, local_session, and local_wire own discovery, authentication, and framing.
- kernel.rs validates and dispatches semantic operations over transport-neutral Request and Response values.
- engine_port and engine_jsonl keep provider selection behind a typed port; JSONL remains development-only.
- release.rs, contract.rs, and availability.rs publish readiness and capability truth.

## Authority boundary

- Orchestrator owns cross-project contract meaning, policy, routing, and availability truth.
- The Go Engine owns exact catalogue observations and live filesystem traversal implementation.
- GUI.Forms owns retained UI mechanics; Orchestrator has no GUI.
- The frontend owns interaction and composition; it does not define Orchestrator semantics.
- Plugin, settings, handler, hive, and semantic-fact implementation remains separately gated.

## Compatibility rule

The service refactor is private implementation work. ORC-COM-001, ORC-LIF-001, ORC-FE-001, ORC-CLI-001, the orchestrator.local 1.0 wire, golden fixtures, and independent C++ behavior remain the compatibility oracle.

## Authority and evidence locators

- [README.md](../../../README.md)
- [../decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md](../../../../decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md)
- [../decisions/ADR-006-ORCHESTRATOR-CORE-1-0-FRONTEND-BOOTSTRAP.md](../../../../decisions/ADR-006-ORCHESTRATOR-CORE-1-0-FRONTEND-BOOTSTRAP.md)
- [planning/AUTHORITY_AND_PROCESS_MODEL.md](../../../planning/AUTHORITY_AND_PROCESS_MODEL.md)
- [spec/CONTRACT_REGISTRY.md](../../../spec/CONTRACT_REGISTRY.md)
