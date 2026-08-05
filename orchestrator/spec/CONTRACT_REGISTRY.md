# Master contract registry

Status: **canonical inventory; implementation versions not frozen**.

## Registry rules

- `Owner` means semantic authority, not necessarily runtime caller.
- `Provider` implements the responding side.
- Every consumer must pass the same golden fixtures for its projection.
- `paper` means definition work is permitted but runtime implementation is not.
- An entry cannot reach `frozen-v0` until objects, errors, cancellation,
  backpressure, version negotiation, and security scope are complete.

| ID | Contract | Owner | Provider | Consumers | Projection | Status |
|---|---|---|---|---|---|---|
| ORC-COM-001 | Common identities, generations, errors, provenance | Oracle spec | all providers | all projects | semantic types | paper |
| ORC-LIF-001 | Service discovery, version, health, shutdown, restart | Oracle spec | Oracle, engine | frontend, CLI, tests | local wire | paper |
| ORC-ENG-001 | Exact/file query, inspect, evidence, pagination | engine semantics registered by Oracle | Go engine | frontend, Oracle | framed local API + JSONL fixtures | importing v0 |
| ORC-ENG-002 | Root policy, scan, integrity, rebuild administration | engine semantics registered by Oracle | Go engine | Oracle/admin tools | privileged local API | importing v0 |
| ORC-ENG-003 | Engine generation/status/backlog event stream | Oracle spec + engine | Go engine | Oracle, frontend diagnostics | local event stream | outline |
| ORC-KOL-001 | Kolmogrov configuration, hash identity, candidate query, evidence | Kolmogrov + engine | Go engine | engine planner, conformance tools | internal library/service seam | outline |
| ORC-GUI-001 | GUI.Forms stable C surface and lifecycle | GUI.Forms registered by Oracle | GUI.Forms | C++ frontend | C ABI | importing experimental ABI |
| ORC-FE-001 | Frontend session, navigation state, selection and command context | frontend + Oracle | C++ frontend | Oracle, diagnostics | local wire/C client | outline |
| ORC-PLG-001 | Package identity, discovery, install, grant, lifecycle | Oracle | Rust supervisor | frontend, CLI | local API | source research |
| ORC-PLG-002 | Sandboxed worker job and capability protocol | Oracle | Rust supervisor/worker | plugin SDKs | hostile local wire | source research |
| ORC-PLG-003 | Preview and thumbnail jobs/results | Oracle | plugin workers | frontend | worker protocol + validated bulk data | source research |
| ORC-PLG-004 | Search and virtual-system provider exchange | Oracle | plugin workers | Oracle query broker | worker protocol | source research |
| ORC-HIV-001 | Provider schema and derived-generation deposit | Oracle | hive service | plugin supervisor, extractors | capability API | outline |
| ORC-HIV-002 | Semantic fact propose/commit/query/erase | Oracle | semantic hive | AI clients, frontend, plugins by grant | capability API | outline |
| ORC-HIV-003 | Hive lifecycle, quota, migration, sync/export | Oracle | hive service | settings/admin, federation | privileged API | outline |
| ORC-HND-001 | Internal handler/type association resolution | Oracle | handler registry | frontend, CLI | local API | outline |
| ORC-CMD-001 | Declarative context-command registration/enumeration/invocation | Oracle | command registry/supervisor | frontend, plugins | local + worker API | outline |
| ORC-SET-001 | Core and plugin settings schema/value transaction | Oracle | settings service | frontend, CLI, plugins by namespace | local API | outline |
| ORC-CLI-001 | Human and structured CLI grammar | Oracle | Oracle CLI | humans, AI/shell clients | command + structured output | outline |
| ORC-INT-001 | Platform handler/shell/desktop integration plan and status | Oracle | first-party platform adapter | frontend, CLI | privileged local API | outline |
| ORC-FED-001 | Machine/network catalogue and federated-query envelope | Oracle | remote Oracle/engine adapter | Oracle query broker | authenticated optional wire | deferred paper |
| ORC-AUD-001 | Local audit, provenance inspection, redacted export | Oracle | audit service | frontend, CLI | local API | outline |

## Explicitly forbidden edges

- plugin worker → Go engine writable/index API;
- plugin worker → GUI.Forms object or native window;
- GUI.Forms → engine, hive, plugin, or filesystem policy;
- engine → plugin executable code;
- AI client → source-file mutation through Oracle search/hive contracts;
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
