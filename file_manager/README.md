# File Manager frontend

Status: **paper-only waiting project**.

This directory will become the C++ application called **File Manager**. It is a
program built on three independently maturing foundations:

- [`../gui_forms/`](../gui_forms/) — retained cross-platform GUI framework;
- [`../engine/`](../engine/) — exact/lexical/Kolmogrov-ready Go file index;
- [`../orchestrator/`](../orchestrator/) — The Oracle, Rust contract authority,
  control plane, hives, settings, CLI, handlers, plugins and integration broker.

The frontend deliberately waits. Building it against moving private interfaces
would fossilize accidental APIs and force the Oracle to reverse-engineer the
program it is meant to organize.

Current work is limited to the
[`planning/`](planning/) documents. Implementation permission is Gate F0 in
[`../orchestrator/planning/DELIVERY_SEQUENCE.md`](../orchestrator/planning/DELIVERY_SEQUENCE.md).
