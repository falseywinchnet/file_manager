# M12-P27 — bounded visual-state inspection

Status: **MEASURED PARTIAL**. Date: 2026-08-13.

## Question

Can an ordinary GUI.Forms consumer identify why a retained native surface looks
wrong—its final geometry, clipping, live state, material, effective font, and
committed paint—without a private header, backend object, destructive paint
read, browser engine, or unbounded diagnostic dump?

## Implemented contract

- `Window::visual_inspection_snapshot` is a UI-thread, renderer-neutral,
  immutable capture. It forces only the normal layout read barrier and does not
  consume pending damage or start a paint lease.
- The complete retained tree is counted while stored control records and paint
  operations are independently bounded. Invalid zero or excessive limits are
  rejected; every truncation is explicit.
- Each control records stable/runtime/parent identity; requested, arranged,
  absolute, client, display, child-viewport, visual, and effective-clip bounds;
  ancestor clipping; layout inputs/transaction; effective visibility/enabled
  state; focus cue, hover, press, capture, visual status; dirty state; paint
  plane; theme source; exact authored material; display-chunk freshness; and a
  public spelling of each retained paint operation.
- The window record includes logical/device/text scale, contrast, reduced
  motion, sound policy, theme/activity/occlusion, paint-lease state, and
  non-destructive damage by paint plane.
- Committed text operations expose the effective GUI.Forms `FontSpec` after
  text-scale resolution. Text bytes remain redacted by default while byte count
  remains visible; explicit local design tooling may opt in. The record does
  not pretend that a backend-private per-glyph fallback run is public.
- `VisualInspectorView` renders the snapshot as an accessible retained control.
  `VisualInspectorOverlay` marks arranged bounds in blue, visual outsets in
  magenta, and the effective ancestor clip in gold; it is pointer-transparent,
  uses the overlay paint plane, and is excluded from accessibility.
- The Visual Inspector Lab is an ordinary macOS/Win32 consumer with a layered
  material card, resolved text, an interactive state button, an intentionally
  clipped child, selectable targets, the inspector view, and the overlay.

## Deterministic evidence

- `gui_forms_visual_inspection_tests` proves exact nested clip geometry,
  ordered material capture, effective font scaling/tracking, default redaction
  and explicit disclosure, live focus/hover/press/capture, stale chunks,
  non-destructive damage, total-versus-bounded traversal, operation truncation,
  presentation inputs, and deterministic JSON.
- `gui_forms_visual_inspector_lab_tests` proves the lab uses a three-layer/two-
  shadow material, committed text, ancestor clipping, overlay plane and hit-test
  transparency, and an accessible inspector summary that omits the overlay.
- `gui_forms_visual_inspector_lab_font_policy` verifies the native bundle carries
  the pinned GUI.Forms font set rather than silently depending on a host font.

## Native dogfood

The release AppKit lab was built on the M4 Mac mini and opened in the logged-in
Aqua session through Screen Sharing. Direct inspection exercised all four
targets. The inspector reported the material card's three layers/two shadows,
the title's committed content font at 17 px/700 with tracking 0.2, a button as
focused and hovered after activation, and a child whose gold effective clip was
visibly narrower than its arranged/visual rectangles. The display generation
advanced without an inspector-induced perpetual frame loop. Window size
transition preserved the two-pane composition and target identity.

Dogfood also found and corrected a real composition error: the first specimen
used `Panel` for authored material, so the Panel's own paint covered the retained
material commands. The specimen now uses base `Control` surfaces, making the
gradient/shadow material visible while retaining the exact inspection record.

Screen Sharing coordinate synthesis on the narrow AppKit traffic lights landed
on the neighboring caption action during this pass. A refreshed screenshot and
adjusted coordinate closed the window normally. This is a verification-tool
limitation, not admitted as a GUI.Forms host-chrome defect; the custom-chrome
workstream must use independent host events/tests before classifying caption
behavior.

## Honest remaining edge

This completes the reusable inspection seam and dogfood board, not all of
FM-R11. Backend glyph-face and per-glyph fallback identities remain private;
semantic pixel probes, profile-qualified PNG export, paint-cost attribution,
cross-host inspector screenshots, and automatic HTML-prototype image diffing
remain open. Those omissions are explicit and do not turn missing data into an
empty success.
