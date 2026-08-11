# File Manager total implementation plan

Date: 2026-08-07; execution update 2026-08-11.

Status: **approved execution program; the corrective protected-root macOS 1.0
profile passed its M4 control audit on 2026-08-11 after the rejected prototype
was replaced**. F10 daily-root/distribution promotion and F11 other-platform
work remain open. No later tranche is promoted from screenshots or subsystem
tests alone.

Owner direction:
[`OWNER_DIRECTION_2026-08-10.md`](OWNER_DIRECTION_2026-08-10.md), superseding
the execution timing in the 2026-08-07 record.

## 1. Outcome

Deliver File Manager as the architect's operational, local-machine file
manager, beginning on macOS and then preserving the same product model on
Windows and Linux. The first honest integrated system must:

1. start as a native packaged C++ GUI.Forms application;
2. obtain one live authenticated Orchestrator bootstrap snapshot;
3. remain a useful directory navigator when Orchestrator or Engine augmentation
   is unavailable;
4. navigate, select, inspect, rename, create, move, copy, drag, delete, and undo
   the admitted operations inside a protected root;
5. search through Engine with explicit catalogue, stale, partial, live,
   unavailable, and degraded source states;
6. expose product-owned multi-tab settings backed by negotiated Orchestrator
   transactions and service controls;
7. publish admitted suite-application launch commands through OS-native menus;
8. pass keyboard, IME, accessibility, scale, high-contrast, reduced-motion,
   failure, cancellation, recovery, and resource gates; and
9. progress from disposable-root dogfood to explicitly admitted daily roots
   without confusing filesystem truth, Engine projections, or Orchestrator
   policy authority.

## 2. Evidence baseline

The plan begins from these facts, not from the apparent completeness of a
demo:

| Object | Current status | Consequence |
|---|---|---|
| Owner opening direction | **GIVEN, recorded 2026-08-10** | Implementation, supporting repo extensions, and M4 dogfood are open. |
| Orchestrator Core 1.0 | **MEASURED installed macOS release ready** | Frontend consumes the live independent C++ bootstrap; supervisor-host state remains visible at runtime. |
| GUI.Forms FM0 | **MEASURED named macOS arm64 snapshot ready** | `GUIForms::Application` and deterministic fonts pass clean external install/consume. |
| GUI.Forms demoboard | **MEASURED partial D0-D6/D9** | Reuse its public-control evidence; do not copy its fixture model or now-rejected path matrix into the product. |
| Engine | **MEASURED named M4 installed service and contained File Manager profile passed** | Exact query/admin, identity rotation and authenticated Orchestrator routing are live; daily-root and other-platform promotion remain open. |
| `ORC-ENG-004` | **MEASURED development and contained installed routes pass** | Keep source/partial/fallback truth visible; native NTFS/ext4 and million-entry gates remain open. |
| Settings/services | **MEASURED ORC-SET/ORC-UI contained profile passed** | Native composition uses typed transactions and identity-bound commands; general audit/event replay remains open. |
| Handlers/commands | **OBSERVED outline/deferred** | Admit real dynamic operations only after canonical contracts and fixtures; trusted built-ins remain application-owned. |
| Plugins/semantic facts | **OBSERVED stubbed** | Display honest unavailability; do not infer operations. |

Historical baseline checks ran on 2026-08-07. The first execution evidence on
2026-08-10 is recorded in
`../results/2026-08-10-m4-dogfood/README.md` and supersedes the old readiness
values above without erasing the historical measurements.

Baseline checks run on 2026-08-07:

- focused GUI.Forms File Manager demoboard tests: 2/2 passed;
- Engine `go test ./...`: passed;
- Orchestrator `cargo test`: passed;
- `orchestrator release --json`: passed as a projection and reported
  `ready: false`, with `daemon.discovery` pending.

These results establish healthy slices only within their recorded scope.

### 2026-08-10 execution checkpoint

| Tranche | Measured status | Remaining edge |
|---|---|---|
| F1-F3 shell/navigation | **MEASURED protected-root implementation** | Daily-root, full accessibility and performance promotion remain F10 gates. |
| F4 operations | **MEASURED disposable-root implementation** | Crash/restart journal recovery and physical fault campaigns remain open. |
| F5 settings/services | **MEASURED contained installed route** | General audit/event replay and providers beyond the two admitted services remain open. |
| F6 search | **MEASURED installed Orchestrator/Engine route** | Broader corpus/performance and non-APFS platform campaigns remain open. |
| F7 commands/previews/menus | **MEASURED trusted built-in subset** | Dynamic handlers/commands, plugin previews and native suite menus remain gated. |
| F8 picker | **MEASURED installed package and external consumer** | VoiceOver, modality/focus restoration and optional native fallback execution remain promotion work. |
| F9 full chain | **MEASURED live contained components; harness partial** | One-command provisioning, corruption/reinstall and exact cleanup campaign remain open. |
| F10-F11 | **OPEN** | Developer ID/notarization, admitted daily roots, Windows and Linux. |

## 3. Authority and process topology

| Responsibility | Authority/provider | Frontend rule |
|---|---|---|
| file bytes, directory contents, platform identity, mutations | filesystem plus first-party platform operation adapter | never infer truth from Engine rows; revalidate before mutation |
| current location, history, selection, panes, view, edit and operation presentation | File Manager frontend | one location per window; no tab or dual-pane state |
| retained controls, layout, drawing, input, accessibility publication, host services | GUI.Forms | public installed surface only; reusable deficits return to GUI.Forms |
| capability, availability, settings, handlers, commands, app registry, service policy | Orchestrator | consume versioned snapshots/transactions; no frontend policy database |
| exact catalogue, retrieval generations, indexed evidence, live search | Engine | consume registered contracts; no Go type or store leakage |
| plugin execution and hostile supervision | future Orchestrator subsystem | no worker in the frontend process and no plugin controls |
| platform menus, drag/drop, dialogs, desktop integration and packaging | GUI.Forms host plus first-party frontend adapters | platform difference is capability-reported, not hidden |

The normal runtime chain is:

```text
OS host / GUI.Forms
        -> File Manager retained application model
        -> frontend-owned bounded I/O workers
        -> authenticated Orchestrator Core session
        -> registered Engine query/admin adapter
        -> exact catalogue or bounded live-filesystem lane
```

Directory navigation remains a direct first-party filesystem activity inside
the frontend's platform boundary. Search does not become an application-owned
tree walk; its live traversal is `ORC-ENG-004` and remains Engine-owned.

## 4. Product corrections to land before visual freeze

### 4.1 Path instrument

- Replace the demoboard's terminal-button/full-path matrix as the primary edit
  route with one stable `BreadcrumbTrail` composition.
- Render adjacent segments as a continuous chevron cascade with shared edges,
  compact vertical metrics, explicit overflow, stable segment identities, and
  platform-path direction isolation.
- Give the trail an inline edit operator. Activation converts the same retained
  identity into an editor, selects according to the path-edit rule, and keeps
  its row height and surrounding search geometry stable.
- Keep canonical resolved destination, validation, generation-cancelled
  suggestions, Tab acceptance, Enter commit, Escape rollback, click-away, focus
  restoration, text ranges, IME geometry, and accessibility relations.
- Treat any recent-full-trail history surface as a separate optional command;
  do not let it own path editing.

### 4.2 Ribbon/control shelf

- Preserve the concept atlas's compact Office Pearl command geography rather
  than the demoboard's flatter toolbar reading.
- Bind ribbon, shortcuts, accessibility actions, context menus, and settings
  visibility to one stable command identity.
- Complete group captions, labelled large/small command hierarchy,
  split/dropdown commands, contextual membership, key tips, disabled reasons,
  selected/multi-selected states, priority reflow, overflow, and 125-200% text
  and display scale.
- Keep Move/Copy, Delete, View, Sort, and Properties in the permanent shelf.
  Preserve the accepted context-only boundaries for New Folder, Rename, and
  Open With unless a later owner record changes them.
- Freeze numerical material, spacing, icon, and break-point tokens only after
  native screenshot, input, contrast, damage, and first-folder-paint review.

### 4.3 Native application menus

- Add a portable value-only native menu model to the GUI.Forms host protocol;
  it contains stable command IDs, titles, shortcuts, enabled/checked state,
  nesting, and activation results, never native handles in portable code.
- Project it to `NSMenu` on macOS, native menus on Win32, and the admitted
  Linux host route. Keep capability reporting honest where a desktop does not
  provide a global menu.
- Reserve a product-owned native menu group for installed Malkuth applications.
  Populate it from an immutable Orchestrator application-availability snapshot.
- Hide or factually disable unavailable applications according to one declared
  rule. Never fabricate future Games, Paint, or Text Editor binaries.
- Keep suite launching out of the styled in-window ribbon. File commands may
  still use the ribbon and contextual surfaces.

### 4.4 Settings surface

The multi-tab topology is **GIVEN**. The following tab names/grouping are a
**CANDIDATE information architecture** to validate against real schemas:

| Tab | Owned content |
|---|---|
| General | startup, window restoration, language, ordinary behavior |
| Appearance & Access | atmosphere/construction, density, sounds, motion, contrast, text/display scale status |
| Navigation & Views | tree mode, hidden files, extensions, default view, sort, breadcrumb/history behavior |
| Search & Indexing | consent roots, exclusions, Engine generation/currentness, live fallback, reconcile/rebuild |
| Services | Orchestrator and Engine identity, lifecycle, availability, restart/shutdown, bounded diagnostics |
| Handlers & Commands | internal handler choices, context-menu visibility/order, terminal choice, trusted built-ins |
| Previews & Extensions | preview/thumbnail providers and future plugin status/grants when their gates open |
| Applications | admitted suite apps, picker profiles, native-menu visibility and availability |
| Privacy & Data | hives/provider data classes, quotas, erase/reset/export boundaries when implemented |
| Advanced | contract versions, resource limits, logs/audit locators, repair tools and explicit dangerous actions |

Every row declares a stable field ID, owner, type, bounds, default, current
value revision, sensitivity, restart effect, availability state, validation,
apply/cancel/reset semantics, and error provenance. Orchestrator may provide
typed schemas and state but never arbitrary control trees. Expensive or
administrative actions are commands, not Boolean settings disguised as values.

## 5. Required GUI.Forms delta

GUI.Forms work stays reusable and independently tested. The minimum File
Manager promotion list is:

1. publish the named FM0 manifest, exact installed targets, compatibility
   horizon, capability states, and test/trace digests;
2. close custom-chrome host behavior without losing system move/resize/
   minimize/maximize/snap/window identity;
3. add the continuous chevron breadcrumb plus identity-preserving inline editor
   and suggestion anchor;
4. finish ribbon groups, key tips, split items, command-state binding, and
   responsive overflow;
5. add portable-to-native application menu publication;
6. close provider-scale TreeView/ObjectView, inline object-label editing,
   details columns, empty/unavailable states, and stable mutation anchoring;
7. close outbound drag, clipboard, cross-window participation, autoscroll, and
   external-peer conformance;
8. close native VoiceOver first, then UIA and AT-SPI publishers, including
   virtual children, focused element, text ranges, actions, and notifications;
9. close physical IME/candidate geometry, selection/range bounds, per-monitor
   scale, high contrast, and large-text evidence;
10. finish the admitted House Material role pack, product body-font license and
    manifest, size-specific icons, theme state matrix, and visual performance
    counters; and
11. maintain a clean `find_package(GUIForms)` install/consume path with no
    demoboard or build-tree dependency.

No frontend milestone may implement a substitute control, native host escape,
renderer call, private include, or second accessibility tree when one of these
is missing.

## 6. Required Orchestrator delta

1. Complete the real installed macOS LaunchAgent lifecycle trial and publish a
   Core 1.0 manifest with `ready: true` only after its declared evidence passes.
2. Reconcile the GUI.Forms reply into `ORC-GUI-001` and surface its named
   manifest state in the atomic frontend bootstrap.
3. Keep the C++ bootstrap client source-consumable and bounded; the frontend
   calls it on its own I/O worker and posts immutable values to the UI thread.
4. Implement and freeze `ORC-SET-001`: schema enumeration, value snapshot,
   optimistic transaction, validation, reset, migration effect, secret-handle
   treatment, cancellation, and audit result.
5. Implement `ORC-UI-001` as bounded administration semantics, not remote UI:
   service status, allowed control commands, progress, terminal result, and
   redacted diagnostics.
6. Complete Engine adapter routing for `ORC-ENG-001` through `004`, including
   installed discovery/authentication, query/admin separation, fallback law,
   cancellation, deadlines, paging, and source-explicit results.
7. Implement immutable handler and command snapshots before dynamic menu
   population; then implement trusted checksum/terminal identities without
   confusing them with plugins.
8. Define the first-party application registry/launch intent needed by native
   suite menus. Availability must reflect installed admitted artifacts.
9. Keep plugin, semantic, hive, handler, command, and application families
   unavailable/stubbed until their actual contracts pass; no empty success.

ADR-018 resolves the bounded first-party scalar settings store after the M4
JSON/SQLite comparison. It selects atomic canonical JSON plus one verified
previous snapshot for this profile only. ADR-019 resolves the contained
installed Engine route and identity-bound `ORC-UI-001` service commands. A
general operational registry, plugin settings store, or universal hive remains
unselected.

## 7. Required Engine delta

1. Implement Engine reply 006 and `engine.query_live` exactly as the independent
   zero-catalogue lane required by ADR-008.
2. Prove authorized-root containment, no directory-link traversal, bounded open
   descriptors/state, progressive pages, expiring source-bound cursors,
   cancellation, mutation/permission partials, and zero durable writes.
3. Run native APFS first, then NTFS/ext4 identity and containment fixtures plus
   wide/deep/million-entry resource measurements.
4. Promote the exact query/inspect/status projection through a separately built
   client and an authenticated installed query endpoint; preserve admin
   separation.
5. Integrate the accepted durable-generation/currentness work only through its
   existing measurement/ADR gates. Do not make frontend dogfood an excuse to
   promote an unaccepted compaction candidate.
6. Add the minimum admitted lexical filename/path channel and evidence-preserving
   pagination. Kolmogrov remains a separately gated candidate lane and does not
   block exact/lexical integration.
7. Expose current, stale, rebuilding, quarantined, unavailable, coverage-
   incomplete, and partial states needed by the Settings and search surfaces.

## 8. Frontend implementation sequence

### F0 — close the opening evidence

- Record this owner direction in the frontend gate ledger.
- Complete the Orchestrator installed launchd trial.
- Obtain GUI.Forms' named FM0 reply and run a clean external install/consume
  probe from outside `gui_forms/`.
- Reconcile the new path and native-suite-menu requirements into GUI.Forms and
  Orchestrator negotiation notes before freezing adapter spelling.

Exit: the live atomic bootstrap says the Orchestrator gate is satisfied, names
the ready GUI.Forms manifest, and this record supplies architect direction.

### F1 — application skeleton and live bootstrap

- Create an independently buildable C++20 frontend with warnings-as-errors,
  sanitizer presets, install/package skeleton, and no source dependency on
  demoboard code.
- Add application/window lifecycle, UI-thread state store, bounded I/O executor,
  immutable result posting, cancellation ownership, structured diagnostics,
  and deterministic headless ports.
- Consume the live Orchestrator C++ source client before constructing
  provider-dependent state. Render ready/degraded/incompatible/unavailable
  states without blocking the base window.
- Create frontend-owned interfaces for filesystem navigation, file operations,
  search, settings, preview, handlers/commands, native app registry, and clock.
  Their fixture implementations are labelled `simulated`.

Exit: clean build, live bootstrap/restart/disconnect run, deterministic shutdown,
and no Engine process required.

### F2 — truthful deterministic product shell

- Build the Watercolor/Office Pearl/Graphite one-location window using public
  GUI.Forms: custom title, polished ribbon, dense inline breadcrumb, separate
  subtree search, collapsible tree, central object field, selection/properties,
  and status.
- Port demoboard fixture scenarios through frontend-owned models; do not link
  demoboard model code.
- Complete icon/list/details modes, selection/focus independence, history,
  responsive pane/ribbon priorities, scroll ownership, empty/degraded states,
  and semantic task order.
- Add headless traces before native screenshot approval.

Exit: Frontend 001's deterministic and visual gates pass against the named FM0
snapshot, including the corrected path instrument.

### F3 — protected live filesystem navigation

- Create a disposable corpus with deep/wide trees, Unicode, combining marks,
  long paths, hidden objects, links, packages, permissions, mount boundaries,
  replacements, and concurrent mutation.
- Implement the macOS navigation adapter using exact platform observations and
  capability-rooted access. Enumeration is asynchronous, generation-tagged,
  cancellable, and bounded; stale pages cannot overwrite a newer location.
- Prove back/forward/up, direct inline path navigation, tree synchronization,
  package/symlink policy, view/sort state, selection preservation, and
  unavailable-volume recovery.
- Keep Engine and Orchestrator augmentation removable throughout the slice.

Exit: repeated real navigation within the protected root with zero out-of-root
access and no UI stall when services are stopped.

### F4 — protected mutations and transfer

- Define and approve the file-operation oracle before implementation: object
  preconditions, collision/replacement, same/cross-volume behavior, atomic
  publication, cancellation, partial failure, trash, and one-step delete/rename
  undo.
- Implement New Folder, inline rename with basename selection, copy, move,
  recycle/delete, admitted undo, and drag/drop through first-party platform
  adapters.
- Revalidate identity at commit, never use Engine records as mutation authority,
  and preserve recoverable operation evidence.
- Add ordinary blocking dialogs and the complex-operation drawer only from
  GUI.Forms primitives; inject permission, disk-full, disappearance, collision,
  partial-copy, cancellation, and crash faults.

Exit: correctness oracle and fault suite pass in the protected root; no daily
personal path is admitted.

### F5 — settings and service control

- Build the tabbed surface first against typed deterministic schemas, including
  unavailable/deferred/stubbed examples.
- Implement real settings only after `ORC-SET-001` fixtures and store ADR pass.
- Connect Engine root plan/apply, consent/exclusion, status, reconcile, rebuild,
  and service controls through Orchestrator-authorized admin routes.
- Add validation, optimistic-revision conflict handling, apply/cancel/reset,
  restart-required presentation, progress, diagnostics, keyboard, accessibility,
  and narrow/large-text layouts.
- Never let a settings tab write an unowned file or bypass the same transaction
  used by the CLI.

Exit: UI and CLI observe identical committed values and service results across
restart, stale revision, invalid value, unavailable provider, and recovery.

### F6 — Engine search through Orchestrator

- Start with frozen exact catalogue semantics and explicit development status in
  the disposable corpus; then replace development JSONL routing with the
  installed authenticated adapter.
- Integrate `ORC-ENG-004` only after its provider and conformance gates pass.
- Implement current-subtree queries, debounce/cancellation, revisioned result
  replacement, focus/selection/scroll preservation, correspondence expansion,
  evidence, excerpts, pagination, no-result, stale, partial, rebuild,
  quarantine, provider absence, and live-source states.
- Verify the fallback law: allowed catalogue failures may start a new live
  query; denial/cancel/timeout/budget and authoritative no-match may not.
- Prove ordinary directory navigation while Engine and Orchestrator are each
  killed and restarted independently.

Exit: a live frontend navigates the real disposable corpus, finds known files
through catalogue and zero-catalogue lanes, and reports each authority/source
truthfully.

### F7 — handlers, commands, previews, native app menus

- Add internal handler resolution and Open With only after `ORC-HND-001` passes.
- Add context-menu configuration and trusted checksum/terminal commands only
  after `ORC-CMD-001` passes.
- Add isolated preview/thumbnail workers only after their supervision contracts
  pass; continue to show safe built-in facts when absent.
- Publish admitted installed applications through the native menu model and
  launch through an Orchestrator/first-party platform intent. Do not couple
  launch availability to frontend build-time knowledge.

Exit: service/plugin absence cannot alter base control composition or crash the
frontend; native menu state tracks immutable availability snapshots.

### F8 — reusable Document Picker

- Extract `FileBrowserModel`, selection controller, bounded picker profile, and
  GUI.Forms view from the proven navigation substrate.
- Prove open, multi-open, folder, save-as, import/export, app-scoped hidden
  policy, overwrite, cancellation, native fallback, and focus/modality.
- Package it independently from the File Manager executable; future consumers
  do not inherit File Manager's full panels or private state.

Exit: a separately built first-party test consumer uses the installed picker
package without linking the File Manager executable.

### F9 — contained full-chain dogfood

- Build one repeatable harness that provisions the protected root, Engine store,
  Orchestrator runtime, settings store, logs, sockets, and frontend profile
  under explicit disposable paths.
- Install/start Orchestrator, verify Core manifest, start the registered Engine,
  admit only the disposable root, reconcile it, launch File Manager, navigate,
  mutate, search, restart each service, corrupt disposable projections, rebuild,
  and cleanly remove every artifact.
- Record exact revisions, manifests, commands, corpus digest, latency/resource
  distributions, screenshots, semantic snapshots, failures, and what the run
  does not prove.
- Keep destructive campaigns in this harness even after read-only daily
  navigation expands.

Exit: repeatable end-to-end macOS run with no path escape, no authority
confusion, no service-caused input freeze, and successful reinstall/recovery.

### F10 — protected daily replacement and packaging

- Produce signed/notarized macOS artifacts, consent/setup flow, explicit root
  admission, visible index agent, uninstall/recovery, schema migration, and
  rollback to the prior compatible component manifest.
- Admit daily roots gradually: read-only navigation, then indexed search, then
  non-destructive operations, then ordinary mutations after each gate passes.
- Keep Finder available as recovery until launch, navigation, operations,
  search, settings, preview, handlers, and failure recovery meet the daily
  workflow definition.

Exit: the architect can use File Manager for ordinary daily work without a
known correctness, accessibility, or service-stall blocker.

### F11 — Windows and Linux

- Port host, platform identity/operations, installed Orchestrator and Engine
  transports, native menus, drag/drop, accessibility, packaging, and service
  activation against the same semantic contracts.
- Run physical NTFS and ext4 campaigns; Wine/Lima remain compatibility aids,
  not native promotion evidence.
- Preserve honest platform differences without forking the product topology or
  visual constitution by default.

Exit: Malkuth 1.0 cross-platform acceptance may begin; later Paint, Text Editor,
Games, Lexicon, and plugins remain independent release-manifest members.

## 9. Verification matrix

Every tranche carries the smallest applicable set below:

| Layer | Required evidence |
|---|---|
| component | GUI.Forms CTest, Engine test/race/vet, Orchestrator fmt/test/clippy, frontend unit/sanitizer/static boundary checks |
| contract | canonical golden/hostile fixtures, cross-version rejection, bounds, cancellation, deadlines, source/authority identity |
| headless UI | retained IDs, layout, focus, command, semantics, damage, clock, popup, resize, shutdown traces |
| native UI | pointer/keyboard/IME, window/menu/drag/dialog, VoiceOver then UIA/AT-SPI, scale, contrast, reduced motion, sound-off |
| filesystem | identity, path replacement, links, packages, Unicode, permissions, storms, missing volumes, collisions, disk full, crash/restart |
| search | exact/live equivalence on quiescent fixtures, fallback law, paging, source, stale/partial states, first-result and p50/p95/p99/max |
| performance | cold/warm launch, first folder paint, input/frame stalls, memory/CPU idle, damage, large directory realization, service restart |
| package | clean install, first activation, update/migration, shutdown, uninstall, artifact provenance, no network requirement |

No milestone is complete solely because a screenshot looks right or a fixture
returns data. Native behavior, semantic publication, authority, and failure
containment receive independent evidence.

## 10. Protected-root harness requirements

The harness must use explicit absolute paths created for the run and refuse a
workspace root, home directory, repository parent, `/`, or an unresolved
symlink as a mutation target. It records and validates:

- one corpus root and one separate Engine store;
- one separate Orchestrator runtime/settings root;
- one frontend profile/cache root;
- canonical device/volume and root identity before every destructive phase;
- an allowlist token inherited by filesystem operations, Engine root policy,
  and test fixtures;
- no writes inside the indexed source tree except operations intentionally
  requested through the File Manager mutation adapter;
- cleanup that removes only the exact harness-owned paths after processes stop;
  and
- a retained manifest if cleanup fails, so recovery is inspectable.

A literal macOS `chroot` and a capability-rooted disposable APFS volume are not
equivalent. The former complicates GUI/window-server and launchd integration and
is not by itself a complete sandbox. The implementation must record which
meaning the owner requires before the F9 harness is frozen.

## 11. Promotion and stop rules

- Stop a frontend slice when it needs a private GUI.Forms facility; promote the
  reusable capability in GUI.Forms.
- Stop a cross-process adapter when the canonical Orchestrator contract lacks a
  reply/reconciliation; do not freeze an implementation accident.
- Stop a mutation campaign on any object-identity ambiguity or out-of-root
  observation. Correctness outranks progress.
- Stop a search promotion when unavailable/partial/stale evidence is collapsed
  into empty success or when a result cannot resolve to exact filesystem
  identity.
- Preserve failed measurements and rejected UI/algorithm variants.
- Do not use future application entries to open Paint, Text Editor, Games,
  Lexicon, or plugin implementation gates.

## 12. First execution backlog

The first implementable queue, in strict order, is:

1. close Orchestrator's installed launchd evidence;
2. obtain and reconcile GUI.Forms' named FM0 manifest;
3. replace the demoboard path-matrix requirement with the reusable chevron/
   inline-editor requirement and add native application-menu capability;
4. prove a clean external GUI.Forms install/consume build;
5. scaffold `frontend/` and connect the live Orchestrator bootstrap;
6. build the corrected deterministic shell;
7. add protected read-only filesystem navigation;
8. approve the file-operation oracle and then add protected mutations;
9. implement settings contracts/surface and Engine `ORC-ENG-004` in their
   owning components;
10. run the first full-chain contained navigation/search dogfood.

Work does not skip from healthy component tests to daily personal-file dogfood.
Each transition is an evidence gate with a reversible fallback.
