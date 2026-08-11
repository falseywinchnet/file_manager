# ADR-016: File Manager Web.Forms authoring and GUI.Forms runtime

Status: **accepted for implementation and M4 dogfood**.

Date: 2026-08-10.

Owner approval: the grand architect explicitly directed File Manager
implementation to begin, asked that the HTML version be inspected on the M4,
permitted supporting repository extensions, and stated that prior blockers are
cleared.

## Question

Should File Manager choose Web.Forms or GUI.Forms for its frontend, and where
may HTML/CSS participate in the shipped application?

## Constraints

- **GIVEN:** File Manager is local native software; its frontend is disciplined
  C++ and must not bundle a web engine.
- **GIVEN:** JavaScript and a runtime DOM/CSS engine are excluded.
- **GIVEN:** the GUI is retained, not immediate-mode.
- **OBSERVED:** GUI.Forms provides the native retained controls, host,
  renderer, semantics, and input lifecycle.
- **OBSERVED:** Web.Forms accepts a strict browser-valid HTML/CSS source profile
  and emits public GUI.Forms C++ through a two-stage, fail-closed compiler.
- **MEASURED:** the manual native demoboard proved mechanics but did not match
  the approved browser composition closely enough.
- **MEASURED:** the Web.Forms compiler suite passes 27 tests; the GUI.Forms M4
  baseline passes 62 tests; the named FM0 installed native consumer passes.

## Candidates

### A. Hand-author the complete GUI.Forms tree

This uses the native runtime directly but repeats the visual drift already
observed in the manual demoboard and weakens browser/native correspondence.

### B. Ship the HTML in a browser or embedded browser engine

This makes browser behavior a product runtime dependency and violates the
accepted native-runtime boundary.

### C. Compile Web.Forms source into a native GUI.Forms application

Keep HTML/CSS as inspectable design/build source. Validate it through the
bounded Web.Forms profile, emit exact public C++ controls and recipes, attach
domain models and commands in handwritten frontend C++, and ship only the
native retained GUI.Forms result.

## Decision

Choose C.

The checked-in Web.Forms document is the visual/structural source for admitted
static application composition. Web.Forms Stage 1 emits versioned IR; Stage 2
consumes only that IR and emits generated C++ into the build tree. Any
unsupported required projection rejects the build. Generated output is never
hand-edited.

GUI.Forms remains the sole product UI runtime. Frontend-owned C++ attaches
filesystem, Orchestrator, Engine, settings, command, and operation state to
generated typed handles or to explicitly admitted public dynamic controls.
HTML identity cannot authorize domain work, and GUI.Forms cannot acquire
filesystem or service policy.

Browser rendering on the M4 is a visual authoring/reference lane. It is not a
runtime test substitute. Promotion requires both compiler tests and native
GUI.Forms dogfood, with raster correspondence recorded as measured or openly
unmeasured.

## Failure modes and tests

- Unsupported CSS, missing stable IDs, script, inline SVG structure, or an
  inexact native capability fails the compiler.
- Generated C++ must compile against only the named installed GUI.Forms FM0
  package and public headers.
- Application startup must work without Python, HTML, CSS, a browser process,
  or network access.
- A browser screenshot alone cannot close native input, accessibility,
  lifecycle, or raster gates.
- A native control added outside the authored composition must have a named
  application-data projection reason and public GUI.Forms API.

## Rejected options

- A is **REJECTED as the sole composition route** because its measured visual
  result drifted from the approved browser source. It remains valid for dynamic
  application-owned controls unavailable in the bounded authoring profile.
- B is **REJECTED** because it violates the native-runtime and no-bundled-web-
  engine constraints.

## Reversal path

The frontend can replace Web.Forms with another build-time authoring compiler
if that route emits the same public retained GUI.Forms surface and passes the
same fail-closed and visual evidence. GUI.Forms can be replaced only through a
separate architecture decision because it owns the admitted native lifecycle.
No authored HTML/CSS state is user data.
