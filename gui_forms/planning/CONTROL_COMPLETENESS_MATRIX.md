# GUI.Forms control completeness matrix

Status: **compatibility and delivery inventory; not a promise that every listed
WinForms object will be implemented**. Date: 2026-08-03.

This matrix turns the WinForms catalogue and retired compatibility specimen evidence into dependency and
verification work. It does not equate a class name, constructor, or visual
resemblance with support. GUI.Forms preserves familiar calls and expected
outcomes where admitted, while documenting corrections rather than preserving
known-bad ordering or Windows implementation accidents.

**GIVEN:** the call surface observed in the authoritative current retired compatibility specimen specimen
and its admitted plugins is presumptively admitted. Each such call must become
supported, be supplied by a named optional package, or carry an explicit
incompatibility and migration path. This admission does not include ActiveX,
browser hosting, printing, MDI, obsolete control families, or undocumented
HWND/WndProc accidents.

**GIVEN:** captured `System.Drawing` calls are now part of the implementation
program under the portable **GUI.Drawing** owner. A generated `System.Drawing`
compatibility facade may preserve the managed names, but the installed .NET
drawing service is an oracle/scaffold, not the implementation. See
`GUI_DRAWING_REVISION_PLAN.md`.

## 1. Status and priority vocabulary

Implementation status for each individual type eventually must be one of:

- `unclassified` — inventoried but not yet admitted or excluded;
- `planned` — admitted by an approved scope/ADR, not implemented;
- `experimental` — usable only by named proving applications;
- `partial` — shipped only when every unsupported behavior is enumerated;
- `supported` — passes all applicable completeness gates;
- `package` — supported by a named optional package, not the base library;
- `excluded` — deliberately absent with reason and replacement/migration path.

The priorities below are **CANDIDATE** delivery order, not architecture
decisions:

- **P0 substrate** — needed by every serious control and by cross-language use;
- **P1 general/SDR daily surface** — ordinary retained Forms applications;
- **P2 complex application surface** — binding, grid, advanced hosting;
- **P3 breadth** — broader plugin and general Forms familiarity;
- **P4 package or explicit decision** — legacy, platform-host, specialized, or
  high-cost behavior not admitted merely because WinForms contained it.

## 2. Completeness gate for one family

A family is `supported` only when it has all applicable items:

1. reusable public control/component/model independent of the gallery;
2. manifest entry with properties, defaults, events, unsupported behavior, and
   deliberate compatibility deviations;
3. construction, initialization, component ownership, visual parentage,
   reparenting, and deterministic disposal tests;
4. layout, paint, style state, resource, scaling, and damage tests;
5. pointer, keyboard, focus, command, validation, and event-order traces;
6. text/IME/culture tests where text is displayed or edited;
7. default semantic-hook adapter and platform accessibility tests where
   applicable, without making metadata mandatory for construction;
8. DML and imperative construction equivalence plus generated C++/C# coverage;
9. headless, macOS, Windows, Wayland, and X11 conformance or a documented honest
   platform difference;
10. approved performance workloads for virtual/high-rate controls;
11. no backend/platform type in the public control contract;
12. documentation and an example that uses only public APIs.

## 3. Dependency-ordered matrix

Evidence locators abbreviate:

- `WF` — `planning/gui_forms/WINFORMS_CONTROL_INVENTORY.md`;
- `SDR` — `planning/gui_forms/retired compatibility specimen_COMPATIBILITY_INVENTORY.md`;
- `GF` — the current `gui_forms/` proving slice and tests.

### 3.1 GUI.Drawing substrate

The pinned catalogue has 353 drawing rows: 307 required static-IL rows across
34 types and 46 deferred metadata-only rows. A family is complete only when its
managed facade identity, native behavior, renderer result, lifetime, limits,
and applicable platform qualification pass. Interface presence or a .NET
passthrough is not support.

| Order | Family | Required rows | Priority | Present state | Exit evidence |
|---:|---|---:|---|---|---|
| D0 | value geometry, ARGB/named/system-role color | 130 | P0 | partial native equivalents; managed behavior currently runtime-supplied | property/value oracle, checked geometry, color conversion and role tests |
| D1 | brushes, pens, fonts, string formats, stock resources | 38 | P0 | passthrough | deterministic ownership/disposal, clone/stock-object rules, text-service binding |
| D2 | graphics targets, state stack, clip, transform, quality, measurement | 25 | P0 | narrow native recorder plus passthrough | renderer-free command traces, state/clip/transform oracle, bounded stacks |
| D3 | primitive/path/text/icon/image draw operations | 33 | P0 | rectangle/line/text/image subset native; remainder missing | canonical overload normalization, raster probes/goldens, damage integration |
| D4 | paths, matrices, regions and hit testing | 31 | P0 | missing | geometry oracle, transform/bounds/hit tests, complexity limits |
| D5 | linear/path gradients, color blends and hatches | 15 | P1 | missing | stop/wrap/hatch fixtures, alpha/color-space policy, renderer parity |
| D6 | images, bitmaps, pixels, locks, codecs, thumbnails | 24 | P0 | PNG presentation subset native; mutation passthrough | owned CPU pixels, stride/lock rules, format trace, decoder limits/fuzzing |
| D7 | image remap and color-matrix adjustment | 11 | P1 | missing | clone/lifetime tests and adjusted-image raster oracle |
| D8 | HDC/HWND/HBITMAP compatibility surface lease | 7 cross-cutting | P1/host | passthrough | Windows/Wine handle round trips; explicit capability result elsewhere |

The detailed required operation groups, boundaries, and M11e–M11h order are in
`GUI_DRAWING_REVISION_PLAN.md`. D8 overlaps D2/D6 and is not added to the 307-row
total.

File Manager breadth is tracked separately as candidate FMD0–FMD8 oversight:
complete vector/path and region algebra, stroke/paint vocabulary, compositing
and bounded effects, image formats/metadata/color management, thumbnail
primitives, text/glyph integration, pixel surfaces, and recording/inspection.
Those families prevent an SDR-only implementation from becoming an accidental
ceiling; individual operations become required only through a named workload or
decision record. Printing, metafiles, desktop capture, and design-time drawing
editors are not admitted by this oversight.

### 3.2 Forms controls and components

| Order | Family and types | Priority | Minimum behavioral center if admitted | Dependencies | Exclusion/defer boundary | Evidence and required tests |
|---:|---|---|---|---|---|---|
| 0 | Component/object lifetime: `Component`, container/site concepts, visual controls and nonvisual components | P0 | Strong identity; component owner distinct from visual parent; ordered child collection; reparent; parent-strong/child-weak ARC; detached lifetime; idempotent disposal; event/timer/dispatch revocation; stable IDs | UI thread, handles, events, diagnostics | Do not infer GC or .NET lifetime; leaked hostile-plugin references require process policy | **GIVEN** ownership; **OBSERVED** SDR §7.1/§18, WF §9, GF `Control`. Test duplicate IDs, cycles, callback disposal, detached objects, stale work/handles, partial construction |
| 1 | Base `Control` and custom-control contract | P0 | Requested/arranged/client geometry; visible/enabled inheritance; focusability/tab metadata; name/tag/text; style/font/cursor; z-order; coordinate conversion; invalidation; measure/arrange/paint/input extension | lifetime, renderer vocabulary, events, style, host coordinates | **MEASURED PARTIAL ABI 0.19:** bounded arrow/text/hand/crosshair/resize/wait/forbidden roles project from generated cursor singletons into the inherited retained cursor model and Win32 target selection. Strict HWND, `WndProc`, `CreateParams`, or undocumented message emulation is not implied | **OBSERVED** SDR §5.1/§7, WF §4.2/§9, GF headers; **MEASURED** native cursor contract and host/Wine facade round trip. Test inheritance, z-order, conversion, invalidation declaration, reparent/dispose in callback |
| 2 | Property metadata, initialization, and change events | P0 | Typed property registry; default/reset/serialization metadata; bounded invalidation effects; `BeginInit`/`EndInit`; ordered synchronous change events; generated binding metadata | component model, event tokens, DML registry | No opaque observer-everywhere graph; no silent undeclared mutation | **GIVEN** invalidation rule; **OBSERVED** SDR §8.4/§19. Test default/reset, init batching, event order, undeclared dev failure and production fallback |
| 3 | Retained invalidation, display chunks, damage, buffering, active surfaces | P0 | Typed measure/arrange/paint/text/style/semantic dirtiness; nested update scopes; coalesced/compacted regions; cached chunks; clipped z order; isolated CPU bitmap surfaces; no idle frame loop | base control, scheduler, resources, GUI.Drawing, CPU renderer, metrics | No GPU and no immediate-mode reconstruction; codecs remain a bounded GUI.Drawing resource service, not display-chunk logic | **GIVEN**; **OBSERVED** SDR §6/§9 and GF. Test 10k mutations, theme batch, overlapping damage, 30 Hz bitmap band, idle zero paints |
| 4 | Geometry, preferred size, Dock/Anchor, margins/padding, scaling | P0 | Bounds/client/min/max/preferred/autosize; all dock directions; compound anchors; margin/padding/baseline; logical scale and pixel snapping | layout scheduler, text measurement, host scale | **MEASURED PARTIAL M11h:** `ScrollableControl.DockPaddingEdges` now owns and projects edge mutations into retained `Padding`; fill children relayout immediately on host and Wine. Pixel-identical Windows metrics are not evidenced; define tolerance | **OBSERVED** SDR §7.2/§8, WF §9; **MEASURED** dock-padding projection/inset/relayout fixture. Golden nested geometry, hidden children, min/max conflicts, font/scale platform matrix |
| 5 | Layout batching/read barriers | P0 | Nested suspend/resume/perform; committed geometry in scopes; minimal read barriers; bounded reentrant passes; pass diagnostics | property effects, layout families, metrics | A no-op compatibility shim is insufficient; DML reorder policy remains explicit | **GIVEN/CANDIDATE L4**; **OBSERVED** GF partial. Test designer construction, nested scopes, dynamic insertion/removal, read during scope, pass limit |
| 6 | Input routing, focus, validation, commands, capture | P0 | Internal preview/target/bubble; Forms events; focus scopes/active control; tab/mnemonics/default/cancel; validation; capture/hover/press; wheel/double click; safe handler mutation | tree/lifetime, host input, scheduler, semantics | **MEASURED PARTIAL M11h:** the renderer-free core owns bounded nested focus scopes, stable Tab traversal/restoration, UI-thread enforcement, and an owner-tokenized root overlay controller. Public ComboBox proves contained popup focus, outside-pointer dismissal, commit, restoration, automatic owner revocation, and AppKit semantic focus/action publication. C ABI projection, mnemonics, validation, UIA/AT-SPI focus publication, generalized screen-edge placement, and remaining oracle ordering are open | **GIVEN** broad familiarity; **MEASURED** `experiments/M11H_RETAINED_FOCUS_SCOPES.md`, `experiments/M11H_COMPLETE_SHOWCASE_TEXT_COLLECTIONS.md`, form-semantics, and physical dialog-key fixtures. Oracle traces for nested popups, Space/Enter, mnemonic, validation, double-click, handler disposal/modal |
| 7 | Unicode text, caret, selection, clipboard, undo, IME | P0 | Typed UTF-8/cluster positions; shaping/fallback; caret/selection; committed/preedit text; candidate geometry; clipboard; undo; read-only/password/multiline; bidi/culture | font/text service, focus, host IME, semantics | **MEASURED PARTIAL M11h:** public `TextBox` now owns grapheme-safe directional selection, renderer-metric click/drag hit testing, captured range selection, clipped selected glyph repaint, horizontal viewport, read-only/placeholder states, UTF-16 replacement ranges, bounded undo/redo, and deadline-driven caret blink. The Skia raster uses matching mixed-script fallback runs for paint and measurement; Japanese/emoji are live-dogfooded. Clipboard commands, composition/preedit overlays, word/line navigation, multiline/password/bidi, history coalescing, and accessible ranges remain open | **GIVEN** staged correctness; **MEASURED** 766-case grapheme corpus, ABI field probes, public input-control tests, native showcase dogfood, and `experiments/M11H_COMPLETE_SHOWCASE_TEXT_COLLECTIONS.md`. Test complex shaping, composition/candidate geometry, clipboard commands, password leakage, bidi/multiline |
| 8 | Style roles, owner draw, resources, localization | P0 | Named relational roles; material/state recipes; inherited appearance; owner measure/draw through GUI.Drawing; explicit image/font lifetime; bounded codec service; message IDs/fallback | GUI.Drawing D0–D7, renderer chunks, text, theme/language packs | Plugins cannot override host controls/style; no pixel-identical Windows theme promise; admitted codecs require named bounds | **GIVEN**; **OBSERVED** SDR §7.3/§14. Test all states, theme batch, missing resource/string, image scale/alpha, owner-measure/draw order and zero-passthrough cutover |
| 9 | Semantic hook, tooltip/help/automation | P0 | Stable semantic IDs; default stock adapters; role/name/value/state/action/relations/order/bounds/text ranges; independent tooltip/help/accessibility/test consumers; virtual children | stable IDs, focus, localization, host publishers | **MEASURED PARTIAL M11h:** renderer-free snapshots now publish deterministic hierarchy/JSON, stock roles/names/string and numeric values/bounds/states/actions, virtual ListBox rows, compound boundaries, UI-thread enforcement, and semantic action routing. The AppKit publisher reconciles native virtual elements by stable ID and live automation proves choice, text, range, list, combo-popup, and numeric actions. Relations, editable text ranges, tooltip/help consumers, UIA/AT-SPI, and AT conformance remain open | **GIVEN**; **MEASURED** semantic tests, 38-test suite, fresh six-test renderer-free build, and `experiments/M11H_COMPLETE_SHOWCASE_TEXT_COLLECTIONS.md` |
| 10 | Dispatcher, timers, invocation, shutdown | P0 | UI-thread ownership; sync/async marshal; ordered input/layout/paint/timers; cancellable UI timer; deadline wakeup; shutdown revokes pending work | native/headless host, lifetime, callbacks | Unsafe cross-thread access rejected; nested `DoEvents` is a separate porting decision | **OBSERVED** SDR §11; GF `next_wake` is empty. Test invoke order, wrong-thread mutation, timer dispose race, callback disposal, shutdown, idle |
| 11 | `ScrollableControl`, `ContainerControl`, `UserControl`, `Form` | P1 | Active-control/validation container; reusable composition/load; owned/top-level/modal windows; accept/cancel; close cancellation/reason; state/chrome; auto-scroll | rows 0–10, host/modal, scale | **MEASURED PARTIAL M11h:** owned relationships are bidirectional and cycle-safe; modal entry suppresses its owner and restores prior focus; retained-hosted modeless close is cancellable and raises one close event with reason. Independent modeless native top-level windows, richer chrome/state, validation, and complete auto-scroll remain open. MDI P4; per-pixel alpha unresolved | **OBSERVED** SDR §5.1/§12, WF §4.2; **MEASURED** host/Wine form and secondary-form fixtures. Test owned/modal focus restore, nested modal, close cancel, load once, reparent, scroll extent |
| 12 | Basic containers: `Panel`, `GroupBox` | P1 | Child clipping/borders/caption; radio grouping; dock/anchor; scrolling variants and optional wheel suppression | base, layout, text, scrolling input | Split/tab are separate later families | **OBSERVED** SDR §5.1/§8. Test nested clipping, child mutation, radio group boundary, scroll changes, focus traversal |
| 13 | `FlowLayoutPanel`, `TableLayoutPanel` | P1 | Flow direction/order/wrap/break/autosize; table absolute/percent/auto rows/columns, spans, cell insertion/lookup, convergence | layout batching, preferred size, visibility | Not CSS Flexbox; gallery layout is not conformance | **OBSERVED** SDR §5.1/§8.3. Golden spans, percent/auto conflict, hidden child, runtime tracks, wrap thresholds, RTL |
| 14 | `Label`, `LinkLabel` | Label P1; LinkLabel P3 | Alignment/autosize/ellipsis/mnemonic target; optional multiple link spans, visited/enabled state, hit testing | text, input, semantics, layout | **MEASURED PARTIAL M11h:** public `Label` preserves explicit breaks and provides word wrapping, per-line horizontal alignment, vertical alignment, validated line spacing, UTF-8 scalar width fallback, non-interactive hit behavior, and default static-text semantics; LinkLabel publishes link/visited/press behavior. Ellipsis, mnemonic relations, renderer-authored exact wrap metrics, and multiple LinkLabel spans remain open | **OBSERVED** SDR §5.2, WF §4.3; **MEASURED** public control/semantic tests and complete-showcase clipping/automation correction. Test ellipsis, mnemonic, exact fallback-cluster wrapping, link spans/navigation/accessibility |
| 15 | `ButtonBase`, `Button`, `CheckBox`, `RadioButton` | P1 | Pointer/keyboard/mnemonic activation; synchronous click/check order; default/cancel; image/text; tri-state; radio exclusion by logical container | input/commands, style, semantics, group/container | **MEASURED PARTIAL M11h:** generated `Button` implements `IButtonControl`; programmatic/default/cancel activation raises Click before applying a validated dialog result, and default-state paint invalidates deterministically. Mnemonics, full image/text layouts, tri-state breadth, and remaining keyboard ordering stay open | **OBSERVED** SDR §5.2/§10, WF §4.1/§4.3; **MEASURED** M11h accept/cancel ordering fixture. Windows trace plus corrected-order manifest; disabled, handler disposal/reparent |
| 16 | `TextBoxBase`, `TextBox` | P1 | Complete row-7 editor; single/multiline, read-only/password, scroll, validation, key/text events | text, input, scrolling, binding, semantics | **MEASURED PARTIAL M11h:** reusable public single-line `TextBox` covers Unicode selection/edit/history, pointer capture, keys, replacement ranges, viewport, placeholder/read-only, scheduled caret behavior, named value/read-only semantics, and AppKit set/focus actions. Multiline, password, clipboard command binding, composition overlay, validation, word navigation, data binding, and editable semantic text ranges remain open; rich/masked behavior stays separate | **OBSERVED** SDR custom textbox, WF §4.1/§4.3; **MEASURED** public headless/semantic tests, native host automation dogfood, and showcase conformance. Full editor corpus, focus/validation, composition, clipboard, disposal |
| 17 | `ListControl`, `ListBox`, `ComboBox` | P1 | Stable items; selection modes; collection mutation; display/value binding; dropdown commit/cancel/focus; type search/autocomplete; owner draw | item model, popup host, text, scrolling, binding | **MEASURED PARTIAL M11h:** public `ListBox` provides bounded row realization, stable top index, single/extended selection, toggle/range keyboard semantics, wheel traversal, activation, mutation index remap, and selectable/pressable semantic virtual rows. Public noneditable `ComboBox` uses the tokenized root popup controller with contained focus, keyboard/open state, outside dismissal, commit, owner revocation, and detached-popup semantic cleanup. Binding/display members, editable combo, type search/autocomplete, owner draw, horizontal scrolling, and stable model IDs beyond index-backed items remain open | **OBSERVED** SDR §5.2; **MEASURED** public input/semantic tests, complete-showcase conformance, and native automation dogfood. Test large data source, insert identity, editable/type search, owner draw, binding refresh, accessibility |
| 18 | `UpDownBase`, `NumericUpDown` | P1 | Composite edit/spinner; min/max/increment; decimal/hex/culture; intermediate edit validation; commit/cancel; acceleration | text editor, commands, culture, binding | **MEASURED PARTIAL M11h:** public `NumericUpDown` composes public TextBox editing with retained spinner buttons; finite ordered range, increment, decimal/hex formatting, intermediate invalid preservation, Enter restore, Up/Down, wheel, pointer step, boundary clamp, post-synchronization value events, and one compound numeric semantic node with min/max/set/increment/decrement pass headless and native dogfood. Culture-aware formatting/parsing, acceleration/repeat capture, thousands separator, binding/validation semantics, DML/C ABI projection, and DomainUpDown remain open | **OBSERVED** SDR custom numeric, WF §4.1/§4.3; **MEASURED** input/semantic/showcase conformance plus native spinner/automation dogfood. Culture/intermediate corpus, repeat acceleration, disposal, accessibility |
| 19 | `ScrollBar`, `HScrollBar`, `VScrollBar`, custom range base/`ColorSlider`, `ProgressBar` | P1 | Effective range; small/large changes; orientation; keyboard/wheel; drag capture; Scroll/Value order; determinate progress; accessible value | capture, layout, scheduler, semantics | **MEASURED PARTIAL M11h:** public `TrackBar` has classic/filled/compact paint and continuous captured dragging. Public `HScrollBar`/`VScrollBar` add proportional thumbs, arrows, page regions, continuous captured drag, keyboard/Home/End/Page, wheel, deadline-driven hold repeat, detach/focus cleanup, and set/increment/decrement scroll-bar semantics. Public `ProgressBar` has blocks/continuous/marquee/pulse, bounded active-surface deadlines, pause, hidden-surface quiescence, numeric value, and busy state. WinForms `Scroll` event-type parity, DML/C ABI projection, and sustained 30/60 Hz damage measurements remain open | **OBSERVED** SDR §5.2/§6; **MEASURED** range, animation, semantic, showcase headless tests, and live AppKit automation. Add event-type oracle, disabled/reentrant repeat, sustained cadence |
| 20 | `PictureBox` and image presentation | P1 | Explicit image ownership; normal/stretch/autosize/center/zoom; alpha/scale/color policy; disposal | GUI.Drawing image resource, layout, renderer | **MEASURED PARTIAL M11h:** public `PictureBox` consumes window-owned generational `ImageId` values and implements deterministic Normal, StretchImage, AutoSize, CenterImage, and aspect-preserving Zoom geometry, border-aware layout, bounded opacity, noninteractive hit policy, intrinsic image semantics, and stale-ID rejection. Registry removal now retires all retained paint chunks so no renderer can replay a dead ID. Remote loading/general media remain excluded; animation, error/initial images, DPI interpolation policy, DML/C ABI projection, and consumer-scoped removal are open | **OBSERVED** SDR §5.2, WF §4.3; **MEASURED** basic-control/showcase tests, 38-test native suite, six-test renderer-free build, and live AppKit visual/AX dogfood. Add replace/remove stress, malformed/large images, DPI scaling, interpolation oracle |
| 21 | `DateTimePicker` | P1 | Date value/format/culture; dropdown editor; focus/keyboard lifecycle; optional checkbox/nullable semantics when declared | text, popup, modal focus, binding, culture | `MonthCalendar` separate P3; platform visual identity not required | **OBSERVED** SDR §5.2/§13. Commit/cancel, format/culture, dropdown focus, DataGrid edit host |
| 22 | `ContextMenuStrip`, `ToolStripMenuItem`, `ToolStripSeparator`, base item tree | P1 | Owned item tree; measure/layout; submenu; shortcut/mnemonic; check; image; opening/closing order; screen-edge placement; disposal | popup host, focus/commands, owner draw, semantics | Full strip merge/rafting/overflow later; plugins cannot restyle host | **MEASURED PARTIAL M11g:** owned item trees, nested lifetime, checked/image paint, physical keyboard traversal, click-away/Escape, cancellable hover-open, edge reversal, and repeated teardown pass. Dynamic opening mutation, mnemonic/shortcut collision, full semantics, and dispose-during-callback remain open |
| 23 | `ToolTip`, `HelpProvider`, `ErrorProvider` | ToolTip P1; others P3 | Per-control attached values; delay/show/hide; hover/focus policy; accessible relations; validation error association | timers, semantic graph, popup/overlay, localization | **MEASURED PARTIAL M11h:** public nonvisual `ToolTip` owns tokenized per-control mappings, initial/reshow/auto-pop deadlines, hover and keyboard-focus policies, explicit persistent/duration show, target-anchored and pointer placement with edge clamping, input-transparent retained overlays, semantic `tool_tip` nodes, moving-target reposition, multi-provider coexistence, and synchronous owner/provider disposal cleanup. Accessible described-by relations, localization refresh, title/icon/balloon variants, `HelpProvider`, and `ErrorProvider` remain open | **OBSERVED** SDR ToolTip; WF §6. **MEASURED** delay/cancel, moving/disposed target, keyboard focus, multiple-provider, renderer-free, sanitizer, headless-showcase, and live AppKit dogfood |
| 24 | Common dialogs and modal services: `OpenFileDialog`, `SaveFileDialog`, `FolderBrowserDialog`, `ColorDialog`, `MessageBox` | P1 | Owned request/result contract; filters/default extension/initial selection; cancel preservation; overwrite/error; platform adapter | form/modal host, UTF paths | Pixel-level Windows dialog emulation unnecessary; font/print dialogs P4 | **OBSERVED** SDR §5.3/§12. Fake adapters for every result; owner reactivation; nested dialog; shutdown; platform smoke |
| 25 | `Timer`, early nonvisual component model | P1/P0 | UI-loop tick; interval/start/stop; deterministic disposal and no post-dispose callback | dispatcher, component owner, scheduler | **MEASURED PARTIAL M11h:** public renderer-free `Timer` uses the Window deadline queue without a hidden visual control, preserves registration order and phase, coalesces late ticks without bursts, continues UI callbacks while paint is occluded, restarts on interval mutation, enforces UI-thread mutation, and revokes on stop/component disposal/window shutdown. Disabled timers publish no wake. General posted dispatcher work, synchronization-context projection, DML/C ABI, and background-worker semantics remain open | **OBSERVED** SDR §5.3/§11. **MEASURED** fake-clock ordering, stop/restart, dispose race, shutdown, occlusion, no-idle-wake, renderer-free, sanitizer, showcase, and live AppKit cadence tests |
| 26 | `BindingSource`, binding/currency/format-parse substrate | P2 | Type-erased descriptors; current item; add/remove/edit; list/property changes; format/parse/update modes; errors | events, components, dispatcher, control properties | Immutable view-model replacement must not be required; managed reflection is not native contract | **OBSERVED** SDR §5.3/§13, WF §6/§8/§9. Currency, insert/delete, property change, parse failure, suspension, source disposal |
| 27 | `DataGridView` and columns/cells/edit controls | P2 | Explicit columns; row/cell model; style inheritance; selection/sort; formatting; virtualization; hosted edit lifecycle; custom cell cloning; binding errors | binding, text/date/combo, focus, scrolling, semantic virtual children | Painted table is not compatibility; legacy `DataGrid` P4 | **OBSERVED** SDR §5.3/§13, WF §5.3. Calendar edit enter/dirty/validate/commit/cancel; row removal; custom clone; large virtual data |
| 28 | `BackgroundWorker` | P2 | Start/cancel/progress/completion; background exception and UI marshaling; owner teardown | dispatcher, cancellation, component lifetime | Unsafe worker mutation of controls rejected | **OBSERVED** SDR §5.3. Progress order, cancellation race, exception, owner disposal, shutdown |
| 29 | Trusted custom controls and owner-render extension | P1 substrate/P2 laboratory | Schema registration; typed properties/effects; custom measure/chunks/hit/input; retained bitmap updates; semantic virtual nodes; initialization/disposal | all P0 systems, DML registry, resource API | Prefer ported public behavior over Win32 message emulation | **OBSERVED** SDR §6/§17. Port ColorSlider, FrequencyEdit, owner-drawn combo/numeric/text, waterfall; fault in paint/timer/dispose |
| 30 | `TreeView`, node model | P3; File Manager may promote | Stable node ownership/reparent; expand/check/select; images; keyboard; label edit; virtual realization; semantic children | virtualization, text edit, scrolling, images, semantics | Only weak SDR theme evidence; File Manager demand is separate | **OBSERVED** WF §4.3/§5.2; weak SDR §5.4. Reparent/expand/check order, label edit, million-node model, keyboard/accessibility |
| 31 | `ListView`, item/subitem/column/group/insertion models | P3; File Manager may promote | View-independent item IDs; views/columns/subitems/groups; selection/check; label edit; virtual retrieve/cache/search; insertion mark | virtual collections, text, images, scrolling, semantics | Not proven by gallery list rows | **OBSERVED** WF §4.3/§5.2; weak SDR §5.4. Million rows, mode switches, retrieval bounds, edit, selection across mutation |
| 32 | `SplitContainer`, `SplitterPanel`, legacy `Splitter` | P1 legacy splitter/P3 split container; File Manager FM0 | Physical splitter; min sizes; fixed/collapsed panels; orientation; keyboard/pointer resize; nested layout | layout/capture, semantics | **MEASURED PARTIAL M11h:** public retained split composition now has stable panel/seam IDs, separate 3-pixel paint/9-pixel hit geometry, constrained vertical/horizontal allocation, live captured drag, keyboard resize, focus transfer, collapse/restore, and fixed-second resize. Generated facade behavior passes host and physical Wine. Automatic/user-collapse distinction, persistence, nested stress, DML/C ABI, and native semantics remain open | **OBSERVED** SDR Splitter; WF §4.3; **MEASURED** `experiments/M11H_RETAINED_SPLIT_CONTAINER.md`. Min/collapse/fixed, resize feedback, scale, accessibility |
| 33 | `TabControl`, `TabPage` | P3 framework; excluded from File Manager shell unless separately admitted | Page identity/collection; selection; keyboard traversal; hidden-page layout/focus; owner draw | containers, focus, text/images, semantics | **MEASURED PARTIAL M11h:** the independently reusable framework now has retained page ownership, insertion/removal/direct-disposal reconciliation, ordered selection events, top/bottom/left/right headers, normal/button/flat appearances, pointer/arrows/Home/End/Ctrl+Tab traversal, per-page focus memory, hidden-page paint/hit/focus/semantic exclusion, and stable semantic tab actions. This does not reopen File Manager's separate no-tabs product decision. Overflow/multiline headers, images, tooltips, owner draw, RTL, DML/C ABI, and WinForms oracle ordering remain open | **GIVEN** framework showcase breadth; **OBSERVED** WF; **NOT OBSERVED** SDR; **MEASURED** dedicated tab tests, complete-showcase headless/live AppKit dogfood, 39-test suite, and seven-test renderer-free build |
| 34 | `ToolStrip`, `MenuStrip`, `StatusStrip`, hosted items, `ToolStripContainer/Panel/ContentPanel`, `BindingNavigator` | P3 | Item ownership; overflow/dropdowns/hosted controls; split buttons; shortcuts/merge if admitted; strip layout/rafting; status spring; binding navigation | row 22, layout, binding, popup/focus/semantics | Not current SDR core. A ribbon is a separate composition, not automatically ToolStrip | **OBSERVED** WF §4.3/§5.1; indirect SDR. Test overflow, hosted editor focus, split activation, dynamic reparent, binding currency |
| 35 | `CheckedListBox` | P3 framework | Checked state independent of selection; check-on-click; keyboard; item-check cancellable/order; binding/owner draw | ListControl, input, semantics | **MEASURED PARTIAL M11h:** public `CheckedListBox` extends the retained ListBox row pipeline with independent checked/unchecked/indeterminate state, default deliberate two-click and CheckOnClick policies, Space toggle, selection-preserving mutation remap, cancellable/modifiable pre-change event, post-commit event, check glyph paint, checked-index projection, and checkable virtual rows/actions. AppKit publishes checkbox rows including mixed value `2`, and live semantic press selects and toggles one row. Binding, owner draw, stable item IDs beyond indices, DML/C ABI, and complete WinForms ItemCheck oracle ordering remain open | **GIVEN** framework showcase breadth; **OBSERVED** WF; not evidenced by SDR; **MEASURED** dedicated tests, complete-showcase headless/live AppKit dogfood, 40-test suite, and eight-test renderer-free build |
| 36 | `MaskedTextBox` | P3 | Mask provider; prompt/literal behavior; incomplete/invalid input; culture; validation and commit | full text editor, culture, binding | No native Windows edit dependency | **OBSERVED** WF. Test masks with IME/clipboard/undo/culture and invalid states |
| 37 | `DomainUpDown` | P3 | Ordered domain; editable/noneditable text; wrap; acceleration; selection/value events | up-down/text/list model | Not evidenced by SDR | **OBSERVED** WF. Test mutation, wrap, typed lookup, event order, culture |
| 38 | `MonthCalendar` | P3 | Culture calendar; selection ranges; keyboard; bold dates; multi-month layout as admitted | date/culture, popup, semantics | Not evidenced by SDR; DateTimePicker can use native/custom popup independently | **OBSERVED** WF. Culture/range/keyboard/scale/accessibility suite |
| 39 | `TrackBar` stock facade | P3 | Forms-shaped range/orientation/ticks/keys/events over range substrate | row 19 | SDR uses custom range control; no need to copy platform pixel style | **OBSERVED NOT USED** SDR; WF. Oracle range/event tests |
| 40 | `ImageList`, `NotifyIcon` | ImageList P3 when consumed; NotifyIcon P4/platform package | Key/index images with scale/color variants and lifetime; tray icon/menu/events if admitted | resources, host shell, menu | `ImageList` not observed in SDR; tray is OS integration not core visual control | **OBSERVED** WF §6; SDR negative. Test key/index mutation/scale; platform tray lifecycle separately |
| 41 | `PropertyGrid`, descriptor/editor/component-editor infrastructure | P4/tooling candidate | Descriptor/category/editor/reset/default; nested values; collection editors; commit/cancel; serialization | metadata, binding, grid/tree, modal editors, localization | Not SDR core. Designer need does not automatically put PropertyGrid in runtime core | **OBSERVED** WF; SDR negative. Test descriptor order, invalid edit, reset, nested, localization, DML round trip |
| 42 | `RichTextBox` | P4/package | Styled document model; RTF import/export scope; links/protected ranges; selection/clipboard/undo/IME/accessibility | mature text document/editor, scrolling, parser security | Not SDR core; no full RTF promise without separate threat model | **OBSERVED** WF; SDR negative. Corpus/fuzz/round-trip and editor tests only after admission |
| 43 | GUI.Forms-native docking family | unadmitted parent-use candidate | If separately admitted: nested dock/floating/hidden/auto-hide panes; drag indicators; stable persist IDs; layout save/load; teardown/rebuild | forms/split/top-level/popup/input/serialization | **GIVEN** not an retired compatibility specimen compatibility obligation and **REJECTED** as a base control; third-party docking internals do not enlarge the facade | **OBSERVED** concepts in SDR §15. Require a separate parent-product admission record before implementation |
| 44 | Map and satellite view adapters | unadmitted parent-use candidate | If separately admitted: replaceable hosted view appropriate to approved application use | custom control, input, semantic, capability policy | **GIVEN** current map toolkit is not an retired compatibility specimen Forms obligation; **REJECTED** as base control; no implicit network/web capability | **GIVEN/OBSERVED** SDR §16/WF satellites. Separate license/network audit and admission required |
| 45 | Legacy Forms: `DataGrid`, `Splitter`, `StatusBar`, `ToolBar`, legacy `Menu` hierarchy | P4 facade/excluded | Only selected migration facade over modern corrected behavior | modern equivalents | Do not recreate obsolete implementation quirks absent a named corpus | **OBSERVED** WF §4.4/§5.4. Require an approved consumer and oracle per family |
| 46 | MDI: `MdiClient` and Form MDI behavior | P4/excluded by default | Multi-document parent/child activation/menu/window arrangement only if admitted | forms, native windows, menus, focus/modal | Current product direction does not require it; SDR negative | **OBSERVED** WF only. Separate ADR and native-host tests required |
| 47 | Printing: `PrintPreviewControl/Dialog`, `PrintDialog`, `PageSetupDialog`, printing pipeline | P4 package | Page lifecycle, setup, preview/zoom/navigation, platform print contract | document renderer, dialogs, platform print APIs | Not SDR core; large platform/security surface | **OBSERVED** WF only. Separate package and print oracle required |
| 48 | Browser/interop hosts: `WebBrowser`, `WebBrowserBase`, `WebView2`, `AxHost`, `ElementHost`, arbitrary ActiveX | excluded from base | None in base GUI.Forms | external runtimes/platform embedding | Browser engine forbidden in core; ActiveX universe unbounded; WPF/.NET interop contradicts native core. Could exist only as explicitly external host packages | **OBSERVED inventory, not requirement**. No placeholder type should imply support |
| 49 | Satellite controls: chart/report/ink/Power Packs shapes/DataRepeater/PrintForm | P4 package/unclassified | One separately specified object model and behavior contract per package | varies | Microsoft-era existence is not base compatibility scope | **OBSERVED** WF §8. Require owner decision, threat model, oracle, packaging per package |
| 50 | Classic non-Forms component tray: directory services/search, event log, performance counter, process, file watcher, message queue, serial, service controller, data set | excluded from GUI.Forms base | None; these are application/OS/data services | none | Not widgets and not framework UI responsibilities. A visual designer may host metadata for external application components without implementing the service | **OBSERVED** WF §7. Do not count as control completeness |

### 3.3 File Manager consumer-promotion rows

These rows do not change captured WinForms/SDR accounting. They promote
reusable behavior required by the first-party File Manager consumer. Detailed
requirements and milestone mapping live in
`FILE_MANAGER_CONSUMER_CAPABILITY_PROFILE.md`.

| ID | Priority | Promoted capability | Minimum behavioral center | Existing substrate | Exit evidence |
|---|---|---|---|---|---|
| FM1 | FM0 | Custom chrome and window participation | host caption/system behavior plus authored title content; key/secondary/drop-proximate/deactivated states; min geometry; scale/monitor transitions | rows 1, 6, 10, 11; M3 | headless trace plus AppKit/Win32/Linux chrome, snap, focus, drag, accessibility smokes |
| FM2 | FM0 | Priority-responsive shell and panes | content-aware shrink; large-text reflow; priority collapse; split/collapse grips; one-scroll-plane inspector | rows 4, 5, 11–13, 32; M5 | geometry corpus at narrow/short/150x150, 100–225% text, 1–2x display, keyboard collapse |
| FM3 | FM0 | Command shelf/ribbon | shared commands; tabs/groups/captions; large/small/split/hosted items; key tips; overflow/collapse | rows 6, 15, 22, 34 | ribbon/menu/context/shortcut/accessibility share one command trace |
| FM4 | FM0 | Breadcrumb/path instruments | segmented path; in-place editor; adornments; async cancellable suggestions; anchored recent-stack overlay | rows 7, 16, 17, 22; M4/M5/M7 | deep/malformed/mixed-script path edit/resolve/cancel/focus/semantic/popup corpus |
| FM5 | FM0 | File object field | stable model across icon/list/details/tile; virtual tree/list; spatial navigation; selection; label edit; decorations | rows 7, 14, 29–31; M5/M7 | million items, view switches, sort/mutation preservation, keyboard/accessibility/rename |
| FM6 | FM1 | Inspection and preview | lightweight property list; editable values; preview host; one scroll plane; factual tooltip/popover | rows 16, 20, 23, 29, 41 | copy/drag/focus/collapse/edit/error/unavailable with semantic and resource bounds |
| FM7 | FM1/FM2 | Transient/operation surfaces | popup, expandable row, toast, drawer, hierarchical progress, task dialog, validation | rows 19, 22–25, 29; M5–M9 | focus restore, timeout/pin, variable height, cancel/retry, reduced motion, announcements |
| FM8 | FM1 | Bidirectional external transfer | outbound/inbound drag; operation negotiation; preview/tree/object/window targets; autoscroll/hover-expand; clipboard | rows 6, 10, 29–31; M3/M12 | synthetic peers plus physical cross-window/cross-app tests on every host |
| FM9 | FM0 | Owned typography | HarfBuzz; FreeType; bundled Portsmouth/body/fallback packs; metric generations; no silent host-font fallback | row 7; M4/M9 | multilingual cluster/caret/IME/accessibility corpus, pinned profiles, parser audit/fuzz |
| FM10 | FM0 | Native accessibility/accommodations | always-present semantics, default adapters, task order, virtual children, text ranges, UIA/NSAccessibility/AT-SPI, large text/high contrast/reduced motion | row 9; M8/M12 | headless snapshots plus Narrator/VoiceOver/Orca/keyboard primary scenarios |
| FM11 | FM0/FM1 | House material and assets | paths, clips, gradients/textures, masks, bounded shadow/layers, nine-patch, glyph runs, images, precompiled vectors, trace/export | GUI.Drawing D0–D7/FMD0–FMD8; M2/M9/M11 | material lab for every state/scale/contrast; command trace, limits, raster/damage budgets |
| FM12 | FM1 | Bounded motion and sound | interruptible transitions; reduced-motion substitution; semantic sound IDs; no button/menu/hover sound | rows 3, 10; M2/M3/M9 | fake-clock damage/idle-zero, cue coalescing, muted equivalence, host smoke |

## 4. Hosted model completeness

The following objects are first-class compatibility surface even when they do
not inherit `Control`. They must not be hidden behind an owner's constructor:

| Owner | Hosted models to represent if owner is supported | Minimum contract |
|---|---|---|
| ToolStrip/menu/status | `ToolStripItem`, dropdown item/control-host bases, buttons, labels, status labels, separators, menu items, dropdown/split buttons, hosted combo/text/progress, overflow items | independent identity/ownership, measurement, enabled/visible/checked state, input/events, overflow/dropdown/focus, semantics |
| TreeView | `TreeNode`, collections | parent/tree ownership, expand/check/select, images, cloning/serialization policy |
| ListView | items/subitems, columns, groups, insertion mark, collections | stable view-independent identity, retrieval/virtualization, label edit, groups/columns/images |
| DataGridView | bands/rows/columns/cells/styles/collections/editing interfaces | clone semantics, style inheritance, selection, virtualization, edit host, data errors, semantic virtual children |
| Legacy facades | toolbar buttons, status panels, menu items, grid table/column styles | only if their owner family is separately admitted |

## 5. Cross-cutting contracts

No control family may locally reinvent these behaviors:

| Contract | Authoritative subsystem |
|---|---|
| ownership, disposal, weak events | retained kernel/component model |
| property defaults/effects/serialization | typed property registry and DML schema |
| requested vs arranged geometry, transactions | layout kernel |
| focus, commands, capture, activation, modal | event/focus engine |
| Unicode positions, shaping, editing, IME | text engine and host IME adapter |
| color/material/state recipes | immutable style system |
| PNG/font/string lookup and fallback | resource/language services |
| role/value/action/text ranges | retained semantic-hook graph and native publishers |
| invalidation, display chunks, damage | retained renderer contract |
| UI timers/invoke/shutdown | host scheduler/dispatcher |
| C/C++/C# identity and callbacks | versioned C ABI and generated wrappers |

## 6. Compatibility priorities and corrections

- **GIVEN:** native DML ordering is explicit and deterministic. A WinForms
  facade may emulate selected dock/order outcomes, but known-bad behavior is not
  the native default.
- **GIVEN:** ordinary authors receive Forms-like events. Preview/target/bubble
  remains internal/custom-control machinery.
- **GIVEN:** authored accessibility metadata is optional, but every stock
  control supplies a tested default semantic adapter. File Manager's native
  publishers are available without a user-facing off switch.
- **REJECTED:** “painted approximately like the widget” as a compatibility
  criterion.
- **REJECTED:** one universal `GalleryControl` switch as the reusable library.
- **REJECTED:** calling DockPanel, maps, browsers, printing, MDI, ActiveX, or
  classic component-tray services base controls because they appeared in a
  designer ecosystem.
- **OPEN:** whether independent GUI.Forms ships `TabControl` despite File
  Manager's deliberate no-tabs product design.
- **OPEN:** whether broad P4 catalogue coverage is an eventual mission or a
  permanent manifest of deliberate exclusions.

## 7. Recommended implementation order within controls

1. Complete P0 kernel contracts before adding reusable widgets.
2. Extract `Panel`, `Label`, `Button`, `CheckBox`, and `RadioButton` from the
   gallery as real consumers of P0 behavior.
3. Complete the text editor and range substrate before TextBox/Numeric/Combo and
   slider facades multiply incomplete input behavior.
4. Complete layout/scrolling/virtualization before Tree/List/Grid.
5. Complete command/popup/modal models before menus, strips, combo dropdowns,
   and common dialogs.
6. Complete binding descriptors/currency before DataGridView.
7. Add P3 families only against a named application or completeness objective.
8. Require a separate approval and package boundary for every P4 family.
