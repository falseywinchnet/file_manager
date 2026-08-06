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

## Orchestrator proposal 002 — first-party application surfaces

Date: 2026-08-06.

Status: **proposal; no ABI or implementation priority frozen**.

The grand architect has introduced File Manager's reusable Document Picker,
future Paint and Text Editor consumers, a shared Orchestrator administration
surface, application-local help, and File Manager → Paint compositional drag.
Canonical proposal:
[`../../orchestrator/proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md`](../../orchestrator/proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md).

GUI.Forms is asked to report capability and counterpropose on these generic
mechanics only; it does not gain file-browser, settings, help-content, Paint, or
Orchestrator policy.

### Requested GUI.Forms capability groups

1. **Application-owned modal surface:** an arbitrary retained view may run as an
   owned modal window or platform-appropriate sheet with owner suppression,
   exactly one terminal result, Escape/cancel, shutdown handling, and focus
   restoration. A queued/asynchronous modal session must exist even if a
   Forms-shaped synchronous `ShowDialog` convenience wraps it.
2. **Portable presentation route:** a shared File Manager picker composition can
   use only public C++/C ABI controls and host services. It must not require a
   native file-dialog widget or platform handle in portable state.
3. **Native fallback identity:** existing `OpenFileDialog`, `SaveFileDialog`,
   and folder adapters report which route completed, cancellation preservation,
   owner identity, and supported filter/access-grant semantics.
4. **Outbound transfer:** complement the measured inbound drag destination with
   drag-source initiation, typed file references, bounded media bytes,
   lazy/promised data, cancellation/expiry, clipboard formats, and
   cross-participating-window behavior.
5. **Help mechanics:** complete Forms-shaped `HelpProvider` attachment,
   help-key/F1 command routing, stable help-topic/context IDs, and reuse of the
   semantic graph/greaseboard anchors without coupling help availability to
   accessibility publication.
6. **Inspection:** consumption manifests and headless traces name modal, native
   dialog, help, inbound/outbound transfer, lazy payload, and clipboard
   capability independently.

### Required fixtures

- nested owned popup → custom modal → focus restore;
- accept, cancel, duplicate-terminal rejection, owner close, and host shutdown;
- headless custom picker result distinct from native common-dialog fallback;
- application-modal versus window-modal participation on each host;
- outbound file-reference drag to synthetic Paint target;
- lazy alpha-clipart success, cancellation, expiry, timeout, and oversized
  rejection;
- F1/context help, missing topic, disposed owner, overlay toggle, and proof that
  disabling help leaves native accessibility intact;
- capability manifest distinguishing unsupported Linux/platform adapters from
  empty success.

### GUI.Forms reply 002

Status: **awaiting project reply**.

Please identify which requirements are already covered by M11h evidence, which
need additive public projection only, which require new implementation, and any
request that would improperly place application policy in GUI.Forms.
