# File Manager program map

Status: **DECIDED repository topology and delivery dependency order** under
[`ADR-002`](../decisions/ADR-002-ORACLE-CONTRACT-AUTHORITY-AND-PROGRAM-SEQUENCE.md).
This is the parent routing document; component details remain governed by each
subproject's records and evidence gates.

## Components and current permission

| Component | Path | Role | Current permission |
|---|---|---|---|
| GUI.Forms | `../gui_forms/` | Retained custom-rendered C++ UI framework and bindings | Active implementation toward a semicomplete, inspectable framework release |
| Engine | `../engine/` | Go exact catalogue, index, retrieval, and core Kolmogrov candidate integration | Active implementation toward a mostly running standalone service |
| Kolmogrov | `../kolmogrov/` | Formal and empirical fixed-width perceptual-similarity program | Active independent research and conformance work |
| Oracle | `../orchestrator/` | Rust control plane, program contract authority, hives, settings, handlers, command/CLI authority, plugin supervision, and platform policy | Paper specification only until Gate O0 |
| File Manager | `../file_manager/` | C++ end-user program built on GUI.Forms, the engine, and Oracle | Waiting until Gate F0 |
| Plugin Runtime research | `../plugin_runtime/` | Earlier containment and capability study | Frozen source material; implementation moves into Oracle's plugin-supervisor work |

“Oracle” is the product component. `orchestrator/` is its repository path and
the home of the canonical cross-project contract registry.

## Authority map

- The filesystem and platform file identity remain authoritative for files.
- The Go engine owns exact catalogue observations and retrieval mechanics.
- The Kolmogrov channel proposes fuzzy/structural candidates; it does not own
  file identity or personal semantic memory.
- Oracle owns the meaning, version, capability, and conformance record for every
  cross-project API/ABI. That ownership does not imply every hot call must be
  proxied through an Oracle process.
- Oracle owns durable non-file knowledge: semantic/provider hives, settings,
  handler registry, plugin grants and lifecycle, CLI/command grammar, and
  platform-integration policy.
- GUI.Forms owns rendering and retained control behavior, not product policy.
- File Manager owns end-user composition and interaction. It is a client, not
  the hidden authority for Oracle or engine state.

## Delivery order

1. GUI.Forms reaches its declared semicomplete snapshot.
2. The Go engine reaches its declared mostly-running standalone snapshot while
   Kolmogrov continues its gated transfer work.
3. Oracle remains paper-only while it imports, reconciles, and versions the
   contracts those projects actually expose.
4. After Gate O0, Oracle builds its contract laboratory, then hives, settings,
   handlers, command/CLI services, plugin supervision, and platform policy.
5. After Oracle reaches the Gate F0 integration snapshot, the File Manager
   frontend begins and dogfoods the combined system on macOS first.

The exact gates and artifacts are in
[`orchestrator/planning/DELIVERY_SEQUENCE.md`](../orchestrator/planning/DELIVERY_SEQUENCE.md)
and [`file_manager/planning/DEPENDENCY_GATES.md`](../file_manager/planning/DEPENDENCY_GATES.md).

## Cross-project change rule

Each producer may freely evolve private implementation details inside its own
boundary. A proposed cross-project call, event, capability, error, identifier,
or lifecycle change must be submitted to `orchestrator/proposals/`, assigned a
contract ID, and reconciled with all affected consumers before bindings freeze.
No C++ layout, Rust enum, Go struct, pointer, callback, platform handle, or
storage representation becomes an ABI merely because one implementation
currently uses it.
