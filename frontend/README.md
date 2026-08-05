# File Manager frontend

Status: **Frontend 001 specified; waiting for GUI.Forms go-ahead and explicit
architect start direction**.

This directory will become the C++ application called **File Manager**. It
normally consumes Orchestrator as the integration/policy authority while using
GUI.Forms in-process. The systemwide engine remains available through
Orchestrator and through a registered degraded fallback:

- [`../gui_forms/`](../gui_forms/) — retained cross-platform GUI framework;
- [`../engine/`](../engine/) — systemwide exact/lexical/Kolmogrov-ready Go file
  index and degraded fallback;
- [`../orchestrator/`](../orchestrator/) — Orchestrator, Rust contract authority,
  control plane, hives, settings, CLI, handlers, plugins and integration broker.

Frontend 001 may begin as soon as GUI.Forms gives its named consumption
go-ahead and the grand architect directs work to start. Engine and Orchestrator
are represented by deterministic frontend fixtures in that slice; their real
adapters open later through their own negotiated contracts. File Manager will
render Orchestrator's settings and service controls; Orchestrator itself has no
GUI.

The exact first slice and gate are in
[`planning/FRONTEND_001.md`](planning/FRONTEND_001.md) and
[`planning/DEPENDENCY_GATES.md`](planning/DEPENDENCY_GATES.md). Its visual
constitution is
[`planning/visual/DESIGN_DNA_006.md`](planning/visual/DESIGN_DNA_006.md).
