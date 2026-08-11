# Web.Forms Language Profile 0.1

Date: 2026-08-10

Status: **partly DECIDED boundary; detailed element/property limits remain
CANDIDATE; bounded dogfood implementation is authorized by ADR-003 without a
0.1 compatibility promise**.

## 1. Source form

The candidate source pair is:

- `*.wf.html` — browser-valid HTML containing structure, text, resources, stable
  identity, control kinds, and compiler metadata;
- `*.wf.css` — browser-valid CSS restricted to a versioned selector/property/
  value profile.

An HTML file may contain a `<style>` block for a self-contained specimen. A
production form should prefer local `.wf.css` files so style assets can be
hashed, shared, and compiled independently. Remote URLs are invalid.

Web.Forms is a strict profile of HTML/CSS plus inert `wf-*` class tokens and
`data-wf-*` attributes. A normal browser ignores the compiler metadata and
renders the ordinary HTML/CSS. The compiler rejects anything outside the
profile.

**OBSERVED dogfood implementation:** `webforms profile` emits the current
machine-readable elements, control kinds, attributes, properties, states,
pseudo-elements, and numeric limits from the same constants used by Stage 1.
The profile remains experimental rather than a 0.1 compatibility promise.

## 2. JavaScript exclusion

Reject:

- `<script>` and `<noscript>`;
- all `on*` attributes;
- `javascript:` or executable/data-document URLs;
- DOM mutation, custom-element registration, Web Components, shadow DOM, and
  script-backed templates;
- general expressions, callbacks, loops, imports, timers, storage, fetch, and
  browser APIs;
- CSS expressions or any future property whose value can execute code.

Web.Forms does not translate JavaScript. Domain behavior is C++ attached to
generated GUI.Forms handles.

## 3. Identity and naming

### Addressable nodes

Every retained control, command, popup, component instance, model host, and
explicit semantic node requires a stable `id`. Static grouping and decoration
may omit one and are then not addressable.

Candidate canonical grammar:

```text
[a-z][a-z0-9_-]*(.[a-z][a-z0-9_-]*)*
```

The ID is globally unique in the compiled form. A retained child's dotted path
must extend the actual retained parent's ID; the compiler derives and verifies
`parent_id` from DOM/control ownership rather than asking the author to repeat
it. Popups, labels, descriptions, and other non-containment relationships use
typed ID references.

Generated C++ exposes a strongly typed handle for every addressable ID. Source
renames are explicit migrations because tests, saved pane state, accessibility,
hot reload, and handler code may depend on them.

### Repeated content

Static source IDs may not be duplicated by a repeater. Virtual tree/list/grid
items use an application model's stable key under a separately identified host
control. The generated form owns the host ID; the model owns item identity.

### Decoration

Pseudo-elements, shadows, gradient layers, separators, and anonymous structural
nodes lower into the owning control's immutable display/style record. Requiring
IDs for these would bloat the retained tree and falsely imply behavior.

## 4. `class` and compiler metadata

Do not make all classes compiler settings. CSS classes are the authoring
language's reusable visual vocabulary and need ordinary cascade semantics.

The split is:

- `class="command quiet"` — visual/component recipe selectors;
- `class="... wf-static-backplane"` — reserved Boolean compiler traits only;
  order has no meaning;
- `data-wf-style-exposure="baked|exposed"` — whether typed presentation
  properties are omitted or generated; the default is `baked`;
- `data-wf-control="tree-view"` — typed control kind when the HTML tag is not
  sufficient;
- `data-wf-owner="file-manager.root"` — typed ID relationship;
- `data-wf-command="copy-selection"` — stable command identity/reference;
- `data-wf-field="name"` — an optional direct field slot only inside a separately
  admitted bounded virtual-item template; never a general binding path;
- `data-wf-preview-state="pressed"` — capture-fixture metadata only, stripped
  from production output.

Every `data-wf-*` name and value type belongs to the versioned schema. Arbitrary
application metadata is not executable and is not silently forwarded.

## 5. Element categories

The 0.1 profile should admit elements by lowering category, not promise that
every HTML element is a control.

| Category | Initial candidates | Lowering |
|---|---|---|
| document | `html`, `head`, `title`, `meta`, `body`, local `link`, `style` | compiler/document metadata |
| layout/semantic | `main`, `header`, `nav`, `section`, `article`, `aside`, `footer`, `div` | retained container or elided layout group |
| text | text nodes, `span`, `strong`, `b`, `em`, `small`, `mark`, `br`, headings, `p`, `dl/dt/dd` | shaped text runs, labels, or semantic groups |
| controls | `button`, `input` with admitted types, `label`, `textarea`, `select`, `option` | stock GUI.Forms controls |
| collections | `ul/ol/li`, admitted `table` subset, typed `data-wf-control` hosts | static or virtual collection controls |
| resources | local PNG `img`; later closed SVG files as indivisible image resources | resource/drawing records |
| compile-time reuse | inert `template` with `data-wf-component` if approved | bounded static expansion only |

Reject `iframe`, `frame`, `object`, `embed`, media playback, browser canvas,
portal, form submission/navigation, arbitrary custom elements, and elements
whose semantics require a web document environment.

Inline `<svg>` is also rejected in 0.1. Vector art may later enter only through
a local image resource hosted by a normal HTML element. Its host participates
in layout, surface inheritance, state, semantics, and hit testing; vector
descendants do not. The resource's intrinsic coordinates scale into the
already-resolved host box and never determine surrounding GUI geometry.

## 6. CSS selector profile

Candidate allowed selectors:

- type, class, and ID selectors;
- direct-child and bounded descendant combinators;
- typed `data-wf-state`/theme/variant attribute equality selectors;
- a closed structural set such as `:first-child`, `:last-child`, and bounded
  `:nth-child` where static expansion makes it deterministic;
- GUI state pseudo-classes: `:hover`, `:active`, `:focus`, `:focus-visible`,
  `:disabled`, `:checked`, `:selected`, and approved popup/expanded states;
- `::before` and `::after` only for nonsemantic decoration owned by the parent.

Use relational CSS decoration for geometry whose position or size depends on a
control box. For example, a breadcrumb chevron is a bounded `::after` recipe
anchored by layout, not a fixed-coordinate SVG subtree.

Reject relational `:has`, general/sibling combinators in 0.1, unbounded selector
nesting, arbitrary substring attribute tests, dynamic language/direction policy,
visited-link state, and selectors whose truth requires a live DOM query engine.

The compiler resolves the cascade at build time into finite state/style tables.
There is no runtime selector matching.

## 7. CSS property/value profile

The first property families should be selected from the atlas corpus:

- size/min/max, margin, padding, gap, and box sizing;
- GUI.Forms flex, grid/table, stack, split, canvas, and control-specific virtual
  layout projections;
- relative positioning plus owned anchored-popup/top-layer placement;
- overflow clip/scroll and explicit scroll ownership;
- typography roles, weight, size, line height, alignment, wrapping, and
  decoration over pinned font packs;
- named colors/custom properties, opacity, borders, radii, outlines, and depth;
- solid, linear, radial, and repeating-gradient fills under numeric stop limits;
- bounded shadows, text shadows, and supported filters which lower exactly;
- finite 2D transforms;
- z-order within declared paint/control/top-layer planes;
- cursor and hit-target metadata;
- bounded state transitions which have a real GUI.Forms state source.

Reject implicit host fonts, arbitrary URLs, 3D transforms, blend/filter features
without exact GUI.Forms lowering, layout containment tricks, sticky/fixed web
viewport semantics, CSS Houdini, keyframe animation, infinite/repeating motion,
and any unknown declaration. `!important` should be rejected in authored 0.1;
specificity conflicts should be fixed at the source.

Units and functions are a closed set. `px` means logical GUI units, not a
physical device pixel. Percent, `fr`, `auto`, `minmax`, `repeat`, `calc`, and
viewport-relative values enter only where GUI.Forms has a named equivalent and
the fidelity test passes. Browser acceptance alone is insufficient.

## 8. Content, style generations, and nested composition

Web.Forms compiles the nested ambient context defined in
`NESTED_LANDSCAPE_001.md`; it does not implement arbitrary behavioral multiple
inheritance.

Stage 1 keeps application content/identity separate from three independently
interned style domains:

- geometry/layout;
- typography metrics and shaping intent;
- material, including color, fills, borders, radii, shadows, and effects.

The same accepted content tree may compile with a different attached CSS
generation. Dogfood requires content and unaffected geometry/typography records
to remain byte-equivalent while material records change. A target capability
loss is reported against the generated requirements; identity, layout,
behavior, ownership, and accessibility never silently fall back.

- One node maps to at most one concrete GUI.Forms control kind.
- Visual classes and typed theme roles may compose around it.
- Custom properties are typed at compile time and resolve to immutable values.
- Identical computed style/layout records are interned and shared.
- A theme switch chooses a precompiled style generation under one GUI.Forms
  update scope; it does not reevaluate CSS selectors.
- A component template, if admitted, expands statically under the instance's ID
  prefix and has bounded typed slots/parameters. It cannot execute.
- Static background/fresco layers may be precompiled or prerasterized. Dynamic
  controls, text, focus, semantics, and hit testing remain independent retained
  objects above that backplane.
- A child explicitly reveals its nearest ancestor surface, paints one named
  surface, or is baked into the ancestor. CSS `background` itself is not
  incorrectly treated as inherited.
- Presentation is `baked` by default. `exposed` generates a bounded typed style
  handle; it does not imply multiple themes.

No diamond control inheritance, runtime mixin lookup, prototype chain, or
scripted custom-control definition is admitted.

## 9. Interaction states without interaction simulation

Visual state is part of the design contract. The language therefore keeps a
finite pseudo-state matrix even though it excludes JavaScript. A design tool or
capture harness can force `hot`, `pressed`, `focused`, `selected`, `disabled`,
`checked`, `expanded`, `drag-target`, and `unavailable` specimens.

The actual state comes from GUI.Forms controls and C++ application logic. Markup
does not declare filesystem actions, fake navigation, path resolution, search,
validation policy, timers, or side effects. Essential information may not be
reachable only by hover.

Allowing state-linked transitions is a separate 0.1 decision. If admitted, they
must be interruptible, reduced-motion aware, non-looping, duration bounded, and
limited to properties GUI.Forms can update without a perpetual frame loop.

## 10. Formal bounds required before 0.1

The accepted profile must assign numeric limits to at least:

- source bytes, stylesheet bytes, static node count, retained control count,
  tree depth, component expansion, classes/attributes per node, and ID length;
- selector count, selector segments, specificity, declarations per rule, custom
  property count, cascade resolution, and state-product expansion;
- text bytes/runs, grid tracks, flex children, virtual templates, popup depth,
  z-order planes, and layout passes;
- gradients/stops, shadows, clips, paths/verbs, vector resource bytes, decoded
  PNG dimensions/bytes, font roles, and generated static data;
- generated C++ size, compiler memory/time, runtime construction work, layout
  work, and first-paint command count.

The File Manager atlas and GUI.Forms gallery form the calibration corpus. Limits
remain CANDIDATE until those fixtures are counted; they may not remain “large
enough” prose when implementation opens.
