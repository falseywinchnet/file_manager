# File Manager frontend

Status: **Frontend 001 specified; waiting for Orchestrator Core 1.0, GUI.Forms
go-ahead, and explicit architect start direction**.

This directory will become the C++ application called **File Manager**. It
normally consumes Orchestrator as the integration/policy authority while using
GUI.Forms in-process. The systemwide engine remains available through
Orchestrator and through a registered degraded fallback:

- [`../gui_forms/`](../gui_forms/) — retained cross-platform GUI framework;
- [`../engine/`](../engine/) — systemwide exact/lexical/Kolmogrov-ready Go file
  index and degraded fallback;
- [`../orchestrator/`](../orchestrator/) — Orchestrator, Rust contract authority,
  control plane, hives, settings, CLI, handlers, plugins and integration broker.

Orchestrator advances headlessly and independently of GUI.Forms. Frontend 001
may begin when Orchestrator Core 1.0 is available, GUI.Forms gives its named FM0
consumption go-ahead, and the grand architect directs work to start. Product
startup uses the live Orchestrator; deterministic Core 1.0 fixtures remain test
doubles only. Engine and later providers open through their own negotiated
contracts. File Manager renders Orchestrator's settings and service controls;
Orchestrator itself has no GUI.

The exact first slice and gate are in
[`planning/FRONTEND_001.md`](planning/FRONTEND_001.md) and
[`planning/DEPENDENCY_GATES.md`](planning/DEPENDENCY_GATES.md). Its visual
constitution is
[`planning/visual/DESIGN_DNA_006.md`](planning/visual/DESIGN_DNA_006.md).
