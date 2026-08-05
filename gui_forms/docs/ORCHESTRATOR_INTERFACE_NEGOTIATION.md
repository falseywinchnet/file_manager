# Orchestrator ↔ GUI.Forms interface negotiation

Status: **round 001 Orchestrator proposal; awaiting GUI.Forms reply**.

Participants: Orchestrator integration authority, GUI.Forms provider, and File
Manager consumer. Canonical families: `ORC-COM-001`, `ORC-GUI-001`,
`ORC-FE-001`.

ADR-006 makes GUI.Forms one of two independent technical predecessors for
Frontend 001. Orchestrator advances headlessly to Core 1.0 while GUI.Forms
advances to FM0. The grand architect starts the frontend after both are ready;
this note is the provider-owned place for the GUI.Forms reply.

Orchestrator has no GUI and does not call controls remotely. This negotiation
registers the ABI File Manager consumes and the capability facts Orchestrator
may report.

## Orchestrator proposal 001

GUI.Forms should publish a machine-readable experimental consumption manifest
containing:

```text
abi family and version; target triple; exported symbol/table identity;
thread/lifecycle rules; host protocol version; implemented capabilities;
explicitly unsupported capabilities; fixture/trace digests;
build/source manifest; compatibility horizon
```

The manifest must distinguish construction/API presence from behavioral
support. File Manager negotiates a minimum set; Orchestrator records compatible,
degraded, or unavailable state. Orchestrator never receives GUI handles,
callbacks, native windows, or renderer objects.

### First requested capability groups

1. runtime/table negotiation and deterministic disposal;
2. UI-thread identity and queued dispatch;
3. retained control tree, initialization, and lifecycle;
4. invalidation/layout/paint transaction behavior;
5. pointer, keyboard, focus, committed text, and cancellation-safe close;
6. host capability discovery rather than simulated support;
7. headless trace projection for frontend fakes;
8. explicit control-family support matrix.

### Required fixtures

- compatible and incompatible ABI-table negotiation;
- stale/wrong-kind/disposed/wrong-thread handles;
- nested initialization and failed attachment rollback;
- dispatch during shutdown;
- unsupported host capability;
- byte-stable headless lifecycle/input trace.

## Frontend 001 go-ahead request

GUI.Forms should reply with one named consumption snapshot and state either
`ready` or `not ready` for each item in
[`../../frontend/planning/FRONTEND_001.md`](../../frontend/planning/FRONTEND_001.md):

- public build/install target and exact C ABI/C++ wrapper identity;
- application/window/tree/UI-thread/ownership/shutdown lifecycle;
- layout, resize, DPI, damage, input, focus, and keyboard behavior;
- compositional primitives for the shelf, path/search row, collapsible panes,
  object field, status, and overlay;
- admitted 001 text display/editing behavior;
- PNG/resources, semantic naming, theme/state, and headless traces;
- unsupported 001 behavior that the frontend must omit or visibly mark.

The reply need not claim framework completeness. It must identify the stable
public seam Frontend 001 may consume without reaching into private GUI.Forms
sources.

## GUI.Forms reply 001

Status: **awaiting project reply**.

Please provide:

- the proposed manifest location and schema counterproposal;
- which ABI 0.6 behaviors are ready to name in a consumption snapshot;
- authoritative test/trace locators for each requested group;
- unsupported groups that must block the first File Manager slice;
- versioning and table-extension constraints;
- any request that would leak platform/backend state and must be rejected.

## Orchestrator reconciliation 001

Status: **not started; waits for GUI.Forms reply 001**.
