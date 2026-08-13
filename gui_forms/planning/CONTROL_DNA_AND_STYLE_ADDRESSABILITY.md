# Control DNA and style addressability

Date: 2026-08-13

Status: **GIVEN requirement; CANDIDATE vocabulary and lowering contract**.

## 1. Question

What is the smallest superset of control elements and relationships that a
style must be able to address so that rich themes compose instead of becoming a
manual paint implementation for every control?

The answer cannot be merely an exhaustive list of stock widgets. Controls are
systems assembled from elements that can also be used to make other controls.
The vocabulary must therefore describe both ordinary leaves and reusable
composition.

## 2. Required distinction

**GIVEN:** how elements are addressable and scriptable is exposed by authored
HTML to generated C++ so the program can work. How those elements look is
exposed to CSS and admitted style images.

The compiler must preserve two address spaces:

| Address space | Typical consumers | Required stability |
|---|---|---|
| Semantic identity | generated C++, accessibility, commands, tests, application state | explicit stable IDs and typed handles |
| Style identity | selectors, parts, states, relationships, generated decoration | stable structural roles/classes/parts within a versioned control contract |

A style-addressable element does not automatically become a C++ handle. A
semantic handle does not justify hiding its internal style anatomy.

## 3. Candidate DNA superset

The following is a candidate minimum vocabulary, not a frozen source grammar.

### 3.1 Authoritative regions

Containers that own child composition and arbitration within a granted region.
They determine visual order, interactive order, clipping, overflow, overlap,
focus relationships, and transitions involving their children.

### 3.2 Structural elements

- groups and containers;
- named slots;
- rows, columns, tracks, stacks, flows, and canvases;
- repeated-content hosts with application-owned item keys;
- templates and typed content hosts;
- conditional or state-selected authored branches, within a bounded profile.

### 3.3 Content elements

- text and editable text;
- glyphs and icons;
- PNG images and density-aware style assets;
- application-projected image/content surfaces;
- labels, descriptions, values, and generated counters.

### 3.4 Interactive elements

- activators/buttons;
- toggles and selections;
- editors;
- disclosures;
- sliders, seams, and drag actuators;
- scrolling and viewport actuators;
- explicit hit regions where visual and interactive geometry differ.

These are behavioral primitives, not visually indivisible stock widgets. A
button is itself a custom composed system whose internal parts remain
style-addressable.

### 3.5 Surface and decorative elements

- faces and material layers;
- borders and per-edge keylines;
- seams and joins;
- shadows, including back/underside and inset depth;
- masks, clips, and compositing groups;
- generated before/after decoration;
- focus, default, validation, selection, and drag cues.

Decoration may lower into immutable style/display records rather than semantic
retained controls. It still requires inspectable selector/owner identity.

### 3.6 Relational elements

- adjacency;
- attachment and anchoring;
- overlap and join topology;
- shared boundaries and ownership of those boundaries;
- leading, middle, trailing, and sole-child roles;
- source/target and activator/disclosure relationships;
- parent-owned transition relationships.

This is essential. A chevron breadcrumb segment is not a new primitive button
class. It is a reusable button system placed into a parent-authored overlap and
ordering relationship.

### 3.7 State elements

At minimum, styles may need to distinguish:

- normal, hover, pressed, captured, focused, and default;
- selected, checked, current, pending, and indeterminate;
- enabled, disabled, unavailable, invalid, and read-only;
- open, closed, expanding, collapsing, and interrupted;
- active/inactive window, high contrast, reduced motion, scale, and density;
- application-authored finite states declared in the source contract.

State exposure must be typed and bounded. A stylesheet cannot execute product
logic merely because it can select a state.

### 3.8 Projection elements

- menus and anchored popups;
- drawers;
- tooltips and guidance bubbles;
- transient editors and selectors;
- independently hosted owned surfaces when a projection crosses a native
  window boundary.

A projection retains logical ownership while receiving explicit visual and
interactive jurisdiction from an overlay or host region. It is never permission
for an arbitrary child to paint or receive input outside its grant.

## 4. Recursive construction examples

### 4.1 Button system

```text
button region
├── surface
├── content slot
│   ├── optional icon
│   └── label
├── optional disclosure
└── focus/default/validation cues
```

The button owns activation semantics and its internal composition. Its parent
owns where the complete button participates among siblings.

### 4.2 Breadcrumb system

```text
breadcrumb authoritative region
├── ancestor button system × N
├── current button system
├── terminal operator button system
└── location-history drawer projection
```

The parent supplies leading/middle/trailing roles, overlap, z-order, shared hit
boundaries, and the transition between the trail and drawer. Segments supply
button semantics, content, and local interaction state.

### 4.3 Open-dialog system

```text
open-dialog authoritative region
├── title system
├── navigation system
│   ├── back/root activators
│   └── location editor
├── object-browser system
├── filtering system
└── command system
    ├── cancel
    └── accept
```

C++ binds selection authority and commands. CSS can address the complete
presentational anatomy without acquiring filesystem authority.

## 5. Candidate authored shape

The following is explanatory, not a grammar decision:

```html
<nav id="location.breadcrumb" class="breadcrumb" data-state="closed">
  <button class="breadcrumb__segment" data-repeat="ancestor">
    <span class="breadcrumb__label"></span>
  </button>
  <button id="location.history" class="breadcrumb__operator"
          aria-haspopup="menu">
    <span class="breadcrumb__operator-label">./</span>
    <span class="breadcrumb__disclosure"></span>
  </button>
  <menu class="breadcrumb__history" data-source="location-history"></menu>
</nav>
```

Candidate CSS responsibilities include complete segment silhouettes, overlap,
backshadow, local states, terminal-operator presentation, the anchored drawer,
and the parent open-state transition. Behavior attaches to stable semantic IDs
and declared commands, not JavaScript.

## 6. Documentation and inspection requirement

Every reusable system must publish a versioned anatomy reference containing:

- semantic handles and commands;
- style-addressable elements and generated parts;
- parent and slot relationships;
- state vocabulary and state ownership;
- render-order and interactive-order rules;
- intrinsic, visual, hit, overflow, and projected geometry;
- accessibility roles and relationships;
- supported transition hooks;
- selector compatibility and migration policy;
- an inspection example showing authored source through committed display
  operations.

“Individually rich and highly addressable” is not satisfied by a property list
alone. A theme author must be able to discover the complete anatomy and its
relationships without reading private C++ renderer code.

## 7. Gates

1. Express two materially different breadcrumb themes over the same authored
   structure and behavior without product-specific C++ paint changes.
2. Express an ordinary button and an overlapping chevron segment from the same
   button system plus different parent relationships.
3. Inspect every styled part and relational decision from source selector to
   retained node/style record to display operation.
4. Demonstrate that semantic handles remain stable when decoration changes.
5. Demonstrate that a CSS-only change cannot alter domain commands, filesystem
   authority, accessibility role truth, or application data ownership.
6. Fail compilation when a required structural part, relationship, state, or
   style capability has no truthful native lowering.
