# M12-P26 — editable raster canvas and color truth

Date: 2026-08-07
Status: **MEASURED PARTIAL**

## Question

What reusable GUI.Forms/GUI.Drawing substrate must exist before Paint can own a
document model without copying pixels through a demo-private control or
pretending full-control repaint is local damage?

## Boundary

- **GIVEN:** GUI.Forms remains retained and renderer-neutral; Skia is private.
- **GIVEN:** Paint owns tools, selection, undo, persistence, and document truth.
- **OBSERVED:** GUI.Drawing already owned COW bitmap storage, full-image locks,
  snapshots, PNG, retained commands, and CPU raster execution.
- **OBSERVED GAP:** mutation had no cancel path or region history, Window had no
  public local-rectangle invalidation, raw resource replacement copied a full
  image, and no reusable bitmap viewport consumed generations.
- **DECIDED FOR THIS 0.x SLICE:** extend the existing bitmap rather than create
  a second canvas storage model. This is reversible before ABI 1.0.

## Implemented contract

### Transactional bitmap edits

`gui_drawing::Bitmap::begin_edit(RectI)` returns one exclusive writable region.
The pointer starts at the region origin and retains the image stride. Exactly
one `commit_edit` or `cancel_edit` completes the token.

- begin rejects empty/out-of-image rectangles and concurrent locks;
- cancel restores the leased bytes byte-for-byte and preserves generation;
- commit compares the bounded backup with current bytes;
- no byte change publishes neither generation nor damage;
- changed scanline spans merge vertically when their x/width agree;
- more than 256 spans compact conservatively to the edit rectangle;
- prior `ImageSnapshot` storage remains immutable through COW;
- 256 mutation generations remain queryable by any number of consumers;
- an older query reports `history_complete=false` and full-image damage.

Legacy full-image write locks retain their conservative full-image damage rule.
`set_pixel` no longer advances generation when encoded bytes are unchanged.

Drawing ABI 0.2 appends edit begin/commit/cancel and `changes_since`. ABI 0.1
still negotiates its size-prefixed prefix and receives its requested version.

### Retained canvas consumption

`RasterCanvas` is a public control over an application-owned shared
GUI.Drawing bitmap. It implements:

- bounded zoom `[1/64, 256]` and finite pixel-space pan;
- client↔bitmap coordinate transforms and visible-source calculation;
- explicit nearest or linear retained sampling;
- bounded transparency checkerboard presentation;
- BGRA direct projection and RGBA→BGRA projection without mutating the bitmap;
- generation synchronization through rectangular registry patches;
- local pixel damage mapped into exact client/window rectangles;
- one-pixel conservative expansion for linear-filter neighborhood sampling;
- pre-detach Window resource retirement and semantic dimensions/zoom.

`Control::invalidate(Rect)` now exposes local client invalidation without
weakening complete retained display chunks. `ImageRegistry` and `Window` accept
an atomic raw BGRA patch whose final source row may be tightly terminated.

The retained painter vocabulary now carries `ImageSampling`. Skia honors
nearest/linear filtering and CoreGraphics honors none/high interpolation.
Minimal painters retain the existing coherent region fallback.

### Explicit color truth

GUI.Drawing adds named straight-alpha values and checked conversions for:

- IEC 61966-2-1 encoded sRGB ↔ linear sRGB;
- linear sRGB ↔ normalized XYZ D65;
- linear sRGB/XYZ D65 ↔ OKLab;
- OKLab ↔ canonical OKLCH hue `[0, 360)`;
- OKLCH → sRGB with unclamped linear channels plus gamut/clipping status;
- 24-iteration deterministic OKLCH chroma reduction preserving L, h, alpha.

The known red vector, mixed-alpha/channel round trips, invalid alpha/chroma,
out-of-gamut disclosure, and mapped invariants are executable tests.

## Evidence

Measured on macOS arm64 in the repository build on 2026-08-07:

```text
cmake --build build -j4
ctest --test-dir build --output-on-failure -j4
64/64 passed

cmake -S . -B build-renderer-free-polish \
  -DGUI_FORMS_ENABLE_SKIA=OFF \
  -DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=OFF \
  -DGUI_FORMS_ENABLE_MACOS_HOST=OFF \
  -DGUI_FORMS_BUILD_TESTS=ON \
  -DGUI_FORMS_BUILD_GALLERY=OFF
focused renderer-free gates: 5/5 passed

cmake --build build-win64 \
  --target gui_drawing_c_api gui_forms_controls -j4
strict Win64 cross-build passed
```

The full suite includes Skia/CoreGraphics raster compilation and smoke,
renderer-boundary audits, C11 ABI behavior, AppKit lifecycle, File Manager
demoboard, canvas, raw registry patch, and GUI.Drawing color/edit tests.

## Honest limits and next gate

- Registry patching avoids a full application→registry copy, but current CPU
  renderer caches still rebuild a complete cached image after resource
  generation changes. No large-canvas performance claim is made.
- `ImageRegistry` default limits remain 4096×4096/64 MiB per decoded resource,
  while GUI.Drawing permits a separately bounded 256 MiB bitmap. Paint must
  choose and measure an admitted limit rather than silently widening it.
- Pressure-bearing pointer samples, selection/overlay primitives, lazy typed
  image transfer, and complete CPU path/text/image parity remain open.
- Color conversions name sRGB and D65. ICC loading/embedding, wide-gamut
  documents, perceptual mapping, and profile-backed CMYK remain open.
- A detailed owned Color dialog is not implemented by this slice. Its reusable
  field, numeric/hex/OKLCH editors, swatches, modal cancel/no-mutation session,
  keyboard/accessibility path, and modeless owner-close behavior are M12-P27.
