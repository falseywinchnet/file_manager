# File Manager frontend charter

Status: **product responsibilities accepted; Frontend 001 scoped and waiting on
its GUI.Forms/architect opening events**.

## Mission

Consume Orchestrator as the normal integration/policy authority, compose
GUI.Forms and filesystem/platform operations into the fast, quiet,
location-oriented File Manager, and retain a registered direct-engine degraded
fallback. The frontend is where interaction becomes a coherent program and
where Orchestrator settings/service controls are rendered; it is not a place to
reimplement dependency internals.

## Frontend-owned state

- windows, panes, navigation history, breadcrumbs, current scope and selection;
- list/icon/criteria-view presentation and sorting intent;
- drag/drop and file-operation user flows;
- preview/properties composition using host controls and validated provider data;
- context-menu rendering from core plus Orchestrator declarations;
- search presentation, result revision, focus preservation and explanations;
- accessibility/help overlay presentation hooks;
- theme, language and interaction-sound application;
- error, unavailable, degraded and restart presentation;
- first-run/setup orchestration as experienced by the user.

## Dependency-owned state

- GUI.Forms owns retained controls, layout, rendering, input dispatch and its ABI;
- the Go engine owns exact catalogue and core retrieval generations;
- Kolmogrov owns the formal/hash family; engine owns its core file-index use;
- Orchestrator owns public interoperability, capabilities, registries, hives,
  settings, handlers/commands, plugin workers, CLI and integration policy;
- first-party platform adapters own OS-specific integration operations;
- filesystem/platform operation machinery owns actual source-file mutations.

## Non-goals

- no plugin execution or package parsing;
- no semantic model runtime;
- no second search index;
- no general settings database;
- no direct systemwide association mutation;
- no GUI framework fork hidden in application code;
- no web engine, .NET, Java, or Godot runtime;
- no tabs, dual-pane infection, Quick Access, opaque saved searches or modern
  shell bloat rejected by the parent product.

## Application failure law

The frontend must remain a useful local navigator when the engine or Orchestrator is
temporarily unavailable. Exact indexed search, semantic/provider evidence,
plugins, and registry mutation may degrade separately. A dead auxiliary service
must not freeze input, lose the current location, or make ordinary folders
unopenable.
