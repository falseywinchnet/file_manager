# DEMO-D2 Folder proving evidence

Date: 2026-08-06

Revision base: `9b0d79977570` plus the uncommitted GUI.Forms/demoboard slice
recorded by this directory.

Environment: macOS 14.8.7 (23J520), arm64, CPU Skia host; Win64 target
cross-compiled with the repository MinGW configuration.

Fixture authority: `file-manager-demoboard-001`, generation 86. No real
filesystem was read or mutated by the demoboard.

## Measured claims

- The product and Controller are independent retained roots in one native
  application session. Closing Controller leaves Product alive; closing Product
  ends the owned session. `gui_forms_macos_multi_window_tests` covers callback
  cardinality and final snapshots.
- Public `TreeView` exposes stable item IDs, hierarchy depth, expansion,
  pointer/keyboard selection, type-to-select, and bounded visible semantic
  children without per-item retained controls.
- Public `ObjectView` exposes stable virtual items, icon/details projections,
  model-order multi-selection, primary/anchor/independent focus, keyboard
  spatial navigation, type-to-select, context requests, and bounded visible
  semantic children. Selection survives sort/view replacement and the 1,000-item
  regression fixture remains bounded.
- Public retained `ContextMenu` snapshots shared `Command` state at opening,
  renders nested/check/radio/disabled/destructive rows, bounds long menus,
  avoids client edges, contains/restores focus, and converges pointer, keyboard,
  and native accessibility Show Menu activation on the same command path.
- Public retained `MenuStrip` keeps the bar in normal layout while its active
  menu remains a window-owned popup root. Pointer hover, Left/Right/Down/Escape,
  semantic expansion, click-away, and nested commands share one focus scope and
  command path. Distinct menu-bar/menu-bar-item roles reach AppKit.
- Public tokenized window accelerators run only after the focused route declines
  a chord, unless an application-navigation registration explicitly requests
  preemption. Active popup scopes always retain first refusal. Owner disposal
  deterministically revokes each registration.
- The Folder navigation authority snapshots location items, selection, primary
  focus identity, and scroll before transitions. Tree, clickable breadcrumb,
  Back, Forward, Up, title, search scope, object model, and status update inside
  one retained update scope; new navigation after Back clears Forward.
- The Folder consumer uses those APIs for object/background menus. New Folder
  and Delete mutate only the session view; reset/restart restores catalogue 001.
- Public `Command`/`CommandBinding` makes ribbon and status view presentations
  execute one authority. Name sort preserves selected object identity while
  changing presentation order.
- Native AppKit accessibility actions were used to change view mode, select an
  object, select a tree location, and sort. Selection/status/navigation
  projections updated through the same command/event paths as headless tests.
- Portsmouth remains the preferred control/title face. Refreshed regular/bold
  faces each expose 349 codepoints including `← ↑ → ▼`; the complete navigation
  sample shapes wholly in Portsmouth Rapids. Carlito/Noto remain packaged
  fallback for ordinary body and mixed-script coverage on macOS and Win64.

## Verification

- Full native suite: 53/53 tests passed.
- Clean installed `GUIForms::Controls` consumer: configured, compiled, linked,
  and ran from an isolated prefix, including MenuStrip and accelerator use.
- Win64: `File Manager Demoboard.exe` compiled and received the complete
  12-face font directory plus notices.
- Public demoboard boundary policy passed.
- Retained idle gate passed: zero pending damage, layout, paint, active surface,
  or frame deadline after quiescence.

## Native capture

`native-folder-sapphire-house.png`

SHA-256:
`2f8ecbe0d61a01aab76401b72adeb4b9f2765b2732da4bd04c3991831a0ab4f3`

The Controller was intentionally closed before this product capture, as
required by the capture contract. It was relaunched afterward for continued
dogfood.

`native-folder-context-menu.jpeg`

SHA-256:
`facc8d86891ee51201e12cbd6b3666301205a8d88e2097867c54e7b38e18af37`

This physical AppKit capture records the public retained object menu and its
keyboard-opened `Open with` submenu. The checked Image Laboratory command,
disabled Rename row, destructive Delete row, shortcuts, focus highlight, and
edge-bounded placement are visible. Native Show Menu opened the exact same
path used by secondary pointer input.

`native-folder-menu-strip-nested.jpeg`

SHA-256:
`5d16f810763502a0bfd9631fd6e00b288399ee116325760d54d9821d39f0b82d`

This physical AppKit capture records Home selected in the public MenuStrip and
its keyboard-opened Move / copy submenu. Shortcut text, disabled Rename,
destructive Delete, active focus rows, and two retained popup panels are visible.

`native-folder-history-restored.jpeg`

SHA-256:
`52bd1eb4b1f7d59c29f0d03c1140a5b6b81528802d0d2a1e9a5099d5b886ec1d`

This capture follows physical Up navigation to Work and Back activation. It
records Projects restored with the prior Print Masters selection, Forward
enabled, breadcrumb/title/search scope synchronized, and Portsmouth navigation
glyphs in the live controls.

## Negative result retained

Initial dogfood exposed an accessible-but-invisible popup whenever the consumer
root was a `TableLayoutPanel`: `Window::open_popup` had attached the overlay to
the application layout tree, allowing that layout manager to assign it a cell.
The core now owns popup subtrees as independent window-attached retained roots.
ComboBox and ContextMenu overlays subsequently painted and remained present in
the native semantic tree; the menu regression now deliberately uses a table
root so this failure cannot silently return.

## Inference boundary / open work

This evidence does not claim DEMO-D2 complete. Fake rename, provider-scale
million-item tests, the complete responsive ribbon/key-tip vocabulary, and a
dedicated property-panel primitive remain open. Physical Narrator/Orca evidence
and installed native host targets also remain open.
