# GUI.Forms architecture interview ledger

Status: **grand-architect constraints and candidates; not an ADR**. This ledger
supersedes simple A/B/C polling. Every open choice must state the capability
gained, the cost accepted, the credible alternatives, and which future choices
it closes. Renderer and host selections remain CANDIDATE/HYPOTHESIS until fair
spikes produce MEASURED evidence and a numbered ADR records owner approval.

## Recorded architect constraints

### GF002 — CPU-only rendering

**GIVEN:** GUI.Forms has no GPU capability. This is not merely a CPU fallback
or an initial implementation order. The drawing ABI, caches, effects, and
control contracts must not contain GPU devices, command buffers, textures,
shaders, swap chains, or GPU-resource lifetime.

The operating system may ultimately composite the completed window bitmap using
hardware; that is outside GUI.Forms and does not make the framework GPU-aware.

What this gains:

- deterministic behavior on old machines, remote sessions, virtual machines,
  sandboxes, and systems with broken or absent graphics acceleration;
- no driver-dependent resource loss, shader compilation, graphics API matrix,
  or duplicated CPU/GPU rendering semantics;
- a renderer optimized around retained state and small damaged regions instead
  of perpetual full-window frames;
- simpler capture, screenshot testing, headless rendering, and crash analysis;
- and a much smaller public and internal capability surface.

What this costs:

- large translucent layers, blur, scaling, and full-window animation can become
  expensive and must be bounded deliberately;
- high-rate scientific plots require carefully isolated CPU surfaces, dirty
  bands, decimation, and efficient blitting;
- fashionable effects cannot be added casually;
- very high-DPI full-window redraws must remain exceptional.

A Godot prototype is compatible with this decision only as a disposable visual
or interaction mock-up. It proves composition, proportions, motion taste, and
workflow. It is not evidence for GUI.Forms hosting, rendering performance,
damage behavior, text, accessibility, or ABI design.

### GF004 — typography split

**GIVEN owner revision:** product UI text does not rely on host `system-ui`.
HarfBuzz is the common shaper and FreeType is the common glyph loader, hinter,
and rasterizer. Controls use the bundled **Portsmouth Rapids** font. Portsmouth
Rapids is Latin-only and remains a face, not a script engine, symbol grammar,
control notation, or text-rendering procedure.

GUI.Forms treats Portsmouth as the preferred face for declared control-label
roles. A separately selected bundled Tahoma/Calibri-like humanist face serves
content/body roles. Uncovered clusters resolve through bounded bundled fallback
packs. Layout records name a typography role and font-pack metric generation;
fallback may not turn one missing cluster into a different control-wide style.
Exact font bytes, coverage, Portsmouth production rights, body-face selection,
and raster profiles remain gated in M4/M9.

### GF005 — staged text correctness

**GIVEN:** design the text architecture for full grapheme-safe, bidirectional,
dead-key, accessible-range, and multi-stage IME behavior, but reach a useful
Latin/system-text milestone before that entire contract is finished.

The first milestone may be called a development milestone, not a complete text
system. It must not encode Latin-only assumptions in indices, selection ranges,
undo records, layout caches, or the ABI. Storage and APIs use stable Unicode
text/cluster concepts from the beginning even while some shaping and IME tests
remain incomplete.

### GF007 — undeclared invalidation

**GIVEN:** development builds fail at the mutation that lacks declared
invalidation effects. Production builds conservatively invalidate the affected
subtree and record a diagnostic rather than corrupting the screen.

### GF008 — two-scale layout

**GIVEN:** application/window composition uses a GUI.Forms-owned flex
algorithm. Sub-panels and conventional forms use explicit layout families:
anchor/dock, grid/table, flow, stack, split, canvas, and virtual control-specific
layout.

“Flex” describes the resizing behavior, not HTML, CSS, a browser layout engine,
or a promise to reproduce every CSS Flexbox edge case. The shell layer handles
major panes and adaptable workspace regions. The sub-panel layer preserves the
precise, stateful behavior expected by Forms programs and instrument panels.

### GF009–GF011 — compatibility and input behavior

- WinForms dock-order quirks exist only where the compatibility facade requires
  a close result. Native DML ordering is explicit and deterministic.
- Capture/target/bubble is an internal engine and custom-control mechanism.
  Ordinary application authors receive Forms-like events.
- GUI.Forms offers broad source and behavioral familiarity, not strict
  compatibility. Incorrect historical ordering is not preserved merely because
  WinForms exposed it.

A candidate corrected input order is recorded below for separate approval.

### GF012–GF013 — optional semantic hook system

**GIVEN:** accessibility is not mandatory metadata on Forms controls and is
not a prerequisite for instantiation. GUI.Forms instead exposes an optional
secondary semantic hook system. Applications may attach semantic descriptions,
tooltips, relationships, actions, and guidance to controls without changing the
controls' visual or layout contract.

Consumers of that optional graph can include:

- platform accessibility publication;
- hover tooltips;
- keyboard help and command discovery;
- automated tests and AI-readable inspection;
- and a toggled greaseboard/help-overview layer inspired by the Intel
  optimization-engine overview.

The help overview is composited per panel. Each panel supplies anchors and
explanations; an overlay plane draws its dimming, arrows, labels, and highlights
without rebuilding the panel. Toggling it adds or removes those overlay chunks
smoothly. Accessibility and help may share descriptors, but they remain
separate consumers: disabling one does not silently disable the other.

Stable DML IDs may be used directly by tests and inspection in addition to any
platform accessibility identifier.

### GF014–GF016 — DML and language surfaces

- DML is authoritative and round-trippable. Generated code is disposable.
- Production retains compiled schema, stable IDs, debug/semantic names, and the
  metadata needed for inspection; it need not embed source DML text.
- The native API may improve names and type safety. A generated C# facade keeps
  familiar Forms/Modern.Forms shapes. GUI.Forms never depends on .NET.

### GF017 — tree ownership plus ARC

**GIVEN:** combine tree ownership with automatic reference counting.

- A parent holds strong references to its children.
- A child holds a weak/non-owning parent link.
- A detached control dies when its final strong reference is released.
- C++ bindings express strong references through a small RAII handle.
- C# bindings use a safe generated wrapper around retain/release.
- Event subscriptions and delegates are tokenized and weak by default where a
  strong edge would create an ownership cycle.
- The C ABI still uses opaque generational handles so a stale numeric identity
  cannot become a use-after-free.

This gains forgiving reparenting and detached object construction without making
garbage collection or a managed runtime part of the framework. It costs atomic
or thread-confined retain/release bookkeeping and requires explicit cycle
discipline. If all controls remain UI-thread-affine, reference counts should be
non-atomic inside the UI domain; only cross-domain resource objects need atomic
ownership.

### GF018–GF019 — style backplane and control plane

**GIVEN:** ordinary controls use named relational color pairs and authored
GUI.Forms foreground/material recipes. Applications may additionally supply a
style-only composited backplane beneath the controls.

The two planes are:

1. **Style backplane:** fills, frescoes, procedural patterns, textures,
   illustrations, wells, and other non-interactive backfills. It may use
   transparency and application-authored material.
2. **Control plane:** GUI.Forms controls at the higher z-order. It owns hit
   testing, focus, state, event behavior, text, and ordinary control relief.

Backplane objects cannot replace a control, intercept its events, mutate its
semantic role, or impersonate plugin UI. Their damage and cache generations are
separate so changing a button does not rerasterize a full fresco and changing a
background does not reconstruct the control tree.

Raw colors are reserved for custom drawing/backplane work. Ordinary DML control
styles consume named foreground/background pair roles.

### GF020 — guarded integrated subproject

**GIVEN:** retain the specification here for another round or two, then
create GUI.Forms as a guarded subdirectory rather than a permanently independent
repository.

The intended build boundary is:

```text
repository root
  build: File Manager + GUI.Forms + integration demonstrations

gui_forms/                  guarded parent and public project boundary
  build: GUI.Forms library and its own demonstrations only
  AGENTS.md: scope, dependency, ABI, generated-file, and ownership guardrails
```

The lower build must never reach upward into File Manager sources. The upper
build may consume the lower library and combine everything. Sibling tasks begin
only after the guardrails and first schema/ABI draft agree.

## GF001 — platform hosting, properly decomposed

The host is not a window-creation helper. It is the framework's treaty with the
operating system. It must cover at least:

- application and window lifecycle;
- event-loop wakeup and queued invocation;
- monitors, scaling, coordinates, occlusion, and damaged presentation;
- keyboard, pointer, capture, cursors, and multi-click normalization;
- IME/text-input sessions and candidate-window positioning;
- clipboard and typed drag/drop;
- native file dialogs and any deliberately native menus;
- host text-raster, scale, and accessibility preference signals (font
  selection/fallback itself is owned by bundled GUI.Forms packs);
- native accessibility publication, required for File Manager even if another
  independently packaged consumer does not ship a publisher;
- power/session/display-change notifications;
- and native handles required by preview/plugin isolation.

### H0 — AppKit directly, extract portability later

The first implementation calls AppKit throughout the engine and postpones the
host seam until a Windows or Linux port begins.

**Gain:** the shortest route to one correct native macOS application; no early
abstraction is invented from guesses about platforms not yet implemented.

**Loss:** AppKit object identities, coordinate rules, scheduling, text ranges,
and lifecycle assumptions spread into controls and layout. The later port must
both discover the abstraction and remove those assumptions. This is precisely
the retrofit risk behind “build it here first, build it once.”

**Usable distillation:** implement AppKit first, but require every AppKit call to
remain inside the host adapter from the first commit. The protocol can grow from
real macOS evidence without letting the portable engine depend on AppKit.

### H1 — our narrow host protocol with native adapters

AppKit, Win32, Wayland, and X11 adapters implement a GUI.Forms-owned host
protocol. The native application loop owns scheduling; it calls into the
portable retained engine. GUI.Forms normalizes events after the OS has done the
platform-specific work.

**Gain:** no dependency controls our architecture; every important desktop hook
is reachable; AppKit can be correct first without pretending Linux already has
identical semantics; the portable core remains testable through a headless host.

**Loss:** this is the greatest amount of platform work. Ports can drift, and
each one needs a conformance/event-trace suite. Wayland and X11 are genuinely
different hosts, not one Linux checkbox.

**What it closes:** very little. The renderer, control tree, DML, and ABI remain
portable. This is the recommended direction.

### H2 — SDL3 as the permanent host

SDL3 supplies a maintained cross-platform window/input/display/event layer and
native-window escape hatches.

**Gain:** rapid bring-up, one common event vocabulary, broad platform testing,
and less initial host code.

**Loss:** GUI.Forms still needs native adapters for complete IME behavior,
typed drag/drop, accessibility, menus/dialogs, and desktop lifecycle. The result
can become two overlapping host abstractions: SDL for common behavior and our
code for everything consequential. SDL's abstractions then constrain the common
path even though we cannot delete the native path.

**What it closes:** it makes removal expensive once SDL event identities and
window ownership leak into framework behavior.

### H3 — GLFW or another smaller window shell

GLFW provides windows, scale/monitor events, keyboard and character input,
clipboard text, cursors, and path-drop callbacks, plus native handle access.

**Gain:** smaller and simpler than a complete application framework.

**Loss:** an even larger share of desktop semantics remains ours. Its character
stream is useful, but it is not the complete editable-text/IME contract needed
by a custom Forms editor. This buys less than SDL without eliminating native
work.

### H4 — adopt the platform layer of a complete GUI framework

Qt QPA is an instructive example: it spans windows, accessibility, backing
stores, clipboard, drag/drop, fonts, input contexts, native dialogs, menus, and
platform services.

**Gain:** it demonstrates the actual breadth of a mature host abstraction and
could supply much of it immediately.

**Loss:** its platform interface has no source or binary compatibility promise;
pulling it in effectively imports Qt's lifecycle, dependency, licensing/build,
and policy surface. JUCE, wxWidgets, and similar frameworks create the same
strategic problem in different proportions: we would be building GUI.Forms
inside somebody else's GUI framework.

### H5 — native top-level shell plus portable engine-owned event loop

Each platform creates a native window but GUI.Forms owns a polling/pumping loop.

**Gain:** apparently uniform scheduling and easy headless simulation.

**Loss:** macOS and other systems expect their application loop to remain the
authority. Modal sessions, menus, services, accessibility calls, drag/drop, and
IME introduce nested or asynchronous behavior that fights a foreign master
loop. This is not recommended.

### H6 — native loop plus GUI.Forms host protocol (**recommended form of H1**)

The useful portability seam is a service protocol, not a universal windowing
library. AppKit owns the first real application loop. It delivers normalized
host events and services to the retained core. A deterministic headless adapter
replays the same event structures for tests. Later hosts implement the protocol
and must pass recorded behavioral traces.

The remaining GF001 choices are therefore:

1. Are ordinary application menus GUI.Forms-drawn, with only the macOS global
   menu exported natively, or are menus native on every platform?
2. Should the native host own only top-level windows, or also child/native
   surfaces used by isolated preview providers?
3. Is a headless conformance host part of the first AppKit milestone?
4. May the Godot mock-up precede the native host spike, provided it is clearly
   labelled non-architectural?

## GF003 — CPU rendering families

There are more choices than Blend2D, Skia, and CoreGraphics. They occupy
different layers and should not all be evaluated as interchangeable monoliths.

| Family | What it gives | What it costs or fails to give | Plausible role |
|---|---|---|---|
| Blend2D | Fast C++ analytic CPU rasterization, SIMD/JIT pipelines, paths, images, gradients, compositing | JIT/sandbox policy; its text and command model can overlap ours | Complete CPU backend candidate |
| Cairo image surfaces | Mature C API, paths, glyphs, patterns, recording and platform font/surface adapters | Older architecture; dependency/performance and cross-backend variation | Conservative complete backend/reference |
| Skia CPU raster | Extremely broad, mature image/path/text/color/filter support | Large build and policy surface; easy for GUI.Forms to become a Skia wrapper | Quality/reference backend, possible production backend only if weight wins |
| AGG | High-quality, subpixel, platform-independent C++ scanline machinery with unusually inspectable algorithms | Old/dormant integration surface; not a modern complete text/image/color system | Vendored algorithm source or path raster core |
| PlutoVG | Small current MIT C vector renderer with paths, strokes, gradients, textures, fonts, and memory surfaces | Smaller ecosystem and less proven GUI-scale behavior | Compact path/asset backend candidate |
| ThorVG software | Active CPU/SIMD renderer, partial rendering, paths, composition, images, effects | Retained scene/animation model overlaps ours; GPU features must be excluded | Vector assets, frescoes, complex backplane effects |
| Pixman | Low-level CPU pixel compositing and trapezoid rasterization | Not a complete drawing or text engine | Foundation beneath our own raster core |
| Platform trio | CoreGraphics on macOS, bitmap/GDI/DirectWrite path on Windows, Cairo on Linux | Three visual implementations and screenshot drift | Native text/presentation reference or final platform-specific renderers |
| Custom GUI raster core | Exact fast paths for rectangles, bevels, lines, fills, masks, images, glyph blits, and damaged tiles | We own antialiasing, filtering, blending, color correctness, and maintenance | Permanent hot path if deliberately narrow |

The strongest fourth architecture is **not one general renderer**:

```text
GUI.Forms display chunks
  +-- specialized GUI raster core
  |     rectangles, bevels, fills, masks, images, glyph blits, tiles
  +-- replaceable path/vector module
  |     icons, frescoes, unusual shapes, complex clipping
  +-- platform text/font module
  +-- platform bitmap presenter
```

This gives the common control path a small predictable implementation while an
existing renderer handles the mathematically expensive long tail. It costs an
extra seam and requires exact compositing/color rules between modules.

The next renderer decision should choose a **portfolio to test**, not a winner.
Recommended portfolio:

1. custom rectangle/bevel/tile kernel plus PlutoVG or AGG for paths;
2. Blend2D as the performance-oriented complete backend;
3. Cairo as the conservative mature backend;
4. CoreGraphics on macOS as a platform quality reference;
5. ThorVG only for the vector/backplane lane;
6. Skia measured as a weight/quality ceiling, not presumed finalist.

The test must include control-heavy scenes, translucent backplanes, text-heavy
lists, large images, complex vector icons, scrolling damage, and SDR-style
high-rate bitmap bands. It should measure cold start, code/dependency size,
resident memory, 1/10/100-percent damage, pixel determinism, and implementation
complexity.

### Skia subset profile

**OBSERVED:** Skia is configurable rather than indivisible. Its GN build exposes
the CPU `skia` target separately from optional Ganesh and Graphite GPU systems,
PDF, SVG, Skottie, codecs, font managers, tools, tracing, and text modules such
as SkShaper, SkUnicode, and SkParagraph. SkParagraph is a separate target that
depends on the core plus SkShaper and SkUnicode.

There is no normative product called “Skia Lite.” Toolkits commonly create a
binding or adapter over a configured native Skia package—for example Avalonia's
`Avalonia.Skia` imports SkiaSharp, while JetBrains Skiko exposes a significant
part of Skia plus platform glue. GUI.Forms should not copy that API exposure.

**CANDIDATE `skia-cpu-min`:** pin one Skia revision and build a private static
library containing only what the renderer experiment needs:

- CPU raster `SkSurface`/`SkCanvas` drawing;
- paths, clips, masks, gradients, images, blending, and color conversion;
- a platform font manager only where needed to obtain/rasterize glyphs;
- positioned glyph-run drawing, while GUI.Forms owns text storage, shaping,
  fallback policy, line breaking, selection, and IME;
- selected image decoders only if they belong inside the renderer boundary.

Explicitly exclude:

- Ganesh, Graphite, GL, Vulkan, Metal, Direct3D, Dawn, and WebGPU;
- SkParagraph, Skottie, SVG DOM, PDF/XPS, CanvasKit, viewers, tools, and samples;
- unused codecs, ICU, HarfBuzz, Perfetto, and Rust dependencies when equivalent
  GUI.Forms/platform services already own the work;
- and Skia types at every public GUI.Forms, DML, C, C++, C#, and plugin seam.

The exact GN arguments are revision-specific and must be emitted from a lockfile
after the Skia commit is pinned. Current upstream configuration includes
`skia_enable_ganesh`, `skia_enable_graphite`, `skia_enable_pdf`,
`skia_enable_skottie`, `skia_enable_svg`, `skia_use_gl`, `skia_use_metal`,
`skia_use_vulkan`, codec toggles, and platform font-manager toggles.

What this gains:

- mature CPU geometry, clipping, blending, images, and color work without any
  GUI.Forms GPU capability;
- one cross-platform pixel implementation instead of three unrelated drawing
  APIs;
- a smaller initial rendering-research burden than a custom general rasterizer;
- and a clean comparison against a narrow custom control raster core.

What it costs:

- even a trimmed core remains substantial and fast-moving source;
- current upstream Skia requires a C++20-capable toolchain and says its software
  paths are materially better optimized under Clang;
- the GN/dependency build must be pinned and maintained on every platform;
- optional does not automatically mean small—only a measured linked artifact,
  resident set, cold start, and symbol/dependency inventory can establish that;
- and Skia's convenient objects must not become GUI.Forms' retained scene model.

The candidate is therefore not “use Skia.” It is “test a hermetic CPU-only Skia
artifact behind our display-chunk ABI, with a gate that can reject it without
rewriting controls.”

**GIVEN refinement:** unused GPU modules may remain in the fetched/pinned Skia
source tree so long as GUI.Forms does not compile, link, expose, initialize, or
probe them. C++20 is acceptable. The internal decoder boundary includes PNG
because rich UI and theme surfaces may be PNG; other image formats belong to
preview/resource plugins rather than the renderer. Theme, language, pack, and
configuration consequences are recorded in
[`GUI_FORMS_RESOURCES_AND_CONFIGURATION.md`](GUI_FORMS_RESOURCES_AND_CONFIGURATION.md).

## GF006 — when layout becomes observable

Property state and derived geometry are different kinds of state. A setter can
be immediately authoritative without recomputing every descendant's final
bounds. Four coherent contracts exist.

### L1 — eager layout after every relevant setter

**Gain:** simplest observation rule; geometry is current immediately; familiar
to code expecting Forms-like synchronous behavior.

**Loss:** setting five related properties may perform five layouts; transient
half-configured states emit layout events; reentrant layout becomes common;
construction and theme application need pervasive suspend/resume discipline.

### L2 — event-loop deferred layout

**Gain:** maximum automatic coalescing and naturally cheap batches.

**Loss:** geometry getters can be stale; tests depend on loop pumping; input or
accessibility can observe a different tree from application code; failure timing
becomes obscure. This resembles the disconnected declarative pipeline being
avoided.

### L3 — explicit transactions only

Mutations are deferred until `Commit`/`PerformLayout`.

**Gain:** deterministic batch cost and no hidden read expense.

**Loss:** ordinary Forms code becomes ceremony-heavy; forgetting a boundary
produces stale layout; generated and hand-authored code behave differently.

### L4 — synchronous state with a geometry read barrier (**recommended**)

- A setter changes the requested property immediately and marks typed layout
  dirtiness.
- Outside an update scope, the engine may eagerly run a cheap local layout.
- Outside an update scope, reading arranged geometry, hit-testing, painting,
  publishing semantic bounds, or dispatching position-dependent input flushes
  the minimum dirty layout.
- An outer `UpdateScope`/`SuspendLayout` guarantees coalescing until exit.
  Inside it, ordinary geometry reads return the last committed arrangement;
  `PerformLayout` is the deliberate way to break the batch and obtain a new
  arrangement.
- `PerformLayout` is an explicit early flush.
- A read barrier may never reenter user layout callbacks recursively; phase
  guards queue consequent mutations for a second bounded pass.

**Gain:** stateful immediate APIs and correct observed geometry without paying
for unobserved intermediate layouts. Generated code can wrap construction in
one scope automatically.

**Loss:** a geometry read can be computationally expensive; performance bugs can
hide in innocent-looking getters; read barriers and bounded multi-pass rules
must be documented and instrumented.

An additional useful distinction is:

- `RequestedBounds`: synchronous input constraint set by the application;
- `ArrangedBounds`: final parent/layout result, which invokes the read barrier.

The remaining GF006 decision is whether to adopt L4, and whether ordinary
`Bounds` means requested or arranged geometry. The recommendation is arranged
geometry for compatibility and an explicitly named requested constraint for
advanced layout code. A development diagnostic should identify reads of dirty
arranged geometry from inside an update scope without forbidding them.

## Proposed corrected event ordering

This is deliberately a GUI.Forms order, not a reconstruction of every WinForms
accident.

### Pointer press

1. host normalizes pointer event and establishes the hit-test/capture target;
2. internal preview route;
3. cancellable focus request and old-control validation;
4. focus transition completes;
5. capture and pressed visual state are established;
6. target `PointerDown`/Forms mouse-down event;
7. typed invalidation is committed.

### Pointer release and activation

1. internal preview route;
2. target release event;
3. capture is released and pressed state clears;
4. activation/click is synthesized only if the control remains eligible, so a
   click handler observes the control as released rather than physically down;
5. typed damage commits.

### Keyboard/text

1. physical key preview;
2. mnemonic/command/default-button resolution;
3. key-down event if not consumed;
4. text or IME composition event as a distinct semantic stream;
5. key-up event.

Focus validation may cancel a transition before the new target receives focus.
Text input is never reverse-engineered from key-down events. Exact enter/leave,
validation, focus, activation, and disposal edge cases still need executable
traces before this ordering becomes normative.

## Primary technical references

- SDL3: <https://wiki.libsdl.org/SDL3/FrontPage>
- GLFW input and window scope: <https://www.glfw.org/docs/latest/input>,
  <https://www.glfw.org/docs/latest/window.html>
- Qt Platform Abstraction breadth and stability:
  <https://doc.qt.io/qt-6/qpa.html>
- Blend2D software renderer: <https://blend2d.com/about.html>
- Cairo image surfaces and API: <https://www.cairographics.org/manual/>
- AGG design: <https://agg.sourceforge.net/antigrain.com/about/index.html>
- PlutoVG: <https://github.com/sammycage/plutovg>
- ThorVG: <https://github.com/thorvg/thorvg>
- Pixman: <https://www.pixman.org/>
- DirectWrite bitmap targets:
  <https://learn.microsoft.com/en-us/windows/win32/directwrite/render-to-a-gdi-surface>
- Skia build configuration: <https://skia.org/docs/user/build/>,
  <https://skia.googlesource.com/skia/+/refs/heads/main/gn/skia.gni>
- Skia paragraph-module boundary:
  <https://skia.googlesource.com/skia/+/refs/heads/main/modules/skparagraph/BUILD.gn>
- Avalonia Skia adapter:
  <https://github.com/AvaloniaUI/Avalonia/tree/main/src/Skia/Avalonia.Skia>
- JetBrains Skiko: <https://github.com/JetBrains/skiko>
