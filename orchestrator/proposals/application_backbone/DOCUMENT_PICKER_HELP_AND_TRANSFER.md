# Proposal — application backbone, document picker, help, and transfer

Date: 2026-08-06.

Status: **CANDIDATE cross-project proposal; no runtime operation admitted**.

Source direction:
[`../../../planning/APPLICATION_BACKBONE_AND_DOCUMENT_PICKER.md`](../../../planning/APPLICATION_BACKBONE_AND_DOCUMENT_PICKER.md).

## User operations

1. Paint or Text Editor opens an owned house file picker without launching the
   full File Manager application.
2. The same bounded browser/selection surface is used inside File Manager.
3. A caller receives an exact accepted/cancelled/failed selection result and,
   where required, a platform access grant.
4. Orchestrator settings/service controls can appear in File Manager and a
   possible standalone first-party shell without duplicating policy.
5. F1/context help resolves to local application-owned content.
6. File Manager objects and Paint clipart/canvas objects transfer through typed
   drag/clipboard flavors without GUI injection or unbounded eager copies.

## Provisional contract families

| ID | Proposed meaning | Provider | Consumers |
|---|---|---|---|
| `ORC-APP-001` | Stable first-party application identity, profile namespace, capability/availability snapshot | Orchestrator | File Manager, Paint, Text Editor, Games, picker/control surfaces |
| `ORC-PCK-001` | File Selection Session request, route, cancellation, and result | Orchestrator policy plus selected first-party/native UI provider | File Manager, Paint, Text Editor |
| `ORC-UI-001` | Bounded Orchestrator administration presentation model | Orchestrator services/settings registries | File Manager settings and possible standalone control shell |
| `ORC-HLP-001` | Local help-provider registration and topic resolution | application packages through Orchestrator registry | application help surfaces, possible shared Help Viewer |
| `ORC-XFR-001` | First-party transfer-flavor identity, capability, and handler declaration | Orchestrator registry; bytes remain peer/OS-owned | File Manager, Paint, Text Editor, platform adapters |

These identifiers are provisional until negotiation reconciliation. GUI.Forms'
in-process modal/drag/help mechanisms are capabilities of `ORC-GUI-001`, not
remote Orchestrator control calls.

## Authority and privacy

- Orchestrator authenticates the requesting application and owns the effective
  application profile, route availability, and capability ceiling.
- The caller may narrow a request but cannot widen its registered powers.
- Hidden-object presentation is app-scoped policy, not filesystem authority.
- The shared picker renders File Manager-owned first-party composition; it does
  not accept plugin controls or styling.
- The host application owns document parsing and writing. The picker never
  becomes a generic content-read or file-write API.
- Help content is local and application-owned. Orchestrator resolves identity
  and availability; it does not fetch the web or execute provider UI.
- Persistent side panels are a File Manager/embedded-picker topology only.
  Paint, Text Editor, Games, and other applications render secondary tools as
  owned popup dialogs or overlays; Orchestrator does not project either panels
  or dialog layout.
- Transfer bytes remain on the direct GUI.Forms/platform data path. Orchestrator
  registers meaning and availability but is not a drag-motion relay.

## Request shape draft

`FileSelectionRequest`:

```text
application_id / application_instance
session_id / capability_context
purpose / cardinality / object_kinds
type_filters / default_extension / proposed_name
initial_location_hint
visibility_policy
admitted_picker_commands
preferred_route / allowed_fallbacks
locale_generation / theme_generation
deadline / cancellation_id / response_budget
```

`FileSelectionResult`:

```text
terminal_status / session_id / route_used
selected object and path snapshots
platform access grant or handle metadata when required
existing-target observation / overwrite disposition
effective type and extension
policy/profile generation / provenance
warnings / staleness / expiry
```

Raw platform handles never cross a wire unless a platform-specific transfer
mechanism defines ownership, lifetime, duplication, and peer authentication.

## Modality and lifecycle

The recommended first-party route is an in-process shared picker view using the
same GUI.Forms consumption snapshot as the host. Orchestrator supplies a
selection-session policy/result envelope, not a remote window.

Required lifecycle states:

```text
requested -> policy_resolved -> presenting -> accepted | cancelled | failed
```

- Exactly one terminal result.
- Cancellation before presentation and during presentation are distinct but
  idempotent.
- Owner close, host shutdown, Orchestrator restart, route loss, and provider
  crash have named terminal behavior.
- Modal owner suppression and focus restoration remain GUI.Forms behavior.
- A native common-dialog route is an explicit fallback, not silent substitution.

## Bounds and backpressure

- bounded filter/type counts, selection cardinality, paths, labels, and result
  bytes;
- paged/virtual directory enumeration rather than eager unbounded lists;
- lazy/promised large drag payloads with expiry and cancellation;
- no synchronous content decode in Orchestrator or the picker policy path;
- no plugin participation in pointer-move, layout, or paint loops;
- request deadlines must not force-close a platform dialog without a supported
  cancellation path; unsupported cancellation is reported honestly.

## Required negotiation rounds

### GUI.Forms

- application-owned modal session and owner/focus semantics;
- shared picker view composition without native-widget assumptions;
- host common-dialog fallback identity;
- outbound drag source, lazy payload, clipboard and cross-window participation;
- HelpProvider/F1 and greaseboard help anchors;
- capability manifest entries and deterministic headless fixtures.

### File Manager frontend

- reusable browser/selection model and picker view boundary;
- live filesystem navigation without Engine;
- feature-profile matrix and host-owned state boundary;
- exact object/path snapshots and degraded availability presentation;
- no full-application linkage requirement.

### Orchestrator

- application identity/profile namespace;
- picker policy/route/result contract;
- bounded administration presentation model;
- help registry and first-party transfer-flavor declarations;
- capability, settings, audit, restart, and version behavior.

## Golden fixtures

- Paint open-one PNG with house route;
- Text Editor open hidden dotfile with app-scoped visibility;
- save-as new file and existing-target confirmation;
- cancel before presentation and cancel from the modal surface;
- house picker unavailable with permitted native fallback;
- route unavailable with fallback forbidden;
- Orchestrator restart during an in-process presentation;
- host application closes while picker is open;
- stale initial location and unavailable volume;
- security-scoped access result with explicit expiry;
- local help topic available, missing locale fallback, and missing provider;
- File Manager file-reference drag into Paint;
- Paint alpha clipart lazy transfer, cancellation, and oversized rejection.

## Hostile fixtures

- caller forges another `ApplicationId`;
- request contains an unknown capability-widening override;
- hidden names leak through search/completion while effective policy hides them;
- more selections or filters than declared bounds;
- stale/wrong session terminal result;
- duplicate accept and cancel terminals;
- platform grant attached to the wrong selected object;
- help provider returns a web URL or executable/native UI payload;
- transfer provider never materializes promised bytes or exceeds its quota;
- plugin attempts to register a GUI control or direct Engine mutation under a
  picker/transfer declaration.

## Alternatives and reversal

Native-only, out-of-process-only, and full-File-Manager linkage remain recorded
alternatives in the parent document. The semantic request/result can survive a
later route change. No process or source-package location should freeze until
the GUI.Forms and frontend negotiations reply.
