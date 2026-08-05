# Product negative

Status: architect-confirmed exclusions, round 1. These are GIVEN unless marked
for clarification.

## Interface trajectories rejected

- No tabs. A window represents one place; folders retain a physical-folder
  quality rather than becoming browser documents.
- No dual-pane commander layout.
- No Recents, Recommended, activity feed, or abstract Home landing page. “Home”
  means the user's home directory.
- No automatic grouping. It is unpredictable, intrusive, and not a substitute
  for intentional criteria views.
- No tags or notes system.
- No immediate-mode GUI.
- No platform-defined visual appearance as the product aesthetic. Consistency
  across systems matters more than imitating platforms that have lost their own
  visual discipline.
- No bundled browser/web engine.
- No oversized, nonsensically structured modern ribbon. A compact, dynamic
  control shelf in the lineage of earlier Office ribbons or NTLite is allowed.
- No free-placement/spatial icon canvas.
- No floating Quick Look window.
- No automatic scope widening from current-folder search to whole-machine
  search.
- No split modern/legacy context menus. The user chooses context-menu commands
  in Settings.
- No synthetic `This PC`, `Quick Access`, `Libraries`, or provider-branded tree
  aliases.
- No provider plugin may insert, replace, or restyle core controls.
- No Mica/rounded-corner modern-Windows styling trajectory.

## Runtime and framework exclusions

- File Manager may not rely on .NET.
- Java would be a fundamental betrayal of the project.
- Godot is not an acceptable delivery shortcut.
- The native GUI engine is disciplined C++.
- Rust owns Orchestrator's trusted control plane and hostile plugin supervision; it is
  not the GUI rendering language.
- Go owns the persistent indexing/search backend.
- C# interoperability is allowed as a binding to a native engine, not as a File
  Manager runtime dependency.

## Core feature exclusions

- No hidden assistant-specific authority tier. Humans, local AI tools, and
  developer agents use the same local CLI operations and their declared
  authority. Future plugin AI is separately sandboxed; no fact or plugin-AI API
  is inferred before its contract exists.
- No personal-assistant AI inside File Manager.
- No core web search or web-store search.
- No automatic indexing of removable or network volumes. New-drive consent
  defaults to no.
- No indexing without affirmative user consent.
- No automatic inclusion of dot folders, hidden files, logging/temp trees, or
  GitHub repositories in the proposed default home-root selection.
- No backups or general rollback subsystem masquerading as undo.
- No promise to undo replacement. Replacement requires a dialogue.
- No deletion confirmation when ordinary trash/recycle semantics are available.
- No internal handler registry changes to operating-system defaults.
- No in-process third-party/native preview execution. Preview failure must not
  crash File Manager.
- No thumbnails outside an admitted indexed tree.
- No use of `.DS_Store` as view-state truth.
- No native edit-control overlay as a shortcut around custom text, IME, tooltip,
  keyboard, or accessibility engineering.
- No production telemetry or automatic crash upload. Non-development builds may
  submit reports manually only. Development builds may report automatically.

## Plugin boundaries confirmed so far

Plugin classes admitted for later design:

- previews;
- thumbnails;
- virtual systems;
- search providers.

Other plugin powers are not admitted merely by analogy with extensible shells.
Orchestrator may expose separately specified handler, icon, metadata, command-menu,
and semantic-hive contracts only after capability review; this does not grant a
plugin GUI control injection or direct engine-catalogue mutation. Whether an
in-application plugin catalogue is permitted remains unresolved, including its
networking and trust model.

## Anti-model work still required

The architect asked the project to propose Finder and modern Explorer behaviors
one by one rather than infer a blanket rejection. The next interview therefore
uses concrete behavior checklists in `SURFACE_PIPELINE.md` and the current task
response.
