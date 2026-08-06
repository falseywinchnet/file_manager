# M11h complete showcase, public text, collections, and popups

Date: 2026-08-05
Status: **MEASURED PARTIAL** — experimental public control breadth, not a 1.0
support declaration.

## Objective

Build a second GUI.Forms demonstration independent of the File Manager mockup,
then use it as a live and headless conformance board. The proving surface must
exercise reusable public controls and shared renderer/host behavior; a
demo-private visual substitute does not close a framework row.

## Implemented and measured

- Public animation timeline with eight easing curves, delay, iteration,
  direction, exact endpoints, retained deadlines, pause, and hidden-surface
  quiescence.
- Public visual variants for Button, CheckBox, RadioButton, TrackBar, and
  ProgressBar, including marquee and pulse animation.
- Public Label explicit-line and word wrapping, horizontal/vertical alignment,
  and bounded line spacing.
- Public single-line TextBox with grapheme-safe directional selection,
  renderer-metric hit testing, captured drag, selected-glyph repaint,
  horizontal viewport, UTF-16 replacement ranges, placeholder/read-only state,
  Undo/Redo, and deadline caret blink.
- Skia mixed-script fallback runs shared by draw and measure. Native dogfood
  renders Latin combining text, Greek, Japanese, and emoji in one TextBox.
- Public ListBox with single/extended selection, range/toggle keys, activation,
  bounded visible rows, top index, wheel movement, and mutation remapping.
- Public noneditable ComboBox over a Window-owned tokenized popup controller.
  Popup focus is contained and restored; selection commit, outside dismissal,
  Escape, and automatic owner hide/detach/dispose revocation are bounded.
- Public `NumericUpDown` composes TextBox editing with retained spinner buttons,
  finite range/increment, decimal and hexadecimal formatting, invalid
  intermediate preservation, Enter restore, Up/Down/wheel/pointer stepping,
  and ordered value publication.
- Public horizontal and vertical `ScrollBar` controls with proportional thumb,
  arrow/page input, continuous captured drag, keyboard/wheel control,
  deadline-driven hold repeat, cleanup, and native-neutral range semantics.
- Public `PictureBox` consumes window-owned generational images and implements
  Normal, StretchImage, AutoSize, CenterImage, and Zoom, plus bounded opacity,
  intrinsic image semantics, and stale-ID rejection. Image removal was found to
  leave cached display chunks replayable; the registry boundary now retires all
  paint chunks on unscoped removal.
- Public `TabControl`/`TabPage` provides retained page ownership, mutation and
  disposal reconciliation, ordered selection, four alignments, three
  appearances, pointer/keyboard/Ctrl+Tab traversal, per-page focus memory,
  hidden-page isolation, and stable semantic tab actions. This framework family
  does not alter File Manager's independent no-tabs product decision.
- Public `CheckedListBox` retains checked/mixed state independently from
  selection, with deliberate and CheckOnClick pointer policies, Space toggle,
  mutation remapping, cancellable/modifiable pre-change and post-commit events,
  row adornment extension, checked-index projection, and checkable virtual
  semantics.
- Public renderer-free `Timer` delivers registration-ordered UI-thread ticks
  from the Window deadline queue, coalesces lateness without catch-up bursts,
  remains active during render occlusion, and deterministically revokes on
  stop, component disposal, and Window shutdown. A disabled timer adds no wake.
- Public nonvisual `ToolTip` owns tokenized per-control text, hover/focus and
  explicit-show policies, initial/reshow/auto-pop deadlines, edge-clamped
  pointer/target placement, moving-target reposition, input-transparent
  retained overlays, semantic tooltip nodes, multiple providers, and owner
  revocation. Accessible target relations and the Help/Error providers remain
  open.
- Separate twelve-page `GUI.Forms Complete Showcase` native app and a headless
  interaction fixture that traverses all pages and exercises slider drag,
  linked progress, split collapse/restore, animation quiescence, TextBox edit
  and history, ListBox state, ComboBox popup lifecycle, UI Timer cadence and
  quiescence, and retained ToolTip lifecycle.
- Renderer-free semantic snapshots with stable control identities, roles,
  names, string/numeric values, bounds, enabled/focused/selected/checked/mixed/
  read-only/expanded/busy states, actions, deterministic JSON, compound-control
  boundaries, and virtual ListBox rows.
- Default semantic adapters for Label, GroupBox, Button, CheckBox, RadioButton,
  LinkLabel, TextBox, ListBox, ComboBox, TrackBar, ProgressBar, NumericUpDown,
  and SplitContainer. Semantic actions execute the same reusable control
  behavior as pointer/keyboard input and retain UI-thread enforcement.
- An AppKit publisher maps the renderer-free graph to NSAccessibility roles,
  values, bounds, focus, setters, press/select, range, and popup actions. Its
  generation-keyed stable-ID reconciliation preserves virtual element identity
  across mutations and removes detached popup rows.

## Exact gates

Native build and all tests:

```sh
cmake --build build -j 8
ctest --test-dir build --output-on-failure
```

Result after adding the timing/tooltip provider round:
**42/42 pass** on the
local macOS/AppleClang build.

Fresh renderer-free build:

```sh
cmake -S . -B /tmp/gui-forms-core-only.R5LDMK \
  -DGUI_FORMS_ENABLE_SKIA=OFF \
  -DGUI_FORMS_ENABLE_MACOS_HOST=OFF \
  -DGUI_FORMS_ENABLE_WINDOWS_HOST=OFF \
  -DGUI_FORMS_BUILD_GALLERY=OFF \
  -DGUI_FORMS_BUILD_TESTS=ON
cmake --build /tmp/gui-forms-core-only.R5LDMK --target \
  gui_forms_core_tests gui_forms_basic_controls_tests \
  gui_forms_input_controls_tests gui_forms_range_controls_tests \
  gui_forms_tab_control_tests gui_forms_semantic_tests \
  gui_forms_frame_scheduler_tests gui_forms_timer_tests \
  gui_forms_tooltip_tests -j 8
ctest --test-dir /tmp/gui-forms-core-only.R5LDMK \
  -R 'gui_forms_(core|basic_controls|input_controls|range_controls|tab_control|semantic|frame_scheduler|timer|tooltip)_tests' \
  --output-on-failure
```

Result: core, controls, and tests build without Skia or native hosts; **9/9
focused tests pass**.

An Address/Undefined Sanitizer build at `/tmp/gui-forms-asan.FtGdfq` ran the
Timer, ToolTip, and complete-showcase interaction binaries cleanly. AppleClang
reported that leak detection is unsupported on this platform; ASan/UBSan
memory and undefined-behavior checks completed without findings.

## Native dogfood observations

- All twelve pages render without the prior fixed-child scaling overflow.
- Label wrapping removed sidebar, container-copy, and coverage clipping.
- TextBox accepted `Hello Ω GUI.Forms`, painted continuous drag selection,
  replaced the selected range, and restored it through Undo.
- Mixed Japanese and emoji initially painted tofu; the shared Skia fallback
  correction removed it. This negative observation is retained because it
  changed renderer behavior rather than demo content.
- ListBox initially scrolled preselected items to row zero before first arrange;
  visibility reconciliation moved to arrange and the initial viewport is now
  stable.
- ComboBox popup paints above ordinary page/status content, commits through a
  real pointer click, detaches, and restores owner focus.
- NumericUpDown decimal spinner input updates the value, formatted editor,
  selection, and live event status in one retained transaction.
- Horizontal and vertical scroll bars render within their retained bounds;
  native value automation synchronizes the public pair, and headless dragging
  updates before pointer release.
- The image page visibly distinguishes all five sizing modes and three opacity
  levels. AppKit publishes eight stable image elements with intrinsic `128 x 80`
  values, and the clipped drawing-composition board remains renderer-neutral.
- The tab page visibly distinguishes top, bottom, left, and right alignment and
  normal, button, and flat appearances. AppKit selection of the Security tab
  swapped the retained page, selected state, status copy, and native semantic
  children in one transaction.
- Direct selected-page disposal initially removed the child without dirtying
  its live parent's retained layout. `Window::dispose_subtree` now marks that
  structural parent mutation, fixing reconciliation for every composite family.
- The initial tab dogfood sequence intentionally left a pointer down unmatched;
  this exposed that TabControl selected on down but did not acknowledge its
  captured release. The public control now closes the press lifecycle on up; an
  Address/Undefined Sanitizer showcase build was clean.
- The checked-collection page visibly distinguishes selection highlight from
  check and mixed glyphs. Native accessibility press on `Semantic press`
  selected the row, toggled it on, and updated the ordered status text.
- The timing page visibly drove retained status, progress, and a moving target
  from a public UI Timer. Live AppKit automation changed its interval from 250
  to 500 ms, observed the next published tick at 500 ms, stopped it to zero
  wakeups, and resumed it by page re-entry.
- Persistent ToolTip dogfood initially placed the popup at the client origin
  because programmatic show had no pointer coordinate. The provider now tracks
  target anchoring separately from pointer placement; live inspection shows
  the tooltip beneath its button, and a regression test rejects origin fallback.
- The first AppKit semantic attempt exposed only the canvas because its virtual
  elements were regenerated per query. Retaining and reconciling them by stable
  ID exposed 38 page-one controls. Live automation then changed a checkbox,
  typed mixed-script text through the normalized host path, set and linked a
  slider/progress pair, selected ListBox rows, committed and detached a ComboBox
  popup row, and set a named NumericUpDown value without duplicate editor nodes.

## Honest open boundary

This is not “all WinForms” and not toolkit 1.0. Still open include multiline,
password, clipboard command binding and full IME/preedit TextBox behavior;
editable/data-bound/owner-drawn ComboBox and large data sources; scroll bars,
  date controls, menus/tool strips, tree/list/grid, tab/layout families,
validation, posted-dispatcher breadth, editable semantic
text ranges, UI Automation/AT-SPI publishers, DML/C ABI projections, independent native secondary windows,
Windows/Wine showcase dogfood, Linux host work, and sustained performance/soak.
