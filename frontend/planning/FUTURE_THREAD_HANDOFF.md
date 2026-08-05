# Future frontend task handoff

Do not use until GUI.Forms gives its Frontend 001 consumption go-ahead and the
grand architect explicitly directs implementation to begin.

> Work only in `/Users/quentinkuttenkuler/file_manager/frontend`. Read its
> `AGENTS.md`, `planning/FRONTEND_001.md`, Design DNA 006, the named GUI.Forms
> consumption snapshot, and accepted ADRs. Confirm both Frontend 001 opening
> events. Do not require Engine or Orchestrator implementation for this slice;
> use the declared deterministic fixture ports and do not compensate with a
> private dependency reach-through.
>
> Build the C++ File Manager application as a client and composition layer. Use
> GUI.Forms for retained UI, the Go engine for exact/core Kolmogrov-ready search,
> and Orchestrator for public interoperability, settings, handlers, declarative
> commands, hives, CLI, plugins, and integration policy. Do not duplicate these
> systems inside the frontend.
>
> Implement Frontend 001 on macOS first, beginning with fake peers and without
> real filesystem mutation. Preserve service-absent behavior, selection,
> evidence, cancellation, and exact identity at every later slice. Plugins never
> supply controls or execute in-process. Stop rather than inventing an
> unregistered API or weakening the sandbox/file-operation boundary.
