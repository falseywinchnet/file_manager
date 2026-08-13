# M12P33 — Fixed and responsive retained tracks

Date: 2026-08-13  
Status: **MEASURED PARTIAL** — portable solver, retained panel, headless File
Manager geometry corpus, native macOS build, Windows cross-build, public
inspection, font policy, and native macOS interaction review are green. Windows
native interaction and profile-qualified raster evidence remain open.

## Question and boundary

Can GUI.Forms preserve the House Composite's fixed nominal shell bands, assign
all remaining space to the primary content track, grow text-bearing rows from
real measurement, and then apply only an authored collapse order across narrow,
short, and large-text surfaces?

This tranche adds a one-dimensional retained track primitive which can be
nested into a grid. It does not parse CSS, import browser media queries, infer
business priority, implement command overflow, or replace `SplitContainer`.
The File Manager-shaped lab supplies policy; the reusable control knows only
fixed, content, remaining, min/preferred/max, weight, gap, authored visibility,
collapse priority, and explicit focus protection/reveal.

## Existing layout inventory

**OBSERVED:** the public control library before this tranche already supplied:

| Family | Existing useful contract | Remaining gap addressed here |
|---|---|---|
| `TableLayoutPanel` | absolute, auto, weighted-percent tracks; spans; gaps; cell docking | no min/preferred/max track record or authored priority collapse |
| `FlowLayoutPanel` | wrapping, flow breaks, alignment, spacing, per-child flex growth | no stable shell tracks or collapse thresholds |
| Dock/Anchor | all edge/fill docking; compound anchors; deterministic z order | relational child placement, not responsive region policy |
| `SplitContainer` | retained panes, minima, fixed panel, enlarged seam hit zones, collapse/restore | user pane instrument, not a general fixed/remaining shell grid |
| `ScaledPanel` | fixed design-space scaling | deliberately unsuitable for relationship-derived large text |

`ResponsiveTrackPanel` fills only the final column. Horizontal and vertical
instances nest; existing controls continue to own their internal layout.

## Public contract

`ResponsiveTrackSpec` declares:

- `fixed`: allocate the authored preferred extent independent of content;
- `content`: elevate the runtime minimum to the measured main-axis demand,
  bounded by the authored maximum, so reflow/growth precedes collapse;
- `remaining`: retain its minimum/preferred extent and consume free space by
  positive authored weight;
- finite ordered `minimum`, `preferred`, and `maximum`, where zero maximum is
  unbounded;
- an optional unique `collapse_priority`; smaller values collapse first;
- independent `hidden` state for an authored-hidden track.

`resolve_responsive_tracks` is renderer-free. It subtracts relational gaps,
shrinks desired whitespace toward the resolved minima, applies only declared
collapse candidates when minima no longer fit, and then distributes free
extent to remaining tracks. The result exposes per track:

- measured content, resolved minimum, desired and allocated extent;
- logical start/end and cumulative snapped device-pixel start/end;
- collapse reason (`authored_hidden` or `insufficient_extent`), threshold, and
  protection state;
- total minimum/desired/allocated extent, overflow, and collapse order.

Device scale never changes logical allocation. Cumulative boundaries are
rounded once from logical coordinates, preventing per-track rounding residue
from drifting the final boundary.

`ResponsiveTrackPanel` maps retained direct children to tracks, publishes a
committed resolution revision alongside the existing pending/committed
`LayoutTransactionState`, and accepts persistent explicit reveal protection.
When the focused track is the final track that cannot fit, the author may name
one exact retained fallback control. Focus moves to that target before collapse;
without a live eligible target the focused track stays protected and overflow
is reported.

`Control::layout_collapsed()` is separate from authored `visible()`. A layout-
collapsed subtree is effectively absent from painting, hit testing, focus, and
semantics while its authored visibility and retained identity remain intact.
Public visual inspection and JSON report both `visible` and
`layout_collapsed`.

All records are portable C++ and contain no Skia, AppKit, Win32, browser, or
CSS type.

## Invalid and overconstrained input

The complete candidate track vector validates before mutation. The panel and
standalone solver reject:

- zero or more than 64 tracks;
- unknown modes/orientations;
- NaN, infinity, negative, inverted, or excessively large bounds;
- nonpositive/excessive remaining weights;
- duplicate collapse priorities;
- invalid child/protection indices, foreign focus targets, invalid gap/device
  scale, and measurement vectors whose size or values do not match the specs.

If protected or non-collapsible minima remain larger than the surface, the
solver reports `overflow=true` and clamps published geometry to the available
extent. It does not silently invent another collapse priority. The accepted
150×150 lab corpus does not enter this fallback.

## House-shaped demoboard

`gui_forms_responsive_tracks_lab` places a 44-pixel review controller above an
exact product-surface viewport. The viewport uses nested public responsive
panels:

- vertical identity/title, menu, command shelf, navigation, workspace, and
  status tracks;
- horizontal grouped tree+3-pixel seam, remaining object field, and grouped
  3-pixel seam+Selection/inspector tracks;
- public `VisualInspectorView`, pointer-transparent inspection overlay, focus
  target/fallback, committed revision, allocation, and threshold diagnostics;
- 1450×850, 1200×760, 960×680, 720×520, 480×360, 300×240, and 150×150 presets;
- 100%, 125%, 150%, and 200% text-scale presets plus explicit Selection reveal
  and return-to-authored-auto-collapse controls.

The specimen is product-shaped but fixture-only. Core APIs contain no band
name, pane name, threshold, or File Manager string.

## Headless measurements

Environment: M4 Mac mini, arm64 Release model/tests. At 1450×850 and nominal
text scale, the nested committed track snapshots are:

```text
rows     40 / 23 / 66 / 40 / 657 / 24
columns  221 / 938 / 291
```

The grouped columns preserve the reference `218 + 3 / remaining / 3 + 288`
relationship without asking the generic solver to couple two unrelated track
identities.

Authored width collapse is deterministic:

| Available width | Selection | Tree | Primary content | diagnostic |
|---:|---|---|---:|---|
| 436 | present | present | 150 | minimum composition fits |
| 435…273 | collapsed | present | remaining | Selection threshold = 436 |
| 272…150 | collapsed | collapsed | all available | Tree threshold = 273 |
| 150 | collapsed | collapsed | 150 | bounded, no overflow |

The 100/125/150/200% corpus proves every visible content row allocates at least
its resolved measurement-driven minimum. Large text at short height collapses
the command shelf and status before the primary workspace. Twenty repeated
resize cycles settle without bounded-pass hits or a phantom committed
resolution revision.

A fractional three-track corpus proves identical logical boundaries at 1× and
2×, with terminal device boundaries of 100 and 200 pixels respectively and no
cumulative overlap. Focus tests prove fallback-before-collapse, explicit
reveal, authored visibility preservation, semantic removal, and public
inspection of `layout_collapsed`.

## Commands and current results

Mac targets:

```text
cmake --build gui_forms/build --target
  gui_forms_responsive_tracks_lab
  gui_forms_responsive_tracks_lab_tests
  gui_forms_responsive_track_panel_tests --parallel 10
```

**MEASURED:** targets build. Focused and boundary tests currently pass:

- `gui_forms_responsive_track_panel_tests`
- `gui_forms_responsive_tracks_lab_tests`
- `gui_forms_responsive_tracks_lab_font_policy`
- `gui_forms_layout_panel_tests`
- `gui_forms_split_container_tests`
- `gui_forms_typography_scale_lab_tests`
- `gui_forms_visual_inspection_tests`
- `gui_forms_host_boundary_audit`
- `gui_drawing_renderer_boundary_audit`

Windows cross-build:

```text
cmake --build gui_forms/.build/windows-x64-skia-make \
  --target gui_forms_responsive_tracks_lab_windows --parallel 10
```

**MEASURED:** `GUI.Forms Responsive Tracks Lab.exe` links successfully. This is
build evidence, not Windows native interaction evidence.

## Native dogfood

Environment: M4 Mac mini, macOS native AppKit/Skia host, Release app, controlled
through Screen Sharing at fit and actual-size zoom. The lab was launched from
the logged-in Aqua Terminal through a unique temporary symlink below
`CodexRuns`; only the lab and that symlink were closed/removed afterward.

**MEASURED:** every declared surface preset was exercised: 1450×850, 1200×760,
960×680, 720×520, 480×360, 300×240, and 150×150. The first native pass found a
fixture defect: the buttons changed the retained `Window` client size, which
the AppKit host correctly reasserted from its physical client. The fixture now
keeps its review window stable and previews the exact selected product-surface
extent inside it. A focused model test prevents that regression. The corrected
native pass showed:

- reference and ordinary presets retain the title/menu/shelf/navigation/status
  hierarchy, both side panes, 3-pixel seams, remaining object field, overlay,
  and live `VisualInspectorView`;
- 300×240 removes Selection first while keeping the tree and focused primary
  field; 150×150 removes both side panes and remains a bounded primary surface;
- explicit `Reveal` at 300×240 restores Selection and reports honest overflow;
  `Auto` returns to the authored Selection-first collapse;
- 100/125/150/200% presets progressively grow the measured title, menu, shelf,
  navigation, and status line boxes before any short-height collapse. Native
  review also found that the fixture's long non-wrapping primary button and
  verbose review-only reveal labels clipped at 200%; those fixture strings were
  shortened and the final focused test/build stayed green;
- taking the window inactive and active again did not change allocation,
  collapse order, color roles, or legibility. The native title bar alone follows
  AppKit active-state treatment, as intended.

At reference size, the sapphire title, compact pearl command bands, graphite
navigation/status, white object field, and asymmetric side panes preserve the
HTML prototype's title/body/control hierarchy and density relationships. This
is a layout-contract lab, not pixel parity evidence: inspector text is
intentionally diagnostic-dense, OS Login Items notifications occluded part of
the rightmost pane in some fit-to-window captures, and no screenshots are used
as authoritative threshold or device-pixel measurements.

## Remaining limits

- Track collapse is instantaneous retained state; motion is outside this item.
- Ribbon/menu group shortening and overflow are command-composition policy, not
  inferred by this solver.
- The panel is one-dimensional by design; callers nest panels rather than
  receiving a CSS/Grid engine.
- Coupled tracks such as pane+seam are represented by a grouped child; there is
  no generic track dependency language.
- Explicit reveal is retained until the author clears it. Automatic
  focus-triggered reveal callbacks and scroll-plane focus reveal remain part of
  FM-LY06.
- Windows native review and native 2× raster review remain open.
