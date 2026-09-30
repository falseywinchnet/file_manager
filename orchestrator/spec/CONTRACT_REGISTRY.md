# Master contract registry

Status: **canonical inventory; Core 1.0 compatibility horizon accepted under
ADR-010, Engine semantic v0 frozen independently, implementation stages vary**.

## Orchestrator Core 1.0 profile

Core 1.0 is a release/readiness profile, not a global API version. It requires
stable compatible projections of `ORC-COM-001`, `ORC-LIF-001`, `ORC-FE-001`,
and `ORC-CLI-001`, plus a production local transport, user-session discovery and
authentication, immutable availability snapshots, a release manifest, and an
independent client conformance suite. Other contract families may truthfully
remain unavailable, deferred, negotiating, or stubbed.

## Registry rules

- `Owner` means semantic authority, not necessarily runtime caller.
- `Provider` implements the responding side.
- Every consumer must pass the same golden fixtures for its projection.
- `stubbed` means a capability is named and reported unavailable but has no
  operation schema beyond status.
- An entry cannot reach `frozen-v0` until objects, errors, cancellation,
  backpressure, version negotiation, and security scope are complete.

| ID | Contract | Owner | Provider | Consumers | Projection | Status |
|---|---|---|---|---|---|---|
| ORC-COM-001 | Common terminal status, request envelope, and bootstrap provenance | Orchestrator spec | Orchestrator bootstrap | CLI, tests; future all projects | semantic types + JSON fixtures + ADR-009 local framing | stable 1.0 Core bootstrap projection under ADR-010; additive family expansion remains possible |
| ORC-LIF-001 | Service version, Core release readiness, lifecycle, status, shutdown, restart | Orchestrator spec | Orchestrator; future engine | frontend, CLI, tests | CLI + JSONL stdio + authenticated Unix local wire | stable 1.0 Core bootstrap projection; installed macOS discovery/status/shutdown/supervisor-reactivation conformant |
| ORC-NEG-001 | Project-local interface proposal, reply, and reconciliation | Orchestrator | Orchestrator specification process | all projects | Markdown ledgers | accepted process |
| ORC-ENG-001 | Exact/file query, inspect, evidence, pagination | engine semantics registered by Orchestrator | Go engine | frontend, Orchestrator | semantic v0 + JSON fixtures; authenticated `ENG1` installed query wire | frozen-v0 semantics; provider, development JSONL adapter, and contained M4 installed adapter implemented; other-platform promotion remains open |
| ORC-ENG-002 | Root policy, manual reconcile, integrity, rebuild administration | engine semantics registered by Orchestrator | Go engine | Orchestrator/admin tools | semantic v0 + JSON fixtures; authenticated `ENG1` admin wire | frozen-v0 semantics; provider and contained M4 Orchestrator admin adapter implemented with no automatic admin replay |
| ORC-ENG-003 | Engine status and currentness snapshot | Orchestrator spec + engine | Go engine | Orchestrator, frontend diagnostics | snapshot semantic v0; separate local event-stream extension | snapshot frozen-v0 and contained M4 installed snapshot implemented; subscription extension negotiating |
| ORC-ENG-004 | Catalogue-independent bounded live filesystem search | Engine semantics registered by Orchestrator | Go engine | Orchestrator query broker; frontend only through unified Orchestrator search or registered direct fallback | semantic contract + JSON fixtures + development JSONL/Rust/C++ and contained M4 installed conformance | provider development and contained installed adapter observed; 10k APFS scale point measured; million-entry and native NTFS/ext4 gates open |
| ORC-KOL-001 | Kolmogrov configuration, hash identity, candidate query, evidence | Kolmogrov + engine | Go engine | engine planner, conformance tools | internal library/service seam | negotiation round 001 |
| ORC-GUI-001 | GUI.Forms consumption manifest, stable C surface, and lifecycle | GUI.Forms registered by Orchestrator | GUI.Forms | C++ frontend | installed C++ package + C ABI + manifest | frozen v0 for `gui-forms-fm0-macos-arm64-2026-08-10`; later FM rows remain negotiating |
| ORC-GUI-002 | Web.Forms authoring schema, nested ambient context, GUI.Forms capability manifest, and generated-construction compatibility | Web.Forms source semantics + GUI.Forms control semantics, registered by Orchestrator | Python Web.Forms compiler + future GUI.Forms manifest | Web.Forms/GUI.Forms conformance; future frontend builds | two-stage dogfood IR, then versioned capability manifest and C++17-compatible generated public C++ | proposed; Python descriptor dogfood open, production schema and ABI not frozen |
| ORC-FE-001 | Frontend bootstrap session, lifecycle, availability, unified search, and fallback | Orchestrator + frontend | Orchestrator | File Manager, diagnostics, independent conformance client | atomic immutable snapshot plus `orchestrator.search` over production local wire + thin C++ source client | stable 1.0 Core bootstrap projection; additive unified search development operation passes zero-catalogue Rust/C++ route without frontend lane selection |
| ORC-APP-001 | First-party application identity, profile namespace, and capability snapshot | Orchestrator | Orchestrator application registry | File Manager, future Paint/Text Editor/Games, shared surfaces | local API + immutable snapshot | proposed in application-backbone round 001; no operation admitted |
| ORC-PCK-001 | File Selection Session policy, route, cancellation, and result | Orchestrator plus selected first-party/native provider | Orchestrator policy; UI provider remains negotiated | File Manager, future Paint/Text Editor | local API plus in-process GUI.Forms surface or native fallback | proposed; GUI.Forms/frontend replies pending |
| ORC-UI-001 | Bounded Orchestrator administration presentation model | Orchestrator settings/service registries | Orchestrator | File Manager settings; possible standalone first-party control shell | local immutable service snapshot + closed identity-bound command | frozen-v0 1.0 for Orchestrator/Engine contained profile; arbitrary server-driven UI forbidden |
| ORC-HLP-001 | Local application help-provider registration and topic resolution | Orchestrator registry + application package | Orchestrator resolver | File Manager, future Paint/Text Editor/Games/help dialogs | local API; content remains package-owned | proposed; no web/provider UI admitted |
| ORC-XFR-001 | First-party transfer-flavor identity and capability declaration | Orchestrator registry | applications/platform adapters | File Manager, future Paint/Text Editor | registry only; bytes use GUI.Forms/OS path | proposed; no hot-path relay admitted |
| ORC-PLG-001 | Package identity, discovery, install, grant, lifecycle | Orchestrator | future Rust supervisor | frontend, CLI | local API | stubbed; source research only |
| ORC-PLG-002 | Sandboxed worker job and capability protocol | Orchestrator | future Rust supervisor/worker | plugin SDKs | hostile local wire | stubbed; source research only |
| ORC-PLG-003 | Preview and thumbnail jobs/results | Orchestrator | future plugin workers | frontend | worker protocol + validated bulk data | stubbed; source research only |
| ORC-PLG-004 | Search, virtual-system, and plugin-AI exchange | Orchestrator | future plugin workers | Orchestrator query broker | worker protocol | stubbed; source research only |
| ORC-LEX-001 | Bounded exact lexical lookup and immutable corpus-generation enumeration | Orchestrator provider semantics + Lexicon | future Lexicon provider | File Manager query composition, Games/Crossword | local provider API; worker placement unresolved | proposed in Lexicon intake; no operation admitted and no Engine ownership |
| ORC-HIV-001 | Provider schema and derived-generation deposit | Orchestrator | future hive service | plugin supervisor, extractors | capability API | stubbed |
| ORC-HIV-002 | Semantic fact operations | Orchestrator | future semantic hive | undecided | capability API | stubbed; architect design pending |
| ORC-HIV-003 | Hive lifecycle, quota, migration, sync/export | Orchestrator | future hive service | settings/admin, federation | privileged API | stubbed |
| ORC-HND-001 | Internal handler/type association resolution | Orchestrator | handler registry | frontend, CLI | local API | outline |
| ORC-CMD-001 | Declarative context-command registration/enumeration/invocation | Orchestrator | command registry/supervisor | frontend, plugins | local + worker API | outline |
| ORC-SET-001 | Core and plugin settings schema/value transaction | Orchestrator | settings service | frontend, CLI; plugin namespace remains gated | local typed schema/snapshot/optimistic transaction | frozen-v0 1.0 for the bounded first-party scalar profile under ADR-018 |
| ORC-CLI-001 | Human and structured local CLI grammar | Orchestrator | Orchestrator CLI | local humans, AI tools, developer agents | command + JSON + JSONL stdio | stable 1.0 bootstrap projection under ADR-010 |
| ORC-INT-001 | Platform handler/shell/desktop integration plan and status | Orchestrator | first-party platform adapter | frontend, CLI | privileged local API | outline |
| ORC-FED-001 | Machine/network catalogue and federated-query envelope | Orchestrator | remote Orchestrator/engine adapter | Orchestrator query broker | authenticated optional wire | deferred paper |
| ORC-AUD-001 | Local audit, provenance inspection, redacted export | Orchestrator | audit service | frontend, CLI | local API | outline |

## Windows development consumption receipt (2026-09-29)

**GIVEN:** the current Windows recovery tranche may build the additive public
GUI.Forms Application projection carried by Plan Paint source `64248bcc06a1`
and its six locked patches. This is an `ORC-GUI-001` development consumption
projection with host protocol 7; the canonical GUI.Forms checkout owns the
backport and its installed SDK. Parent coordination records SDK build and
runtime evidence separately; no Windows SDK acceptance is claimed here yet.

The named macOS FM0 snapshot and its readiness manifest remain unchanged.
Windows installed Orchestrator discovery, authentication, local transport,
supervision and end-to-end frontend bootstrap remain unavailable. A compiled
C++ client with explicit unavailable errors does not satisfy those gates.

### Development presentation additions — 2026-09-29

The File Manager ergonomics slice consumes additive public GUI.Forms C++ font
metrics for MenuStrip, BreadcrumbTrail and PropertyList, PropertyList row height,
and an opt-in raised breadcrumb appearance. The owner explicitly directed
practical UI improvement using the Paint toolkit. Provider negotiation and
validation live in GUI.Forms/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md. These
in-process presentation settings add no wire operation, service authority or
stable ABI promise; existing Windows service availability remains unchanged.
### Native Windows service and offline picker reconciliation — 2026-09-29

**GIVEN:** the owner opened actual separate Engine integration and explicitly
approved trusted first-party offline picker authority. The unavailable receipt
above is the earlier checkpoint, superseded for explicit-process transport by
[WINDOWS_LOCAL_PROJECTION.md](WINDOWS_LOCAL_PROJECTION.md). ORC-LIF-001,
ORC-FE-001 and ORC-ENG-001/002/003/004 retain existing messages over current-user
ORC1/ENG1 named pipes. Engine reply 008 is reconciled there. Supervision,
durable settings and Windows release readiness remain open.

ORC-PCK-001 now has the bounded first-party in-process development projection
in [contracts/DOCUMENT_PICKER_LOCAL.md](contracts/DOCUMENT_PICKER_LOCAL.md).
Its explicit typed local grant is independent of daemon/index availability;
the future daemon/plugin capability service remains proposed.

ORC-GUI-001 additionally admits first-party development consumption of public
TextBox multiline/wrap/newline/Tab/validation APIs. The GUI.Forms negotiation
receipt and `gui_forms/experiments/MULTILINE_TEXT_BOX_2026-09-29.md` record
65/65 Windows tests and SDK installation. The 1 MiB document and 4096-byte logical
line guards remain provisional provider limits, not final Notepad semantics.
Consumers validate before replacement and report refusal without truncation.
No stable C ABI, large-document decision, IME/bidi/accessibility parity or final
application acceptance is inferred.

**OBSERVED source addition, 2026-09-30:** ORC-GUI-001's development TextBox
surface includes `clear_undo_history()` for an explicit consumer-owned
successful-save boundary. It clears undo/redo without changing document text,
selection or presentation and emits no edit notifications. Save success and
modified-state policy stay with the consumer. The provider negotiation record
documents focused tests and unchanged object layout; a matching provider build
is still required for the new symbol. This is source availability only: no SDK
installation or released application update is recorded by this addition.

**MEASURED subsequent local publication, 2026-09-30:** the matching Windows
development provider and picker SDKs were installed into separate versioned
`dbe3766` prefixes. All 65 toolkit tests, two picker tests and two independent
installed-package consumer checks passed, including a call to the new history
symbol. The GUI.Forms negotiation record names the prefixes and hashes.
SwiftEdit received that coherent checkpoint for its own build; prior SDKs and
released File Manager binaries remain unchanged. Consumer application
acceptance and any other platform publication are separate evidence.

### Ordinary frontend text correction — 2026-09-29

The owner directed the bounded ORC-FE-001 predicate reconciliation in
[FRONTEND_TEXT_PREDICATE.md](FRONTEND_TEXT_PREDICATE.md) after measured indexed/live
mismatch. New ordinary text has the existing live name/path substring meaning;
the current catalogue adapter plans it as unsupported before sending a query,
then uses the allowed live fallback. Exact `filters.name`, successful catalogue
no-match, metadata authority and issued cursor source/predicate remain unchanged.
This is a documented development compatibility change; indexed substring
acceleration is not implemented or claimed.

## Explicitly forbidden edges

- plugin worker → Go engine writable/index API;
- Lexicon provider → Go engine fact mutation or fake file/path result;
- plugin worker → GUI.Forms object or native window;
- GUI.Forms → engine, hive, plugin, or filesystem policy;
- Orchestrator → arbitrary GUI control/layout/native-window injection;
- picker caller → unregistered capability widening through presentation
  overrides;
- Orchestrator → drag pointer-motion or canvas-byte hot path;
- engine → plugin executable code;
- AI client → source-file mutation through Orchestrator search/hive contracts;
- provider fact → exact filesystem field overwrite;
- plugin worker → ambient destination path, arbitrary directory enumeration, or
  direct source replace/delete under a transform/extraction job;
- raw Rust ABI, Go ABI, C++ object layout, allocator, exception, panic, or
  process-local pointer crossing a registered boundary.

## Required contract files

- [`contracts/COMMON.md`](contracts/COMMON.md)
- [`contracts/ENGINE_AND_KOLMOGROV.md`](contracts/ENGINE_AND_KOLMOGROV.md)
- [`contracts/FRONTEND_AND_GUI_FORMS.md`](contracts/FRONTEND_AND_GUI_FORMS.md)
- [`contracts/PLUGIN_AND_CAPABILITIES.md`](contracts/PLUGIN_AND_CAPABILITIES.md)
- [`contracts/HIVES_AND_SETTINGS.md`](contracts/HIVES_AND_SETTINGS.md)
- [`contracts/HANDLERS_COMMANDS_AND_PLATFORM.md`](contracts/HANDLERS_COMMANDS_AND_PLATFORM.md)
- [`contracts/CLI_AI_AND_FEDERATION.md`](contracts/CLI_AI_AND_FEDERATION.md)

Application-backbone families remain proposal-only in
[`../proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md`](../proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md)
until project-local replies permit canonical contract files.

`ORC-LEX-001` remains proposal-only in
[`../../lexicon/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../lexicon/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md).
Archive Viewer and Image Converter do not yet receive new frozen IDs; their
host-mediated capability proposals live under
[`../proposals/first_party_extensions/`](../proposals/first_party_extensions/)
until the plugin-supervisor and frontend/picker negotiations open.
