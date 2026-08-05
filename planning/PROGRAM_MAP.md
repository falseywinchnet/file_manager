# File Manager program map

Status: **DECIDED repository topology and negotiated integration order** under
[`ADR-003`](../decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md),
[`ADR-004`](../decisions/ADR-004-FRONTEND-001-LOCATION-AND-OPENING-GATE.md), and
[`ADR-006`](../decisions/ADR-006-ORCHESTRATOR-CORE-1-0-FRONTEND-BOOTSTRAP.md),
with Engine search readiness corrected by
[`ADR-008`](../decisions/ADR-008-CATALOGUE-INDEPENDENT-LIVE-SEARCH.md).
This is the parent routing document; component details remain governed by each
subproject's records and evidence gates.

## Components and current permission

| Component | Path | Role | Current permission |
|---|---|---|---|
| GUI.Forms | `../gui_forms/` | Retained custom-rendered C++ UI framework and bindings | Active implementation toward a semicomplete, inspectable framework release |
| Engine | `../engine/` | Go catalogue-independent live search, exact catalogue, index, retrieval, and core Kolmogrov candidate integration | Active implementation toward a mostly running standalone service; required live-query lane open under ADR-008 |
| Kolmogrov | `../kolmogrov/` | Formal and empirical fixed-width perceptual-similarity program | Active independent research and conformance work |
| Orchestrator | `../orchestrator/` | Headless Rust integration authority, capability/availability map, control plane, hives, settings, handlers, command/CLI authority, plugin supervision, and platform policy | Active implementation toward Orchestrator Core 1.0; independent of GUI.Forms |
| File Manager frontend | `../frontend/` | Visual C++ end-user program built on GUI.Forms and bootstrapped by Orchestrator | Frontend 001 waits for Orchestrator Core 1.0, GUI.Forms FM0 go-ahead, and explicit architect start direction |
| Plugin Runtime research | `../plugin_runtime/` | Earlier containment and capability study | Frozen source material; implementation moves into Orchestrator's plugin-supervisor work |

`orchestrator/` is both the product component's repository path and the home of
the canonical cross-project contract registry.

## Authority map

- The filesystem and platform file identity remain authoritative for files.
- The Go engine owns exact catalogue observations, retrieval mechanics, and the
  required bounded catalogue-independent traversal provision in ADR-008.
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
4. Orchestrator advances independently to the Core 1.0 bootstrap profile while
   GUI.Forms advances independently to its FM0 consumption snapshot.
5. After both are available and the architect explicitly directs work to begin,
   File Manager Frontend 001 bootstraps against the live Orchestrator and
   consumes GUI.Forms in-process.
6. Kolmogrov yields admitted work to Engine independently. Real Engine and
   other provider capabilities join through their own negotiated snapshots.

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
