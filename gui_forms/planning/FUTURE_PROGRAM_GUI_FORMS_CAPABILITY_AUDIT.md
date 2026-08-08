# Future-program GUI.Forms capability audit

Status: **OBSERVED planning audit; implementation remains dependency-gated**.
Date: 2026-08-07.

## Boundary

**DECIDED:** Paint, Text Editor, and Games own application state, algorithms,
documents, rules, and persistence. GUI.Forms owns only reusable retained UI,
GUI.Drawing, input, timing, accessibility, owned-window, and transfer
substrates. This record does not open those application implementations.

Sources inspected:

- `planning/FUTURE_APPLICATION_CONSUMER_PROFILE.md`
- `../paint/planning/CAPABILITY_PROFILE.md`
- `../paint/planning/COLOR_DIALOG.md`
- `../paint/planning/IMPLEMENTATION_SEQUENCE.md`
- `../text_editor/planning/CAPABILITY_PROFILE.md`
- `../text_editor/planning/CHARACTERS_DIALOG.md`
- `../games/planning/SHARED_GAME_KERNEL.md`
- `../frontend/planning/FRONTEND_001.md`
- `../frontend/planning/DOCUMENT_PICKER_SURFACE.md`

## Paint delta

**OBSERVED required:** CPU bitmap/canvas with local damage, alpha compositing,
clipping, paths, shapes, fills, stroke, text, interpolation, pointer capture and
pressure, typed clipboard/drag with lazy image payload, file/color dialogs, and
headless/native evidence. The detailed Color dialog is an owned dialog with
foreground/background/swap, visual field, sliders/numerics, current/new
swatches, alpha, recent colors, eyedropper import, text copy/paste, RGB/hex,
OKLCH, and explicitly profiled or informational-only CMYK.

**MEASURED available:** retained captured input, local invalidation/display
chunks, GUI.Drawing recorder plus CPU Skia execution, transactional bounded
CPU bitmap edits with multi-consumer damage history, a retained `RasterCanvas`
with zoom/pan/nearest-linear sampling and exact damage projection, explicit
linear sRGB/XYZ D65/OKLab/OKLCH conversion and gamut mapping, alpha-aware
`Color`, TextBox/NumericUpDown/slider controls, popup/focus ownership, nullable
and culture-aware property values, and specialized color property editing.

**OPEN reusable substrate:** pressure-bearing pointer samples, complete CPU
path/text/image raster semantics, lazy typed image transfer, a modal detailed
`ColorDialog`, ICC/profile policy and renderer-side partial image-cache upload.
No ICC asset was found in the inspected BFFT viewer
tree; its programs are an **OBSERVED algorithm reference**, not an admitted
profile package. RGB truth must name transfer function, primaries, and white
point before CMYK claims are made.

## Text Editor and Characters delta

**OBSERVED required:** mature multiline editing with large-document behavior,
IME/bidi/shaping/fallback, accessible text ranges, decorations, modeless Find/
Replace, and an owned Characters dialog. The dialog needs a virtualized glyph
grid, local code-point/name/keyword search, block/category filters, glyph/code
point/name/UTF-8/escape/font/missing detail, multi-selection, copy/insert,
keyboard navigation, high DPI, and pinned Unicode/font data. Startup font or
Unicode scanning is rejected.

**MEASURED available:** strict UTF-8 and Unicode grapheme/line indexing,
single-line editing/selection/clipboard, retained scrolling and virtual rows,
font fallback service boundaries, owned dialog/focus infrastructure, and the
updated Portsmouth interface glyph set.

**OPEN reusable substrate:** multiline viewport/editor engine, shaping and
fallback backend closure, bidi/IME and accessible text ranges, two-axis
virtualized grid selection, pinned Unicode name/block/category dataset, glyph
coverage lookup, and modeless owner tracking.

## Games delta

**OBSERVED required:** board/grid/card primitives, z-ordered overlapping hit
testing, captured drag, deterministic command/replay presentation, test-clock
animation, owned dialogs, accessible virtual children, keyboard equivalence,
and sound hooks. Perpetual redraw loops are rejected; animation must wake only
for active deadlines/damage.

**MEASURED available:** retained z-order/hit/capture, deterministic timers and
motion clock, local invalidation, virtual semantics, keyboard/pointer routing,
dialogs, progress/motion styles, and audio cue contracts.

**OPEN reusable substrate:** reusable board/card presentation controls,
overlap/drop-target conformance fixtures, deterministic animation-command
replay adapter, richer virtual-child grid semantics, and packaged sound assets/
host verification. Game rules remain outside GUI.Forms.

## File Manager/frontend delta

**OBSERVED required:** responsive retained composition, document picker,
virtualized object list/tree/grid, split panes, navigation/commands, settings
and owned dialogs, drag/clipboard, accessibility, and stable availability/
capability reporting.

**MEASURED available:** the mockup dogfood surface, responsive flex/split/
scroll composition, lists/trees/property settings, commands/menus/toolbars,
input and text refinements, popup/modal focus, semantic snapshots, and native
macOS plus Win32/Wine laboratories.

**OPEN reusable substrate:** FM0 consumption snapshot/go-ahead, full document
picker conformance, remaining large-data virtualization and accessible range
publication, finalized shaping/font packaging, Linux host evidence, and stable
availability/version manifests.

## Dependency-ordered next work

1. Define the DML-safe property schema; ABI 0.24 now carries TypeDescriptor,
   TypeConverter, UITypeEditor drop-down/modal services, standards,
   reset/change, nullable, and atomic multi-owner behavior.
2. **MEASURED M12-P26:** GUI.Drawing editable bitmap/canvas, local-damage
   fixtures, linear sRGB ↔ XYZ D65 ↔ OKLab/OKLCH, gamut status and deterministic
   chroma-only mapping.
3. Build the reusable detailed Color dialog over P26; keep CMYK
   profile-explicit and disclose the still-open ICC policy.
4. Close multiline shaping/IME/a11y text, then build the virtualized Characters
   grid/dialog on pinned datasets.
5. Add board/card/grid dogfood controls and deterministic replay animation.
6. Re-audit the File Manager consumption profile and issue the named FM0
   snapshot only when its gates are measured.
