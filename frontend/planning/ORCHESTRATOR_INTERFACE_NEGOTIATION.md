# Orchestrator ↔ File Manager interface negotiation

## Details consumer development request — 2026-10-02

File Manager requests retained factual columns through the reconciled
[Details development contract](../../orchestrator/spec/contracts/GUI_OBJECT_DETAILS_DEVELOPMENT.md).
The application owns metadata observations, availability, display labels,
column policy and sorting; the provider owns collection mechanics. The reviewed
provider candidate is being wired into File Manager. Existing history uses
Alt+Left/Right; the reconciled development width/pan chord is Alt+Shift+Left/Right,
and plain Alt chords pass through the Details control. Enter routes to the
focused header before the application object-opening command. Native conformance and independent matching-SDK consumption
remain pending; this record does not advertise a delivered capability.

Status: **round 010 settings/service/installed-Engine reconciliation recorded;
live Core 1.0, the named GUI.Forms FM0 consumption manifest, and owner start
direction are satisfied; Frontend 001/F1 is active while later-provider rounds
remain independent**.

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

## Frontend opening reconciliation 008

Date: 2026-08-10.

Status: **OBSERVED opening predicates satisfied; F1 implementation active**.

This round appends the evidence that was unavailable in rounds 003, 004, and
007; it does not rewrite their historical gate state:

- Orchestrator Core 1.0 is installed and its atomic C++ bootstrap projection is
  exercised by the frontend worker/UI-queue adapter.
- GUI.Forms reply 001 now names
  `gui-forms-fm0-macos-arm64-2026-08-10`, exporting the installed
  `GUIForms::Application` target and the public retained host boundary.
- [`OWNER_DIRECTION_2026-08-10.md`](OWNER_DIRECTION_2026-08-10.md) records the
  explicit architect implementation direction carried into this round.
- The first frontend source, build, Web.Forms-authored surface, protected-root
  filesystem model, and focused tests now live under `../`.
- M4 Screen Sharing evidence for both the browser-valid authoring source and
  native retained application is recorded under
  `../results/2026-08-10-m4-dogfood/`.

This satisfies ADR-006's opening predicate. It does not freeze the later
settings, handlers, command, picker, Engine-search, plugin, packaging, signing,
or daily-replacement contracts. Those retain their own rounds and evidence.

## Settings reconciliation 009

Date: 2026-08-10.

Status: **semantic v1 accepted for contained implementation under ADR-018;
provider/consumer conformance pending**.

The frontend requirement in direction 007 is reconciled as `ORC-SET-001` 1.0
in `orchestrator/spec/contracts/HIVES_AND_SETTINGS.md`. Orchestrator returns
bounded typed field schemas and immutable value snapshots, never controls.
File Manager composes its own tabbed GUI.Forms surface. The CLI and C++ client
submit the same optimistic transaction and observe the same committed revision,
validation, restart effect, recovery provenance, and audit identity.

The first physical profile is the bounded atomic store selected by ADR-018 after
an M4 comparison against SQLite WAL FULL. It is not a hive/database decision.
Secret values, plugin namespaces, arbitrary settings files, and destructive
administrative service commands remain unavailable until separately admitted.

## Installed Engine and service-control reconciliation 010

Date: 2026-08-10.

Status: **ORC-UI-001 v1 and the contained installed Engine route accepted and
measured under ADR-019**.

The frontend reply to direction 007 is now executable without server-driven
UI. `orchestrator.services.snapshot` returns immutable Orchestrator/Engine
facts and a closed command allowlist. The native Settings page composes those
facts with GUI.Forms controls, disables unavailable operations and confirms
rebuild/restart/shutdown. It receives no layout, callbacks or policy secrets.

Commands carry the optimistic identity from the displayed snapshot:

- Orchestrator requires both its authenticated ORC1 process identity and
  lifecycle generation;
- Engine requires its process identity, plus `fm1-contained` for root-bound
  reconcile/rebuild;
- admin transport calls are never automatically replayed; and
- the frontend discards the old snapshot and refreshes after a terminal result.

The normal search route now reaches the separately installed host-bound Engine
through Orchestrator's authenticated `ENG1` adapter. The contained service owns
one durable catalogue for the explicit protected dogfood root; File Manager
still rechecks result containment and filesystem identity before presenting an
object. The direct Engine route remains registered fallback policy rather than
the normal C++ application path.

M4 evidence covers Rust CLI/C++ agreement, exact catalogue search, integrity,
reconcile, both supervised restart paths, instance rotation, stale-command
refusal, the short Unix-socket repair, and the retained code-signing failure:
`../../orchestrator/conformance/evidence/M4_FILE_MANAGER_SERVICES_2026-08-10.md`.

This round does not freeze event subscriptions, cross-platform installed
transports, durable general audit, plugin administration, handler/command
registries, or the reusable picker surface.


## Windows development projection — 2026-09-29

**GIVEN:** the owner authorized direct Shadow checkout development and a native
Windows navigation slice, preserving the existing component boundaries and
truthful unavailable states.

**OBSERVED:** the frontend now consumes the separately owned C++ client's
Windows build through the unchanged `fileman::orchestrator_client` API. That
client explicitly reports live transport unavailable. The frontend does not
replace bootstrap, settings, service state, or search with successful fixtures.
Canonical fixtures remain limited to tests. Useful local browsing is independent
of that unavailable augmentation.

**OBSERVED:** GUI.Forms consumption uses the development manifest
`gui-forms-shadow-windows-x64-2026-09-29`, its installed public Application API,
and the added wake/drain callbacks for frontend-owned worker completion. The
macOS native host branch remains to preserve its existing titlebar/drag-region
projection. No new Orchestrator wire payload, capability ID, or provider authority
is introduced by the private Windows file/path/launch adapters.

The existing C++ picker source package must be rebuilt with this model: its
`ObjectIdentity` additionally retains the upper 64 bits of a Windows file ID.
This is a source-package layout change, not a frozen cross-process ABI or a new
filesystem identity authority. Windows live IPC and installed service readiness
remain Orchestrator-owned gates. See the native frontend evidence receipt under
`../results/2026-09-29-shadow-windows/`.

## Windows service and trusted local picker reconciliation — 2026-09-29

**GIVEN:** current owner direction admits actual separate Engine search and an
offline first-party picker. **OBSERVED:** the C++ source client now projects the
unchanged API over authenticated current-user ORC1 pipes; Windows explicit-process
semantics and remaining gates live in
`../../orchestrator/spec/WINDOWS_LOCAL_PROJECTION.md`. The earlier unavailable
client receipt above is historical. `FILEMAN_ORCHESTRATOR_RUNTIME_DIR` chooses
an absolute private discovery directory for every default connection. It does
not install services or change readiness. Real Go/Rust/C++ live-query evidence
is separate from fixtures; supervisor restart and durable settings remain open.

The owner-approved typed local picker grant is reconciled in
`../../orchestrator/spec/contracts/DOCUMENT_PICKER_LOCAL.md`. The independently
installed source package defaults unavailable, requires explicit first-party
identity/root/purpose policy, and revokes each completed owned presentation.
It returns only revalidated selection; the host revalidates before its own I/O.
No daemon session, plugin privilege or inferred drive authority is fabricated.
Consumer instructions/evidence live in `../docs/document_picker.md`.

### Ordinary text predicate correction

The owner directed practical ordinary text consistency after the native indexed
fixture exposed text-as-exact-name substitution. Canonical reconciliation is
`../../orchestrator/spec/FRONTEND_TEXT_PREDICATE.md`. Ordinary text currently uses
bounded live name/path substring search even with a persistent catalogue; exact
criteria still use the catalogue. The frontend retains the one-operation API,
worker scheduling, source/completeness presentation and identity revalidation.
It must not claim indexed substring acceleration. Old exact text callers migrate
to explicit `filters.name`; issued cursor source/predicate remains bound.

### Search coverage preservation — 2026-10-03 UTC

**OBSERVED producer reply:** `orchestrator/src/kernel.rs` already forwards
catalogue stale/unavailable roots and warnings, or live unavailable paths,
warnings and scan identity. The C++ client previously discarded them.

**Frontend reply:** consume these existing fields as owned optional values;
preserve unknown versus explicitly empty coverage. Summarize accumulated gaps
across appended pages, reset them for replacement, and make zero displayed
results distinct from a guaranteed complete no-match. Path observation alone
does not establish that a cached criteria match is still current.

**Orchestrator reconciliation:** the source-only additive projection and failure,
ownership, optionality and UI rules are recorded in
`../../orchestrator/spec/FRONTEND_SEARCH_COVERAGE.md`. Existing wire versions,
Engine contracts and source-bound cursors are unchanged. Rebuild the C++ source
library and consumers together; no binary ABI compatibility is claimed.
Per-row identity/evidence preservation and currentness comparison remain open.

### Source object/revision preservation — 2026-10-03 UTC

**Orchestrator request:** preserve existing object identity and stored revision
facts through ORC-FE-001 without equating them with current filesystem facts.

**Observed producer reply:** Engine `api/types.go` and Orchestrator
`kernel.rs::search_response` already carry the runtime fields. Semantic fixture
identity spellings differ. Signed size/time and unsigned mode/generation require
exact integer projection, not unsigned-size substitution.

**Frontend reply:** retain owned source/current pairs through worker publication
and UI result lifetime. Refuse malformed projections; preserve absent fields.
Retire on replacement/navigation; keep original pairs when skipping duplicates.
Native operations continue their independent identity revalidation.

**Orchestrator reconciliation:**
`../../orchestrator/spec/FRONTEND_SEARCH_SOURCE_RECORDS.md` defines alias conflict,
integer domains, ownership, optionality and source-API rebuild rules before this
adapter repair. No provider call or capability is added. Identity equivalence,
full ranking/evidence projection and thumbnail eligibility remain unresolved.

### Retained match and request context — 2026-10-03

**Orchestrator request:** project existing rank/certainty/ordered evidence without
discarding unknown channel claims, and keep each displayed row's submitted query
and own page provenance. Appending must not relabel older rows with newer context.

**Observed producer reply:** api.Result/api.Evidence already serialize these
observations, including optional nested inert details and observation timestamps.
The source client can preserve them without changing Engine transport or storage.

**Frontend reply:** one immutable request owner per page records arguments used
for the actual client call; result cards describe their own retained page and
provider claims. Current path observations and operation validation remain separate.

**Orchestrator reconciliation:**
`../../orchestrator/spec/FRONTEND_SEARCH_MATCH_CONTEXT.md` records numeric domains,
optional values, inert details, lifetime and presentation before implementation.
This extends the source-record repair, with no new query capability or wire method.
Provider speed, current-identity comparison and thumbnail eligibility remain open.

### Ordinary local New Folder/Rename — 2026-10-03

**Orchestrator request:** honor the existing core-local-operation failure law
without making ordinary New Folder/Rename depend on an Engine grant or the
protected fixture quarantine. Preserve independent source/parent observation,
no-replace publication, explicit results, and honest one-step undo lifetime.

**Observed frontend reply:** the read-only launch audit found that controller
creation, command availability, and source/destination validation all use the
protected profile. A launch-flag change alone is insufficient. Browsing,
operation authority, and indexing consent must remain distinct.

**Reconciliation:** `../../orchestrator/spec/FRONTEND_ORDINARY_LOCAL_ACTIONS.md`
opens development of ordinary New Folder, same-parent Rename, and their Undo;
ordinary transfers, native Trash, root setup, and package acceptance remain
separate work. No new wire method or stable ABI is declared. The source-only
implementation and semantic review must precede any availability/delivery claim.

### Bounded selected-preview intake 001 — 2026-10-04 UTC

**Orchestrator request:** define an independently bounded first-party JPEG/PDF
render request/result profile. Retain the current UTF-8/PNG behavior while
comparing provider placement, source-read authority, resource enforcement and
failure semantics. Canonical intake:
`../../orchestrator/negotiations/FILE_MANAGER_PREVIEW_2026-10-04.md`.

**Observed frontend reply:** `Application::request_preview`,
`on_preview_availability_changed` and `apply_preview` already retain selected
generation/demand state and refuse obsolete UI publication. PR35/36 provide
fixture evidence and adaptable public PictureBox/Label composition. This is
consumer feasibility, not worker execution or a generalized decoding contract.
The consumer needs owned, validated raster results and structured terminal
reasons; it must retire output on selection/visibility/lifetime changes and
account for retained GUI.Forms copies. No foreign controls or worker callbacks
enter the frontend.

Selected-file experiments require current native identity/revision validation
but no persistent cache or Engine identity equivalence. Index-only thumbnail
eligibility, derivative storage and cross-component identity reconciliation
remain separate questions. The reviewed candidate paper is
`../../planning/PREVIEW_AND_THUMBNAIL_NEXT_SLICE.md`; its numbers are unmeasured
experimental limits, not this component's available contract.

**Disposition:** paper negotiation continues. No frontend adapter or new format
is admitted by this reply; ORC-PLG stubs and ADR-020 retain their current status.

### Selected-preview lifecycle fragment 002 — 2026-10-04 UTC

**Orchestrator request:** compare a fixed-client lifecycle with one replaceable
pending selection per window, one decode/result slot, explicit stop/reap,
validated source/raster result, expiring offer and named retirement effects.
The proposed fragment is `../../orchestrator/spec/SELECTED_PREVIEW_LIFECYCLE_DRAFT.md`.

**Frontend source reply:** `request_preview`, `apply_preview` and demand-loss
handling already distinguish current selection from stale completion. The
future adapter can map this selection/demand state to opaque request tickets;
it must not treat the laboratory's numeric source identifiers as native file
authority. Temporary hiding of completed content keeps its existing grant;
selection replacement and window destruction retire it. The consumer must
reject late/expired results and execute image retirement before admitting new
storage. Actual GUI.Forms copies, bulk transfer and cross-process source
validation remain unimplemented and cannot be inferred from this source reply.

**Reconciliation:** provider-independent fixture development is open under B0.
This records consumer feasibility only; no codec/provider adapter or new format
is admitted and no service capability is reported available.
