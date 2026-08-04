# Interview round 2: concrete anti-models

Status: open. Answer each with **reject**, **adopt**, **modify**, or **irrelevant**,
plus nuance where useful. These are behavior probes, not allegations that every
OS release exhibits every failure.

## Finder behavior checklist

- **FND001:** Search scope can jump between current folder and whole Mac, and
  indexed results may disappear until Spotlight is rebuilt.
- **FND002:** “Current folder” search is recursively beneath the folder; there is
  no simple immediate-children-only search.
- **FND003:** Starting a search replaces the current file surface and makes the
  physical location/scope less obvious.
- **FND004:** View mode, columns, sorting, and geometry persist inconsistently per
  folder through hidden `.DS_Store` state rather than one understandable policy.
- **FND005:** Column view creates horizontal-management problems; current Finder
  versions have even allowed scrollbars to obstruct column resize handles.
- **FND006:** Sidebar Favorites look like places in a hierarchy but are aliases
  that can disappear when dragged out.
- **FND007:** The sidebar is not a persistent, freely expandable filesystem tree.
- **FND008:** The current path is not normally an editable address field; direct
  path entry is displaced into “Go to Folder.”
- **FND009:** Recents is an index-dependent virtual view whose membership and
  ordering are difficult to predict.
- **FND010:** Tags occupy core sidebar/context-menu surface and create an
  organization system independent of real folders.
- **FND011:** Icon view can become a spatial canvas whose positions, alignment,
  and sorting vary by folder.
- **FND012:** There is no ordinary first-class “New empty file” operation.
- **FND013:** Inline rename affordance is inconsistent: click timing, Return, and
  context-menu availability can differ or fail.
- **FND014:** Copy conflict and folder merge/replace language does not make the
  resulting operation sufficiently concrete.
- **FND015:** Dragging means move or copy depending on volume and modifier state;
  the operation is not always visually obvious before drop.
- **FND016:** File cut/move does not use the plain, discoverable Cut/Paste model
  familiar from Explorer.
- **FND017:** Recursive folder sizes are normally absent or require an expensive
  explicit calculation.
- **FND018:** Thumbnail/metadata generation can stall or churn large folders.
- **FND019:** Application bundles/packages deliberately hide their directory
  nature unless “Show Package Contents” is invoked.
- **FND020:** Tabs and tab-preference rules make window/folder identity less
  physical.
- **FND021:** Quick Look is good as a concept, but its global floating-window
  model is worse than a stable preview/properties pane.
- **FND022:** Column view itself—is it useful, irrelevant, or an anti-model?
- **FND023:** Asynchronous refresh or sort can disturb selection and spatial
  orientation.
- **FND024:** Finder conflates desktop management, file browsing, mounted-volume
  presentation, and shell lifecycle in one privileged application.
- **FND025:** Eject, mount, unavailable-volume, and permission failures are often
  represented by modal or vague errors instead of persistent object state.
- **FND026:** Hidden files and extensions require global toggles or shortcuts
  rather than clear view-local controls.
- **FND027:** Search syntax and metadata predicates are powerful but poorly
  surfaced; ordinary users cannot see the query being executed.
- **FND028:** The absence of a status-rich bottom bar makes size, count, selection,
  and operation state less glanceable than classic Explorer.

## Post-Windows-7 Explorer change checklist

- **WEX001:** Windows 8 introduced the Explorer ribbon. Keep only the compact,
  well-structured earlier-Office/NTLite idea, not that exact implementation?
- **WEX002:** Windows 10 Quick Access automatically promotes frequent folders and
  recent files.
- **WEX003:** Explorer opens to an abstract Home/Quick Access page instead of a
  real filesystem location.
- **WEX004:** Home includes Recommended, Recent, Favorite, Office.com, and cloud
  content.
- **WEX005:** Windows 11 adds tabs to the title area.
- **WEX006:** Windows 11 splits the context menu into a simplified icon-heavy menu
  and “Show more options” legacy menu.
- **WEX007:** The ribbon was replaced by a sparse icon-only command bar with many
  commands hidden behind menus.
- **WEX008:** OneDrive status, quota, and account integration enter the file
  manager's primary surfaces.
- **WEX009:** Gallery adds another content-derived virtual landing surface.
- **WEX010:** Downloads and other folders may acquire automatic date grouping or
  reset custom views.
- **WEX011:** Touch-oriented spacing reduces desktop information density.
- **WEX012:** Mica/acrylic/translucency, rounded geometry, and animated surfaces
  displace crisp opaque control structure.
- **WEX013:** Modern command bars, legacy property sheets, old file dialogues, and
  extension UI coexist without one visual grammar.
- **WEX014:** This PC injects Desktop, Documents, Downloads, Music, Pictures, and
  Videos as privileged nodes rather than presenting a clean volume tree.
- **WEX015:** The navigation pane can show the same folder through Quick Access,
  OneDrive, This PC, Libraries, and the actual hierarchy.
- **WEX016:** Search becomes slower, index-dependent, and visually detached from
  the straightforward filename filtering users expect.
- **WEX017:** Archive formats increasingly behave like browsable folders in core.
- **WEX018:** Selection checkboxes and touch affordances enter ordinary desktop
  views.
- **WEX019:** Share, account, recommendation, and Copilot actions enter file
  surfaces.
- **WEX020:** Row height, chrome, padding, and context menus become less compact.
- **WEX021:** The status bar communicates less exact information than older
  Explorer versions.
- **WEX022:** The details pane shifts between useful editable properties and large
  decorative metadata/thumbnail space.
- **WEX023:** Address-bar behavior is simplified or changed at the expense of
  direct path manipulation and drag targets.
- **WEX024:** Extensions remain hidden by default and their toggle moves between
  ribbon/menus across releases.
- **WEX025:** Delete confirmation was reduced for recyclable deletion. This one
  appears aligned with File Manager's current rule—confirm?
- **WEX026:** Libraries were deemphasized rather than developed into explicit,
  comprehensible virtual criteria views.
- **WEX027:** HomeGroup disappeared. We currently consider that irrelevant and do
  not intend to recreate it—confirm?
- **WEX028:** The old Properties dialogue largely survives. Is its compact density
  and editable field model a positive reference despite its age?
- **WEX029:** Explorer increasingly owns cloud and remote-provider suggestions.
  Our future multi-machine federation must remain explicit and file-oriented
  rather than recommendation-oriented—confirm?
- **WEX030:** Modern Explorer frequently resets or overrides custom view/sort/group
  state after navigation from other applications. Must File Manager make view
  policy deterministic and inspectable?

## Where minimalism ends and glorious excess begins

For each, choose **minimal**, **rich**, or **progressively disclosed**.

- **RICH001:** Main navigation chrome.
- **RICH002:** Folder tree rows and disclosure controls.
- **RICH003:** File icons and selection treatment.
- **RICH004:** Ribbon/control shelf.
- **RICH005:** Preview/properties pane.
- **RICH006:** Search result explanations.
- **RICH007:** Copy/move/replace progress and conflict UI.
- **RICH008:** Settings and expert configuration.
- **RICH009:** Sounds and micro-interactions.
- **RICH010:** Themes, materials, and icon packs.
- **RICH011:** Virtual criteria views.
- **RICH012:** Semantic search and derived metadata.
- **RICH013:** Multi-machine federation.
- **RICH014:** CLI/API introspection.
- **RICH015:** Plugin management.

Then answer:

- **RICH016:** Which three surfaces should feel almost austere?
- **RICH017:** Which three should make a person stop and admire the craft?
- **RICH018:** Should visual richness remain constant, or concentrate in selected
  objects while the surrounding shell stays calm?
