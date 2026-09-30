# ORC-GUI / ORC-FE: frontend and GUI.Forms contracts

Status: **GUI.Forms negotiation active; Orchestrator Core bootstrap projection
stable at 1.0 under ADR-010; platform packaging remains artifact-specific**.

GUI.Forms owns retained control/rendering semantics and its C ABI implementation.
Orchestrator registry records the version consumed by the frontend; it does not
move GUI state into the Orchestrator.

File Manager normally consumes Orchestrator as its integration/policy surface
and renders all Orchestrator settings/service controls. GUI.Forms remains direct
and in-process. The File Manager frontend owns window, navigation, selection, file-operation
presentation, preview/properties composition, and user interaction state. It
must be able to:

- render cached handler/command declarations without plugin code;
- submit command context as exact object IDs plus immutable selection snapshot;
- receive Orchestrator results without foreign callbacks on arbitrary threads;
- use the registered direct-engine route only as explicit degraded fallback;
- preserve selection across generation updates;
- display unavailable, stale, partial, denied, and provider-derived states;
- continue basic navigation and file operations while Orchestrator/plugin services
  restart or are unavailable.

The frontend does not expose GUI.Forms controls to plugins. Orchestrator defines
settings/service semantics and availability; File Manager owns their house
rendering.

Active dialogue:

- `../../../gui_forms/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`;
- `../../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`;
- `../../../web_forms/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md` for the
  proposed build-time `ORC-GUI-002` authoring/capability-manifest edge. It adds
  no runtime Orchestrator, Python, or browser dependency. That proposal also
  requires nested ambient layout/surface/state capability and a C++17-compatible
  orthodox code-generation seam.

Frontend 001 product startup uses the live Core 1.0 Orchestrator edge. Tests may
replay canonical Orchestrator fixtures; application-owned fixture schemas do not
become an ORC-FE contract.

## Atomic frontend bootstrap

`orchestrator.frontend.bootstrap`, contract `ORC-FE-001` 1.0, is the product
startup read. One successful response contains:

- schema family/range and an `immutable` snapshot identity;
- lifecycle and configuration generations;
- version and Core release/provenance state;
- complete contract and availability catalogues;
- normal integration route, Engine scope, and direct-Engine fallback
  registration, current eligibility, state, and reason;
- shutdown/restart eligibility, restart strategy/effect, and a nullable
  redacted diagnostics locator;
- an attributed Frontend 001 opening projection that reports only
  Orchestrator's own gate as locally decidable, mirrors the current GUI.Forms
  consumption-manifest state, and attributes recorded architect direction.

The opening projection deliberately has no aggregate `frontend_ready` or
`implementation_authorized` boolean. Orchestrator is authoritative for its Core
release and availability evidence, not for GUI.Forms owner approval or grand
architect direction. Its `orchestrator_gate.satisfied` value requires both the
Core release manifest and `frontend.bootstrap` capability to be ready and the
live daemon to expose the admitted supervisor restart path; blockers carry
exact release-requirement, capability, or runtime-hosting evidence. The
GUI.Forms gate is an attributed mirror of
`gui_forms.consumption_manifest`. It is `available` for the named
`gui-forms-fm0-macos-arm64-2026-08-10` installed snapshot; later FM rows and
other targets remain independently gated. Architect direction is `recorded`
from the authority-owned frontend start records, most recently the explicit
2026-08-10 implementation and M4 dogfood direction.

Core restart is supervisor-mediated. A client requests clean shutdown, closes
the old session, and reconnects through the stable launchd socket; the new
snapshot must carry a new daemon instance identity. The bootstrap reports this
as `shutdown_then_supervisor_reactivate`, not as an unimplemented in-process
restart method.
Unsupervised local and stdio processes report `restart_eligible: false`,
`restart_strategy: unavailable`, and do not satisfy the live Orchestrator
opening gate even when their embedded Core artifact is ready.

Engine, Kolmogrov, settings, handlers, commands, plugins, and semantic facts
remain visible through the full availability catalogue but do not enter the
Frontend 001 opening predicate. The projection encodes that separately gated
provider absence does not block opening, while still requiring a live snapshot
for product startup and limiting stale snapshots to display-only authority.

The embedded status may add `runtime_health` as explicitly non-authoritative,
relaxed operational telemetry. Frontend may display it for service health, but
must not derive permission, fallback eligibility, or cache validity from those
counters.

The authenticated server hello supplies daemon `instance_id`. The frontend
cache key is therefore `(instance_id, lifecycle_generation,
configuration_generation)`. Configuration generation is zero while no mutable
Core configuration service exists. Any instance or generation change replaces
the snapshot; records from different keys are never merged into one authority
view.

On disconnect, the last compatible snapshot may remain visible only as
explicitly stale, read-only state. It cannot authorize mutation, plugin work,
or fallback routing that was not registered by that same compatible snapshot.
`registered` and `eligible` are separate: the first records policy, while the
second records whether the provider/transport can currently be used.

## Bounded administration presentation

`ORC-UI-001` 1.0 freezes facts and command semantics, not controls. File
Manager owns every tab, row, label, confirmation and focus decision. The
Orchestrator returns no markup, control type, color, geometry, callback, native
handle or executable presentation content.

`orchestrator.services.snapshot` returns one bounded immutable value:

- schema family `ORC-UI-001`, major 1, minor 0, and `snapshot_kind`;
- stable service ID/title, lifecycle state, readiness, process instance ID,
  generation, transport and optional unavailable reason;
- optional currentness plus admitted root IDs for Engine-like providers; and
- a closed list of command ID/title/availability/effect descriptors.

The first profile contains `orchestrator` and `engine`. Absence is one explicit
unavailable service row, never an empty successful provider. A snapshot is
display authority only; it cannot authorize a later command after identity or
generation changes.

`orchestrator.services.command` accepts `service_id`, `command_id`, and the
optimistic identity fields for exactly one observed snapshot. Orchestrator
commands require both the ORC1 `instance_id` and lifecycle generation because a
new process may begin again at generation one. Engine commands require its
current instance ID; reconcile/rebuild also require one currently admitted root
ID. Unknown fields and command shapes are invalid. Identity mismatch is
`stale`. Missing transport/authority is `unavailable`. Provider faults retain a
typed terminal failure rather than becoming empty success.

The first closed command set is:

| Service | Commands | Effect rule |
|---|---|---|
| Orchestrator | `restart`, `shutdown` | clean stop; restart relies on the admitted supervisor and changes process identity |
| Engine | `integrity_check`, `reconcile`, `rebuild`, `restart` | integrity is read-only; root commands are root-bound; restart relies on the host-bound supervisor |

Query calls through the installed Engine adapter may rediscover and retry once
after a transport failure because they are idempotent. Administrative calls are
never automatically replayed. The response's `provider_result` is a bounded
provider diagnostic/result projection; frontend policy derives only from the
ORC-UI terminal/effect and a new immutable snapshot.

**MEASURED contained M4 evidence:** both services were observed through the
Rust CLI and independent C++17 client; integrity, reconcile, Engine restart and
Orchestrator restart completed. Each restart rotated the applicable instance
ID and replay with the prior identity was rejected as stale. Evidence and
retained failures are in
`../../conformance/evidence/M4_FILE_MANAGER_SERVICES_2026-08-10.md` and
ADR-019.

## Unified frontend search

`orchestrator.search` is the single frontend search operation in the additive
ORC-FE-001 development projection. Its request carries the authorized root and
relative scope, predicate, bounded work/result constitution, and an optional
opaque source-bound cursor. It carries no catalogue/live selector. Orchestrator
prefers a usable catalogue, falls back to live traversal only for unsupported,
unavailable, stale, or quarantined catalogue outcomes, and never falls back
around denied, invalid, budget, timeout, or cancellation outcomes. An
authoritative catalogue no-match is terminal.

The 2026-09-29 [ordinary text reconciliation](../FRONTEND_TEXT_PREDICATE.md)
defines the current development text predicate and compatibility impact.
Catalogue planning must support the requested predicate, not substitute exact
name equality for substring search. The current adapter reports unsupported
before any catalogue call for new ordinary text, permitting bounded live fallback
even with indexing enabled. Exact callers use explicit `filters.name`.

The response uses one result identity shape and terminal vocabulary, plus
`source`, `complete`, optional source-bound cursor, and source-specific
generation or scan/work details. Frontend may display live, partial, stale, or
complete state; it does not choose or switch lanes. Continuations preserve the
selected lane until the caller starts a new cursorless query.

**OBSERVED development conformance:** an authenticated local daemon spawns the
separately built Go Engine development process, derives live availability from
`engine.version`, and returns a match from an unreconciled root through this
operation. The Rust integration client and independently built C++17 client
pass. Installed Engine discovery/authentication remains a separate red gate.

The C++ source client performs one synchronous bounded request and owns no
thread or callback. File Manager calls it from a frontend-owned I/O worker and
posts the completed value snapshot to its UI-thread queue. This preserves the
accepted no-foreign-callback rule without moving UI scheduling into
Orchestrator. Cancellation remains explicitly unsupported for this atomic Core
operation; connection and framing deadlines stay bounded.

**OBSERVED conformance projection:**
`../../conformance/clients/cpp/` is a separately built C++17 client for the
first Unix platform slice. It owns no frontend state and links no Rust code. It
validates discovery/session identity, reads typed bootstrap snapshots, and
reconnects across daemon instances. Its bootstrap call now validates the atomic
schema, cache generations, route agreement, complete catalogues, service-control
eligibility, and the Orchestrator-owned opening predicate. It exposes no helper
that can mistake Orchestrator readiness for global Frontend 001 authorization.
This is source-client consumer evidence, not permission to open Frontend 001 or
a frozen binary C++ ABI.
