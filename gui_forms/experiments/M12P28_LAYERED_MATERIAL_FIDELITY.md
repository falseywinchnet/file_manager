# M12-P28 — layered material fidelity

Status: **MEASURED PARTIAL**. Date: 2026-08-13.

## Question

Can an ordinary GUI.Forms consumer express the accepted House Composite depth
vocabulary—Watercolor fresco, Office Pearl stock, Workshop Graphite chassis,
and Studio 2003 caption/well—through bounded retained paint, without a backend
escape, theme engine, browser runtime, or File Manager policy in the library?

## Reference inventory

**OBSERVED:** the accepted HTML/CSS uses ordered linear and radial gradients,
transparent glows, sharp highlight/lowlight boundaries, inset and outset
shadows, a low-amplitude repeating graphite texture, edge-specific seams, and
cap-inset-like pane/control stock. It also distinguishes normal, hot, pressed,
focused, and disabled control states. The sources inspected were
`file_manager_demoboard/reference/frontend-concept-atlas-reference.html` and
`frontend/ui/boards/file_manager/file_manager.wf.css`.

**OBSERVED:** GUI.Forms already retained and replayed most of that vocabulary:

- up to eight ordered solid/linear/radial/image fill layers;
- pad, repeat, and reflect linear-gradient spread;
- normalized and logical gradient geometry;
- rounded clipping;
- up to four inset/outset shadows with bounded outsets;
- whole and per-edge borders;
- stretched, tiled, and nine-patch image layers;
- normal/hot/pressed/pending/invalid/disabled/deactivated state recipes; and
- renderer-neutral recording plus Skia and CoreGraphics CPU replay.

**OBSERVED GAP:** a material could retain one border per edge, but could not
retain two ordered lines on one physical edge. That prevented the accepted dark
outer boundary plus separately inset specular line from being represented as
one reusable material without nested controls.

## Implemented contract

- `MaterialEdge` names top/right/bottom/left without exposing a renderer type.
- `MaterialKeyline` retains edge, color, width, and nonnegative inward inset.
  `inset == 0` is the physical outer keyline; a positive inset is a separately
  placed inner highlight or lowlight.
- `SurfaceMaterial::keylines` retains authored order with a hard maximum of
  eight. Width is finite in `(0,16]`; inset is finite in `[0,4096]`.
- Pointer/count construction validates before constructing the result, and
  `MaterialPanel::set_material` rejects before mutation. A ninth keyline
  therefore fails atomically and leaves the prior retained material unchanged.
- Replay emits each keyline in authored order, collapses an over-inset line to
  no operation, and clips rounded materials to their owned rounded bounds.
  Keylines are internal and do not inflate visual/damage outsets.
- Visual-inspection JSON retains every keyline in authored order with edge,
  color, width, and inset. The inspector material summary now reports keyline
  count beside fill-layer count, shadow count, and radius; its existing bounds,
  outsets, committed operation list, and chunk state remain unchanged.

The dedicated `layered_material_lab` is an ordinary public-API consumer. It
presents named side-by-side Watercolor, Office, Graphite, and Studio specimens;
five simultaneous Office state swatches; one live hover/press/focus button; one
actually disabled button; selectable inspector targets and overlay; high-
contrast and text-scale profile controls; and a visible deterministic rejection
of a nine-keyline recipe. Studio loads a deterministic 12×12 BGRA cap-inset
image into the window-owned image registry and replays it as exactly nine image
regions.

## Material judgments

- **Watercolor fresco — MEASURED REPRESENTABLE:** a 92-degree sapphire-to-
  coral base plus three bounded radial glows, one outset and one inset shadow,
  then a dark outer bottom keyline and bright keyline inset by two logical
  pixels. This is not a flat-gradient substitute.
- **Office Pearl — MEASURED REPRESENTABLE:** four-stop pearl body, a reflected
  logical-period filament, inset/outset depth, physical border, and separate
  top/bottom keylines. Hot, pressed, focused, and disabled recipes differ in
  retained material or focus semantics, not only caption text.
- **Workshop Graphite — MEASURED REPRESENTABLE:** middle-value chassis rather
  than black, a bounded four-logical-pixel repeating texture, inset depth, and
  four directional edge keylines.
- **Studio 2003 caption/well — MEASURED REPRESENTABLE FOR THE ACCEPTED
  SPECIMEN:** nine-patch cap inset, warm overlay, inset well, blue identity
  filament, and factual lower boundary.

## Deterministic evidence

The release configuration on the M4 Mac mini built these targets:

- `gui_forms_layered_material_lab_model`
- `gui_forms_layered_material_lab`
- `gui_forms_layered_material_lab_windows`
- `gui_forms_layered_material_lab_tests`
- `gui_forms_layered_material_raster_tests`

`gui_forms_material_tests` proves exact outer/inset line geometry, authored
replay order, invalid-value rejection, the hard eight-keyline ceiling, atomic
panel rejection, and generated pointer/count rejection.

`gui_forms_layered_material_lab_tests` proves exact recipe composition, pad /
reflect / repeat identity, inset/outset shadow identity, Studio nine-patch
identity and nine-region replay, all five visual states, live hover/press/
capture, semantic profile controls, current retained chunks, at most 24
committed operations per reference specimen, stable resize geometry, and
outer-before-inset order in inspection JSON.

`gui_forms_layered_material_raster_tests` renders the same four recipes at 2×
through CPU Skia and CoreGraphics. It probes Watercolor blue/coral geography
and the two lower keylines, Office crown-to-low depth, bounded Graphite value /
texture, and Studio filament/well identity. The worst summed RGB distance among
eight semantic probes was **5**, against the declared provisional tolerance of
**120**.

The focused CTest set passed **9/9**: drawing trace equivalence, visual
inspection, material, display chunks, layered-material model, layered-material
raster equivalence, bundle font policy, Skia smoke, and CoreGraphics smoke.

The MinGW build compiled both the layered-material model and
`layered_material_lab_main.cpp`. Its final executable link is presently blocked
by two unrelated concurrent connected-control symbols
(`valid_connected_control_topology` and `paint_connected_surface_material`)
missing from the shared Windows link, not by a material-lab symbol. The Windows
native launch/review therefore remains explicitly open for the parent
integration pass.

## Native dogfood

The release AppKit bundle was opened through Terminal in the logged-in Aqua
session and inspected through Screen Sharing at original screenshot detail.
Every material remained visibly unobscured by child/control paint:

- Watercolor showed its sapphire/coral geography, separate soft glows, and
  distinct dark/specular lower edge. Its inspector reported 4 layers, 2
  shadows, 2 keylines, radius 5, 14 current committed operations, and current
  chunk identity.
- Office showed a raised pearl crown and cool lower edge rather than a flat
  white card. Its inspector reported 2 layers, 2 shadows, 2 keylines, radius 3,
  and 13 current operations.
- Graphite remained a middle-value chassis with a subtle diagonal texture, not
  a black surface or a conspicuous stripe. Its inspector reported 2 layers, 1
  shadow, 4 keylines, radius 2, and 15 current operations.
- Studio retained a warm factual well, blue left filament, sharp cap-inset
  boundary, 2 layers, 1 shadow, 2 keylines, and 13 current operations.

The target controls moved the overlay and inspector to all four specimens, the
focused swatch, and the live Office control. Live interaction was captured
while focused/pressed/pointer-captured and again after release. High contrast
turned interactive controls into the host semantic black/white treatment while
leaving the intentionally static material-reference specimens available for
comparison, then returned to standard contrast. The window was exercised at
125% and 150% text scale, resized to the 1200×680 logical minimum without
specimen/inspector overlap, deactivated against Terminal, and reactivated.

Dogfood found one lab-only defect: explanatory copy and the header clipped at
large text. Copy was shortened, title/detail bounds enlarged, and the selector
label reduced from `Focused swatch` to `Focus`. A deterministic 150% test now
uses retained resolved-text metrics to prove every piece of material-lab-owned
copy remains inside its control. Parent-owned inspector-row truncation at its
minimum width is unchanged and is not classified as a material primitive
defect. A second native pass was unnecessary because the correction was only
copy/bounds and the scale geometry is now measured.

## Bounds and remaining gaps

The admitted limits are eight fills, four shadows, and eight keylines per
material; each gradient and image layer retains its existing independent
bounds. The lab's four reference chunks remain at or below 24 committed paint
operations. No specimen starts an animation or an unbounded texture/image
expansion.

This experiment does **not** admit arbitrary masks, isolated compositing groups,
general blend modes, color-management policy, backend-specific blur profiles,
or a general application theme engine. The four accepted specimens do not
require those features for their tested visual hierarchy. Nine-patch currently
stretches its center rather than exposing an independent center tile policy.
Per-edge keylines are straight clipped strokes; they do not provide a separate
curved-corner join language. Cross-host Windows raster/native review and a
profile-qualified PNG exporter remain open. These are explicit FM-R05/R06/R11
edges, not silent empty success.
