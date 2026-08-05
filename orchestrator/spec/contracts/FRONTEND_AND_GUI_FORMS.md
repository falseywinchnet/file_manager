# ORC-GUI / ORC-FE: frontend and GUI.Forms contracts

Status: **GUI.Forms negotiation active; live Orchestrator Core 1.0 frontend
bootstrap semantics accepted, production transport/authentication pending**.

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
