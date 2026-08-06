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
| ORC-LIF-001 | Service version, Core release readiness, lifecycle, status, shutdown, restart | Orchestrator spec | Orchestrator; future engine | frontend, CLI, tests | CLI + JSONL stdio + authenticated Unix local wire | stable 1.0 Core bootstrap projection; macOS discovery/status/shutdown implemented |
| ORC-NEG-001 | Project-local interface proposal, reply, and reconciliation | Orchestrator | Orchestrator specification process | all projects | Markdown ledgers | accepted process |
| ORC-ENG-001 | Exact/file query, inspect, evidence, pagination | engine semantics registered by Orchestrator | Go engine | frontend, Orchestrator | semantic v0 + JSON fixtures; production local wire gated | frozen-v0 semantics; provider and Orchestrator development JSONL query adapter implemented; installed adapter open |
| ORC-ENG-002 | Root policy, manual reconcile, integrity, rebuild administration | engine semantics registered by Orchestrator | Go engine | Orchestrator/admin tools | semantic v0 + JSON fixtures; authenticated admin wire gated | frozen-v0 semantics; provider implemented; Orchestrator adapter open |
| ORC-ENG-003 | Engine status and currentness snapshot | Orchestrator spec + engine | Go engine | Orchestrator, frontend diagnostics | snapshot semantic v0; separate local event-stream extension | snapshot frozen-v0; subscription extension negotiating |
| ORC-ENG-004 | Catalogue-independent bounded live filesystem search | Engine semantics registered by Orchestrator | Go engine | Orchestrator query broker, frontend through Orchestrator or registered direct fallback | semantic contract + JSON fixtures; production local wire gated | required product capability under ADR-008; negotiation round 006; provider work in progress |
| ORC-KOL-001 | Kolmogrov configuration, hash identity, candidate query, evidence | Kolmogrov + engine | Go engine | engine planner, conformance tools | internal library/service seam | negotiation round 001 |
| ORC-GUI-001 | GUI.Forms consumption manifest, stable C surface, and lifecycle | GUI.Forms registered by Orchestrator | GUI.Forms | C++ frontend | C ABI + manifest | negotiation round 001 |
| ORC-FE-001 | Frontend bootstrap session, lifecycle, availability, and fallback | Orchestrator + frontend | Orchestrator | File Manager, diagnostics, independent conformance client | atomic immutable snapshot over production local wire + thin C++ source client | stable 1.0 Core bootstrap projection; one-read Rust/C++ macOS bootstrap and restart client implemented |
| ORC-PLG-001 | Package identity, discovery, install, grant, lifecycle | Orchestrator | future Rust supervisor | frontend, CLI | local API | stubbed; source research only |
| ORC-PLG-002 | Sandboxed worker job and capability protocol | Orchestrator | future Rust supervisor/worker | plugin SDKs | hostile local wire | stubbed; source research only |
| ORC-PLG-003 | Preview and thumbnail jobs/results | Orchestrator | future plugin workers | frontend | worker protocol + validated bulk data | stubbed; source research only |
| ORC-PLG-004 | Search, virtual-system, and plugin-AI exchange | Orchestrator | future plugin workers | Orchestrator query broker | worker protocol | stubbed; source research only |
| ORC-HIV-001 | Provider schema and derived-generation deposit | Orchestrator | future hive service | plugin supervisor, extractors | capability API | stubbed |
| ORC-HIV-002 | Semantic fact operations | Orchestrator | future semantic hive | undecided | capability API | stubbed; architect design pending |
| ORC-HIV-003 | Hive lifecycle, quota, migration, sync/export | Orchestrator | future hive service | settings/admin, federation | privileged API | stubbed |
| ORC-HND-001 | Internal handler/type association resolution | Orchestrator | handler registry | frontend, CLI | local API | outline |
| ORC-CMD-001 | Declarative context-command registration/enumeration/invocation | Orchestrator | command registry/supervisor | frontend, plugins | local + worker API | outline |
| ORC-SET-001 | Core and plugin settings schema/value transaction | Orchestrator | settings service | frontend, CLI, plugins by namespace | local API | outline |
| ORC-CLI-001 | Human and structured local CLI grammar | Orchestrator | Orchestrator CLI | local humans, AI tools, developer agents | command + JSON + JSONL stdio | stable 1.0 bootstrap projection under ADR-010 |
| ORC-INT-001 | Platform handler/shell/desktop integration plan and status | Orchestrator | first-party platform adapter | frontend, CLI | privileged local API | outline |
| ORC-FED-001 | Machine/network catalogue and federated-query envelope | Orchestrator | remote Orchestrator/engine adapter | Orchestrator query broker | authenticated optional wire | deferred paper |
| ORC-AUD-001 | Local audit, provenance inspection, redacted export | Orchestrator | audit service | frontend, CLI | local API | outline |

## Explicitly forbidden edges

- plugin worker → Go engine writable/index API;
- plugin worker → GUI.Forms object or native window;
- GUI.Forms → engine, hive, plugin, or filesystem policy;
- engine → plugin executable code;
- AI client → source-file mutation through Orchestrator search/hive contracts;
- provider fact → exact filesystem field overwrite;
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
