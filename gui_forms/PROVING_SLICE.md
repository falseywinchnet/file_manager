# GUI.Forms gallery proving slice

Status: implementation acceptance contract for the first demo.

## Deliverable

A native macOS application named **GUI.Forms Gallery**. It is a compact control
gallery comparable in purpose to the Modern.Forms control-gallery sample. It
must not browse files or depend on File Manager.

## Required visible behaviors

- one main window with a classic menu/toolbar or command strip;
- a left category/navigation region and right demonstration surface, or another
  equally legible compact gallery arrangement;
- button default/hover/pressed/disabled states;
- editable text field with visible focus and actual keyboard input;
- checkbox and mutually exclusive radio controls;
- slider changing a progress/value display;
- grouped controls and a list/tree-like retained collection;
- one custom instrument or live-value surface;
- a theme/backplane area proving transparent controls above a style plane;
- a diagnostics toggle which reveals current counters without opening another
  process;
- window resizing which exercises top-level flex and sub-panel layout;
- no perpetual repaint when idle.

Diagnostics must identify the Gallery's Portsmouth Rapids title/control role
and its Lucida Grande field-content role.

## Required structured instrumentation

At minimum expose snapshot counters for:

- control count and stable IDs;
- measure, arrange, and paint passes;
- controls/display chunks rebuilt;
- requested and painted damage area;
- full-window versus partial paints;
- input events, focus transitions, and activations;
- update-scope depth and flush count;
- frame/present duration and worst observed duration;
- renderer name and CPU-only capability declaration.

The live diagnostics UI and machine-readable output must consume the same
snapshot object. A command-line or environment-controlled JSON/line output is
acceptable for the spike.

## Automated gates

1. retained mutation changes only declared dirty state;
2. nested update scopes coalesce to the outer boundary;
3. paint-only mutation does not force measurement;
4. layout mutation updates arranged geometry before hit testing/paint;
5. hit testing selects the topmost eligible retained control;
6. press/release produces one activation only when release remains eligible;
7. checkbox and radio state transitions are deterministic;
8. slider change updates the bound progress/value state;
9. disabled controls neither focus nor activate;
10. stable IDs resolve gallery controls and remain unique;
11. a headless or offscreen raster smoke test produces non-empty pixels;
12. idle instrumentation proves no continuous render loop;
13. lower build and tests run without repository-root sources.

## Final manual inspection

Root launches the application after automated gates pass, inspects the window
and accessibility/UI state, exercises several controls and resizing, observes
instrumentation, captures a screenshot for evidence, and closes the application
cleanly. No app process may remain running afterward.
