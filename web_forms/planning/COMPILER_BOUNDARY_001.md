# Web.Forms compiler and runtime boundary 001

Date: 2026-08-10

Status: **HYPOTHESIS; intended to falsify with the first atlas slice**.

## 1. Pipeline

```text
HTML/CSS source
  -> standards-aware parse with source locations
  -> Web.Forms profile validation and budget checks
  -> typed source graph (structure, identity, layout, style, state, resources)
  -> compile-time cascade/nested-context/component expansion
  -> GUI.Forms construction IR and capability validation
  -> generated C++ + immutable records + typed handles + source map
  -> ordinary native compiler/linker
```

Unknown or inexact lowering stops compilation. The compiler never emits “close
enough” default controls for unsupported source.

## 2. What is compiled away

The product binary contains no HTML text requirement, CSS parser, DOM, selector
engine, JavaScript engine, layout interpreter, network loader, or browser event
model.

The compiler may retain compact inspection metadata:

- schema/profile and compiler version;
- source digest and optional source-map locator;
- stable IDs, parent/owner relationships, debug names, and control kinds;
- computed style/layout/state record identities;
- resource digests and provenance;
- generated-handle and semantic mappings;
- unsupported/degraded capability facts where a deliberately narrower profile
  permits them.

## 3. Generated construction

Generated C++ constructs stable retained controls once under GUI.Forms
initialization/update scopes, applies properties in the source-defined
deterministic order, attaches the tree, and returns a typed handle object.

Illustrative shape only (conforming to the generated C++ profile):

```cpp
generated::file_manager::View view = generated::file_manager::create(host);
view.command.copy.clicked().connect(on_copy_selection);
view.navigation.search.text_changed().connect(on_search_changed);
view.selection.properties.set_model(selection_snapshot);
```

The exact API spelling is not selected. Generated spelling must conform to
`GENERATED_CPP_PROFILE_001.md`. The invariant is that an invalid or
renamed source ID becomes a compile error rather than a runtime string lookup.
String-based inspection may coexist with typed handles for tests and tools.

Generated files are not the application extension point. Application-owned C++
files consume the generated header/constructor and attach handlers, models,
bindings, validators, commands, and domain services through public GUI.Forms.

## 4. Order and atomic initialization

GUI.Forms ADR-003 requires authored initialization assignment order to be
preserved. Web.Forms must define which source order is observable:

- DOM/control construction follows document order unless a named layout/model
  contract owns item order;
- source attributes and explicit compiler properties retain lexical order;
- CSS cascade/source order resolves computed style deterministically;
- an interned computed style may apply atomically as one style record rather
  than replaying unrelated CSS declarations as user-visible runtime events;
- batching may coalesce layout and paint, never reorder observable assignments.

HTML tooling often treats attribute order as semantically irrelevant. Profile
0.1 must either freeze lexical order for `data-wf-*` initialization properties
or move order-sensitive assignments into an explicitly ordered construct. This
edge remains unresolved and must close before round-trip tooling is promised.

## 5. Control, layout, and decoration lowering

One HTML element does not imply one C++ control.

- Interactive/semantic elements lower to retained controls.
- A layout-only element may lower to a layout node or be folded into its parent.
- Adjacent compatible text may lower to one shaped text object with runs.
- Pseudo-elements and anonymous decorations lower into display chunks.
- Identical immutable style, gradient, shadow, and resource records are interned.
- Dynamic collections lower to one virtual host plus an item-template recipe,
  not one static constructor per possible data item.
- Popups lower through GUI.Forms ownership/focus/top-layer primitives, not an
  arbitrary absolute-positioned child or unbounded z-index.

This is how generated output can be denser than a literal DOM while preserving
the authored visual relationships.

Lowering also preserves the nested ambient landscape. Children receive their
containing layout, clip, surface, typography, theme/accommodation, and effective
state context through compiled records. Independently positioned and painted
controls are not a valid approximation of nested source.

## 6. Design-time composition versus runtime composition

The architect's shared-background goal is admitted as two explicit planes:

1. **Style backplane:** immutable fills, gradients, textures, illustrations,
   wells, shadows, and other noninteractive material. These may be flattened,
   interned, compiled into drawing commands, or prerasterized when scale and
   bounds make that exact.
2. **Control plane:** retained controls, dynamic text, focus/selection,
   semantics, editors, virtual collections, popups, and hit targets. These remain
independently invalidatable.

Each visible region has one nearest surface owner. A child either reveals it,
paints a named local surface, or is baked into it. A control's unrelated default
background is a compilation/capability error, not an acceptable visual seam.

Precompositing the entire canvas is rejected as a default. It would make a text
change or focus ring repaint a large bitmap, erase accessibility structure, and
turn dynamic layout into image regeneration. Design-time compilation should
precompute *recipes and static layers*, not collapse the application into one
picture.

## 7. Build-time vector and image policy

GUI.Forms admits PNG as its runtime decoder. The atlas nevertheless relies on
inline SVG symbols. The compiler has two bounded candidates:

- parse a closed SVG subset at build time and emit GUI.Forms path/gradient
  records; or
- rasterize approved SVG inputs at declared scale variants into provenance-
  tracked PNG resources.

No SVG parser ships in the application. Unsupported SVG features fail the
build. Which candidate is default remains an experiment decision because path
output, scale quality, generated size, and renderer fidelity differ.

## 8. WYSIWYG contract

“Looks the same” needs four separate comparisons:

1. **Structural equivalence:** control kinds, IDs, ownership, states, text,
   resources, and z-order agree exactly.
2. **Geometric equivalence:** named bounds/baselines/scroll extents compare in
   logical units under a declared tolerance. This is the accepted fidelity
   target; literal cross-rasterizer pixel equality is not promised.
3. **Raster equivalence:** non-text pixels compare under a color-profile-aware
   error rule.
4. **Typography equivalence:** the same font bytes, shaping inputs, line breaks,
   and glyph positions are required; browser and native antialiasing are scored
   separately from geometry.

A browser preview is conformant only with a pinned browser build/profile, exact
font assets, fixed logical viewport/scale, local resources, and the Web.Forms
validator overlay. An arbitrary browser screenshot is a convenient sketch, not
the production oracle.

The first experiment should report per-element bounds error, text-baseline
error, non-text image difference, text-mask difference, and named unsupported
constructs. One aggregate screenshot score would conceal the mechanism.

## 9. Hot reload and designer boundary

Hot reload is optional tooling, not runtime architecture. If later admitted, it
compiles a new static graph, diffs stable IDs and compatible control kinds, and
applies one GUI.Forms update transaction. It must report destructive migrations
and may never interpret arbitrary HTML/CSS in a production process.

A future visual designer edits the authoritative `.wf.html`/`.wf.css` source or
a lossless syntax tree. Generated C++ remains disposable and is not round-trip
input.
