# File Manager frontend operating instructions

This nested directory is the future C++ File Manager application—not the parent
multi-project workspace. It consumes GUI.Forms, the Go engine, and The Oracle.

## Current phase: waiting

Do not create application source, CMake/build files, generated bindings, assets,
packages, or prototypes here until every Gate F0 condition in
`../orchestrator/planning/DELIVERY_SEQUENCE.md` is met and the grand architect
explicitly opens frontend implementation.

Permitted work before F0:

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
4. `planning/DOGFOOD_SEQUENCE.md`
5. `../orchestrator/spec/CONTRACT_REGISTRY.md`
6. the accepted root ADRs and relevant parent planning records.

## Boundary rules

- This project owns application interaction and composition, not GUI.Forms
  internals, Go engine storage, Oracle policy/hives, or plugin execution.
- Do not duplicate a dependency to work around an unfinished contract. File an
  Oracle proposal instead.
- Do not let plugins supply controls, styles, callbacks, or native windows.
- Do not bypass Oracle capability policy or call plugin workers directly.
- Do not expose private engine/GUI.Forms/Oracle implementation types through the
  frontend.
- Core navigation and file operations remain usable when Oracle augmentation is
  unavailable; reduced behavior must be explicit.
