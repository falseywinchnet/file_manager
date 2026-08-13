# M12p35 retained command overflow projection

Date: 2026-08-13

Status: **MEASURED**

## Question

Can an authored command shelf collapse complete command groups in a stable,
inspectable order without inferring business priority from coordinates, moving
focused commands out of the focus tree, or giving GUI.Forms authority over the
overflow menu?

## Contract

`CommandOverflowPanel` is a horizontal retained container with explicit
`CommandOverflowGroupSpec` records. Each direct command group declares bounded
minimum, preferred and maximum extents and an optional unique collapse
priority. Smaller priorities collapse first. A separately identified overflow
actuator is layout-collapsed while all groups fit and becomes available once a
group moves out of layout.

`CommandOverflowSnapshot` publishes available extent, gap, actuator state,
overflow state, committed revision and one result per group, including the
group's stable ID, measured/allocated extent, priority and collapse state.
GUI.Forms owns only this layout decision. The consumer uses those stable IDs to
compose authorized menu items and execute commands.

Focused descendants protect their group when another collapsible group can be
removed. Group metadata validates before mutation; invalid bounds, duplicate
priorities, foreign controls and invalid actuator relationships preserve the
previous committed model. Collapse uses `layout_collapsed`, so authored
visibility, command semantics and later restoration remain distinct.

## File Manager and Web.Forms projection

Web.Forms admits bounded `command-overflow` and `dropdown-button` control kinds
plus explicit group extents, priority and actuator metadata. Stage 2 emits the
typed panel, typed drop-down controls, group specifications and actuator
relationship. File Manager no longer rebuilds the command shelf or computes a
second responsive projection in `application.cpp`; it only turns the snapshot's
collapsed group IDs into the live overflow menu.

## Evidence

**MEASURED:** focused GUI.Forms tests prove the full, narrow and compact shelves;
whole-group collapse; actuator preservation; focus protection; stable-ID
publication; and atomic duplicate-priority rejection. The complete M4 GUI.Forms
Release suite passes 86/86. The File Manager M4 application suite passes 11/11,
including ordinary and compact shelf behavior and three retained drop-down
commands. The complete MinGW/Skia build compiles and links the new control,
tests and Windows labs.

The Web.Forms suite passes 37/37 and verifies deterministic C++17 generation,
typed command/drop-down construction and manifest-gated overflow metadata.

The later 11-profile fidelity matrix exercises the generated File Manager at
150x150: both authored groups are layout-collapsed as whole units and the
explicit 72-logical overflow actuator is visible. Raw HTML accepts the 150x150
viewport but does not execute the retained priority solver, so the oracle
records the resulting state difference rather than pretending CSS performed
that work.

## Boundary

- Menu contents, command enablement, authorization and execution remain product
  code; they are not inferred from HTML labels.
- This is priority collapse, not a general ribbon reflow or command-ranking
  algorithm.
- Keyboard/pointer behavior of the overflow popup is supplied by the existing
  `DropDownButton` and `ContextMenu` contracts.
- Overflow-menu contents and ordering are derived from the retained snapshot by
  File Manager; GUI.Forms does not infer them from labels or coordinates.
- Advanced masks and blend modes remain outside this tranche. The accepted
  command shelf is represented by ordered fills, shadows, borders and keylines;
  a future specimen must supply a bounded source grammar, renderer-neutral
  semantics and cross-renderer evidence before that decision reopens.
