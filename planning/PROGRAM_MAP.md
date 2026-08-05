# File Manager program map

Status: **DECIDED repository topology and negotiated integration order** under
[`ADR-003`](../decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md)
and [`ADR-004`](../decisions/ADR-004-FRONTEND-001-LOCATION-AND-OPENING-GATE.md).
This is the parent routing document; component details remain governed by each
subproject's records and evidence gates.

## Components and current permission

| Component | Path | Role | Current permission |
|---|---|---|---|
| GUI.Forms | `../gui_forms/` | Retained custom-rendered C++ UI framework and bindings | Active implementation toward a semicomplete, inspectable framework release |
| Engine | `../engine/` | Go exact catalogue, index, retrieval, and core Kolmogrov candidate integration | Active implementation toward a mostly running standalone service |
| Kolmogrov | `../kolmogrov/` | Formal and empirical fixed-width perceptual-similarity program | Active independent research and conformance work |
| Orchestrator | `../orchestrator/` | Rust integration authority, capability/availability map, control plane, hives, settings, handlers, command/CLI authority, plugin supervision, and platform policy | Provider-independent bootstrap kernel open under ADR-003; adapters negotiated per edge |
| File Manager frontend | `../frontend/` | C++ end-user program built on GUI.Forms with negotiated Engine and Orchestrator adapters | Frontend 001 specified; waits only for GUI.Forms go-ahead and explicit architect start direction |
| Plugin Runtime research | `../plugin_runtime/` | Earlier containment and capability study | Frozen source material; implementation moves into Orchestrator's plugin-supervisor work |

`orchestrator/` is both the product component's repository path and the home of
the canonical cross-project contract registry.

## Authority map

- The filesystem and platform file identity remain authoritative for files.
- The Go engine owns exact catalogue observations and retrieval mechanics.
- The Kolmogrov channel proposes fuzzy/structural candidates; it does not own
  file identity or personal semantic memory.
- Orchestrator owns the meaning, version, capability, and conformance record for every
  cross-project API/ABI. That ownership does not imply every hot call must be
  proxied through an Orchestrator process.
- Orchestrator owns durable non-file knowledge: semantic/provider hives, settings,
  handler registry, plugin grants and lifecycle, CLI/command grammar, and
  platform-integration policy.
- GUI.Forms owns rendering and retained control behavior, not product policy.
- File Manager owns end-user composition and interaction. It is a client, not
  the hidden authority for Orchestrator or engine state.

## Delivery order

1. GUI.Forms, the Go engine, and Kolmogrov continue independently while
   publishing evidence and answering explicit interface proposals.
2. Orchestrator builds its provider-independent contract/lifecycle/availability
   kernel and proposes required interfaces in each affected project's notes.
3. Producers reply and Orchestrator reconciles each edge; adapters, hives,
   settings, handlers, command/CLI services, plugin supervision, and platform
   policy open behind their own fixture-backed gates.
4. GUI.Forms gives the named Frontend 001 consumption go-ahead; after explicit
   architect direction, File Manager begins a fixture-backed application slice.
5. Kolmogrov yields admitted work to Engine independently. Real Engine and
   Orchestrator frontend adapters replace fixtures after their own negotiated
   snapshots, then combined-system macOS dogfood begins.

The exact gates and artifacts are in
[`orchestrator/planning/DELIVERY_SEQUENCE.md`](../orchestrator/planning/DELIVERY_SEQUENCE.md)
and [`frontend/planning/DEPENDENCY_GATES.md`](../frontend/planning/DEPENDENCY_GATES.md).

## Cross-project change rule

Each producer may freely evolve private implementation details inside its own
boundary. Orchestrator places proposed cross-project calls, events,
capabilities, errors, identifiers, and lifecycles in that project's
`ORCHESTRATOR_INTERFACE_NEGOTIATION.md`; the project replies and Orchestrator
reconciles the canonical contract. Subprojects may also originate proposals
under `orchestrator/proposals/`.
No C++ layout, Rust enum, Go struct, pointer, callback, platform handle, or
storage representation becomes an ABI merely because one implementation
currently uses it.
