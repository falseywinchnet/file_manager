# M11h compatibility catalogue and retained paint leases

Status: **MEASURED PARTIAL M11h** on 2026-08-06.

## Question

Does the captured 1,104-identity compatibility facade describe GUI.Forms'
nominal base, and can current owner painting recursively enter or discard the
last coherent retained state?

## Observations

- LibreWinForms commit `1457ed5beef24a7d7e3688a4725c7487da09791e`
  publishes 14,575 `System.Windows.Forms` API rows.
- The pinned .NET 10.0.3 Drawing reference XML publishes 3,522 documented
  `System.Drawing.Common` and primitives rows.
- The prior specimen catalogue covers only 1,104 required identities. It is a
  workload floor, not a Forms/Drawing completeness denominator.
- `Window::paint` previously exposed `in_paint_` without using it to reject
  recursion. It cleared dirty state while callbacks were executing and replayed
  retained chunks directly to the host painter as it traversed.

## Implemented

`tools/winforms_catalogue/generate_catalogue.py` deterministically produces:

- `planning/generated/WINFORMS_API_CATALOGUE.tsv`, one row per published API;
- `planning/WINFORMS_API_CATALOGUE.md`, counts, policy, and state summary.

The current conservative ledger reports:

| Surface | Rows | Admitted | Excluded | Missing |
|---|---:|---:|---:|---:|
| Forms | 14,575 | 13,732 | 843 | 11,157 |
| Drawing | 3,522 | 3,050 | 472 | 1,897 |

`partial_native` means only that a corresponding retained type exists; exact
member behavior remains open unless separately evidenced.

The retained kernel now implements:

- stored Forms-compatible `ControlStyles` and `DoubleBuffered` requests;
- mutable `Name` distinct from immutable stable identity, finite constrained
  bounds, minimum/maximum size, client/window coordinate conversion,
  containment, retained z-order commands, and TabIndex/TabStop traversal;
- content/rendered/presented revisions and a resize/scale surface epoch;
- clean, dirty, rendering, ready, and occluded-dirty lease states;
- one exclusive UI-thread paint lease with deferred nested paint;
- complete transaction recording before commands reach the host painter;
- damage and retained-chunk restoration after callback failure;
- later-pass preservation for mutation during paint; and
- explicit host `notify_presented` release after AppKit/Win32 presentation.

The lease continuation also adds a renderer-neutral, coalesced paint-wake seam.
Model mutation requests one host collection wake even when it originates in a
different Window's callback; consuming all damage rearms the next wake,
occlusion suppresses wakes without losing dirtiness, and exposure wakes once.
AppKit, Win32, and the deterministic headless host implement the same contract.
The public diagnostic state now distinguishes `dirty_queued` from
`rendering_dirty` and reports queued/coalesced wake counts. The complete
high-rate buffering invariant, current evidence, and required compatibility
surface/candidate-raster/pressure gates are frozen in
`planning/PAINT_PIPELINE_AVAILABILITY.md`; coherent latest-state presentation is
mandatory, while only platform accelerations enter availability reporting.
The following M11h-P1 continuation implements revisioned coalesced drains for
the generated direct-GDI bridge and preserves bounded raw-GDI fallback; see
`experiments/M11H_COMPATIBILITY_SURFACE_DRAINS.md`.

The first reusable visual-material correction also implements:

- retained rounded clip/fill/stroke, multi-stop linear gradients with
  pad/repeat/reflect spread, elliptical radial gradients, and bounded box
  shadows;
- validated content-agnostic `SurfaceMaterial`/`MaterialPanel` recipes with
  normalized resize-aware geometry, ordered layers, border, and atomic change;
- bounded `Control::visual_outsets()` so current and former decoration extents
  participate in damage without changing layout, hit testing, or parent clips;
- native CPU realization in Skia, CoreGraphics, and Win32 DIB painters plus
  deterministic minimal-painter fallbacks; and
- File Manager title/ribbon dogfood replacing strip-painted gradients, with a
  public logical-period repeating texture in the title material.

The following reusable visual-composition continuation also implements:

- immutable complete theme recipe matrices for eleven relational roles across
  seven surface states, selected/unselected, and ordinary/high-contrast axes;
- atomic application-wide theme replacement plus inherited/local overrides and
  deterministic state precedence;
- theme adoption in Panel, standard/accent/command Button, CheckBox,
  RadioButton, Card, TextBox, ListBox selection, ComboBox, MenuStrip, and
  ProgressBar, with explicit local-style compatibility preserved;
- public atomic header/body/footer `Card` composition with full interaction and
  selected/busy/invalid semantics;
- public `MasterDetailView` with arbitrary retained roles, genuine splitter
  input, focus-safe compact navigation, and automatic wide restoration; and
- File Manager twelve-card palette-laboratory dogfood with public semantics,
  selection transfer, controller routing, and wide reflow;
- File Manager four-record DNA decision-browser dogfood using public Card and
  MasterDetailView, including wide and compact navigation; and
- a retained full-coordinate themed Window backplane before application planes,
  which clears detached-overlay pixels under transparent layout roots while
  preserving damage clipping; and
- immutable validated spacing, geometry, typography, and motion structure
  tokens, with inherited Card/MasterDetail defaults and explicit override/reset.
- theme-aware Label body/control/caption/heading/title/monospace roles with
  independently authoritative font/foreground overrides and reset; and
- public typed `ReviewCard` composition for atomic validated title, summary,
  verdict, disposition, interaction, and complete semantic projection.

The first rich-image continuation implements the previously absent ImageList
behavioral center rather than a handle-shaped facade:

- one Window-bound nonvisual Component with ordered ASCII-case-insensitive keys,
  stable indices, bounded logical size, Tag, revision, and tokenized changes;
- atomic PNG or existing-ImageId entries with normal/hot/pressed/selected/
  disabled and 0.5x–8x density variants;
- deterministic state fallback and exact/nearest-larger/nearest-smaller density
  choice without changing logical layout size;
- synchronous release of owned generational resources on explicit disposal;
- direct Image, ImageList, ImageKey/ImageIndex, nine-way alignment, five
  text/image relations, gap, preferred measurement, and actual state painting
  across the Button family; and
- keyed virtual-row consumption in TreeView and ObjectView, retaining their
  bounded realization and procedural glyph fallback.

The next image-material continuation closes the source-crop substrate rather
than implementing nine-patch as showcase paint code:

- `Painter` and retained display chunks carry source-pixel image regions;
- Skia, CoreGraphics, and Win32 DIB painters realize the same crop contract;
- public `SurfaceMaterial` owns validated stretch, exact-period tile with
  cropped partial edges, and density-aware nine-patch image layers;
- attached `MaterialPanel` rejects resources missing from its Window or carrying
  stale declared dimensions atomically;
- GUI.Drawing owns a bitmap-snapshot `TextureBrush` with tile/mirror/clamp,
  affine transforms, cloning, deterministic traces, and Skia execution; and
- the Complete Showcase dogfoods public nine-patch and tiled material panels
  with native pixels and semantic descriptions.

The physical Wine automation target now navigates to that image-material page,
captures the 1280×820 Win32 DIB surface, and closes normally. The captured
surface visibly preserves the authored nine-patch corners/rules and repeated
diagonal tile period; Skia and CoreGraphics source-crop raster probes pass
independently.

## Measured gates

The focused lease suite proves deferred reentry, touch-during-render,
failure/restore/retry, ready-to-clean presentation release, and buffering style
retention. Ten impacted native CTest targets pass, including core, damage,
display chunks, scheduler, headless trace, host protocol, controls, showcase,
File Manager demoboard, and macOS close. The complete macOS build and the strict
Win64 `gui_forms_showcase_windows` cross-build pass.

The material slice passes material/display-list and stock theme-role routing
tests, exact-period Skia and
CoreGraphics raster smokes, File Manager behavior and public-source policy gates, and a
strict Win64 host plus File Manager consumer cross-build. Live Computer Use of
the individual AppKit product window shows the material bands filling maximized
geometry; the clipped view was a gathered/downscaled overview plus responsive
compact collapse, not a fixed product canvas.

Live individual-window inspection also reproduced a controller-to-product
stale-frame failure: semantic state changed to Program DNA while pixels remained
on Folder until input reached the product host. After the coalesced paint-wake
seam, the identical controller action immediately presents Program DNA without
a product-window click. Core and headless tests cover coalescing, rearm,
occlusion, exposure, and model-originated wake independence.

The ImageList continuation passes focused Button and virtual-collection tests,
the Complete Showcase interaction/source/font policies, and native AppKit
inspection. The Showcase's Standard button visibly presents a public keyed
folder image before text and selects its authored hot raster while the pointer
remains over it. Native HIMAGELIST handles/streams, AddStrip, color-key
transparency, palette quantization, and the role/provenance/vector pack remain
open rather than being represented as no-ops.

## Honest boundary and next tranche

This does not close the 18,097-row catalogue. It also does not yet queue input
that is synthetically dispatched during paint, swap a second raster after a
backend replay exception, preserve all damage rectangles through every native
presenter, or project native style/lease facts through ABI 1.0. The
material/composition tranche still does not provide masks/groups/effects,
vector/provenance role assets, complete theme/structural-token adoption and
density variants, remaining typed specimen/disclosure/step specializations,
rich icon packs, or C ABI projection.

Next work should process the high-leverage P0 gaps mechanically, beginning with
Control geometry/events/style completeness, scrollable containers and
scrollbars, validation/dialog-key ordering, multiline/IME text, then reusable
theme/material/card/icon composition. Drawing proceeds in parallel by exact
Graphics/GraphicsPath/Region member groups rather than specimen-only calls.
