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

- **OBSERVED DESIGN CORRECTION, 2026-08-06:** the board still contained eight
  local `Control` subclasses (`LayoutPanel`, `LayoutGroup`, `Surface`,
  `EasingBoard`, `DiagnosticsCard`, `DrawingEffectsBoard`, `TimerMotionBoard`,
  and `ShowcaseRoot`). That was invalid framework evidence: reusable layout,
  painting, scheduling, diagnostics, and lifetime behavior existed only in the
  demonstration. All eight were removed. GUI.Forms now publicly owns
  `ScaledPanel`, `ScaledGroupBox`, `DrawingSurface`, `EasingPreview`,
  `MetricsView`, and `Control::Tag`; timer motion is ordinary public-control
  composition. A CMake source-policy gate rejects all showcase-local class or
  struct inheritance, including default-private inheritance.
- Public-control promotion passes focused ownership/layout/drawing/metrics/
  easing tests, the complete-showcase interaction suite, native 48/48,
  renderer-free 36/36, strict Win64 compilation, and the Wine host-services
  smoke. Live AppKit dogfood visually verified the rebuilt public
  `EasingPreview`, `MetricsView`, `DrawingSurface`, scaled shell, and moving
  timer target. No demo-only control behavior remains.

- Public animation timeline with eight easing curves, delay, iteration,
  direction, exact endpoints, retained deadlines, phase-preserving
  `pause(time)` / `resume(time)`, repeated-transition idempotence, and
  hidden-surface quiescence.
- Public visual variants for Button, CheckBox, RadioButton, TrackBar, and
  ProgressBar, including marquee and pulse animation.
- Progress animation now consumes a public orthogonal `MotionPolicy`:
  application enablement, user pause, and reduced-motion accommodation are
  retained independently and applied in one lease transaction. Determinate
  values remain live; reduced motion stays animated at low cadence, slower
  accumulated phase, and limited excursion; pause and disable relinquish the
  continuous-frame subscription.
- Public font specifications carry bounded renderer-neutral letter spacing.
  HarfBuzz applies it between shaped clusters and across fallback runs, while
  CoreText and GDI consume the same logical value. Small Portsmouth control text
  uses an optical-spacing policy rather than demo-private string changes.
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
- Host protocol v5 adds bounded semantic sound cues (`notification`, `success`,
  `warning`, `error`, and `operation_complete`) with gain validation,
  deterministic coalescing/mute counters, renderer-free headless traces, and an
  AppKit adapter. The showcase exposes explicit cue buttons and preserves the
  same visual meaning when sound is disabled; it adds no automatic hover noise.
- Public `DateTimePicker` owns validated Gregorian values and ranges, four
  formatting modes with a caller-owned provider, optional checkbox and spinner
  forms, a retained calendar popup, contained focus, keyboard/pointer/semantic
  commit and cancel, stable virtual date-cell semantics, semantic optional-value
  activation, range-aware semantic month buttons, and synchronous popup cleanup.
  Its disposal gate exposed FIFO resource revocation as incorrect; component-owned
  work now disconnects in reverse acquisition order.
- Separate fifteen-page `GUI.Forms Complete Showcase` native app and a headless
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

Result after the typography, sound, reduced-motion, split-surface,
DateTimePicker, and reverse-revocation round: **45/45 pass** on the local
macOS/AppleClang build.

Fresh renderer-free build:

```sh
cmake -S . -B build-renderer-free-polish \
  -DGUI_FORMS_ENABLE_SKIA=OFF \
  -DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=OFF \
  -DGUI_FORMS_ENABLE_MACOS_HOST=OFF \
  -DGUI_FORMS_ENABLE_WINDOWS_HOST=OFF \
  -DGUI_FORMS_BUILD_GALLERY=OFF \
  -DGUI_FORMS_BUILD_TESTS=ON
cmake --build build-renderer-free-polish -j 8
ctest --test-dir build-renderer-free-polish --output-on-failure
```

Result: core, controls, and tests build without Skia, HarfBuzz, or native hosts;
**34/34 tests pass**.

An Address/Undefined Sanitizer build at `/tmp/gui-forms-asan.FtGdfq` ran the
Timer, ToolTip, and complete-showcase interaction binaries cleanly. AppleClang
reported that leak detection is unsupported on this platform; ASan/UBSan
memory and undefined-behavior checks completed without findings.

## Native dogfood observations

- All fifteen pages render without the prior fixed-child scaling overflow.
- The dates board proves exact long/short/time/custom formatting, an owned French
  provider, optional and spinner forms, disabled state, bounded date stepping,
  and retained calendar commit/cancel. Native accessibility toggles the optional
  value and drives enabled month navigation; a single-month range publishes dimmed,
  disabled month buttons and rejects activation.
- The host-services board drives all five common-dialog request families,
  cancellation preservation, clipboard, monitor geometry, and the complete
  semantic sound-cue set through the public host boundary. Live AppKit proves
  physical Escape cancellation and owner restoration; Wine proves native
  MessageBox/ChooseColor cancellation and exact modal cleanup. Evidence:
  `experiments/M11H_DIALOGS_AND_HOST_SERVICES.md`.
- The states board now drives the public UI dispatcher without executing work
  inside its click callback. Wine records one `A/B/C` FIFO snapshot followed by
  nested `D` on the next host turn (`posted=4`, `invoked=4`, `pending=0`), then
  a separately posted operation cancelled before execution (`cancelled=1`,
  `faulted=0`), then a worker `Invoke` completing on the UI thread
  (`synchronous_invocations=1`, `marshalled_invocations=1`). AppKit separately
  proves worker-originated async, nested, and synchronous callbacks execute on
  the UI thread and the original synchronous fault returns to the worker. Evidence:
  `experiments/M11H_UI_DISPATCHER.md`.
- Label wrapping removed sidebar, container-copy, and coverage clipping.
- TextBox accepted `Hello Ω GUI.Forms`, painted continuous drag selection,
  replaced the selected range, and restored it through Undo.
- Mixed Japanese and emoji initially painted tofu. A shared, role-independent
  fallback registry plus exact-hash Noto Sans CJK JP removed Japanese tofu.
  Current Noto COLRv1 then shaped the rocket with zero missing clusters but
  painted no pixels in the admitted CPU Skia/FreeType build, so that candidate
  is **REJECTED** for this pack. Pinned Noto Emoji Regular 1.05 monochrome
  renders real fallback ink. The HarfBuzz test proves Japanese and rocket
  coverage across control/content/monospace roles without tripling face bytes;
  the Skia smoke separately fails if either fallback shapes without raster ink.
  Live AppKit crop inspection proves `á`, Greek, Japanese, and the rocket in
  one field. The strict Win32 adapter now uses Uniscribe itemization,
  `ScriptShape`/`ScriptPlace`, and glyph-index output over its privately loaded
  faces; a Wine capture proves Japanese plus the monochrome rocket, while a
  gated content-free trace requires actual Noto face selection, a non-default
  astral glyph, and successful drawing. These negative observations are
  retained because they changed renderer and pack behavior rather than demo
  content.
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
- The split-container board initially dragged the real seam while fixed demo
  background children obscured pane allocation. Pane-owned paint removed that
  false surface: live drag now visibly reallocates both panels before release.
- Reduced motion initially stopped Motion Lab and left pause/global-live in
  conflicting states, requiring repeated toggles. A later live pass disproved
  the shallow `next_wake` assertion, and AppKit scheduled wakes were
  default-run-loop-mode only. Live-off now disables the page command without
  destroying the independent pause latch; Live-on restores the exact retained
  transport state, every compound state writes one coherent status, and AppKit
  registers its wake in common modes. Eight consecutive pause/resume cycles must
  advance rendered marker geometry; the native host test cancels and replaces
  an active-surface lease and requires autonomous post-rearm ticks. The first
  reduced-motion substitute still advanced in visible 4 Hz jumps, which made
  otherwise-correct pause/resume transitions look broken. The intermediate
  policy replaced it with one wake-free retained frame and passed an exact
  stability gate. **REJECTED 2026-08-06:** direct user observation established
  that this made “reduced” functionally indistinguishable from “stopped.” The
  later corrective policy is recorded below; this negative result is retained
  rather than rewritten as success.
- **OBSERVED then MEASURED FIX, 2026-08-05:** the easing board's phase retention
  hid a second defect in the reusable `ProgressBar`: showcase pause called
  `set_animation_enabled(false)`, which reset phase, and re-registration rebased
  the first resumed frame to zero. Reduced motion also overwrote the model phase
  with its representative value. `ProgressBar` now keeps feature enablement,
  pause, and reduced presentation as three orthogonal states. Pause revokes the
  lease without changing phase; resume advances from that phase; reduced motion
  paints a stable `0.5` substitute without mutating the retained phase. Focused
  tests cover the first resumed frame, eight repeated lease replacements, exact
  active-surface counts, and phase preservation across pause/reduced/full
  transitions. Live AppKit stress dogfood completed ten full-motion, eight
  reduced-mode, and eight master-gate transitions, then demonstrated autonomous
  frame movement after a single reduced-mode exit.
- **OBSERVED then MEASURED CORE/HOST FIX, 2026-08-05:** foreground dogfood still
  reported incoherent pause/resume behavior. The showcase had accumulated its
  own phase instead of consuming the public timeline, changed ProgressBar pause
  and reduced flags in two lease-producing calls, froze reduced mode on a
  random frame, and allowed back/elastic markers to escape the board. Motion Lab
  now consumes the reusable pause/resume timeline and applies ProgressBar
  policy atomically,
  presents reduced mode at an exact `0.5` phase, publishes the phase readout,
  and bounds overshoot geometry. AppKit uses one retained re-armable main-queue
  deadline source rather than destroying/recreating timer objects. Sixteen core
  pause/resume cycles, six native semantic transition cycles, autonomous native
  lease-replacement ticks, 46 native tests, 35
  renderer-free tests, strict Win64 construction, and the Wine motion/services
  sequence pass. **HYPOTHESIS:** the foreground presentation defect is closed;
  final direct user observation remains the visual acceptance gate because an
  automation client that covers the window correctly triggers host occlusion.
- **OBSERVED then MEASURED COMPOUND-POLICY FIX, 2026-08-05:** direct dogfood
  found that pause/resume was still visually order-dependent. The four-value
  enum had collapsed three independent facts; with reduced motion selected,
  Pause changed presentation from the fixed representative frame to the stale
  retained phase, and Resume changed it back. `MotionPolicy` now retains
  `enabled`, `paused`, and `reduced` independently. Pausing or resuming while
  reduced leaves geometry at exact phase `0.5`; leaving reduced while paused
  reveals the retained phase without starting a wake; only the fully active
  combination owns a frame lease. The master gate preserves the pause latch
  instead of silently rewriting it. Sixteen compound ProgressBar cycles,
  eight full showcase pause/resume cycles, and eight native AppKit
  disarm/quiescent-gap/rearm cycles pass. The rebuilt native board exposes
  `REDUCED + PAUSED` explicitly and resumes from each state with one
  unambiguous action.
- **OBSERVED then MEASURED APPKIT PRESENTATION FIX, 2026-08-06:** direct native
  dogfood disproved scheduler-only acceptance: one run emitted 7,459 active-
  surface callbacks but presented only two frames. `drawRect` synchronously
  called `collectDamage`, and AppKit retained the dirty bit without enqueueing
  the next draw requested from inside the active paint transaction. The host now
  completes presentation, rearms the retained deadline source, and collects the
  next damage from that main-queue wake. The native regression begins with a
  scheduled surface hidden, reveals it after the host has gone quiescent, then
  requires both autonomous ticks and actual paints through eight disarm/rearm
  cycles. Rebuilt-app dogfood visibly held pause at an exact phase, resumed on
  one click, held reduced motion at 50%, preserved pause/resume while reduced,
  and restored continuous motion on one reduced-mode exit.
- **OBSERVED then MEASURED REDUCED-MOTION CORRECTION, 2026-08-06:** direct user
  observation rejected the wake-free fixed frame: reduced motion was simply
  stopped. `MotionPolicy::active()` now excludes only pause and disable.
  Reduced sources retain one lease at a minimum 100 ms cadence; ProgressBar
  advances at `0.35` phase speed and admitted reduced presentations use a
  centered half-width excursion. Reduced animated progress retains busy
  semantics. Focused tests fail on a frozen reduced frame, prove real geometry
  and phase advancement, prove pause stability/no-wake behavior, and repeat the
  compound master/reduced/pause transition matrix. Live AppKit comparison
  measured 45% → 62% over 650 ms in reduced mode, an unchanged 37% across a
  650 ms reduced+paused interval, and 58% after one Resume. Native 47/47,
  renderer-free 36/36, strict Win64, and Wine motion/services gates pass. Wine
  now sees active surfaces in reduced mode and zero only while paused. The
  rebuilt showcase remains open on the Animation page for direct inspection.
- **OBSERVED USER ACCEPTANCE, 2026-08-06:** direct inspection accepted the Dates
  & Calendar page visually. No calendar-specific change was made in this round.
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
- The ranges page now dogfoods six public `ProgressBar` styles: blocks,
  continuous, marquee, slow luminance pulse, marching stripes, and laser etch.
  The latter three are reusable control behavior with the same retained motion
  policy and semantic node, not page-local painting. Fresh native inspection
  observed active marquee movement, clipped stripe marching, and the laser's
  repeated phase field and leading edge while the accessibility tree remained
  stable.
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
bidi/culture, history coalescing, accessible editable ranges, and full
IME/preedit/candidate-geometry TextBox behavior;
editable/data-bound/owner-drawn ComboBox and large data sources; advanced
calendar/date culture breadth, menus/tool strips, tree/list/grid, remaining
tab/layout families,
validation, synchronous invoke/synchronization-context projection, editable semantic
text ranges, UI Automation/AT-SPI publishers, DML/C ABI projections, independent native secondary windows,
Windows HarfBuzz/FreeType raster integration and showcase dogfood, Linux host
work, and sustained performance/soak.
