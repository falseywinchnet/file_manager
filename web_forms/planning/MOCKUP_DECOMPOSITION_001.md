# File Manager concept-atlas decomposition 001

Date: 2026-08-10

Status: **OBSERVED source evidence; Web.Forms implications are CANDIDATE**.

## Source identity

Primary source:
`frontend/planning/visual/frontend-concept-atlas.html`

Frozen GUI.Forms consumer copy:
`gui_forms/file_manager_demoboard/reference/frontend-concept-atlas-reference.html`

Both files are 183,048 bytes and have SHA-256
`aeab8bd4796ef05403d0cc3845cc4800a417fcde0a50fed9d3e1a2fe031021f7`.
The source describes itself in the visible page as Concept Atlas 007; its HTML
`title` still says 005. The identical copy is the demoboard's latest reference
authority as of this intake.

## Mechanical inventory

| Region | Observed size / count |
|---|---:|
| embedded CSS | 81,979 bytes |
| static body before script | 54,263 bytes |
| JavaScript | 46,543 bytes |
| CSS rule blocks | 556 |
| custom-property declarations | 399 over 48 distinct names |
| body elements | 1,159 |
| buttons / inputs | 94 / 5 |
| explicit IDs | 65 |
| buttons or inputs with IDs | 7 of 99 |
| inline style attributes | 22 |
| `data-*` attributes | 76 |
| ARIA attributes | 36 |
| SVG symbols / uses | 25 / 68 |
| JavaScript event-listener sites | 22 |

The page has no inline `on*` handlers, no media queries, and no keyframe
animations. It uses three 90 ms transition groups.

## HTML language in the specimen

The document combines four different things that a compiler must separate:

1. **Retained controls:** buttons, text inputs, split controls, tree rows,
   correspondence rows, criteria modules, preview/property controls, and popup
   actions.
2. **Layout and semantic grouping:** `main`, `section`, `header`, `nav`, `aside`,
   `article`, `footer`, `div`, labels, definition lists, and text hierarchy.
3. **Decoration:** many `span`/`i` fragments, pseudo-elements, icon pieces,
   gradient fills, lines, shadows, and texture layers which should lower into
   parent paint/style records rather than retained controls.
4. **Planning-only content:** palette/style laboratories, the DNA review board,
   fixture text, and comparison specimens which are not product behavior.

The document is not yet addressable as an application form. Ninety-nine native
form elements exist, but only seven have IDs. Many important visual controls are
`div`/`span` structures with class-based identity, while most SVG definitions
also consume the HTML ID namespace. A Web.Forms intake pass therefore needs a
typed identity inventory, not a blind “add IDs to every element” rewrite.

## CSS language carrying the fidelity

The visual result is driven primarily by:

- a 48-name custom-property vocabulary;
- twelve atmosphere families which replace relational color/token values;
- twelve construction families selected by `data-style` attribute selectors;
- seven surface modes selected by `data-state` attribute selectors;
- Grid and Flex layout, including `fr`, `minmax`, `repeat`, `auto-fill`,
  percentages, viewport units, and `calc`/`min`;
- absolute overlay placement for the path matrix;
- overflow/scroll planes and collapsed grid variants;
- solid, linear, radial, and repeating gradients;
- borders, radii, outlines, shadows, text shadows, opacity, and drop shadows;
- `hover`, `active`, `focus`, `focus-visible`, structural pseudo-classes, and
  generated `::before`/`::after` decoration;
- a small amount of bounded transition styling.

The most frequent CSS properties are `background` (238 declarations), `color`
(135), `display` (134), `box-shadow` (103), `padding` (101), `border` (91), and
`height` (85). This is material/layout language, not evidence that a runtime DOM
or browser cascade is necessary.

## JavaScript language in the specimen

| Script responsibility | Product compiler treatment |
|---|---|
| switch Folder/Search/Criteria and planning boards | external preview fixture; do not compile as domain behavior |
| switch atmosphere and construction families | design-time variant selector or typed runtime theme choice |
| open/close path matrix and pane/image collapse | real GUI.Forms control state and C++ handlers |
| fake navigation and shell-variable path resolution | fixture/prototype logic; product path authority remains application code |
| filter autocomplete rows | real model/control behavior, not translated JavaScript |
| DNA decision data, filtering, localStorage verdicts | planning tool only; exclude from product form |
| query-string state selection | external capture harness only |
| DOM creation and `innerHTML` for DNA specimens | static planning generation; reject in Web.Forms source |

**OBSERVED conclusion:** none of the JavaScript establishes a need for a
JavaScript runtime or source-language escape hatch. It establishes the states
and commands a native implementation must own.

## First lowering map

| Atlas construct | Candidate Web.Forms lowering |
|---|---|
| semantic container plus layout CSS | GUI.Forms container/layout record; elide if it owns no behavior |
| `button`, input, selection/editor surface | concrete retained GUI.Forms control |
| tree/object/result repeated rows | virtual collection control plus keyed fixture/model template |
| class-based theme and construction variants | interned immutable style generations |
| CSS pseudo-state | GUI.Forms control-state style matrix |
| pseudo-element or decorative `i` | parent display chunk/backplane decoration |
| inline SVG symbol library | build-time closed SVG lowering or precompiled resource; no runtime SVG parser |
| absolute path matrix | owned anchored popup/top-layer contract, not arbitrary z-index escape |
| fake state/content changes | C++ model/handler logic using generated stable handles |

## Pressure points exposed by the atlas

- Browser font fallback and browser rounding are not a native fidelity oracle.
- The current CSS uses layout features not yet proven equivalent in GUI.Forms,
  notably `auto-fill`, viewport-relative sizing, and dynamic grid replacement.
- `display:none` currently conflates alternate planning surfaces, responsive
  collapse, and runtime visibility; the compiler needs typed meanings.
- Classes currently mix component identity, visual style, state, and planning
  taxonomy. Web.Forms must separate those axes.
- Inline SVG is central to the prototype, while GUI.Forms admits no runtime SVG
  decoder. Build-time lowering is therefore a required decision.
- Hover-driven result expansion needs keyboard/touch/pinned equivalents; visual
  pseudo-state alone is insufficient behavior.
- Fully precompositing the canvas would destroy independent damage, focus,
  semantics, and dynamic text. Only static backplanes and immutable style
  recipes should be baked at design time.

