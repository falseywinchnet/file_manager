# GUI.Forms surface pipeline hypothesis

Status: HYPOTHESIS. This is the main UI architecture direction to interrogate,
not an accepted implementation design.

**GIVEN refinement, 2026-08-10:** the proposed authoritative visual source is
now the bounded browser-valid HTML/CSS profile owned by `../web_forms/`, not a
new Athene-derived XML/DML syntax. The DML lineage still supplies the durable
principles—round-trip source, stable IDs, compiled metadata, deterministic
order, and retained native output—and the provisional Gallery DML remains
implementation evidence. The no-JavaScript and nested-landscape boundary is
accepted, and Python is selected for the two-stage build-time compiler; exact profile
grammar and lowering details remain CANDIDATE.

## 1. Deciding properties are independent axes

Framework labels obscure the decisions that matter:

| Axis | Candidate positions |
|---|---|
| Authoring | imperative builder, declarative document, generated API, hybrid |
| Runtime state | retained/stateful, immediate/recomputed |
| Description lifetime | compile-time IR, load-time schema, mutable live graph |
| Rendering | custom renderer, wrapped native controls, mixed |
| Invalidation | explicit damage, dependency propagation, full subtree/window |
| Layout | constraint/grid, flex-like flow, dock/anchor, custom algorithms |
| Input | native event adapters, framework normalization, custom dispatch |
| Accessibility | native proxies over retained nodes, wrapped controls, mixed |
| Text | native edit controls, custom shaping/editing, hybrid overlays |
| Binding | direct calls, generated handles, signals/events, data dependencies |

The desired authoring experience can be imperative while the runtime is retained
and its intermediate representation declarative. Declarative does not imply web,
XAML, MAUI, diff-every-frame, or a managed runtime.

## 2. Named deliverable and live pipeline candidate

The framework deliverable is named **GUI.Forms**. Modern.Forms is evidence and
a possible compatibility source, not the implementation to complete in place.
GUI.Forms should preserve reasonable WinForms call shapes and expected outcomes
where real consumers rely on them, while rejecting accidental backend behavior,
historic bugs, and implementation details.

```text
bounded Web.Forms HTML/CSS / imperative C++ / generated C# / future designer
        |
        v
versioned build-time GUI construction IR
        |
        v
generated forms code + native retained presentation graph
        |
        +--> layout graph
        +--> accessibility graph
        +--> event/focus graph
        +--> precise damage graph
        |
        v
tight custom renderer + platform window/input/accessibility adapters
```

The application owns semantic control state. “The engine is stateless if
possible” is interpreted narrowly: the engine must not become a second
application model, but it must retain presentation nodes, layout results,
focus/capture, text-shaping caches, accessibility identities, animation clocks,
and dirty regions. Otherwise it becomes an immediate-mode rebuild engine and
loses the performance property this architecture exists to preserve.

Public setters are explicit mutation gates. Each setter marks typed dirtiness
(`paint`, `layout`, `hit-test`, `text`, `accessibility`, or `semantics`) and DML
metadata declares the limited dependency propagation. This hybrid avoids an
opaque observer graph while also avoiding manual rectangle bookkeeping at every
call site. Layout and rendering do not reconstruct the whole application merely
because a frame occurred.

Current retired compatibility specimen 1922 independently validates the need for batching: it uses a
nested invalidation-disable counter during startup, whole-tree theme mutation,
and plugin embedding. GUI.Forms therefore needs nestable mutation/layout/damage
transactions, not merely a Boolean “do not paint” flag. Re-enabling the outermost
scope coalesces layout and damage while preserving final synchronous state.

## 3. Athene/Pandora precedent

Public archival material describes Rocklyte's system as **Athene** (often written
Athena in contemporary coverage), built over the Pandora Engine. Its user
environment was assembled at runtime from objects linked through an editable
XML-like Dynamic Markup Language (DML). DML supported object-oriented graphics,
UI construction, calculations, arguments, and variable storage. Pandora was
described as a portable C object engine with shared objects and process
isolation.

The architect confirms **DML** as the historical lineage but has selected a new
bounded HTML/CSS source profile, Web.Forms, rather than recreating Athene syntax.
The public sources found so far substantiate DML/Pandora and runtime object
linkage, but do not name “PTP schemas.” No PTP terminology is required for this
project unless an original artifact later establishes it.

The principle worth reviving is not XML syntax. It is a portable, versioned,
inspectable boundary between application construction and the rendering core.

## 4. Modern.Forms as evidence; GUI.Forms as the output

The current Modern.Forms implementation is .NET and cannot be File Manager's
runtime dependency. Its implementation may be discarded. It can still inform
GUI.Forms in four ways:

1. **API reference:** copy the good imperative shapes and control semantics.
2. **C# binding:** preserve a WinForms/Modern.Forms-like managed API that calls the native
   engine through a stable C ABI.
3. **Schema frontend:** C# object construction emits the same PTP-like IR used by
   C++/Rust authoring.
4. **Compatibility target:** selected existing Modern.Forms applications compile
   against the new frontend and render through the native engine.

The native engine does not know or require C#. File Manager links it directly
from C++. A friend's C# application can use the compatibility layer. Rust is
reserved initially for the hostile plugin wrapper/sandbox boundary, where its
memory-safety properties buy concrete isolation value.

Full source compatibility is probably much more expensive than preserving the
programming model. We must inventory the exact control/API subset needed by the
serious external project before choosing a compatibility promise.

The architect recalls an exhaustive GitHub WinForms listing, possibly in
Avalonia-related material. It covers the control catalogue needed to complete
the framework, but the exact repository is forgotten. Community GitHub listings
and Avalonia migration material must be searched first; Microsoft's WinForms
assemblies/source are the exhaustive verification backstop, not assumed to be
the remembered list. None of this mandates Avalonia, XAML, MVVM, or .NET.

## 5. Engine and language boundary

- Native retained engine: disciplined C++.
- C++ policy starts from BFFT's useful patterns: explicit types, no hidden hot-
  path allocation, stable C ABI, thin RAII wrapper, explicit failure, aligned
  heap arrays, reusable plan/workspace memory, and warnings-as-errors.
- A stable C ABI is the recommended authoritative binary seam: opaque handles,
  versioned function tables, plain data, explicit ownership/allocator rules,
  explicit error objects, and callback/event contracts. C++ and generated C#
  bindings sit above it.
- Web.Forms HTML/CSS is the proposed durable GUI specification and visual
  authoring surface; its build-time Python compiler produces the retained
  construction IR and contributes no product runtime.
- Web.Forms generated product source is a C++17-compatible orthodox subset even
  when GUI.Forms itself is built as C++20.
- A future Visual-Studio-like designer edits round-trippable Web.Forms source
  and generates disposable forms code.
- Python hosts the isolated Web.Forms build tool. Rust hosts untrusted plugin
  adapters behind capability-limited IPC. Neither is the primary widget engine
  or a GUI product runtime.
- No native text-control overlay. IME, tooltip, keyboard navigation, and
  accessibility semantics are deliberate engine features, even where a host OS
  cannot guarantee equal capability.

## 6. Compatibility evidence and first proving slice

The two bridge targets are intentionally separate:

1. every WinForms control/component that exists, for catalogue completeness;
2. every control, property, event, and lifecycle retired compatibility specimen actually exercises, for a
   serious minimum compatibility envelope.

The current retired compatibility specimen `next-x64 (6)` build is authoritative. The debug build may
expose otherwise opaque detail but is significantly different and must never be
silently substituted for current behavior. The plugin SDK already demonstrates
the basic contract: designer-generated `UserControl`, ordinary property writes,
`Controls.Add`, anchor/layout metadata, event subscription, disposal,
`MessageBox`, and lazy GUI creation.

The current application exercises a narrower core than the complete catalogue,
but with deep behavior: Form/UserControl/Control, panels, group/table/flow
layout, dock/anchor/autosize/scroll, labels/buttons/choices/text/numeric/combo,
custom sliders, menus/tooltips/timers, modal dialogs, owner painting, retained
high-rate bitmaps, grid/binding/edit hosts, UI dispatch, and plugin subtree
reparenting/disposal. DockPanel Suite and MapControl remain sibling adapters,
not base GUI.Forms controls.

The first falsifying vertical slice remains: `Window`, `SplitContainer`,
`TreeView`, virtual `ListView`, preview surface, `TextBox`, menu, and status bar.
GUI.Forms is a parallel deliverable, not something postponed until File Manager
is complete.

## 7. Why this might be the right degree of machinery

Potential benefits:

- retained state and precise invalidation;
- one visual and behavioral system across platforms;
- declarative artifacts without declarative-only application programming;
- binding generation for C++, Rust, C#, and possibly other languages;
- deterministic inspection, testing, serialization, and hot reload;
- renderer/layout replacement without rewriting application intent;
- possible out-of-process composition and preview surfaces;
- a reusable Modern.Forms successor as an output of File Manager work.

Potential overkill/failure modes:

- inventing an IR, compiler, widget system, renderer, accessibility bridge, and
  file manager simultaneously;
- schema/version complexity exceeding the value of cross-language binding;
- duplicating platform text editing, accessibility, drag/drop, IME, and window
  behavior badly;
- making every runtime mutation serialize through an unnecessary document layer;
- freezing a premature widget ontology into a compatibility contract;
- spending years on framework completeness before File Manager can dogfood.

The experiment must prove that the schema is a simplifying seam, not a ceremony
generator.

## 8. Surface questions

Detailed backend and behavior choices have moved to
`gui_forms/GUI_FORMS_BACKEND_DECISION_MAP.md`. The relational style/color
contract is recorded in
`../frontend/planning/visual/COLOR_RELATION_MODEL.md`.

- **UI001:** Is “PTP” the exact historical name? Do you have any original schema,
  binary, SDK, DML, manual, or screenshot artifacts?
- **UI002:** Is the IR generated at build time, instantiated at application start,
  or continuously synchronized while the application runs?
- **UI003:** Should the schema be human-authored/readable, or generated and merely
  inspectable?
- **UI004:** Must a serialized UI survive round-trip editing, or is it one-way
  compiler output?
- **UI005:** Does application state live primarily in application objects, engine
  nodes, or explicitly split model/view state?
- **UI006:** Are UI node identities stable across rebuilds so state and tooling can
  refer to them?
- **UI007:** Are mutations command messages, property writes, transactions, or
  replacement of immutable subgraphs?
- **UI008:** Confirm the hybrid invalidation rule: public mutations mark typed
  dirty flags; DML declares only bounded derived dependencies; there is no
  observer-everywhere reactive graph.
- **UI009:** Should DML compilation reject a control whose invalidation effects
  are not declared, or permit a conservative subtree repaint?
- **UI010:** What minimum IME set must the first custom text editor pass on
  macOS before it is allowed to replace Finder for daily use?
- **UI011:** Which controls must be genuinely custom from the first prototype?
- **UI012:** Which controls are acceptable as wrapped native controls, if any?
- **UI013:** Should layout be one composable algebra containing dock/anchor,
  grid, flex-like flow, and constraints, or several explicit layout containers?
- **UI014:** Should layout results be cached until a relevant constraint changes?
- **UI015:** Is pixel-perfect deterministic cross-platform layout more important
  than native text metrics?
- **UI016:** Does custom title chrome belong inside the same retained tree?
- **UI017:** Should sound and animation be declarative effects attached to state
  transitions, or imperative commands?
- **UI018:** Must the IR support process boundaries so preview/plugin surfaces can
  be composed without sharing memory unsafely?
- **UI019:** Confirm a stable C ABI as the authoritative engine boundary, with
  ergonomic C++ and generated C# wrappers above it.
- **UI020:** Which implicit C++ conversions and conveniences join `auto` on the
  forbidden list, and which are permitted under audited local rules?
- **UI021:** What exact Modern.Forms control/API subset does your friend's serious
  project need?
- **UI022:** Does that project require existing Modern.Forms source to compile
  unchanged, or would a closely familiar new API be enough?
- **UI023:** RESOLVED: GUI.Forms is a parallel deliverable.
- **UI024:** What is the smallest vertical slice that could falsify this whole
  pipeline within weeks rather than years?
- **UI025:** RESOLVED AS SEARCH TASK: locate the forgotten exhaustive community
  GitHub/Avalonia listing; do not require the architect to remember its URL.
- **UI026:** Is its full control list the completion target, or may we explicitly
  mark controls unsupported until a real consumer needs them?
