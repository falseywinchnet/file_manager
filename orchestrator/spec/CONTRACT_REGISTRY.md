# Master contract registry

Status: **canonical inventory; implementation versions not frozen**.

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
| ORC-COM-001 | Common terminal status, request envelope, and bootstrap provenance | Orchestrator spec | Orchestrator bootstrap | CLI, tests; future all projects | semantic types + JSON fixtures | fixture-draft, implemented bootstrap subset |
| ORC-LIF-001 | Service version, lifecycle, status, shutdown, restart | Orchestrator spec | Orchestrator; future engine | frontend, CLI, tests | CLI + JSONL stdio | fixture-draft, implemented bootstrap subset |
| ORC-NEG-001 | Project-local interface proposal, reply, and reconciliation | Orchestrator | Orchestrator specification process | all projects | Markdown ledgers | accepted process |
| ORC-ENG-001 | Exact/file query, inspect, evidence, pagination | engine semantics registered by Orchestrator | Go engine | frontend, Orchestrator | framed local API + JSONL fixtures | negotiation round 001 |
| ORC-ENG-002 | Root policy, scan, integrity, rebuild administration | engine semantics registered by Orchestrator | Go engine | Orchestrator/admin tools | privileged local API | negotiation round 001 |
| ORC-ENG-003 | Engine generation/status/backlog event stream | Orchestrator spec + engine | Go engine | Orchestrator, frontend diagnostics | local event stream | negotiation round 001 |
| ORC-KOL-001 | Kolmogrov configuration, hash identity, candidate query, evidence | Kolmogrov + engine | Go engine | engine planner, conformance tools | internal library/service seam | negotiation round 001 |
| ORC-GUI-001 | GUI.Forms consumption manifest, stable C surface, and lifecycle | GUI.Forms registered by Orchestrator | GUI.Forms | C++ frontend | C ABI + manifest | negotiation round 001 |
| ORC-FE-001 | Frontend integration session, lifecycle, availability, and fallback | frontend + Orchestrator | Orchestrator/C++ frontend | File Manager, diagnostics | local wire/C client | negotiation round 001 |
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
| ORC-CLI-001 | Human and structured local CLI grammar | Orchestrator | Orchestrator CLI | local humans, AI tools, developer agents | command + JSON + JSONL stdio | fixture-draft, implemented bootstrap subset |
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
