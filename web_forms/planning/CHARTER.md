# Web.Forms planning charter

Date: 2026-08-10

Status: **GIVEN project mission; architecture and implementation gates open**.

## Mission

Create a formally bounded authoring language that preserves the browser as a
fluid visual design surface while producing a retained, imperative, native C++
GUI.Forms application with deterministic identity and no shipped web runtime.

The project exists because the manual GUI.Forms demoboard path proved useful
retained behavior but failed the owner's visual-similarity and authoring-economy
test. See `NATIVE_TRANSLATION_NEGATIVE_001.md`.

## Recorded direction and working recommendation

- Web.Forms is a new sibling project beside GUI.Forms.
- Source looks and previews like ordinary HTML/CSS, not Athene DML or a new XML
  dialect.
- Source options are constrained and validated rather than inheriting the whole
  evolving web platform.
- **GIVEN:** exclude general JavaScript and simulated product behavior from
  authored source; retain only finite declarative visual states.
- Generated output is dense C++ over GUI.Forms' public controls, layout,
  drawing, styling, state, event, and lifecycle primitives.
- The runtime remains retained and imperative. It is not rebuild-on-frame and
  does not synchronize a DOM.
- Stable element identity must survive compilation so application code can
  address, mutate, read, and subscribe to the intended objects.
- Parent containment must compile into one nested ambient layout, paint, style,
  and effective-state landscape; loose visual overlay is not conformant.
- Product output is C++17-compatible source in the accepted orthodox generated
  profile. The build-time compiler is Rust and adds no product runtime.
- Browser preview and generated-native result are compared explicitly; visual
  accuracy is a conformance claim, not an adjective.

## Questions this project owns

1. Which exact HTML elements, CSS selectors, properties, values, and units form
   the portable source profile?
2. Which source nodes become retained controls, which become layout records,
   and which are flattened into decoration or text runs?
3. How are stable identity, component instances, virtual item keys, commands,
   popups, and semantic nodes named and generated?
4. How are CSS cascade, inheritance, custom properties, themes, state variants,
   and shared backgrounds resolved at build time?
5. What fidelity can a pinned browser preview promise relative to GUI.Forms
   given different font rasterization and pixel rounding?
6. Which metadata must GUI.Forms publish so the compiler never scrapes C++
   headers or freezes private implementation?
7. What limits make parsing, expansion, layout, resource lowering, and generated
   code size mechanically bounded?

## Non-goals

- importing the DOM, JavaScript, a browser layout engine, Web Components, npm,
  remote URLs, or web security policy into the application runtime;
- compiling arbitrary existing websites;
- reproducing every HTML element or CSS edge case;
- expressing filesystem, search, plugin, Orchestrator, or application policy in
  markup;
- using markup to replace ordinary C++ models and event handlers;
- rasterizing the entire interactive window into one design-time bitmap;
- claiming pixel identity across arbitrary browsers, fonts, hosts, scales, and
  color profiles.

## First falsifier

The first corpus is the Sapphire Dusk + House Composite Folder surface from the
latest File Manager concept atlas at a 1450 x 850 logical client. The experiment
must include:

- the title, menu, command shelf, navigation row, tree, object field, selection
  pane, status line, and path matrix;
- default, hover, pressed, keyboard focus, selected, disabled, collapsed, and
  popup-visible state captures where applicable;
- exact stable-ID and generated-handle inventory;
- source diagnostics for deliberately unsupported atlas constructs;
- browser and native bounds traces;
- pinned-font screenshot comparison with text and non-text error separated;
- generated-code size, compile time, cold construction, retained node count,
  first layout, and first paint measurements.
- source lines touched, handwritten C++ extension lines, and elapsed
  reconciliation effort for representative visual revisions.

## Implementation opening gate

Compiler implementation may begin only after:

1. Language Profile 0.1 is owner-approved and fail-closed.
2. GUI.Forms supplies or accepts a versioned control/property/layout/style
   metadata manifest rather than private-header discovery.
3. Stable ID, component instance, virtual-item, and event-handle rules are
   closed.
4. The fidelity oracle names browser/profile, font bytes, logical scale,
   tolerance, and screenshot method.
5. Numeric resource/complexity limits and rejection diagnostics are recorded.
6. The first atlas slice has a mapping table with no silent fallback.
7. Generated-output packaging and GUI.Forms' C++17-compatible public generation
   seam are closed; the compiler language is selected by ADR-002.
