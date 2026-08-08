# Orchestrator ↔ File Manager interface negotiation

Status: **round 003 Core bootstrap reconciled; owner start/composition direction
007 recorded; live Core 1.0 bootstrap edge still requires final launchd
lifecycle evidence before Frontend 001 under ADR-006**.

Participants: Orchestrator integration authority and the future C++ File
Manager frontend. Canonical families: `ORC-COM-001`, `ORC-LIF-001`,
`ORC-FE-001`, `ORC-CLI-001`, plus later settings/handler/command contracts.

File Manager is Orchestrator's GUI. Orchestrator itself renders nothing.

## Orchestrator proposal 001

### First client operations

| Operation | Purpose |
|---|---|
| `orchestrator.version` | Negotiate semantic, wire, and client ranges |
| `orchestrator.release` | Inspect Core 1.0 target, readiness, and blocking requirements |
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

## Architect sequencing correction 002

**GIVEN:** Frontend 001 does not bootstrap on a frontend-owned Orchestrator
fixture. Orchestrator advances independently to Core 1.0 and the product uses
that live authority from startup. Canonical fixture replay remains permitted
for deterministic frontend tests.

## File Manager reply 002

Status: **accepted planning reply; implementation evidence pending**.

- The smallest startup surface is release compatibility, lifecycle generation,
  immutable contract/availability snapshot, provider reasons, and registered
  direct-Engine fallback state.
- The authoritative cross-process projection should be a production local wire
  with a thin C++ client. A Rust/C++ in-process ABI is not requested.
- Delivery is pollable/queued and drained on the frontend UI thread. Deadlines
  and cancellation identities are explicit; arbitrary foreign callbacks are
  rejected.
- Cached declarations are scoped to daemon identity plus lifecycle/configuration
  generation. On disconnect they may remain visibly stale and read-only; they
  cannot authorize mutation or plugin execution.
- Degraded search uses the direct Engine route only when Orchestrator's last
  compatible snapshot registered it. When `ORC-ENG-004` is registered and
  available, catalogue failure may use its source-explicit reduced live search
  under Orchestrator's fallback law. Otherwise live folder navigation remains
  and search is provider-limited.
- Service controls require release identity, lifecycle state/generation,
  provider state/reason, compatibility state, restart/shutdown eligibility, and
  redacted diagnostics locators. Policy secrets and raw grants do not cross for
  display convenience.

## Orchestrator reconciliation 002

Status: **accepted semantic direction; production transport, authentication,
and hostile fixtures remain Core 1.0 work**.

These requirements enter the Core 1.0 profile for `ORC-COM-001`,
`ORC-LIF-001`, `ORC-FE-001`, and `ORC-CLI-001`. Settings, handlers, commands,
Engine queries, plugins, and semantic facts retain separate contract gates.

## Orchestrator implementation evidence after reconciliation 002

Status: **OBSERVED first-platform consumer; no new File Manager reply required**.

Orchestrator now carries a separately built C++17 conformance client under
`../../orchestrator/conformance/clients/cpp/`. It authenticates over the Unix local
wire, materializes the accepted startup fields, and reconnects across daemon
instance replacement without linking Rust or GUI.Forms. This exercises the
consumer side while Frontend 001 remains gated. Pollable delivery, stable client
ABI, full hostile fixtures, and Windows named pipes remain open before product
consumption.

## Orchestrator implementation reconciliation 003

Status: **OBSERVED complete source-client projection; installed launchd gate
pending**.

The sequential conformance reads exposed a future torn-snapshot risk and did
not materialize the complete contract catalogue, fallback eligibility, or
service-control state requested in reply 002. The additive
`orchestrator.frontend.bootstrap` / `ORC-FE-001` 1.0 operation now returns one
bounded immutable value containing:

- version, release/provenance, lifecycle and configuration generations;
- complete contract and availability catalogues;
- normal route, Engine scope, and registered-versus-eligible direct fallback;
- shutdown/restart eligibility and nullable redacted diagnostics locator.

The server hello's instance identity plus the two snapshot generations form the
frontend cache key. The separately built C++17 source client performs one
synchronous request, validates the schema/generation/route invariants, and
computes the strict Orchestrator-owned part of the Frontend 001 opening
predicate. It owns no thread or callback. File Manager therefore owns the I/O
worker and posts completed value snapshots to its GUI.Forms UI-thread queue,
exactly as reply 002 requested.

On disconnect, a prior compatible snapshot is display-only and visibly stale.
It cannot authorize mutation, plugin execution, or an Engine fallback that the
same snapshot did not register. Route registration and current eligibility are
separate fields so an installed-provider gap cannot become accidental access.

This closes the Orchestrator-side data-shape and delivery-model work for the
Frontend 001 bootstrap. The remaining Core release gate is a real installed
launchd activation, shutdown, reactivation, and removal measurement. A frozen
C++ binary ABI and Windows named pipes remain later platform/source-packaging
work; they are not required by the accepted macOS source-client projection.

## Orchestrator availability preparation 004

Status: **OBSERVED consumer gate projection implemented; Frontend 001 remains
closed**.

The raw capability catalogue's `required` field is a product/contract property,
not a statement that every unfinished provider blocks Frontend 001. To prevent
the future application from inventing that distinction, the atomic bootstrap
now includes `frontend_opening`:

- `orchestrator_gate` is the only locally decidable opening gate. It combines
  the Core release manifest with `frontend.bootstrap` availability and carries
  exact blockers. At this round it is `blocked` by `daemon.discovery` and the
  degraded bootstrap capability, both naming the still-open installed launchd
  evidence.
- `external_gates.gui_forms` mirrors the attributed
  `gui_forms.consumption_manifest` row and is currently `negotiating`. It does
  not manufacture a GUI.Forms go-ahead.
- `external_gates.architect_direction` is `not_reported` with nullable
  satisfaction. Orchestrator does not infer owner direction from tests or
  documents.
- policy states that product startup needs a live snapshot, stale state is
  display-only, and separately gated provider absence does not block the 001
  opening predicate.

The independent C++ source client materializes and cross-checks this projection
against the raw availability rows. Its former globally named readiness helper
is replaced by `orchestrator_gate_ready()`. The probe reports the three
attributed gate states and Orchestrator blocker count, giving the future
frontend a deterministic startup/preflight seam without granting permission to
create application source.

No File Manager application source, build files, or GUI.Forms consumption code
is opened by this round. Once installed launchd evidence makes the Orchestrator
gate ready, the remaining opening events are still the named GUI.Forms go-ahead
and explicit architect direction required by ADR-006.

## Architect application-backbone direction 005

Date: 2026-08-06.

Status: **GIVEN product direction; process/package details remain CANDIDATE**.

- File Manager and its open/save-as picker representation are implemented from
  the same navigation substrate when frontend work opens.
- The picker is reusable by future first-party Paint and Text Editor
  applications without linking or launching the complete File Manager program.
- Text Editor may show hidden files under an application-scoped preference that
  does not mutate File Manager's view preference.
- File Manager file/preview objects may transfer into Paint; Paint owns
  alpha-capable reusable clipart/composite objects.
- Orchestrator service/settings UI may appear standalone and within File Manager
  while Orchestrator itself remains headless.
- Application help content remains local and app-owned; GUI.Forms supplies help
  mechanics and Orchestrator resolves registered providers/topics.

Planning detail:
[`DOCUMENT_PICKER_SURFACE.md`](DOCUMENT_PICKER_SURFACE.md) and
[`../../planning/APPLICATION_BACKBONE_AND_DOCUMENT_PICKER.md`](../../planning/APPLICATION_BACKBONE_AND_DOCUMENT_PICKER.md).

## Orchestrator proposal 005

Status: **awaiting future frontend reply; Frontend 001 remains closed**.

Provisional families `ORC-APP-001`, `ORC-PCK-001`, `ORC-UI-001`,
`ORC-HLP-001`, and `ORC-XFR-001` request the frontend to define:

- the reusable file-browser model, selection controller, and picker-view
  boundary;
- the smallest open/open-many/folder/save-as/import/export profile matrix;
- live navigation and visibility behavior without Engine;
- the app-scoped hidden-file setting and session override presentation;
- exact accepted/cancelled/unavailable result mapping;
- native fallback presentation and route disclosure;
- shared Orchestrator administration rendering without arbitrary server-driven
  controls;
- file-reference and preview/clipart drag-source payload behavior.

No answer may require File Manager private executable state, duplicate a
GUI.Forms control, place policy in the view, or open frontend source before its
existing gate.

## File Manager reply 005

Status: **planning counterproposal; implementation evidence unavailable**.

The candidate split in `DOCUMENT_PICKER_SURFACE.md` is:

- a reusable `FileBrowserModel`;
- a purpose-bounded `FileSelectionController`;
- a GUI.Forms `DocumentPickerView`;
- a host adapter that obtains and terminates the Orchestrator session.

The first slice omits the full tree/preview/shelf/recent-path/search surface
unless separately admitted. File Manager executable linkage is rejected; an
independently consumable first-party package is recommended. Final naming,
package location, profile commands, overwrite-dialog ownership, and
out-of-process Picker Host timing remain open.

## Orchestrator reconciliation 005

Status: **not started; waits for GUI.Forms reply 002 and the frontend opening
round**.

## Architect future-scope direction 006

Date: 2026-08-06.

Status: **GIVEN scope under ADR-013; no frontend or plugin implementation
opened**.

- Persistent side panels belong only to File Manager and its embedded picker/
  browser projection. Paint, Text Editor, Games, providers and plugins do not
  inherit that topology.
- Checksum inspection and `Open Command Line Here` are trusted first-party File
  Manager context commands, not plugins. Their on-demand/context/platform
  semantics are in `CONTEXTUAL_BUILTIN_COMMANDS.md`.
- Desktop integration is limited by `DESKTOP_INTEGRATION_BOUNDARY.md`.
- Lexicon may contribute typed exact-definition results without becoming an
  Engine file row or provider-owned UI.
- Archive Viewer may supply a bounded virtual hierarchy through the trusted
  embedded browser and use the trusted Document Picker for extraction
  destination. Image Converter may return bounded create-new outputs. Plugin
  code supplies no controls and receives no ambient filesystem writes.

## Orchestrator proposal 006

Status: **paper routing only**.

The frontend must later define:

- built-in versus plugin command identity and visible trust distinction;
- checksum/terminal contextual applicability, settings and terminal errors;
- typed lexical-result composition independent of file-result ranking;
- virtual archive location identity, paging, stale generation, entry-open and
  extraction destination presentation;
- host-rendered transform/extraction/password/progress/collision dialogs;
- absence/disabled/crashed plugin states with no injected controls;
- proof that the embedded browser package does not grant panels to its host
  application outside the picker/archive surface.

Contract IDs and implementation wait on the existing frontend opening and
plugin/provider gates.

## Architect frontend start and composition direction 007

Date: 2026-08-07.

Status: **GIVEN owner direction; technical dependency and later-contract gates
remain independent**.

The grand architect has now supplied the explicit Frontend 001 start direction.
This satisfies the owner-direction predicate but does not change the current
Orchestrator `ready: false` release manifest or manufacture the outstanding
GUI.Forms FM0 reply.

The same direction adds these frontend requirements:

- an extensive product-owned multi-tab Settings surface that configures and
  truthfully reports backend services;
- installed Malkuth application access through OS-native menu-bar entries,
  never an in-program styled suite launcher;
- an inline path editor in one dense chevron breadcrumb cascade rather than the
  former pulled-down path matrix; and
- contained end-to-end dogfood with the live Orchestrator and Engine before
  daily-root promotion.

The frontend therefore requests future reconciliation of existing families,
not invented payloads:

1. `ORC-SET-001` supplies bounded typed schemas, immutable value revisions,
   optimistic transactions, validation, reset/migration effects, terminal
   results, and audit identity. It supplies no arbitrary controls.
2. `ORC-UI-001` supplies service status and admitted control-command semantics
   for Orchestrator, Engine, and later providers, including availability,
   progress, restart effect, and redacted diagnostics.
3. `ORC-APP-001` supplies immutable installed first-party application identity,
   availability, launch intent, and native-menu eligibility. It does not open a
   future application's implementation gate or make Orchestrator a launcher UI.

GUI.Forms owns portable-to-native menu publication. File Manager owns menu
grouping and Settings tab composition. Orchestrator owns the application and
settings/service facts. The complete staged program is
[`TOTAL_IMPLEMENTATION_PLAN.md`](TOTAL_IMPLEMENTATION_PLAN.md).
