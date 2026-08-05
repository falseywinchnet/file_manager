# Orchestrator ↔ File Manager interface negotiation

Status: **round 001 proposal retained; frontend reply deferred until the real
Orchestrator adapter follows Frontend 001**.

Participants: Orchestrator integration authority and the future C++ File
Manager frontend. Canonical families: `ORC-COM-001`, `ORC-LIF-001`,
`ORC-FE-001`, `ORC-CLI-001`, plus later settings/handler/command contracts.

File Manager is Orchestrator's GUI. Orchestrator itself renders nothing.

## Orchestrator proposal 001

### First client operations

| Operation | Purpose |
|---|---|
| `orchestrator.version` | Negotiate semantic, wire, and client ranges |
| `orchestrator.status` | Display lifecycle and degraded provider state |
| `orchestrator.contracts.list` | Inspect required/implemented contract families |
| `orchestrator.availability.list` | Render required, available, degraded, unavailable, deferred, and stubbed capabilities |
| `orchestrator.shutdown` | Request clean user-daemon shutdown under policy |

Settings, handlers, commands, plugin controls, and semantic facts are not
inferred from these calls. They enter later negotiation rounds.

### Client behavior

- File Manager normally uses Orchestrator for integration/policy and combined
  search.
- GUI.Forms remains in-process and is never exposed to Orchestrator.
- When Orchestrator is unavailable, File Manager may use the registered direct
  engine fallback and cached read-only declarations, clearly marked degraded.
- File Manager renders all Orchestrator service/settings/grant UI with house
  controls.
- Responses arrive through a pollable/queued client projection, never a callback
  on an uncontrolled Orchestrator thread.

### Required fake scenarios

1. Orchestrator ready, engine unavailable;
2. Orchestrator ready, engine degraded/stale;
3. Orchestrator absent with direct engine fallback;
4. incompatible contract major;
5. shutdown/restart invalidating subscriptions;
6. plugin and semantic systems reported stubbed rather than empty-success;
7. unknown future availability records ignored only when noncritical.

## File Manager reply 001

Status: **deferred by ADR-004**.

Frontend 001 does not consume a live Orchestrator contract. It models the
states below through a frontend-owned port and deterministic data marked
`simulated`. That fixture shape is not a counterproposal and does not freeze an
Orchestrator ABI. Reply 001 begins when the real adapter is scheduled.

Please provide:

- the smallest first-screen state that needs Orchestrator;
- preferred C ABI vs local-wire client projection for C++;
- UI-thread delivery and cancellation requirements;
- cached declaration lifetime and stale-display requirements;
- exact degraded-search behavior;
- additional state needed to render service controls without policy leakage.

## Orchestrator reconciliation 001

Status: **not started; waits for File Manager reply 001**.
