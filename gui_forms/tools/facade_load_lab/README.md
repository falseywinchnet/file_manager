# GUI.Forms unchanged-specimen facade-load laboratory

This private, opt-in laboratory attempts to start the authoritative unchanged
retired compatibility specimen build through the generated GUI.Forms facade under Wine. It does not copy,
modify, extract, or redistribute the specimen. The startup hook is a reversible
binding experiment over sidecar assemblies and writes a local diagnostic trace.

The hook preloads the unsigned generated `System.Windows.Forms.Primitives` and
`System.Windows.Forms` assemblies into the default load context before the
application entry point. This is intentionally a stronger and less isolated
test than the earlier synthetic private-`AssemblyLoadContext` laboratory.

Raw exception paths and third-party runtime output remain local. Only redacted
assembly identities, blocker classes, hashes, and aggregate outcomes belong in
checked-in evidence.

Set `GUI_FORMS_RUN_WORKING_DIRECTORY` to a GUI.Forms-owned profile directory
when running the entry point. The runner makes that directory current only for
the specimen lifetime and restores its own working directory afterward. This
keeps settings and layout writes out of the unchanged specimen directory while
allowing retired compatibility specimen to read the intended copied profile instead of silently selecting
its built-in defaults.
