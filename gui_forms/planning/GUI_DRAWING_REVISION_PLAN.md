# GUI.Drawing revision plan

Status: **GIVEN scope, OBSERVED call inventory, CANDIDATE internal design**.
Date: 2026-08-05.

## 1. Revision outcome

**GIVEN:** GUI.Forms will not treat .NET `System.Drawing` as its drawing
implementation. The portable drawing feature set is owned by **GUI.Drawing**.
The managed `System.Drawing` compatibility assembly maps the admitted calls to
GUI.Drawing through the versioned native ABI. File Manager's native engine does
not acquire a .NET dependency.

**OBSERVED:** the current M11d unchanged-specimen demonstration still executes
owner paint with the installed .NET 10 `System.Drawing` implementation. It
creates a managed bitmap, calls `Graphics.FromImage`, runs `OnPaint`, encodes a
PNG, and gives that raster to the retained native tree. This is a useful
compatibility scaffold and an oracle, but it is not GUI.Drawing support.

**OBSERVED:** GUI.Forms itself already owns a much smaller renderer vocabulary:
save/restore, translation, rectangular clipping, rectangle fill/stroke, line,
UTF-8 text, and image presentation. Skia, CoreGraphics, and the bounded Win32
presenter implement variants of that vocabulary. These operations are
`experimental`; they do not close the captured `System.Drawing` surface.

The implementation ledger therefore uses four noninterchangeable states:

| State | Meaning |
|---|---|
| `native` | Implemented by GUI.Drawing/core and covered by semantic plus renderer tests |
| `facade` | Managed identity and marshaling exist, but the behavior reaches a named native implementation |
| `passthrough` | The laboratory delegates behavior to the installed .NET/GDI+ service |
| `missing` | No claimed implementation; a placeholder identity does not count |

No aggregate facade-verifier score may be presented as native drawing
completeness.

## 2. Authoritative captured surface

The pinned `retired compatibility specimen-x64-next (6)` build 1922 catalogue contains **353**
`System.Drawing*` API rows across **58** referenced types. **307** rows across
**34** types have static IL operands, totaling **5,861** observed operands;
**46** rows are metadata-only and remain deferred. The disposition catalogue
now assigns all 353 rows to `gui_drawing_compat_facade` rather than the .NET
runtime.

Static IL evidence establishes required callable surface, not that every path
runs during startup, nor that current output is behaviorally correct. Dynamic
traces and differential fixtures remain required.

### 2.1 Required families

| ID | Family | Required captured rows | Current honest state | Required center |
|---|---|---:|---|---|
| D0 | Value geometry and color | 130 | mixed `native` geometry/color subset; compatibility calls otherwise supplied by .NET | integer/float point, size, rectangle, containment/intersection/inflate/offset, ARGB/named colors, system-role colors, equality and conversions |
| D1 | Resource lifetime and stock objects | 38 | `passthrough` for managed paint | deterministic `Brush`, `SolidBrush`, `Pen`, `Font`, `FontFamily`, `StringFormat`, stock pens/brushes/fonts; clone/dispose and use-after-dispose rules |
| D2 | Graphics target, state, clip, transform and quality | 25 | small native command subset; `passthrough` for managed `Graphics` | image/control targets, save/restore, transform, translate, clip, visibility, flush, measurement, compositing/interpolation/pixel-offset/smoothing policy |
| D3 | Primitive drawing and text | 33 | rectangle/line/text/image subset is `native`; overload and semantic closure is `missing` | clear; line/polyline; rectangle/ellipse/polygon/path fill/stroke; string and icon/image drawing |
| D4 | Paths, matrices and regions | 31 | `missing` | figures, arc/ellipse/line/rectangle/text outlines, append/clone/reset/transform/bounds/hit test, union/exclude |
| D5 | Gradient and hatch paints | 15 | `missing` | linear/path gradients, interpolation colors, blend/wrap behavior, hatch foreground/background |
| D6 | Images, bitmap storage and codecs | 24 | PNG resource presentation is `native`; managed bitmap mutation is `passthrough` | dimensions/pixel format, clone, file/stream load, save, thumbnail, pixel read, transparency, pixel locking, native bitmap bridge |
| D7 | Image adjustment | 11 | `missing` | color matrix alpha, color remap table, clone and bounded application during image draw |
| D8 | Compatibility-native surface leases | 7 of D2/D6 | `passthrough` | `FromHdc`, `FromHdcInternal`, `FromHwnd`, `GetHdc`, `ReleaseHdc`, `GetHbitmap`, `FromHbitmap` behind an explicit platform capability |

The family counts partition the 307 required rows. D8 is a cross-cutting subset
of D2/D6 and is not added again to the total.

### 2.2 Required `Graphics` operations

The 58 captured `System.Drawing.Graphics` rows are admitted:

- target/lifetime: `FromImage`, `FromHdc`, `FromHdcInternal`, `FromHwnd`,
  `GetHdc`, `ReleaseHdc`, `Dispose`, `Flush`;
- state: `Save`, `Restore`, `SetClip(Rectangle)`, `TranslateTransform`,
  `ClipBounds`, `Transform`, `InterpolationMode`, `SmoothingMode`, and setters
  for transform, compositing mode/quality, interpolation, pixel offset, and
  smoothing;
- queries: `IsVisible(PointF)` and both captured `MeasureString` overloads;
- raster/vector operations: `Clear`, `DrawEllipse`, `DrawIcon`, six captured
  `DrawImage` shapes, `DrawImageUnscaled`, four `DrawLine` shapes, two
  `DrawLines` shapes, `DrawPath`, two `DrawRectangle` shapes, five `DrawString`
  shapes, `FillEllipse`, `FillPath`, two `FillPolygon` shapes, four
  `FillRectangle` shapes, and `FillRectangles`.

The catalogue is authoritative for exact signatures. A convenience overload is
not separately implemented when it can normalize without observable loss into
one canonical operation; its facade identity and argument/exception behavior
still receive a test.

### 2.3 Other required objects

- geometry/color: `Color`, `ColorTranslator`, `Point`, `PointF`, `Rectangle`,
  `RectangleF`, `Size`, `SizeF`, system colors, and the observed color array;
- drawing resources: `Brush`, `SolidBrush`, stock brushes, `Pen`, stock pens,
  `Font`, `FontFamily`, system fonts, `StringFormat`, and `Icon`;
- vector: `GraphicsPath`, `Matrix`, `Region`, `ColorBlend`, `HatchBrush`,
  `LinearGradientBrush`, and `PathGradientBrush`;
- raster: `Image`, `Bitmap`, `BitmapData`, `ImageAttributes`, `ColorMap`, and
  `ColorMatrix`.

Enum and delegate types that appear only as metadata rows remain visible in the
353-row owner set. Signature closure may require emitting them before their
metadata-only row is promoted to behavioral work.

### 2.4 Broader File Manager oversight surface

The retired compatibility specimen catalogue is a floor, not the ceiling of GUI.Drawing. File Manager has
**GIVEN** needs for small/icon views, high-color object icons, custom title bars,
selection/depth materials, an optional command shelf, indexed-location
thumbnails, isolated preview surfaces, and consistent rendering across macOS,
Windows, and Linux. The following are **CANDIDATE** oversight families until a
named File Manager workload or decision record promotes individual operations:

| ID | Candidate family | Likely operations to keep visible | File Manager evidence/workload |
|---|---|---|---|
| FMD0 | Complete vector/path construction | cubic Beziers, arcs/pies, polygons, closed/open curves, rounded shapes, fill rules, flatten/reverse/widen, path iteration/data | scalable high-color object icons, focus/selection geometry, title fresco and unusual chrome |
| FMD1 | Complete transform/clip/region algebra | rotate/scale/multiply/reset, point transforms, intersect/exclude/union/xor/complement, nonrectangular clip, device/logical conversion | nested scrolling, clipped previews, transformed icon assets, damage correctness |
| FMD2 | Stroke and paint vocabulary | joins, caps, miters, dash offset/pattern, alignment, texture/image paints, multi-stop gradients, repeat/mirror, opacity masks | crisp small icons, bevels, selection/focus, ceremonial materials without backend-specific styling |
| FMD3 | Compositing and effects | declared blend modes, isolated layers, masks, shadows, bounded blur/color filters | depth grammar, selection glow, preview polish; every effect needs a damaged-area and CPU budget |
| FMD4 | Image formats and metadata | secure decode/encode registry, ICO/ICNS and SVG/vector assets, PNG/JPEG/WebP candidates, EXIF orientation, ICC profile, alpha/premultiplication, multi-frame policy | handler/icon registry, thumbnails, preview surfaces, cross-platform icon variants |
| FMD5 | Thumbnail pipeline primitives | aspect-fit/fill, crop, high-quality sampling, orientation/color normalization, deterministic cache keys, cancellation and bounded incremental decode | thumbnails only for admitted indexed roots; drawing is a service, not thumbnail authority or storage policy |
| FMD6 | Text/glyph drawing integration | positioned glyph runs, outlines, fallback spans, cluster-safe hit geometry, decoration, ellipsis and baseline metrics | filenames, breadcrumbs, fields, list/icon views; shaping remains owned by the shared text subsystem |
| FMD7 | Pixel/surface operations | mapped pixel spans, format conversion, row copy, masks, readback, immutable snapshots, tiled/partial updates | icons, isolated previews, waterfall/active surfaces, renderer diagnostics |
| FMD8 | Recording, inspection and export | stable command trace, bounds/cost inspection, semantic pixel probes, PNG diagnostic export, optional vector debug export | deterministic automation, crash triage, theme QA and renderer comparison |

Likely useful `System.Drawing` compatibility calls not present in the retired compatibility specimen floor
remain visible within those families: `DrawArc`, `DrawBezier(s)`, `DrawCurve`,
`DrawClosedCurve`, `DrawPie`, `DrawPolygon`, `FillClosedCurve`, `FillPie`,
`FillRegion`; complete transform and clip operations; complete `GraphicsPath`
curve/polygon/pie/flatten/widen/fill-mode operations; complete region algebra;
pen caps/joins/miters/alignment; texture brushes; image palette/property/frame
metadata; and additional measured text/layout calls. This is an oversight list,
not an implicit promise to reproduce all of GDI+.

Printing, metafile/EMF playback, arbitrary desktop capture, device-context
escape behavior, and design-time drawing editors stay excluded or require a
separate admission record. SVG/ICNS, color management, effects, and thumbnail
helpers are GUI.Drawing-native candidates even though they are not naturally
modeled as `System.Drawing` calls.

## 3. Portable semantic shape

The names below are **CANDIDATE** until the normal decision protocol accepts
them. They define the separation to test, not a frozen public API.

```text
GUI.Forms Paint/owner-draw callback
        |
        v
GUI.Drawing recording Graphics context ----> immutable display chunk
        |                                             |
        +--> GUI.Drawing ImageSurface mutation        v
        |                                      private renderer
        +--> platform NativeSurfaceLease              |
                                              Skia / other adapter
```

- Imperative drawing calls record into a bounded command list. A control paint
  callback contributes a retained display chunk; it does not establish an
  immediate-mode window architecture.
- `Graphics.FromImage` targets a GUI.Drawing-owned CPU image surface with
  explicit pixel format, dimensions, stride, generation, and lifetime.
- control-paint contexts are leases. They become invalid when the callback
  returns; recorded resources remain strongly owned by the resulting chunk.
- state is a deterministic stack. Underflow, disposed resources, non-finite
  geometry, excess path complexity, excessive save depth, and oversized image
  arithmetic have explicit errors and limits.
- coordinates remain logical and platform-neutral until rasterization. Pixel
  snapping is an operation policy, not a hidden host choice.
- text measurement and drawing use the shared GUI.Forms text/font service so
  layout and paint do not disagree. Compatibility metrics receive measured
  tolerances rather than a claim of pixel-identical GDI+ output.
- Skia types never cross the GUI.Drawing public contract. Skia extensions may
  exist in a private/capability-scoped package.
- HDC/HWND/HBITMAP calls are compatibility platform extensions represented
  internally by an opaque surface lease. They do not put Win32 handles in the
  portable GUI.Drawing API and they do not make Win32 the semantic reference.

## 4. Deliberate boundaries

- **GIVEN excluded:** `System.Drawing.Printing`, print preview/page setup,
  ActiveX, browser hosting, obsolete Forms families, and design-time
  `System.Drawing.Design` services.
- **GIVEN preserved:** DML remains a retained description path. GUI.Drawing
  does not turn DML or GUI.Forms into an immediate-mode UI.
- **GIVEN:** the managed compatibility facade may preserve captured
  `System.Drawing` names. The first-party native API uses GUI.Drawing names and
  need not reproduce GDI+ implementation accidents.
- **CANDIDATE codec boundary:** implement the bounded formats exercised by the
  authoritative specimen and gallery. A dynamic trace must identify format,
  dimensions, and pixel format without retaining private image bytes. PNG is
  already admitted; other codecs require their own dependency/security record.
- **CANDIDATE foreign-handle boundary:** Windows/Wine support is required for
  captured native-surface calls originating from GUI.Forms or a GUI.Drawing
  bitmap. Arbitrary foreign HDC/HWND behavior remains a separately tested
  compatibility capability, not an assumption.

## 5. Implementation stages

### M11e — GUI.Drawing semantic core and ABI

Status: **MEASURED COMPLETE for the proving exit; compatibility rows remain
separately open.** See `../experiments/M11E_GUI_DRAWING_CORE_AND_ABI.md`.

1. Define independent value/resource/state contracts and error/lifetime rules.
2. Add a renderer-free command recorder and deterministic textual trace.
3. Extend the display vocabulary for ellipse, polygon/path, richer stroke/fill,
   transform, compositing, and image attributes without importing Skia types.
4. Add C ABI handles for images, graphics contexts, resources, paths, state
   tokens, and command submission. Prove stale-handle and callback teardown.
5. Implement D0/D1 and the non-native-surface parts of D2 first.

Exit: C++ and C consumers produce byte-identical drawing traces; resources are
deterministically disposed; wrong-thread and stale-context operations fail
without corruption; the renderer-free build remains clean.

**MEASURED:** the exit passes. Independent C++20 and C11 consumers share the
same `gui.drawing.trace/v1` golden; the raw ABI proves generational stale
handles, wrong kinds, idempotent disposal, and wrong-thread rejection. The
Skia-disabled/AppKit-disabled suite passes 24/24 and the renderer-free
drawing-specific subset passes 5/5. Logical
image references intentionally contain no pixel storage; that remains M11f.
No captured family is marked row-complete by this substrate result.
The same three consumers cross-compile to PE64 and pass under Wine; the
warning-as-error ASan/UBSan lane passes 5/5. The existing facade regression
remains 797/797 and passes its host-.NET/Wine managed-loop smoke.

### M11f — rendering, raster storage and compatibility facade

Status: **MEASURED IN PROGRESS.** D3–D7, owned bitmaps/PNG, the 307/307
generated facade, and D8 handle/lease semantics are implemented. The Windows
x64 private Skia raster module and nonempty Wine surface-raster round trip
remain the exit gate. See
`../experiments/M11F_RENDERING_RASTER_STORAGE_AND_DRAWING_FACADE.md`.

1. Implement D3–D7 through the selected private raster adapter.
2. Implement owned CPU image storage, pixel locking, alpha/premultiplication,
   codec bounds, clone, transparency, thumbnail, and color adjustments.
3. Generate the `System.Drawing` facade from the same 353-row catalogue, mapping
   all 307 required rows and signature-closure types to GUI.Drawing.
4. Add the D8 Windows/Wine surface-lease adapter and explicit unsupported
   behavior for unavailable foreign handles on other hosts.
5. Keep the current .NET service only as a differential oracle and temporary
   feature flag during the cutover.

Exit: 307/307 required drawing identities resolve; every row has a native,
normalized, or explicit platform-extension implementation; no production paint
path loads Microsoft's drawing implementation or GDI+ backend.

**MEASURED:** the compiled verifier reports 1,104/1,104 combined required rows
(797 Forms plus 307 Drawing). Host native-raster/facade smokes, 28/28
Skia-enabled native tests, and 24/24 renderer-free tests pass. PE64 C and .NET
10 facade probes pass HBITMAP, HDC, HWND, presentation, tokenized release, and
stale-token behavior under Wine. **OPEN:** the unchanged Wine specimen cannot
use the new raster path until `gui_drawing_raster0.dll` is linked against a
Windows x64 CPU-only Skia build; therefore M11f is not marked complete and the
zero-passthrough counter is not claimed.

### M11g — differential closure and retired compatibility specimen cutover

1. Compare command traces, geometry results, text metrics, path bounds/hit
   tests, resource exceptions, and raster fixtures against .NET 10
   `System.Drawing` under Wine.
2. Add dynamic counters for the unchanged specimen: type/member, call count,
   image format/dimensions, path complexity, target kind, and paint timing only.
3. Replace `__RenderManagedPaint`'s managed bitmap/PNG round trip with a native
   GUI.Drawing recording context and retained chunks/image surfaces.
4. Run startup, interaction, close, source configuration, and active spectrum/
   waterfall workloads with passthrough disabled.

Exit: the unchanged specimen visibly populates and stays open with zero drawing
passthrough calls; all exercised operations pass their behavior/raster oracle or
carry a reviewed deviation; sustained paint has a recorded baseline.

### M11h and later — application closure

After M11g, return to the remaining Forms blockers in this order:

1. secondary windows, popup/dropdown composition, and modal focus;
2. keyboard editing and source selection;
3. active receiver streaming, damage isolation, frame pacing, and soak;
4. accessibility publication and reliable GUI instrumentation;
5. ABI freeze, packaging, physical Windows dogfood, then Linux host dogfood.

Multi-window work may proceed before the full D3–D7 breadth only if a narrow
GUI.Drawing slice can populate the source dialog without falling back to the
managed raster path. The no-passthrough M11g exit remains mandatory.

## 6. Verification and accounting

Every captured row receives a record with identity, normalized native
operation, status, tests, platform qualification, and any deliberate deviation.
Required test layers are:

1. value and property tests for geometry, color, transforms, regions and paths;
2. deterministic headless command traces and resource-lifetime faults;
3. raster semantic probes plus scale/profile-qualified golden fixtures;
4. differential .NET 10/Wine tests for captured overloads and exceptions;
5. hostile image/path/stride/save-depth limits and fuzzing;
6. Windows/Wine native-surface lease tests and honest non-Windows results;
7. unchanged-specimen dynamic coverage and sustained active-radio timing.

Progress is reported as separate fractions: interface rows, native semantic
rows, raster-conformant rows, dynamically exercised rows, and passthrough rows.
Only the last counter reaching zero closes GUI.Drawing compatibility.

The File Manager oversight ledger reports a sixth fraction separately:
candidate FMD operations promoted by an explicit workload. Unpromoted oversight
rows do not dilute the 307-row retired compatibility specimen closure number and cannot be advertised as
supported.

### Revision-round verification — 2026-08-05

- disposition regeneration: 1,952/1,952 rows classified, zero unclassified;
  negative capture-pin rejection still passes;
- drawing ownership: 353 rows owned by `gui_drawing_compat_facade`, 307 required,
  46 deferred, 34 required types, 5,861 captured IL operands;
- Forms isolation: 797/797 required Forms rows still resolve and 191 closure
  types emit; generated projects build with zero warnings/errors;
- native ABI smoke: C11 and C++ tests pass;
- managed loop: headless host and Windows/Wine smoke pass with click, dispatch,
  close, fault containment, thread exception, thread exit, and stale-disposal
  checks;
- the generator now permits the temporary managed drawing path only when the
  runtime reports Windows. Non-Windows headless tests use native control kinds
  and do not attempt to execute a copied Windows reference assembly. This is a
  packaging/test correction, not a GUI.Drawing implementation claim.
