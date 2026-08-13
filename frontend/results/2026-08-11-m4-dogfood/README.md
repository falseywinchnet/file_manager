# File Manager protected-slice prototype corrective M4 dogfood

Status: **SUPERSEDED protected-slice evidence; rejected as File Manager 1.0
promotion evidence**. The exercised ADR-016 through ADR-020 protected-root
macOS slice passed the measurements recorded below, but the owner subsequently
classified the application as `0.001-alpha`. See
[`../../planning/OWNER_CORRECTION_2026-08-11.md`](../../planning/OWNER_CORRECTION_2026-08-11.md)
and
[`../../planning/IMPLEMENTATION_REPAIR_LEDGER.md`](../../planning/IMPLEMENTATION_REPAIR_LEDGER.md).
This is not Developer-ID distribution, daily-root replacement, or Windows/Linux
promotion.

Date: 2026-08-11.

Host: `Joshuah's Mac mini`, Apple M4, macOS arm64. The Neo checkout remained
authoritative and `m4build` mirrored it to
`$HOME/Developer/CodexBuilds/file_manager-2d80cdb86b7d`. Browser and native UI
inspection ran in the Mini's logged-in Aqua session through Screen Sharing.

## Corrective result

The 2026-08-10 executable remains rejected as a 1.0 application. The current
build replaces that surface with the accepted dense House Composite topology:
Watercolor identity, classic application menu, Office Pearl shelf, graphite
path/search chassis, one scope-rooted folder tree, one white object field,
collapsible retained splitters, factual Selection/Properties, and local status.

`Places`, `Recent locations`, `show-recent`, and the corresponding generated
member are absent from the checked-in Web.Forms source and frontend C++—not
merely hidden. New Folder is background-only; Rename and Open are object-only;
Move, Copy, Paste, Delete, view, sort, properties, checksum, Terminal Here, and
Copy Path appear only in their admitted command contexts.

The final Web.Forms lowering reports 116 typed nodes, 43 buttons, 36 labels,
34 flow containers, three grids, a complete bounded native-tree projection,
and source digest
`b6aeb362830b387f09c0be2ec5a63e589376a6ba8c96a943d3ffc788f40193fc`.
The browser-valid source and the concept/material/visual-cue boards were opened
in Brave on the M4; the browser was not launched on the Neo.

## Control-by-control result

| Surface | Exercised result |
|---|---|
| File/Home/Edit menus | Open, New Folder, Settings, Close, selection transfer, Delete, Properties, Select All, Rename, Paste and Undo route to one shared command state; unavailable mutations explain the read-only or selection prerequisite. |
| View/Go/Commands/Help menus | Small icons/details, four factual sorts, refresh, both pane toggles, back/forward/up/root, SHA-256, Terminal Here, Copy Path and About executed their then-admitted action. The rejected build's About surface reported `1.0.0`; that version claim is superseded and is not current product truth. |
| Shelf | Move/Copy opens the transfer menu; Delete uses two-step quarantine; View and Sort open checked menus; Properties restores/focuses the factual inspector. Buttons are context-disabled, not placeholders. |
| Location | Back, Forward and Up follow retained history; the breadcrumb enters inline exact-path editing; direct paths are contained beneath the launch root; search performs an exact Orchestrator Engine subtree request. |
| Folder/object field | Tree activation, object selection, folder activation, details/icons, four sorts, multi-selection, empty folders, refresh and out-of-root/symlink refusal were exercised. The final persisted default is compact small icons. |
| Search | Exact `read-me.txt` catalogue results, source-bound continuation, no-result state and More Results gating were exercised. APFS case-equivalent root spelling is rebased to the exact protected root before result admission. |
| Selection/properties | Selection updates kind, exact location, size, modification time and bounded text/PNG preview; no selection clears every fact. Unsupported and linked objects remain explicit. |
| Operations | New Folder/Undo, inline Rename/Undo, staged Copy, same-volume Move/Undo, collision refusal and two-step Delete/quarantine/Undo were exercised in the disposable `fmsandbox`; no operation followed a symbolic link or overwrote an occupied destination. |
| Settings | All ten tabs open. Available scalar controls stage, Cancel/Reset/Apply through one optimistic transaction. Density, motion and default-view commits were exercised live. General, Search and Services display exact deferred reasons instead of editable fake values; Applications and Advanced state that no settings are admitted. |
| Services | Live Orchestrator/Engine identity and generation are shown. Refresh, Engine integrity and reconcile commands returned identity-bound results; unavailable diagnostics is rendered as deferred with its reason. |
| Trusted local actions | Default Open, fixed-argv Terminal Here, Copy Path, streamed SHA-256/cancel and bounded built-in preview are covered by native observation plus focused tests. Dynamic Open With and plugin commands remain absent because their contracts are deferred. |
| Splitters/window | Both retained splitters collapse/restore, the window resizes through the native host, and Close joins the worker and removes the process. |

## Retained failures and fixes

- The first corrected settings build claimed Restore Last Location, Language,
  live fallback and diagnostics controls were available without consumers.
  Their schema states are now `deferred` with exact reasons; the default view
  schema is `icons`.
- A service command used the frontend bootstrap generation instead of the
  selected service generation. The request now carries the displayed service
  identity/generation; live reconcile and integrity checks passed.
- Engine results whose APFS root differed only by case spelling were rejected
  after an exact query. Result paths are now rebased from the identity-equivalent
  returned root to the exact launch root, with regression coverage.
- Re-copying into an existing staged `.app` preserved a stale `_CodeSignature`.
  Final deployment always copies into a new staging bundle, ad-hoc signs it,
  verifies it strictly, and atomically renames it. The invalid bundle and each
  prior working bundle were retained as named rollback artifacts.
- Applying compact density requested a `94x70` ObjectView cell, below the
  GUI.Forms `86x78` minimum, and produced SIGABRT. Crash report
  `File Manager-2026-08-11-092736.ips` identified
  `ObjectView::set_icon_cell_size`. The application now requests `94x78`.
  It was rebuilt, then launched with the already-persisted compact setting—the
  exact former startup crash condition—and remained operational.
- The owner-rejected `Recent locations` control survived one repair as a
  hidden authored node. It was deleted from Web.Forms and its generated C++
  use; a source audit now returns no `Places`, `Recent locations`,
  `show-recent`, or `sidebar_recent` match in product UI/source/tests.

## Final measured state

- Frontend Release build: 8/8 tests passed on M4 after the final source and
  version changes.
- Web.Forms: 27/27 tests passed after removing the rejected node.
- GUI.Forms: 62/62 tests passed on M4.
- Engine: `go test ./...` passed across every package on M4.
- Orchestrator: the full M4 verifier passed current generated docs, 65 library
  tests, CLI/integration/C++/fixture/live-search/hostile/wire suites,
  warnings-denied Clippy and Release build.
- `git diff --check` passed.
- Live settings snapshot: revision 5, compact density, motion false, text scale
  100, default view icons.
- Installed Orchestrator SHA-256:
  `f7dbc8060cc2b7cf743a164efaad56b9ad9f3ea5cce1fff4c390bf4e8894f79a`.
- Rejected historical app bundle: version `1.0.0`, arm64, strict deep ad-hoc signature valid,
  identifier `local.filemanager.frontend`, executable SHA-256
  `edbcd2999b1cfe3890a399ad987e118bf42fbbdb1a74e0449cae3b0ce1b1993f`.
- Final process observed running from
  `$HOME/Developer/CodexRuns/fmnew.app/Contents/MacOS/File Manager` with the
  explicit disposable root, Engine root identity, mutation opt-in and separate
  quarantine arguments.

## Honest boundary

This record proves only that the named contained protected-root slice exercised
the behaviors and measurements above. It does **not** prove File Manager 1.0,
visual fidelity, complete control behavior, or real-world daily usability. The
app was ad-hoc signed for M4 dogfood. Developer ID,
notarization, installer/update/uninstall, broad daily-root admission,
crash-restart operation recovery, VoiceOver promotion, dynamic handlers and
commands, plugin previews, application registry, other platforms, and Malkuth
suite release remain outside this claim.
