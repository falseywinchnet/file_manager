# Orchestrator ↔ GUI.Forms interface negotiation

Status: **round 001 GUI.Forms reply published; Orchestrator reconciliation open**.

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

## GUI.Forms reply 001 request

Status: **answered by GUI.Forms reply 001 below**.

Please provide:

- the proposed manifest location and schema counterproposal;
- which ABI 0.6 behaviors are ready to name in a consumption snapshot;
- authoritative test/trace locators for each requested group;
- unsupported groups that must block the first File Manager slice;
- versioning and table-extension constraints;
- any request that would leak platform/backend state and must be rejected.

## GUI.Forms reply 001 — named FM0 consumption snapshot

Date: 2026-08-10.

Status: **ready for Frontend 001 on macOS arm64 within the named snapshot**.

The provider manifest is
`../manifests/gui-forms-fm0-macos-arm64-2026-08-10.json`. The stable public
consumer seam is the installed CMake package `GUIForms 0.1`, especially
`GUIForms::Application` for the native AppKit/Skia runtime and
`GUIForms::Controls` for independently testable model consumers. Public host
entry points are `gui_forms::host::run_macos` and
`gui_forms::host::run_macos_application`. The retained C ABI remains
`gui_forms_abi0` with additive experimental tables through 0.26; Frontend 001
does not need to substitute the C table for its public C++ application target.

**MEASURED:** on the named M4, a clean external project used only
`find_package(GUIForms 0.1 CONFIG)`, linked `GUIForms::Application`, found the
installed font payload through `GUIForms_FONT_DIR`, and successfully executed
its model-only construction probe. The installed dylib is arm64 Mach-O and
exports the public Window and macOS application entry points. The manifest
records its SHA-256 digest and the exact commands.

The snapshot admits the retained tree, layout/paint, application/window
lifecycle, public File Manager control primitives, input/focus/commands,
popups, headless conformance, deterministic fonts, CPU Skia host, and
fail-closed Web.Forms generated public C++ tree. Full VoiceOver campaign and
browser/native raster equivalence remain `degraded`, not silently complete.
Signed packaging, other native host platforms, outbound drag source, and any
runtime web stack remain unavailable.

Thread rule: the AppKit host owns the main UI thread. Provider/service calls do
not enter GUI.Forms. Frontend workers post immutable application snapshots to
their own UI dispatch boundary. Native handles and renderer objects never
cross Orchestrator.

## Orchestrator reconciliation 001

Status: **provider reply complete; Orchestrator may project
`gui_forms.consumption_manifest=available` with the exact manifest id**.

The reply preserves `ORC-GUI-001` authority: Orchestrator mirrors availability
and manifest identity but receives no GUI handle, callback, native window, or
renderer state. Any target mismatch or absent required capability fails the
frontend build/opening check. Later FM1/FM2/FMX rows and proposal 002 remain
independent negotiations.

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
5. **Help mechanics:** **MEASURED PARTIAL M12-P4:** Forms-shaped
   `HelpProvider` attachment, help strings/keywords/navigators, automatic and
   explicit ShowHelp, focus-ancestor F1 routing, control-first/provider-second
   handled events, and semantic descriptions are public. The provider emits
   intent and never opens an external resource. Stable TopicId/context-ID
   registry, Orchestrator capability routing, greaseboard anchors, described-by
   relations, and native publisher verification remain to negotiate without
   coupling help availability to accessibility publication.
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

## GUI.Forms implementation fact M12-P5 — local binding projection

Status: **MEASURED; not yet an ABI proposal**.

GUI.Forms now has a renderer-neutral retained binding/currency kernel for local
application models. It deliberately accepts explicit typed property
descriptors and stable `BindingRecord` projections, not arbitrary managed
objects or Orchestrator-owned records. Orchestrator remains the authority for
cross-project contract meaning; a future frontend adapter may project a
negotiated snapshot into this local model without giving GUI.Forms service,
persistence, or capability-policy ownership.

Before this becomes a consumption-manifest capability, GUI.Forms still owes a
versioned C ABI table for value/record/binding handles, bounded string/value
transport, callback cleanup, source replacement, and headless trace identity.
Nested object reflection is explicitly not required of the native ABI. Current
C++ evidence is recorded in
`../experiments/M12P5_BINDING_CURRENCY_KERNEL.md`.

## GUI.Forms implementation fact M12-P6 — validation and local errors

Status: **MEASURED; not yet an ABI proposal**.

GUI.Forms now owns deterministic retained focus validation and a local
BindingSource/ErrorProvider error projection. This is presentation-local
behavior: it does not make GUI.Forms authoritative for Orchestrator service
validity, capability policy, or persisted diagnostic records. A future ABI may
carry bounded error snapshots into `BindingRecord`, but the producer remains
the authority and arbitrary managed-object reflection is not part of the
native contract. Evidence:
`../experiments/M12P6_VALIDATION_AND_BOUND_ERRORS.md`.

## GUI.Forms implementation fact M12-P7 — local dialog commands

Status: **MEASURED; not yet an ABI proposal**.

GUI.Forms now owns renderer-neutral mnemonic parsing/routing and retained
accept/cancel command roles. These are local presentation commands, not
Orchestrator capabilities: they do not authorize work, close a service session,
or select a cross-project action by themselves. A future ABI projection may
carry stable command identity and handled/result state, but Orchestrator remains
the authority for the meaning and availability of the invoked operation.
Evidence: `../experiments/M12P7_DIALOG_KEYS_AND_MNEMONICS.md`.

## GUI.Forms implementation fact M12-P8 — deterministic command arbitration

Status: **MEASURED; not yet an ABI proposal**.

GUI.Forms now precomputes mnemonic candidates before callbacks, cycles
duplicate winners, retains exact DialogResult state, and routes MenuStrip and
popup mnemonics through local shared Command authority. These remain
presentation facts. A non-None local result does not authorize an Orchestrator
operation, and native modal closure is still owned by the presenting host.
Evidence:
`../experiments/M12P8_COMMAND_ARBITRATION_AND_NATIVE_CALLBACKS.md`.
