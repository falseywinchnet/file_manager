# File Manager frontend

Status: **Frontend 001 specified; explicit architect start direction recorded
2026-08-07; waiting for Orchestrator Core 1.0 and the GUI.Forms go-ahead**.

This directory will become the C++ application called **File Manager**. It
normally consumes Orchestrator as the integration/policy authority while using
GUI.Forms in-process. The systemwide engine remains available through
Orchestrator and through a registered degraded fallback:

- [`../gui_forms/`](../gui_forms/) — retained cross-platform GUI framework;
- [`../engine/`](../engine/) — systemwide exact/lexical/Kolmogrov-ready Go file
  index and degraded fallback;
- [`../orchestrator/`](../orchestrator/) — Orchestrator, Rust contract authority,
  control plane, hives, settings, CLI, handlers, plugins and integration broker.

Orchestrator advances headlessly and independently of GUI.Forms. The grand
architect directed the frontend workstream to start on 2026-08-07, satisfying
the owner-direction predicate. Product source begins after Orchestrator Core
1.0 is available and GUI.Forms gives its named FM0 consumption go-ahead. Product
startup uses the live Orchestrator; deterministic Core 1.0 fixtures remain test
doubles only. Engine and later providers open through their own negotiated
contracts. File Manager renders Orchestrator's settings and service controls;
Orchestrator itself has no GUI.

The exact first slice and gate are in
[`planning/FRONTEND_001.md`](planning/FRONTEND_001.md) and
[`planning/DEPENDENCY_GATES.md`](planning/DEPENDENCY_GATES.md). Its visual
constitution is
[`planning/visual/DESIGN_DNA_006.md`](planning/visual/DESIGN_DNA_006.md).
The total implementation and integrated dogfood program is
[`planning/TOTAL_IMPLEMENTATION_PLAN.md`](planning/TOTAL_IMPLEMENTATION_PLAN.md).
