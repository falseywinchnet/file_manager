# File Manager frontend operating instructions

This nested directory is the future C++ File Manager application—not the parent
multi-project workspace. It consumes Orchestrator as the normal integration
authority, GUI.Forms in-process, and the systemwide Go engine through
Orchestrator or the registered degraded fallback.

## Current phase: Frontend 001 waiting on Core 1.0 and GUI.Forms

Do not create application source, CMake/build files, generated bindings, product
assets, packages, or prototypes until Orchestrator Core 1.0 is available through
the real frontend bootstrap edge, GUI.Forms gives the named FM0/Frontend 001
consumption go-ahead, and the grand architect explicitly directs this project to
begin. Engine, Kolmogrov, plugin execution, and semantic facts retain separate
gates; their actual states come from Orchestrator.

Permitted work before that start direction:

- product-flow and anti-model documentation;
- frontend-owned state and responsibility definitions;
- dependency/contract review;
- fake-service scenario design on paper;
- dogfood workflows and acceptance gates;
- visual grammar references owned by the parent planning project.

Before editing, read:

1. `README.md`
2. `planning/FRONTEND_CHARTER.md`
3. `planning/DEPENDENCY_GATES.md`
4. `planning/FRONTEND_001.md`
5. `planning/visual/DESIGN_DNA_006.md`
6. `planning/visual/FRONTEND_DESIGN_BRIEF_001.md`
7. `planning/visual/ICON_AUDIT_MIT_002.md`
8. `planning/DOGFOOD_SEQUENCE.md`
9. `planning/CONTEXTUAL_BUILTIN_COMMANDS.md`
10. `planning/DESKTOP_INTEGRATION_BOUNDARY.md`
11. `../orchestrator/spec/CONTRACT_REGISTRY.md`
12. the accepted root ADRs and relevant parent planning records.
13. `planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md` for the frontend client edge.

## Boundary rules

- This project owns application interaction and composition and renders
  Orchestrator's settings/service controls. It does not own GUI.Forms internals,
  Go engine storage, Orchestrator policy/hives, or plugin execution.
- Do not duplicate a dependency to work around an unfinished contract. File an
  Orchestrator proposal instead.
- Do not let plugins supply controls, styles, callbacks, or native windows.
- Do not bypass Orchestrator capability policy or call plugin workers directly.
- Do not expose private engine/GUI.Forms/Orchestrator implementation types through the
  frontend.
- Fixture payloads are frontend test data, not proposed provider contracts. Mark
  simulated capability state explicitly.
- Frontend tests may replay Orchestrator Core 1.0 fixtures, but product startup
  must negotiate with the live Orchestrator authority.
- Design DNA 006 governs Frontend 001, but its CANDIDATE and HYPOTHESIS entries
  remain unresolved unless a later verdict or ADR promotes them.
- Core navigation and file operations remain usable when Orchestrator augmentation is
  unavailable; reduced behavior must be explicit.
- Persistent side panels are specific to File Manager and its embedded picker/
  browser projection. This permission does not extend through shared packages
  to Paint, Text Editor, Games, plugins, or providers.
