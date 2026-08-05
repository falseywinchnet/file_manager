# File Manager

The pre-architecture workspace for a fast, local-first, cross-platform file
manager: visually descended from classic Windows Explorer, NeXTSTEP, and
Watercolor, with a local CLI surface and future capability-scoped plugin AI.

The parent product remains in **pre-plan** where decisions are unresolved, but
its repository topology, engine spine, Orchestrator integration authority, and
negotiated delivery order are now accepted. The work in [`planning/`](planning/) defines the
remaining decisions, evidence, exclusions, and component gates.

Start with:

- [`planning/README.md`](planning/README.md) — current phase and artifact map;
- [`planning/PROGRAM_MAP.md`](planning/PROGRAM_MAP.md) — component ownership,
  current permissions, contract authority, and build order;
- [`planning/PREPLAN_CHARTER.md`](planning/PREPLAN_CHARTER.md) — the plan for
  producing the actual project plan;
- [`planning/EVIDENCE_REGISTER.md`](planning/EVIDENCE_REGISTER.md) — Zeta
  search research, BFFT research discipline, and Modern.Forms reconnaissance;
- [`planning/ARCHITECTURE_INPUTS.md`](planning/ARCHITECTURE_INPUTS.md) — recorded
  constraints and positive product direction from the architect;
- [`planning/SURFACE_PIPELINE.md`](planning/SURFACE_PIPELINE.md) — native retained
  UI pipeline hypothesis, including a possible Modern.Forms compatibility layer;
- [`planning/QUESTION_ATLAS.md`](planning/QUESTION_ATLAS.md) — numbered
  anti-requirement and architecture questions.
- [`frontend/planning/visual/ceremonial-software-atlas.html`](frontend/planning/visual/ceremonial-software-atlas.html)
  — visible accept/distill/reject board for installer animation, OOBE, and
  Visual Studio/professional-software aesthetics from 2000–2012.
- [`frontend/planning/visual/material-depth-atlas.html`](frontend/planning/visual/material-depth-atlas.html)
  — visible static board for relief, instruments, object icons, illustrated
  utility surfaces, and frozen screensaver-pattern families.
- [`planning/gui_forms/GUI_FORMS_BACKEND_DECISION_MAP.md`](planning/gui_forms/GUI_FORMS_BACKEND_DECISION_MAP.md)
  — the technical decision interview for the independent GUI.Forms framework.
- [`planning/gui_forms/GUI_FORMS_INTERVIEW_LEDGER.md`](planning/gui_forms/GUI_FORMS_INTERVIEW_LEDGER.md)
  — GIVEN GUI.Forms constraints, candidate tradeoffs, and the deeper host,
  renderer, layout, and behavior interview; formal selection comes later.
- [`planning/gui_forms/GUI_FORMS_RESOURCES_AND_CONFIGURATION.md`](planning/gui_forms/GUI_FORMS_RESOURCES_AND_CONFIGURATION.md)
  — compiled theme/language packs, PNG policy, safe fallbacks, and mutable
  runtime-configuration candidates.

The fetched Modern.Forms reference is pinned in
[`third_party/README.md`](third_party/README.md).

## Program repositories

- [`gui_forms/`](gui_forms/) — independently buildable retained C++ GUI
  framework, currently advancing first toward a semicomplete snapshot.
- [`engine/`](engine/) — purpose-built, standalone Go catalogue/index/search
  service with a narrow versioned API, mandatory sandbox, exact-identity oracle,
  crash/corruption testing, and comparison benchmarks against mature stores.
- [`kolmogrov/`](kolmogrov/) — independent formal and experimental program for
  ConeDAG successors, practical algorithmic-information measures, exhaustive
  breakdown, and fixed-width multi-channel perceptual hashes. It pursues proof
  closure and adversarial measurement before production promotion.
- [`orchestrator/`](orchestrator/) — the active user-scoped Rust Orchestrator:
  canonical integration authority, contract and availability registry, CLI,
  and future owner of hives, settings, handlers, commands, platform policy, and
  hostile plugin supervision.
- [`frontend/`](frontend/) — the C++ end-user application. Frontend 001 begins
  after the named GUI.Forms go-ahead and explicit architect direction, using
  deterministic fixtures until real Engine and Orchestrator adapters open.
- [`plugin_runtime/`](plugin_runtime/) — frozen earlier plugin-containment
  research retained as input to Orchestrator, not a parallel runtime implementation.

Each directory contains its own `AGENTS.md`, build boundary, ledgers, acceptance
gates, and a handoff prompt suitable for a dedicated Codex task.

Accepted architecture records are indexed in
[`decisions/README.md`](decisions/README.md). Engine workers must also read both
[`engine/docs/ARCHITECT_HANDOFF_001.md`](engine/docs/ARCHITECT_HANDOFF_001.md)
and [`engine/docs/ARCHITECT_HANDOFF_002.md`](engine/docs/ARCHITECT_HANDOFF_002.md).
