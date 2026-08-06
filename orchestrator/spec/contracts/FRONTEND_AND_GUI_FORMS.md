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
- `../../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`.

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
- shutdown/restart eligibility and a nullable redacted diagnostics locator;
- an attributed Frontend 001 opening projection that reports only
  Orchestrator's own gate as locally decidable, mirrors the current GUI.Forms
  consumption-manifest state, and leaves architect direction explicitly
  unreported.

The opening projection deliberately has no aggregate `frontend_ready` or
`implementation_authorized` boolean. Orchestrator is authoritative for its Core
release and availability evidence, not for GUI.Forms owner approval or grand
architect direction. Its `orchestrator_gate.satisfied` value requires both the
Core release manifest and `frontend.bootstrap` capability to be ready; blockers
carry exact release-requirement or capability evidence. The GUI.Forms gate is
an attributed mirror of `gui_forms.consumption_manifest`. Architect direction
is nullable and remains `not_reported` until an authority-owned recording
mechanism is negotiated.

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
