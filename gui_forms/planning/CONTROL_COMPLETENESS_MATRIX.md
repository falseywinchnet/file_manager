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

| Order | Family and types | Priority | Minimum behavioral center if admitted | Dependencies | Exclusion/defer boundary | Evidence and required tests |
|---:|---|---|---|---|---|---|
| 0 | Component/object lifetime: `Component`, container/site concepts, visual controls and nonvisual components | P0 | Strong identity; component owner distinct from visual parent; ordered child collection; reparent; parent-strong/child-weak ARC; detached lifetime; idempotent disposal; event/timer/dispatch revocation; stable IDs | UI thread, handles, events, diagnostics | Do not infer GC or .NET lifetime; leaked hostile-plugin references require process policy | **GIVEN** ownership; **OBSERVED** SDR §7.1/§18, WF §9, GF `Control`. Test duplicate IDs, cycles, callback disposal, detached objects, stale work/handles, partial construction |
| 1 | Base `Control` and custom-control contract | P0 | Requested/arranged/client geometry; visible/enabled inheritance; focusability/tab metadata; name/tag/text; style/font/cursor; z-order; coordinate conversion; invalidation; measure/arrange/paint/input extension | lifetime, renderer vocabulary, events, style, host coordinates | Strict HWND, `WndProc`, `CreateParams`, or undocumented message emulation is not implied | **OBSERVED** SDR §5.1/§7, WF §4.2/§9, GF headers. Test inheritance, z-order, conversion, invalidation declaration, reparent/dispose in callback |
| 2 | Property metadata, initialization, and change events | P0 | Typed property registry; default/reset/serialization metadata; bounded invalidation effects; `BeginInit`/`EndInit`; ordered synchronous change events; generated binding metadata | component model, event tokens, DML registry | No opaque observer-everywhere graph; no silent undeclared mutation | **GIVEN** invalidation rule; **OBSERVED** SDR §8.4/§19. Test default/reset, init batching, event order, undeclared dev failure and production fallback |
| 3 | Retained invalidation, display chunks, damage, buffering, active surfaces | P0 | Typed measure/arrange/paint/text/style/semantic dirtiness; nested update scopes; coalesced/compacted regions; cached chunks; clipped z order; isolated CPU bitmap surfaces; no idle frame loop | base control, scheduler, resources, CPU renderer, metrics | No GPU and no immediate-mode reconstruction; non-PNG decode is outside core | **GIVEN**; **OBSERVED** SDR §6/§9 and GF. Test 10k mutations, theme batch, overlapping damage, 30 Hz bitmap band, idle zero paints |
| 4 | Geometry, preferred size, Dock/Anchor, margins/padding, scaling | P0 | Bounds/client/min/max/preferred/autosize; all dock directions; compound anchors; margin/padding/baseline; logical scale and pixel snapping | layout scheduler, text measurement, host scale | Pixel-identical Windows metrics are not evidenced; define tolerance | **OBSERVED** SDR §7.2/§8, WF §9. Golden nested geometry, hidden children, min/max conflicts, font/scale platform matrix |
| 5 | Layout batching/read barriers | P0 | Nested suspend/resume/perform; committed geometry in scopes; minimal read barriers; bounded reentrant passes; pass diagnostics | property effects, layout families, metrics | A no-op compatibility shim is insufficient; DML reorder policy remains explicit | **GIVEN/CANDIDATE L4**; **OBSERVED** GF partial. Test designer construction, nested scopes, dynamic insertion/removal, read during scope, pass limit |
| 6 | Input routing, focus, validation, commands, capture | P0 | Internal preview/target/bubble; Forms events; focus scopes/active control; tab/mnemonics/default/cancel; validation; capture/hover/press; wheel/double click; safe handler mutation | tree/lifetime, host input, scheduler, semantics | Bad historic ordering is not automatically preserved | **GIVEN** broad familiarity; **OBSERVED** SDR §10 and GF subset. Oracle traces for tab, Space/Enter, mnemonic, capture drag, wheel suppression, double-click, handler disposal/modal |
| 7 | Unicode text, caret, selection, clipboard, undo, IME | P0 | Typed UTF-8/cluster positions; shaping/fallback; caret/selection; committed/preedit text; candidate geometry; clipboard; undo; read-only/password/multiline; bidi/culture | font/text service, focus, host IME, semantics | Current UTF-8 append is not an editor; no native overlay; Portmouth asset pending | **GIVEN** staged correctness; **OBSERVED** SDR §5.2/§20 and GF limit. Test combining, emoji, bidi, complex script, selection replace, composition, candidate placement, password leakage |
| 8 | Style roles, owner draw, resources, localization | P0 | Named relational roles; material/state recipes; inherited appearance; owner measure/draw; explicit image/font lifetime; PNG core service; message IDs/fallback | renderer chunks, text, theme/language packs | Plugins cannot override host controls/style; non-PNG codecs excluded; no pixel-identical Windows theme promise | **GIVEN**; **OBSERVED** SDR §7.3/§14. Test all states, theme batch, missing resource/string, PNG scale/alpha, owner-measure order |
| 9 | Optional semantic hook, tooltip/help/automation | P0 | Stable semantic IDs; role/name/value/state/action/relations/bounds/text ranges; independent tooltip/help/accessibility/test consumers; virtual children | stable IDs, focus, localization, host publishers | Metadata remains optional for construction; pixels alone are insufficient | **GIVEN**; **OBSERVED** SDR §20, WF §8/§9. Test semantic snapshots, keyboard-only use, value/focus notifications, disabled consumer independence |
| 10 | Dispatcher, timers, invocation, shutdown | P0 | UI-thread ownership; sync/async marshal; ordered input/layout/paint/timers; cancellable UI timer; deadline wakeup; shutdown revokes pending work | native/headless host, lifetime, callbacks | Unsafe cross-thread access rejected; nested `DoEvents` is a separate porting decision | **OBSERVED** SDR §11; GF `next_wake` is empty. Test invoke order, wrong-thread mutation, timer dispose race, callback disposal, shutdown, idle |
| 11 | `ScrollableControl`, `ContainerControl`, `UserControl`, `Form` | P1 | Active-control/validation container; reusable composition/load; owned/top-level/modal windows; accept/cancel; close cancellation/reason; state/chrome; auto-scroll | rows 0–10, host/modal, scale | MDI P4; per-pixel alpha unresolved package/host feature | **OBSERVED** SDR §5.1/§12, WF §4.2. Test owned/modal focus restore, nested modal, close cancel, load once, reparent, scroll extent |
| 12 | Basic containers: `Panel`, `GroupBox` | P1 | Child clipping/borders/caption; radio grouping; dock/anchor; scrolling variants and optional wheel suppression | base, layout, text, scrolling input | Split/tab are separate later families | **OBSERVED** SDR §5.1/§8. Test nested clipping, child mutation, radio group boundary, scroll changes, focus traversal |
| 13 | `FlowLayoutPanel`, `TableLayoutPanel` | P1 | Flow direction/order/wrap/break/autosize; table absolute/percent/auto rows/columns, spans, cell insertion/lookup, convergence | layout batching, preferred size, visibility | Not CSS Flexbox; gallery layout is not conformance | **OBSERVED** SDR §5.1/§8.3. Golden spans, percent/auto conflict, hidden child, runtime tracks, wrap thresholds, RTL |
| 14 | `Label`, `LinkLabel` | Label P1; LinkLabel P3 | Alignment/autosize/ellipsis/mnemonic target; optional multiple link spans, visited/enabled state, hit testing | text, input, semantics, layout | LinkLabel not evidenced by current SDR | **OBSERVED** SDR §5.2, WF §4.3. Test measure/ellipsis, mnemonic, fallback cluster, link spans/navigation/accessibility |
| 15 | `ButtonBase`, `Button`, `CheckBox`, `RadioButton` | P1 | Pointer/keyboard/mnemonic activation; synchronous click/check order; default/cancel; image/text; tri-state; radio exclusion by logical container | input/commands, style, semantics, group/container | Gallery proves only a narrow two-state/model-coupled subset | **OBSERVED** SDR §5.2/§10, WF §4.1/§4.3. Windows trace plus corrected-order manifest; disabled, handler disposal/reparent |
| 16 | `TextBoxBase`, `TextBox` | P1 | Complete row-7 editor; single/multiline, read-only/password, scroll, validation, key/text events | text, input, scrolling, binding, semantics | Rich/masked behavior separate; no source-level native message emulation | **OBSERVED** SDR custom textbox, WF §4.1/§4.3. Full editor corpus, focus/validation, dispose during composition |
| 17 | `ListControl`, `ListBox`, `ComboBox` | P1 | Stable items; selection modes; collection mutation; display/value binding; dropdown commit/cancel/focus; type search/autocomplete; owner draw | item model, popup host, text, scrolling, binding | Gallery rows are not a reusable item model | **OBSERVED** SDR §5.2. Test selection across insert/delete, keyboard, popup cancel, owner draw, data refresh |
| 18 | `UpDownBase`, `NumericUpDown` | P1 | Composite edit/spinner; min/max/increment; decimal/hex/culture; intermediate edit validation; commit/cancel; acceleration | text editor, commands, culture, binding | Domain strings are a separate P3 family | **OBSERVED** SDR custom numeric, WF §4.1/§4.3. Invalid/intermediate edits, boundary, culture, spinner capture/keys, event order |
| 19 | `ScrollBar`, `HScrollBar`, `VScrollBar`, custom range base/`ColorSlider`, `ProgressBar` | P1 | Effective range; small/large changes; orientation; keyboard/wheel; drag capture; Scroll/Value order; determinate progress; accessible value | capture, layout, scheduler, semantics | Stock `TrackBar` P3; marquee only if explicitly admitted | **OBSERVED** SDR §5.2/§6. Boundary changes, outside drag, key commands, disabled state, 30 Hz damage |
| 20 | `PictureBox` and image presentation | P1 | Explicit image ownership; normal/stretch/autosize/center/zoom; alpha/scale/color policy; disposal | PNG resource, layout, renderer | No remote loading; no general media decoder | **OBSERVED** SDR §5.2, WF §4.3. Mode geometry, replace/dispose, malformed/large PNG, high DPI |
| 21 | `DateTimePicker` | P1 | Date value/format/culture; dropdown editor; focus/keyboard lifecycle; optional checkbox/nullable semantics when declared | text, popup, modal focus, binding, culture | `MonthCalendar` separate P3; platform visual identity not required | **OBSERVED** SDR §5.2/§13. Commit/cancel, format/culture, dropdown focus, DataGrid edit host |
| 22 | `ContextMenuStrip`, `ToolStripMenuItem`, `ToolStripSeparator`, base item tree | P1 | Owned item tree; measure/layout; submenu; shortcut/mnemonic; check; image; opening/closing order; screen-edge placement; disposal | popup host, focus/commands, owner draw, semantics | Full strip merge/rafting/overflow later; plugins cannot restyle host | **OBSERVED** SDR §5.3/§14, WF §5.1. Keyboard nested menus, click-away/Escape, dynamic opening mutation, shortcut collision, dispose open |
| 23 | `ToolTip`, `HelpProvider`, `ErrorProvider` | ToolTip P1; others P3 | Per-control attached values; delay/show/hide; hover/focus policy; accessible relations; validation error association | timers, semantic graph, popup/overlay, localization | Help overlay is a separate consumer; no mandatory metadata | **OBSERVED** SDR ToolTip; WF §6. Test delay/cancel, moving/disposed target, keyboard focus, multiple providers |
| 24 | Common dialogs and modal services: `OpenFileDialog`, `SaveFileDialog`, `FolderBrowserDialog`, `ColorDialog`, `MessageBox` | P1 | Owned request/result contract; filters/default extension/initial selection; cancel preservation; overwrite/error; platform adapter | form/modal host, UTF paths | Pixel-level Windows dialog emulation unnecessary; font/print dialogs P4 | **OBSERVED** SDR §5.3/§12. Fake adapters for every result; owner reactivation; nested dialog; shutdown; platform smoke |
| 25 | `Timer`, early nonvisual component model | P1/P0 | UI-loop tick; interval/start/stop; deterministic disposal and no post-dispose callback | dispatcher, component owner, scheduler | Background work distinct from UI timer | **OBSERVED** SDR §5.3/§11. Fake-clock ordering, stop/restart, dispose race, shutdown, no idle wake when disabled |
| 26 | `BindingSource`, binding/currency/format-parse substrate | P2 | Type-erased descriptors; current item; add/remove/edit; list/property changes; format/parse/update modes; errors | events, components, dispatcher, control properties | Immutable view-model replacement must not be required; managed reflection is not native contract | **OBSERVED** SDR §5.3/§13, WF §6/§8/§9. Currency, insert/delete, property change, parse failure, suspension, source disposal |
| 27 | `DataGridView` and columns/cells/edit controls | P2 | Explicit columns; row/cell model; style inheritance; selection/sort; formatting; virtualization; hosted edit lifecycle; custom cell cloning; binding errors | binding, text/date/combo, focus, scrolling, semantic virtual children | Painted table is not compatibility; legacy `DataGrid` P4 | **OBSERVED** SDR §5.3/§13, WF §5.3. Calendar edit enter/dirty/validate/commit/cancel; row removal; custom clone; large virtual data |
| 28 | `BackgroundWorker` | P2 | Start/cancel/progress/completion; background exception and UI marshaling; owner teardown | dispatcher, cancellation, component lifetime | Unsafe worker mutation of controls rejected | **OBSERVED** SDR §5.3. Progress order, cancellation race, exception, owner disposal, shutdown |
| 29 | Trusted custom controls and owner-render extension | P1 substrate/P2 laboratory | Schema registration; typed properties/effects; custom measure/chunks/hit/input; retained bitmap updates; semantic virtual nodes; initialization/disposal | all P0 systems, DML registry, resource API | Prefer ported public behavior over Win32 message emulation | **OBSERVED** SDR §6/§17. Port ColorSlider, FrequencyEdit, owner-drawn combo/numeric/text, waterfall; fault in paint/timer/dispose |
| 30 | `TreeView`, node model | P3; File Manager may promote | Stable node ownership/reparent; expand/check/select; images; keyboard; label edit; virtual realization; semantic children | virtualization, text edit, scrolling, images, semantics | Only weak SDR theme evidence; File Manager demand is separate | **OBSERVED** WF §4.3/§5.2; weak SDR §5.4. Reparent/expand/check order, label edit, million-node model, keyboard/accessibility |
| 31 | `ListView`, item/subitem/column/group/insertion models | P3; File Manager may promote | View-independent item IDs; views/columns/subitems/groups; selection/check; label edit; virtual retrieve/cache/search; insertion mark | virtual collections, text, images, scrolling, semantics | Not proven by gallery list rows | **OBSERVED** WF §4.3/§5.2; weak SDR §5.4. Million rows, mode switches, retrieval bounds, edit, selection across mutation |
| 32 | `SplitContainer`, `SplitterPanel`, legacy `Splitter` | P1 legacy splitter/P3 split container | Physical splitter; min sizes; fixed/collapsed panels; orientation; keyboard/pointer resize; nested layout | layout/capture, semantics | Legacy `Splitter` can be a facade over corrected split behavior | **OBSERVED** SDR Splitter; WF §4.3. Min/collapse/fixed, resize feedback, scale, accessibility |
| 33 | `TabControl`, `TabPage` | P3/open | Page identity/collection; selection; keyboard traversal; hidden-page layout/focus; owner draw | containers, focus, text/images, semantics | Not observed in SDR. File Manager's no-tabs rule may or may not exclude this independent framework control; architect decision required | **OBSERVED** WF; **NOT OBSERVED** SDR. Test page insert/remove, Ctrl+Tab/arrows, focus restore, no hidden hit/paint |
| 34 | `ToolStrip`, `MenuStrip`, `StatusStrip`, hosted items, `ToolStripContainer/Panel/ContentPanel`, `BindingNavigator` | P3 | Item ownership; overflow/dropdowns/hosted controls; split buttons; shortcuts/merge if admitted; strip layout/rafting; status spring; binding navigation | row 22, layout, binding, popup/focus/semantics | Not current SDR core. A ribbon is a separate composition, not automatically ToolStrip | **OBSERVED** WF §4.3/§5.1; indirect SDR. Test overflow, hosted editor focus, split activation, dynamic reparent, binding currency |
| 35 | `CheckedListBox` | P3 | Checked state independent of selection; check-on-click; keyboard; item-check cancellable/order; binding/owner draw | ListControl, input, semantics | Not evidenced by SDR | **OBSERVED** WF. Test insert/remove, selected/checked independence, event order, accessibility |
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
| role/value/action/text ranges | optional semantic-hook graph |
| invalidation, display chunks, damage | retained renderer contract |
| UI timers/invoke/shutdown | host scheduler/dispatcher |
| C/C++/C# identity and callbacks | versioned C ABI and generated wrappers |

## 6. Compatibility priorities and corrections

- **GIVEN:** native DML ordering is explicit and deterministic. A WinForms
  facade may emulate selected dock/order outcomes, but known-bad behavior is not
  the native default.
- **GIVEN:** ordinary authors receive Forms-like events. Preview/target/bubble
  remains internal/custom-control machinery.
- **GIVEN:** accessibility metadata is optional, but every stock control should
  supply a tested default adapter when the semantic consumer is enabled.
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
