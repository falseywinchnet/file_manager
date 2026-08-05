# GUI.Forms backend and behavior decision map

Status: **architecture interview; no backend selected**.

The original questions below are preserved as the first shortlist. The
architect's resolutions, expanded alternatives, gains, losses, and new
subquestions are now authoritative in
[`GUI_FORMS_INTERVIEW_LEDGER.md`](GUI_FORMS_INTERVIEW_LEDGER.md). In particular,
GF002 is resolved as a permanent CPU-only rule; the renderer and host candidate
sets are broader than the initial shortlist.

The purpose of this document is to obtain a sufficiently precise framework
contract before independent sibling tasks build incompatible demonstrations.
The backend must remain replaceable; GUI.Forms is the retained control/runtime
contract, DML schema, ABI, and behavior—not the selected raster library.

## 1. Recommended proving architecture

```text
DML + imperative C++ + generated C# surface
                    |
             versioned compiler IR
                    |
        stable C ABI / generational handles
                    |
        retained semantic presentation tree
         /          |             \
 layout cache   accessibility   damage/display chunks
         \          |             /
             platform host adapter
             renderer adapter(s)
```

Recommended initial interpretation:

- DML is primarily build-time input and an inspectable artifact, not a live
  document database through which every runtime property write must travel.
- Imperative calls mutate the retained tree directly and synchronously.
- Stable DML node IDs permit tooling, testing, state preservation, and later
  development hot reload.
- Application objects own semantic application state. GUI.Forms owns only the
  retained state necessary to behave like a real GUI: bounds, focus, capture,
  hover/press, text selection/composition, scroll, accessibility identity,
  layout caches, display chunks, and dirtiness.
- The UI thread commits coalesced display chunks; a renderer may consume them on
  another thread without taking ownership of application state.

## 2. Platform host

### Candidate A — first-class native adapters (**recommended**)

- macOS: AppKit window/event/drag-drop/IME/accessibility adapter;
- Windows: Win32 windowing, TSF/IME, OLE drag-drop, and UI Automation adapter;
- Linux: Wayland and X11 host adapters with input-method and AT-SPI bridges.

GUI.Forms draws controls itself but uses the host OS for windows, displays,
clipboard, cursors, menus where required, accessibility publication, IME,
drag/drop, native file dialogs, and lifecycle.

This costs more adapter work but avoids discovering late that a game-oriented
window abstraction erased the hooks required for a desktop control framework.

### Candidate B — SDL3 as permanent host

SDL3 now provides released cross-platform low-level window, keyboard, mouse,
and graphics access. It is an excellent bring-up or test host. Its public remit
does not by itself supply the full desktop accessibility, semantic windowing,
native menu/dialog, or automation contract GUI.Forms requires. Selecting it as
the permanent host would still require substantial platform escape hatches.

### Candidate C — SDL3 bring-up plus native production hosts

Useful only if the abstraction boundary is proven identical. Otherwise the easy
prototype becomes a fourth platform which tests the wrong things.

**GF001:** Native hosts from day one, or SDL3 as a deliberately disposable
bring-up host?

## 3. Renderer seam

The retained engine should emit a small versioned drawing vocabulary:

- clipped fills and strokes;
- paths;
- images and nine-patch-like material assets;
- shaped glyph runs;
- shadows;
- compositing groups;
- cached layers/tiles;
- and explicit dirty rectangles.

It should not expose Skia, Blend2D, CoreGraphics, Direct2D, Metal, or Vulkan
objects in public control APIs.

### R1 — Blend2D CPU reference backend

Blend2D is a C++ 2D vector raster engine with C and C++ APIs, JIT-optimized CPU
pipelines, multithreaded rendering, paths, text, gradients, patterns, and
Porter-Duff blending. It is comparatively compact and well suited to a classic
2D UI rendered only in damaged tiles.

Risks: the GUI framework still owns text shaping, bundled font-pack selection,
platform bitmap presentation, and unusual effects; JIT policy may be
undesirable in hardened/sandboxed builds.

### R2 — Skia CPU reference backend

Skia supplies mature CPU raster surfaces, text, geometry, images, color
management, filters, and multiple platforms. GUI.Forms would build and expose
only the CPU path.

Risks: dependency and build weight, API/backend complexity, and the possibility
that GUI.Forms quietly becomes a thin Skia scene wrapper rather than owning a
small stable rendering contract.

### R3 — three platform CPU renderers

CoreGraphics bitmap contexts, Windows bitmap/GDI plus DirectWrite, and Cairo
image surfaces can provide excellent host integration without giving GUI.Forms
a GPU capability.

Risks: three sets of visual bugs and difficult screenshot equivalence. The
renderer seam must be extremely exact for this not to fracture the house style.

### R4 — custom rasterizer

Maximum control and minimum external policy, but an enormous diversion before
text, paths, clipping, filters, images, and color management are correct.

### Superseded initial experiment

The initial three-way experiment was:

1. Blend2D CPU tiles uploaded by the macOS host;
2. Skia CPU raster;
3. a minimal CoreGraphics implementation.

Measure cold start, binary/dependency footprint, idle memory, redraw time for
1%, 10%, and 100% damage, text quality, pixel snapping, and implementation
complexity. Do not select by synthetic shape throughput alone.

**GF002 — resolved:** GUI.Forms is permanently CPU-only. The host operating
system may composite its completed bitmap however it chooses, but GUI.Forms has
no GPU API, capability, resource, or fallback distinction.

**GF003:** May the proving spike compare Blend2D, Skia, and CoreGraphics, with
the loser discarded completely?

## 4. Text system

**GIVEN owner direction:** HarfBuzz is the common renderer-neutral shaper and
FreeType is the common loader/hinter/rasterizer. HarfBuzz converts Unicode runs,
with direction/script/language, into positioned glyphs and supports cached
shape plans. Font-pack selection, shaping, and raster profiles remain separate
internal services even though their implementation families are selected.

Recommended split:

- public ABI strings: length-delimited UTF-8;
- text controls: UTF-8 storage plus explicit grapheme/cluster/line indices;
- shaping: HarfBuzz behind a GUI.Forms service interface;
- font selection/fallback: bounded bundled role/script packs;
- glyph rasterization: FreeType behind the renderer-neutral glyph-run seam;
- IME: platform adapter addressing stable text ranges in the custom editor;
- no invisible native text control overlay.

**GF004 — resolved:** bundle control, body, and required fallback fonts and
target exact layout geometry for a pinned font-pack/shaping/raster profile.
Missing coverage is an explicit pack fault, not silent host-font substitution.

**GF005:** Must the first text editor support bidirectional text, grapheme-safe
selection, dead keys, and multi-stage CJK composition before any dogfood demo is
called valid?

## 5. Retention, damage, and scheduling

Recommended behavior:

1. setters mutate state synchronously on the UI thread;
2. each setter marks typed dirtiness (`measure`, `arrange`, `paint`, `hit-test`,
   `text`, `accessibility`);
3. dependency propagation is bounded by declared control metadata;
4. nestable update scopes coalesce work at the outermost exit;
5. layout runs only for affected constraints/subtrees;
6. paint rebuilds only invalid display chunks;
7. the compositor presents only damaged rectangles/layers;
8. high-frequency custom surfaces may own an isolated CPU bitmap layer so they
   do not invalidate the surrounding control tree.

There is no perpetual frame loop. A clock exists only while a declared effect,
caret, progress object, or high-rate surface is active.

**GF006:** Should a property getter immediately observe final layout after a
setter, WinForms-style where required, or may layout remain deferred until an
explicit `PerformLayout`/transaction boundary?

**GF007:** When invalidation metadata is missing, should development builds fail
and production conservatively repaint the subtree, or should both fail?

## 6. Layout

Recommendation: several explicit containers over one universal constraint or
flex language.

- `AnchorLayout` / WinForms-compatible dock-and-anchor behavior;
- `GridLayout` / table rows, columns, spans, absolute/auto/weighted sizing;
- `FlowLayout` / wrapping ordered children;
- `StackLayout` / simple vertical or horizontal composition;
- `SplitLayout` / physical splitter constraints and collapse;
- `CanvasLayout` / explicit coordinates for custom instruments;
- control-specific virtual layout for trees, lists, grids, and text.

All share cached measure/arrange primitives, preferred/min/max size, margin,
padding, baseline, DPI scale, and pixel snapping. They do not pretend to be one
CSS-derived algebra.

**GF008:** Confirm explicit layout families rather than a universal layout
solver.

**GF009:** Should GUI.Forms reproduce WinForms dock-order quirks for compatibility
bindings, while native DML uses a deterministic declared order?

## 7. Event and control behavior

Recommended baseline:

- one UI-thread event domain per application;
- synchronous property and event order;
- queued `BeginInvoke`, blocking `Invoke` only across threads;
- stable focus, capture, validation, mnemonic, tab, default/cancel, and modal
  rules;
- input normalization at the platform adapter, followed by capture/target/bubble
  routing internally;
- high-level Forms events synthesized in a documented order from internal input;
- disposal immediately detaches input, timers, accessibility, and ownership;
- exceptions never cross the C ABI.

**GF010:** Should capture/target/bubble be public to native GUI.Forms authors,
or remain internal while the public API exposes WinForms-like events?

**GF011:** Is strict WinForms event ordering a compatibility-mode promise only,
allowing the native API to repair known bad orderings?

## 8. Accessibility and automation

Accessibility is a parallel retained semantic tree with stable identities, not
an afterthought inferred from pixels. Each control declares role, name,
description, value, state, actions/patterns, relationships, bounds, focus, and
text ranges. Platform adapters publish this through NSAccessibility, Windows UI
Automation, and AT-SPI.

The same tree powers GUI test automation and the AI-readable introspection API.
That makes accessibility correctness part of the framework's behavior contract,
not a platform-specific bolt-on.

**GF012 — resolved:** Controls do not require accessibility metadata. An
optional secondary semantic-hook graph supplies accessibility, tooltips,
guidance overlays, testing, and introspection without becoming part of the
WinForms-compatible construction contract.

**GF013:** May automated tests address stable DML IDs directly in addition to
the platform accessibility identifier?

## 9. DML and generated Forms

Recommended contract:

- textual, versioned, human-readable source schema;
- compiler validates control/property/event/layout/style/accessibility effects;
- output contains a compact binary form description plus generated strongly
  typed C++ or C# handles;
- runtime imperative mutation bypasses text parsing and does not continuously
  serialize back into DML;
- development hot reload is a separate tool which diffs stable IDs at a
  transaction boundary;
- a future visual designer round-trips DML, not generated C++.

**GF014:** Is DML authoritative and round-trippable, with generated code treated
as disposable output?

**GF015:** Should production binaries contain inspectable DML metadata, or only
the compiled schema and accessibility/debug name table?

## 10. ABI and language bindings

Recommended authoritative seam:

- versioned C function tables;
- opaque generational handles, never raw C++ pointers;
- length-delimited UTF-8 strings and plain spans;
- explicit borrowed/owned lifetime annotations;
- caller-supplied allocator option for cross-module data;
- result codes plus structured error objects;
- callback plus context-pointer events;
- no exceptions, RTTI objects, STL containers, or compiler-specific layout
  crossing the ABI;
- thin RAII C++ wrapper and generated C# P/Invoke/source-generator layer.

The C# binding depends on a native GUI.Forms library. GUI.Forms never depends on
.NET.

**GF016:** May the native API intentionally improve names and type safety while
the C# compatibility facade preserves familiar WinForms/Modern.Forms shapes?

**GF017:** Should handle ownership primarily follow the control tree, with an
explicit external reference only for objects that outlive/reparent outside it?

## 11. Style engine

Required primitives now include:

- relational foreground/background pairs compiled in OKLCH;
- Office-derived seed hue families;
- a global material light direction;
- raised, inset, etched, default, disabled, selected, active, and inactive edge
  recipes;
- depth classes independent of color roles;
- authored scheme bundles with a miniature UI preview;
- explicit color-profile/gamut mapping;
- and high-contrast/non-color semantic fallbacks.

Styles resolve to compact immutable records referenced by presentation nodes.
A theme switch replaces style generations inside one nestable update scope; it
does not set hundreds of unrelated properties while painting remains live.

**GF018:** Should applications be allowed to define entirely new material edge
recipes, or only choose and recolor GUI.Forms-authored recipes?

**GF019:** Should DML permit raw colors in ordinary controls, or require named
pair roles except inside custom drawing code?

## 12. First independent demonstrations

Once the decisions above are stable, sibling tasks can build independent
falsifiers against the same ABI/schema commit:

1. **Material laboratory:** every edge/depth/state recipe, Office-derived pair
   families, theme diorama, DPI, and pixel snapping.
2. **Text/accessibility laboratory:** custom editor, IME, bidi, selection,
   VoiceOver/UIA exposure, keyboard-only traversal, and automated addressing.
3. **retired compatibility specimen laboratory:** retained high-rate bitmap, invalidation transactions,
   dock/anchor/table layout, custom slider, timer, and plugin-subtree lifecycle.
4. **File Manager shell:** window, split/tree/virtual list/preview, breadcrumb,
   menu, search field, and status line.
5. **C# bridge laboratory:** generated facade consuming the same native library
   without any native dependency on .NET.

These demonstrations should live in a dedicated GUI.Forms repository or
worktree only after the contract is pinned. Otherwise parallel work will encode
different answers to event order, ownership, DML lifetime, layout, and rendering
and appear to make progress while actually increasing reconciliation cost.

**GF020:** Separate repository from the outset, or a `gui_forms/` subtree until
the first ABI/schema tag?
