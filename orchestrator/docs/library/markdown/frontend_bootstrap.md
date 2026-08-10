# Frontend bootstrap contract

Status: **Stable ORC-FE-001 1.0 Core projection; GUI.Forms gate remains external**.

orchestrator.frontend.bootstrap returns one immutable authority snapshot. It proves the Orchestrator side of startup without manufacturing an aggregate permission to begin frontend implementation.

## Snapshot contents

- Core release identity, readiness, provenance digest, lifecycle, and runtime health.
- Complete contract and availability catalogues.
- Normal Orchestrator route plus registered but currently ineligible direct-Engine fallback state.
- Shutdown and supervisor-restart eligibility.
- An Orchestrator-owned gate plus attributed GUI.Forms and architect-direction evidence.

## Cache and authority law

- The cache key is daemon instance identity plus lifecycle and configuration generations.
- A changed instance or generation replaces the snapshot; records from different keys are never merged.
- A disconnected snapshot may remain visible only as stale display state.
- Provider absence outside the Core bootstrap profile does not block the Orchestrator gate.
- Frontend 001 still requires the independent GUI.Forms go-ahead and explicit architect direction.

## Authority and evidence locators

- [spec/contracts/FRONTEND_AND_GUI_FORMS.md](../../../spec/contracts/FRONTEND_AND_GUI_FORMS.md)
- [../decisions/ADR-006-ORCHESTRATOR-CORE-1-0-FRONTEND-BOOTSTRAP.md](../../../../decisions/ADR-006-ORCHESTRATOR-CORE-1-0-FRONTEND-BOOTSTRAP.md)
- [../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md](../../../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md)
- [src/kernel.rs](../../../src/kernel.rs)
- [conformance/clients/cpp/README.md](../../../conformance/clients/cpp/README.md)
