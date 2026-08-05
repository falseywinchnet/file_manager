# Future frontend task handoff

Do not use until Gate F0 is explicitly opened.

> Work only in `/Users/quentinkuttenkuler/file_manager/file_manager`. Read its
> `AGENTS.md`, planning documents, Oracle master registry, accepted ADRs, and the
> named GUI.Forms/engine/Oracle consumption snapshots. If any Gate F0 item is
> absent, report it and improve paper scenarios only; do not create application
> code or compensate with a private dependency reach-through.
>
> Build the C++ File Manager application as a client and composition layer. Use
> GUI.Forms for retained UI, the Go engine for exact/core Kolmogrov-ready search,
> and The Oracle for public interoperability, settings, handlers, declarative
> commands, hives, CLI, plugins, and integration policy. Do not duplicate these
> systems inside the frontend.
>
> Implement the dogfood sequence vertically on macOS, beginning with fake peers
> and a disposable filesystem root. Preserve service-absent behavior, selection,
> evidence, cancellation, and exact identity at every slice. Plugins never
> supply controls or execute in-process. Stop rather than inventing an
> unregistered API or weakening the sandbox/file-operation boundary.
