# Current GUI.Forms API reference

Status: **OBSERVED C++/C ABI 0.x surface as of 2026-08-07**. This is a concise
consumer map, not a stability guarantee. The completeness matrix, header files,
and executable tests remain authoritative if this summary disagrees with code.

Umbrella include:

```cpp
#include <gui_forms/gui_forms.hpp>
```

Language level: C++20. Public platform objects and renderer types are absent.

## GUI.Drawing renderer-free core

`drawing.hpp` defines the drawing substrate independently of the Forms tree. It
provides integer/floating geometry, captured colors and a small system-role
palette, affine matrices, thread-affine disposable brushes, pens, fonts,
formats, paths, regions, image attributes, owned COW bitmaps, and graphics
recorders. Brushes include solid, hatch, linear-gradient, path-gradient, and
retained bitmap-backed `TextureBrush` resources. A texture brush snapshots its
source pixels, supports tile/mirror/clamp wrapping, affine set/reset/translate/
scale/rotate operations, cloning, deterministic traces, and CPU Skia execution.
Identity-only `ImageReference` textures record honestly but cannot rasterize
until pixels are supplied.

Owned bitmaps now also expose transactional rectangular edits. A write lease is
committed or cancelled exactly once; cancellation restores the leased bytes,
no-op commits preserve the generation, and commits derive bounded local damage
from actual byte changes. `changes_since` is multi-consumer and degrades
explicitly to full-image damage if its 256-generation history was exceeded.
`RasterCanvas` projects those generations into a retained zoom/pan viewport,
nearest or linear sampling, RGBA/BGRA normalization, transparency grid,
resource retirement, and local window damage. `Control::invalidate(Rect)` and
raw-image registry patches preserve the same bounded damage through replay.

Color conversion is explicit rather than theme-relative: IEC 61966-2-1 sRGB,
linear sRGB, XYZ D65, OKLab, and OKLCH have named value types and checked
conversions. sRGB projection reports unclamped values and gamut/clipping state;
the deterministic OKLCH mapper reduces chroma only while preserving lightness,
hue, and alpha. ICC/profile loading and a detailed `ColorDialog` remain open.

The recorder owns a bounded state stack and typed snapshot commands for clear,
rectangle, line, string, ellipse, polygon, path, and logical image operations.
`gui.drawing.trace/v1` includes resource values, transforms, clip,
quality/compositing state, UTF-8 text, command count, and close state. A
consumer-supplied `TextMetricsProvider` keeps measurement policy outside the
core until the shared text service is connected.

The core remains renderer-free. A separately packaged private raster service
executes retained commands and bounded PNG through CPU-only Skia. The current
archive is measured on macOS arm64 and cross-built for Windows x64; physical
Windows qualification and Linux host packaging remain open.

## Identity, lifetime, and events

### `Component`

- `dispose()` is synchronous and idempotent.
- `component_state()`, `is_alive()`, and `is_disposed()` expose retained state.
- owned revocable work is disconnected before subclass teardown.

### `ComponentContainer`

- owns nonvisual or visual components independently from visual parentage;
- `add`, `remove`, `contains`, and `components` expose ordered ownership;
- container disposal disposes its owned components once.

### `Event<Arguments...>` and `SubscriptionToken`

- subscription callbacks run from a registration-order snapshot;
- handlers added during emission wait for the next emission;
- a token disconnected before its turn is skipped;
- tokens disconnect on destruction and may be explicitly disconnected;
- `subscribe(Component& owner, callback)` lets component disposal revoke the
  subscription without creating a visual ownership edge.

Core control events are synchronous on the UI thread that caused the mutation;
the event template does not imply background delivery.

### UI dispatcher and `DispatchOperation`

- `Window::begin_invoke(callback)` posts window-owned work from any thread;
- `Control::begin_invoke(callback)` posts owner-scoped work and cancels it if
  the control is detached or disposed before its turn;
- `Control::invoke_required()` and `Window::invoke_required()` expose retained
  UI-thread affinity without permitting cross-thread mutation;
- `Window::invoke(callback)` and `Control::invoke(callback)` execute inline on
  the UI thread. From a worker they enqueue one synchronous operation, wake the
  running host, and block that worker until completion without a nested pump;
- worker `invoke` rethrows the callback's original exception. Owner detach or
  dispatcher shutdown wakes the worker with `DispatchCancelledError`; an
  unhosted worker call is rejected instead of waiting indefinitely;
- a dispatch turn consumes one FIFO snapshot. Work posted by a callback remains
  deferred until the next host turn rather than becoming reentrant;
- dropping the returned `DispatchOperation` does not cancel fire-and-forget
  work. `cancel()` is explicit, thread-safe, and effective only while pending;
- callback faults are isolated, retained on the operation as
  `std::exception_ptr`, and do not suppress later work;
- the queue is bounded at 4096 pending callbacks and one turn at 1024 callbacks;
  wake requests coalesce while a wake is already pending; and
- Window/host shutdown synchronously rejects new work and cancels every pending
  operation. AppKit, Win32/Wine, and the explicit headless pump all drain the
  same renderer-free dispatcher.

Synchronization-context projection, a `BackgroundWorker` component, and nested
`DoEvents` are not implemented by this slice. UI-thread code that synchronously
waits on a worker which is itself waiting in `invoke` remains an application
deadlock; GUI.Forms deliberately does not conceal it with reentrant pumping.
The generated C ABI retains its separate compatibility queue, but now uses the
same one-snapshot/next-turn rule for nested `BeginInvoke` callbacks.

### `Timer`

- binds to one live `Window` and emits `tick()` on that Window's UI thread;
- interval is at least 1 ms; `start`, deterministic `start_at`, `stop`, and
  interval mutation are supported;
- late deadlines coalesce to one callback while preserving their cadence and
  registration order;
- rendering occlusion does not suspend component time, while disabled timers
  publish no wake; and
- stop, component disposal, and Window shutdown synchronously revoke future
  callbacks. This is not a background worker.

## Unicode text and shaping foundation

`TextStore` is the renderer- and host-neutral M4 mutable-text baseline.

- `Utf8Offset`, `Utf16Offset`, `ScalarIndex`, `GraphemeIndex`, and `LineIndex`
  are distinct position types; byte and UTF-16 offsets that split a
  scalar/surrogate pair are rejected.
- strict UTF-8 validation rejects malformed, overlong, surrogate, truncated,
  and out-of-range sequences without replacement characters;
- Unicode 17.0.0 extended grapheme segmentation passes all 766 official
  conformance cases; checked conversion, range, membership, and next/previous
  cluster navigation are available;
- bounded `replace` is atomic and reports removed/inserted scalar counts;
- line indexing recognizes CRLF, CR, LF, NEL, line separator, and paragraph
  separator;
- opaque style spans are sorted, nonoverlapping, merged when adjacent/equal,
  and deterministically split or shifted by edits; and
- `snapshot()` exposes revision, edit, style, full metadata-rebuild, and rejected
  operation counters.

`text_shaping.hpp` defines the renderer-neutral `TextShaper` and
`FontFallbackResolver` service boundary. Requests use opaque font IDs, OpenType
tags, absolute UTF-8 ranges, direction/language/features, and complete grapheme
clusters for fallback. Validation rejects split clusters, ambiguous cluster
order, and nonfinite placements. No shaping or font-discovery backend is
selected or implemented yet.

The representation remains contiguous UTF-8 and deliberately provisional.
Bidi/script analysis, actual shaping/fallback, glyph caching, selection,
clipboard, undo, IME, accessible ranges, and editor controls remain open.

## `Control`

Every control has a nonempty immutable `StableId` and a process-local
`RuntimeId`.

### Tree and attachment

- `parent()`, `children()`, `add_child`, `remove_child`, `clear_children`;
- parent-to-child ownership is strong, child-to-parent ownership is weak;
- cycles, duplicate stable IDs, cross-window reparenting, disposed children,
  and structural mutation during lifecycle notification are rejected;
- `attached()` states whether the control currently belongs to a `Window`.

### State and geometry

- mutable `name` is distinct from immutable `StableId` and has a tokenized
  change event (synchronous outside an initialization transaction);
- requested bounds reject nonfinite/negative extents and honor retained
  minimum/maximum size; edge queries, client rectangle, containment,
  window-coordinate conversion, and bring/send z-order operations are public;
- exact `BoundsSpecified` masks support atomic partial bounds mutation;
  point/rectangle transforms round-trip through retained ancestry;
- `child_index` and `get_child_at_point` use index-zero-topmost direct-child
  order. Lookup can independently skip invisible, disabled, and explicitly
  hit-transparent children; ordinary pointer routing passes through the latter;
- default Dock consumes children in reverse public z-order (backmost to
  topmost), while paint remains back-to-front and hit testing front-to-back;
- `get_next_control` returns the preceding/following live descendant in stable
  nested TabIndex order without wrapping or conflating traversal with focus;
- `get_preferred_size`, `AutoSize`, and `AutoSizeMode` provide retained
  GrowOnly/GrowAndShrink sizing over visible child bounds, trailing margins,
  padding, and min/max constraints. Flow/Table panels consume the same base
  policy rather than owning unrelated boolean state;
- `tab_index` defines stable sibling traversal order and `tab_stop` removes a
  focusable control from Tab traversal without preventing explicit focus;
- `ControlStyles` stores the admitted Forms-compatible style bits, including
  resize redraw, user paint, transparency support, standard click/double-click,
  and buffering requests;
- `set_style`, `has_style`, `set_double_buffered`, and `double_buffered` retain
  those facts across attachment. Clearing buffering never disables the
  framework's baseline coherent-paint safety;

- requested, arranged, committed-arranged, and absolute bounds;
- retained `visible`, `enabled`, `focusable`, `allow_drop`, and inherited
  cursor state;
- application-owned `Tag` storage with exact `std::any` type retention and
  synchronous release during disposal;
- effective visibility/enabled/input eligibility queries;
- `PaintPlane` selection and display-chunk information.

Reading arranged or absolute geometry may execute the declared layout read
barrier. It is not a raw field read.

Portable conversion currently terminates at Window client coordinates. The
generated facade owns a top-level presentation offset for PointToScreen, but a
native desktop-screen origin and multi-monitor transform remain host-contract
work; the core does not encode one platform's coordinate system as portable
truth.

### Invalidation

- `invalidate(Dirty)` marks the affected control/path;
- `invalidate_subtree(Dirty)` explicitly marks every descendant;
- `invalidate_declared(Dirty)` rejects undeclared effects in development and
  conservatively invalidates in production;
- dirty vocabulary distinguishes measure, arrange, paint, hit test, text,
  style, resource, semantics, and accessibility.

### Initialization lifecycle

- `begin_init()` and `end_init()` nest;
- stock property/state events remain synchronous outside initialization and
  coalesce by event identity during initialization; the final payloads publish
  in first-event order only after the outer commit;
- native custom controls use the protected `publish_change` helper for the
  same policy; calling an independent `Event::emit` directly deliberately
  retains its ordinary synchronous semantics;
- dirty effects coalesce until the outer `end_init()`;
- entering initialization revokes focus/capture and makes that control
  ineligible for routed input, commands, and mnemonics until commit; cleanup
  callbacks are deferred through the same boundary;
- `initialization_completed()` emits the combined dirty flags and whether the
  operation required a subtree mark;
- unmatched `end_init`, disposed mutation, and wrong-thread mutation are
  rejected.

See `experiments/M12P27_INITIALIZATION_OWNERSHIP_AND_DOCK_ORDER.md` for the
mutation-ownership and reverse-z geometry oracles.

Subclass hooks `on_attached_to_window`, `on_attachment_committed`, and
`on_detached_from_window` define the current deterministic lifecycle. Attachment
observes a fully bound subtree parent-first. Detachment observes a fully unbound
subtree child-first. The commit hook is nonthrowing and runs only after all
throwing attach hooks succeed.

`pointer_observed`, `focus_observed`, and `arranged_bounds_changed` provide
tokenized observation for nonvisual providers without subclassing a target.

`causes_validation` defaults true and publishes a tokenized change event.
`validating` receives a mutable `ControlValidationEvent` before focus loss;
setting `cancel` rejects the focus move under prevent mode. `validated` follows
only an accepted validation. The Window rejects nested focus mutation during a
validation callback, rechecks both endpoints after callbacks, and exposes exact
attempt/success/cancel/block/reentrancy/bulk counts in `ValidationSnapshot`.

`parse_mnemonic_text` and `is_mnemonic` implement the portable ampersand
contract: one marker names the next Unicode scalar, `&&` displays one literal
ampersand, and matching preserves Unicode identity with ASCII case folding.
`process_mnemonic` snapshots eligible effective controls in stable retained/tab
order before any callback runs. Window confines the set to the active focus
scope and cycles duplicate winners after the prior match. `DialogKeySnapshot`
reports candidates, collision-bearing dispatches, and completed cycles in
addition to attempts and handled commands.

### Custom-control overrides

`measure`, `arrange`, `on_paint`, `visual_outsets`, `hit_test_local`,
pointer/key/text/drag route hooks, focus notification, and activation are
available. Drawing uses the renderer-neutral `Painter` vocabulary. Visual
outsets let bounded decoration extend beyond arranged bounds without changing
layout or hit testing; the compositor includes the current and last-presented
outsets in damage while retaining parent-client clipping.

## Containers

### `ScrollableControl` and `ScrollProperties`

`Panel` and `ContainerControl` share a renderer-neutral retained scrolling
base. `AutoScroll`, `AutoScrollMargin`, `AutoScrollMinSize`, and positive
internal position determine a two-axis logical content extent; the familiar
`AutoScrollPosition` view reports its negative display origin. Child requested
bounds remain authored content coordinates and never drift across scroll or
layout. `DisplayRectangle` and `viewport_rectangle` expose the logical display
and clipped child viewport.

Horizontal and vertical `ScrollProperties` expose enabled/visible, min/max,
effective large/small change, and value state. Automatic mode owns range and
visibility; manual mode accepts authored axes. Classic arrow, page, and thumb
input shares captured retained state with wheel fallback and semantic
increment/decrement/set-value actions. Scrollbar chrome is an overlay outside
the child viewport, so descendants cannot paint or receive hits through it.
`scroll_control_into_view` honors each target's `AutoScrollOffset`; resize
clamps stale positions and removes unnecessary bars. Each real input mutation
retains an exact event revision, WinForms `ScrollEventType`, orientation, and
old/new values. ABI subscribers and the generated facade consume that one
native record; reading it from a managed callback never recursively enters
layout. Exact RTL mirroring,
scaling/DPI oracle cases, drag-edge autoscroll, virtual anchoring, and repeated
button timing remain open.

### `ContainerControl`

- `contains_descendant` tests retained logical containment;
- `active_control` returns the focused descendant when present;
- `request_active_control` and `clear_active_control` use the owning window's
  focus contract;
- inherited `AutoValidate` supports disabled, prevent-focus-change, and
  allow-focus-change policies; an unresolved root inheritance defaults to
  prevent-focus-change; and
- `validate` and constrained `validate_children` use the same retained
  cancellable transaction and exact WinForms flag values.

Container mnemonic traversal and retained scrolling are inherited. Scaling,
complete protected managed override projection, and the rest of key
preprocessing remain separate work.

### `UserControl`

- `loaded()` is a tokenized one-shot lifetime event;
- `is_loaded()` means load notification has begun at least once;
- `is_attached()` exposes current attachment;
- `attachment_count()` counts successful whole-subtree attachment commits.

An attach attempt that later rolls back cannot erase an already observed
`loaded` callback, but it does not increment `attachment_count`.

## Basic controls

### `Panel`

Properties: `BorderStyle`, background, provisional `BasicControlStyle`, and the
full `ScrollableControl` family.
Paints none/line/sunken/raised retained panel surfaces.

### `GroupBox`

Adds caption text and font over `Panel`. Radio grouping uses logical retained
containers; full caption-aware child layout remains later work.

### `ScaledPanel` and `ScaledGroupBox`

These public retained containers place children in a caller-declared logical
design coordinate space and scale each child slot into the current arranged
bounds. `add_at`, `set_design_bounds`, and `design_bounds` own the reusable
mapping; nested composition, runtime slot mutation, detachment cleanup, and
finite/nonnegative validation are tested. They are deterministic proportional
layout controls, not a substitute for Dock/Anchor, DPI policy, or the flow/table
families.

### Retained layout inputs

Every `Control` exposes finite, nonnegative `Margin` and `Padding` in logical
pixels. A parent layout container assigns a private retained layout slot; this
does not overwrite the child's authored requested bounds, so preferred-size
measurement remains stable across repeated layout and resize.

### `FlowLayoutPanel`

Orders visible children left-to-right, right-to-left, top-down, or bottom-up.
It honors physical child margins and container padding, optional wrapping,
per-child `FlowBreak`, and `AutoSize`. Hidden children leave no gap. Flow is a
deterministic retained layout family, not a CSS flexbox implementation.

### `TableLayoutPanel`

Owns bounded row and column tracks with absolute, auto-size, and weighted
percent sizing. It supports explicit or deterministic row-major automatic cell
placement, row/column spans, fixed/AddRows/AddColumns growth policy, cell and
position lookup, physical margins/padding, optional cell borders, resolved
track inspection, overflow reporting, and `AutoSize`. Hidden children release
their occupied cells. Alignment/stretch policy, Dock/Anchor integration, and
DML/C ABI projection remain open.

### `Label`

Text, horizontal/vertical alignment, wrapping, line spacing, synchronous
`text_changed`, measurement, and nonintercepting hit testing. `TextStyleRole`
selects inherited body, control, caption, heading, title, or monospace
typography from the active structural theme. Font and foreground remain
independently overridable; `clear_font()` and `clear_foreground()` restore
theme authority. `use_mnemonic` controls marker-aware measure, paint, semantic
text, and next-selectable-control focus. Ellipsis, multiple link spans,
locale-sensitive case folding, underline cue policy, and the final text engine
remain incomplete.

### `ButtonBase` and `Button`

Text/font/style, press/focus visual state, synchronous `clicked` and
`text_changed`, pointer activation, normalized keyboard activation, and default
button cue. A Button may use a direct Window `ImageId` or an `ImageList` plus
mutually exclusive index/key selection. Nine-way image/text alignment,
overlay/before/after/above/below relations, bounded gap, preferred-size
measurement, pressed displacement, and state/density selection are retained and
custom-rendered. `use_mnemonic` applies consistently to measure, paint,
semantics, and activation. `perform_click()` uses the same availability and
validation gate as mnemonic and dialog commands. `DialogResult` publishes the
exact WinForms numeric values; Button and Window reject undefined gaps. A
cancel Button defaults from `none` to `cancel`. `Window::set_accept_button`
transfers the default cue; Enter/Escape route to live accept/cancel targets only
after the focused route declines the key and never move focus. Button publishes
Click before a non-None retained Window result. Independent native modal-loop
closure and full managed protected-call projection remain open.

### `MenuStrip` and `ContextMenu`

MenuStrip is a retained focusable menu bar whose live popup rows share Command
authority with pointer, keyboard, and semantic activation. `use_mnemonic`
removes markers from top-level width, paint, and semantics. Duplicate
top-level mnemonics cycle deterministically; active popup rows resolve Alt
mnemonics inside their focus scope and retain submenu behavior. Overflow,
hosted ToolStrip controls, and protected generated ToolStrip projection remain
open.

### `CheckBox`

Two- and three-state values, `auto_check`, `check_state_changed`, and
`checked_changed`. The declared activation order is checked-state mutation,
state events, then click.

### `RadioButton`

Boolean checked state, logical `group_name`, automatic same-container exclusion,
`auto_check`, and `checked_changed`.

### `LinkLabel`

Single retained link, visited state, link styling, and button-like activation.
External navigation is application policy and is not performed by the control.

### `ImageList`

`ImageList` is a nonvisual Component bound to one Window resource registry. It
owns an ordered, ASCII-case-insensitive keyed collection with stable indices, bounded
UTF-8 keys, logical image size, a type-erased Tag, and tokenized revisioned
change events. Each key may carry normal, hot, pressed, selected, and disabled
variants at multiple density scales. Resolution prefers an exact scale, then
the nearest larger source, then the nearest smaller source; state fallback is
explicit and deterministic. Logical layout size remains unchanged when a 2x
source is chosen.

PNG additions validate and mutate atomically through generational Window image
IDs. Entries may also reference an existing Window image without claiming its
ownership. Explicit component disposal synchronously releases list-owned
images; a failed import/replacement preserves the prior entry and revision.
Button/CheckBox/RadioButton, TreeView, and ObjectView consume the same public
list and invalidate from its tokenized changes. Disabled fallback uses reduced
opacity only when no disabled raster exists.

Native HIMAGELIST handles/streams, strip slicing, color-key transparency,
palette quantization/`ColorDepth`, and designer converters are not implemented
by this tranche. PNG remains the only encoded GUI.Forms import format.

## Motion and animation

`AnimationTimeline` samples a validated delay/duration/iteration/direction/
easing specification against caller-supplied monotonic frame times. `pause()`
retains the exact sampled phase and `resume()` rebases suspended time, so no
hidden catch-up or restart is introduced.

`MotionPolicy` keeps three application facts independent: `enabled`, `paused`,
and `reduced`. `active()` is true whenever the source is enabled and unpaused;
reduced motion remains active with a minimum 100 ms cadence, a `0.35` speed
scale where the control owns phase accumulation, and half-width centered visual
excursion. Controls accept the complete policy atomically so compound changes
cannot briefly publish duplicate frame leases or become order-dependent. Only
pause and disable are quiescent.

### `EasingPreview`

`EasingPreview` is a public owner-scheduled animation control rather than
showcase-private paint code. It owns an `AnimationTimeline`, one bounded frame
lease, a complete atomic `MotionPolicy`, configurable one-to-32 labeled easing
tracks, retained drawing, and image/busy/numeric semantics. Hidden-page
suspension and pause/reduced/full transitions use the same scheduler contract as
other active surfaces.

## Diagnostic and owner-drawn controls

### `DrawingSurface`

`DrawingSurface` exposes a renderer-neutral owner-paint callback over the public
`Painter` vocabulary. The callback receives local bounds and damage, and can be
made hit-test visible explicitly. It publishes image semantics when named. It
does not expose a renderer or platform graphics context.

### `MaterialPanel`

`MaterialPanel` is a content-agnostic retained container for chrome bands,
cards, wells, specimens, and composed surfaces. `SurfaceMaterial` owns up to
eight ordered solid/linear/radial/image fill layers, four bounded box shadows,
one rounded border, and a corner radius. Image layers provide whole-image
stretch, density-aware exact-period tiling with cropped edge tiles, and
nine-patch source slicing whose corners remain fixed and whose edge/center
bands stretch. Image IDs are Window-scoped; attached panels reject resources
missing from that Window or declared with stale pixel dimensions atomically.
Gradient coordinates may be normalized to
the arranged bounds or expressed in logical units. Linear layers select
`GradientSpreadMode::pad`, `repeat`, or `reflect`; `repeating_linear` is the
logical-period convenience for pinstripes, grooves, scanlines, and other
scale-independent texture. Recipes validate atomically;
identical assignment is silent and a change invalidates paint only.

The public `Painter` vocabulary now records rounded clip/fill/stroke,
multi-stop linear and elliptical radial gradients, repeating/mirrored linear
gradients, bounded box shadows, and source-region image replay.
Skia, CoreGraphics, and Win32 DIB painters realize those commands; minimal
painters receive deterministic bounded primitive fallbacks. Gradient stops are
limited to 32, begin at exactly zero, end at exactly one, and increase strictly.
Compositing groups, opacity/luminance masks, blur/color effects, vector assets,
native image-stream/strip imports, and C ABI projection of the new material and
texture objects remain open.

### `Theme`

`Theme` is immutable after `Theme::create`. A complete `ThemeDefinition` owns
renderer-neutral recipes for window, panel, card, button, accent-button,
command-button, choice, editor, menu-item, selection, and progress roles. Every
role has ordinary, selected, high-contrast, and high-contrast-selected recipes
for normal, hot, pressed, pending, invalid, disabled, and deactivated states.
Each recipe combines a `SurfaceMaterial` with text/glyph colors, focus/default
cues, and pressed-content displacement.

`Window::set_theme` replaces the application surface atomically and
invalidates inherited style, paint, and semantic state once. A `Control` may
install or clear a local immutable override; otherwise resolution walks the
retained visual parent chain. State precedence is disabled, invalid, pending,
pressed, hot, deactivated, normal. The built-in `windows-professional` theme is
a Windows 7/10-inspired default, not a pixel-identical system-theme promise.
Every paint transaction first records the full-coordinate `window` material,
clipped to the current damage. This retained backplane restores gaps beneath
detached overlays even when the application root is a transparent layout
container, without changing child layout or z order.
Panel, Button, command/accent Button, CheckBox, RadioButton, Card, TextBox,
ListBox selection, ComboBox, MenuStrip, and ProgressBar consume these recipes.
Explicit local Panel style/background overrides retain compatibility paint.
`ThemeStructureTokens` adds validated logical spacing and geometry scales,
Portsmouth-oriented control/title plus content-field typography roles, and
bounded motion durations. Card and MasterDetailView follow these inherited
tokens by default, preserve explicit component layouts, and provide explicit
reset-to-theme operations. Theme replacement therefore invalidates inherited
measure, arrangement, hit testing, style, paint, and semantics atomically.
Remaining stock-control/type adoption, density variants, serialization, and ABI
projection remain open.

### `Card`

`Card` is a content-agnostic retained header/body/footer composition. It owns
ordinary child controls in each slot, returns the detached predecessor on
replacement, validates a bounded `CardLayout`, and resolves the card theme
role. Its default effective layout derives from structural theme tokens;
explicit layouts remain authoritative until `reset_card_layout_to_theme()`.
Optional interaction adds pointer/keyboard activation, independent
selection, focus/hot/pressed state, tokenized events, visual-status projection,
and selectable list-item semantics. The File Manager atmosphere laboratory
uses only this public API and `MaterialPanel`; it contains no private card
renderer.

### `ReviewCard`

`ReviewCard` is a typed Card specialization for evidence, decision, audit, and
verdict projections. One atomic `ReviewRecord` carries a stable key, title,
summary, verdict, and neutral/information/accepted/pending/warning/rejected
disposition. It owns theme-aware heading, body, and caption Labels, retains
ordinary Card layout and interaction, emits tokenized `record_changed`, and
projects complete semantic name/description/value. Pending and rejected
records also publish busy and invalid state respectively. Text bounds and UTF-8
are validated before mutation, so an invalid replacement cannot partially
change the visible or semantic record.

### `MasterDetailView`

`MasterDetailView` owns arbitrary master and detail subtrees around a genuine
public `SplitContainer`. Its `MasterDetailLayout` controls orientation, master
extent, visible and hit splitter widths, pane minima, compact threshold, and
resize policy. Automatic presentation is side-by-side when both roles fit and
otherwise exposes either master-only or detail-only compact navigation.
Collapse removes the hidden role from visibility and focus order; growth
restores both roles. Explicit side-by-side/master/detail modes, compact
master/detail navigation, atomic role replacement, presentation-change events,
and current-presentation semantics are public.
Default splitter, navigation, minimum-pane, and compact-breakpoint geometry
derives from structural theme tokens. Explicit layout remains authoritative
until `reset_master_detail_layout_to_theme()`.
The File Manager DNA laboratory dogfoods this API with four interactive
decision Cards: wide layouts retain side-by-side evidence and compact layouts
navigate focus-safely between the decision list and projected detail.

### `MetricsView`

`MetricsView` renders a titled, styled snapshot of the owning `Window` metrics
and exposes the same structured text through group semantics. The application
does not need to subclass `Control` to show runtime diagnostics.

### Complete Showcase consumer boundary

The independent Complete Showcase is required to be ordinary application
composition over public GUI.Forms types. It contains no local control subclass.
A configured CMake policy test rejects `class` or `struct` inheritance in the
showcase implementation so layout, painting, animation, diagnostics, input, or
semantic behavior cannot be hidden as demo-only evidence.

## Range controls

### `RangeControl`

- finite minimum, maximum, value, small/large changes, and orientation;
- strict invalid-range/value validation and normalized value;
- programmatic order: mutation then `value_changed`;
- range-boundary order: `range_changed`, then conditional `value_changed`;
- input order: `scroll`, then `value_changed`;
- disposal during `scroll` suppresses the later value callback.

### `TrackBar`

Horizontal/vertical rendering, ticks, pointer capture, outside-drag clamping,
wheel, arrows, Page Up/Down, Home/End, focus cue, and disabled-input rejection.
This is a GUI.Forms range control, not yet the complete stock WinForms facade.

### `ProgressBar`

Horizontal/vertical blocks and continuous determinate display plus bounded
marquee, slow travelling luminance pulse, classic marching stripes, and
laser-etch animation through active-surface deadlines. Laser etch combines a
vertically shifting repeated phase across the fill with a bright leading edge
and deterministic sparks. `ProgressBarAnimationAppearance` atomically sets the
luminance, stripe, phase, edge, and spark colors plus bounded pulse/edge/pitch
geometry; invalid records leave the prior appearance intact. These are public
library styles, not showcase painters. The control is
nonfocusable, does not intercept hit testing, becomes scheduler-quiescent when
paused, disabled by application policy, or effectively hidden, and publishes
numeric or busy semantics. Reduced motion remains busy and visibly animated at
the calmer public policy cadence/speed/excursion without resetting phase. The
older pause and reduced setters delegate to the same atomic policy transaction.

## Binding and currency

### `BindingValue` and `BindableProperty`

`BindingValue` is the renderer-neutral null/Boolean/signed/unsigned/number/text
value union. Conversion is strict and locale-invariant. A control registers a
`BindableProperty` once with a canonical name, value kind, explicit getter,
setter, and tokenized change connector. Binding does not use RTTI, renderer
objects, platform handles, or managed reflection.

### `BindingSource` and `CurrencyManager`

`BindingSource` owns validated stable-ID `BindingRecord` values and one
`CurrencyManager`. It provides current/count/position, deterministic currency
movement, field mutation, add/insert/remove/find, begin/cancel/end edit,
list/current/position/data-source/data-member events, reset operations,
`RaiseListChangedEvents`, and coalesced `SuspendBinding`/`ResumeBinding`.
Currency movement updates bound controls before public `PositionChanged`,
`CurrentChanged`, then `CurrentItemChanged`. The manager also exposes
manager-wide `PullData` and `PushData`, current/list/error events, refresh, and
edit/removal operations.

### `Binding`, `ControlBindingsCollection`, and `BindingContext`

`Binding` supports source-to-control reads, control-to-source writes,
`OnPropertyChanged`, focus-driven and explicit `OnValidation`, and `Never` update modes,
invariant `F0..F12` formatting, Format/Parse hooks, null substitutions,
post-commit BindingComplete and DataError, and reentrancy/lifetime cleanup.
Successful completion runs after the destination commit and propagates from the
binding to its source/currency manager. `ControlBindingsCollection` owns each
control's bindings, rejects duplicate canonical properties, and applies its
default update mode only through the no-options Add overload.
`BindingContext` preserves same-window manager identity and eagerly removes a
disposed source.

`Control::property_descriptor(s)` exposes inert, deterministic value snapshots;
`property_value`, `set_property_value`, `reset_property`,
`should_serialize_property`, and `subscribe_property_changed` use the same
native registrations as binding. Descriptors include authored name,
typed scalar/Point/Size/Rect/Insets/Color/Font/Image/enum/object/collection kind,
category/description, default, declared dirty effects,
local/subtree effect scope, serialization visibility, browse/bind/reset, and
change-notification facts. `kind` remains the non-null payload schema while
`nullable` independently admits null. At most 256 unique typed standard values
may be declared and enforced as an exclusive finite set. Optional bounded
converter, editor, and dynamic standard-value-provider names are inert service
identities; executable callbacks never enter the descriptor snapshot.
Executable access is live/UI-thread guarded, conversion precedes setters,
change tokens are owner-revoked, and mutations retain the existing nested
`BeginInit`/`EndInit` effect union.

`Control::property_value_origin` reports `defaulted`, `local`, `inherited`,
`ambient`, or `computed` independently of `ShouldSerialize`. An explicit
registration origin provider wins; otherwise a readable property with a
default compares its typed live/default values, and a readable property without
a default is computed. Label Font and ForeColor provide exact inherited/local
origins across override and reset.

`PropertyEnumDescriptor` provides a shared immutable type name, at most 256
bounded UTF-8 choices, 256-byte type/choice names, and a flags policy.
Descriptor-aware conversion accepts canonical numeric or
case-insensitive named values, comma/vertical-bar flags, and rejects unknown
names/bits/types before mutation. Diagnostic value strings are not a DML source
format.

Stock descriptors currently cover base `Name`, `Visible`, `Enabled`,
`AutoSize`, `CausesValidation`, Bounds/MinimumSize/MaximumSize, Margin/Padding,
AutoScrollOffset, Dock/Anchor/AutoSizeMode, TabIndex/TabStop, AllowDrop,
HitTestTransparent, and accessibility text; TextBox/Label/ButtonBase `Text`;
CheckBox/RadioButton `Checked`; range and NumericUpDown `Value`; and
noneditable ComboBox `Text`, `SelectedIndex`, and content-serialized `Items`.
`Items` is a homogeneous immutable text collection with truthful reset and
change observation, but deliberately is not a scalar binding target. Label adds inherited
Font/ForeColor override/reset, PictureBox adds Image/SizeMode/ImageOpacity, and
Button adds Font/Image. Properties without a truthful change event are
inspectable/settable/resettable but explicitly non-bindable.

Native `PropertyGrid` projects these browsable descriptors with stable IDs and
categorized/alphabetical sorting. Boolean, text/integer, number, finite
non-flags enum, flags enum, and Color properties use retained CheckBox,
TextBox, NumericUpDown, ComboBox, FlagsValueEditor, and ColorValueEditor
controls. The flags editor owns a CheckedListBox popup through Window
popup/focus-scope tokens. The color editor combines ordinary TextBox behavior
with an alpha-aware swatch, canonical `#RRGGBBAA`, invalid state, cancellation,
and typed failure reporting. Point, Size,
Rect, Insets, Color, and Font values expose expandable typed child paths such
as `Bounds.X`, `Padding.Left`, and `Font.Italic`. Immutable objects and
homogeneous collections recurse through member/index paths such as
`Settings.Endpoint.Host` and `Items[2]`; collection insert/remove/move rebuild
one snapshot through `insert_collection_item`, `remove_collection_item`, and
`move_collection_item`. Trees are bounded to depth 8, 256 members per object,
4,096 items per collection, and 8,192 total nodes. Object members may own their
payload/null/enum/standard-value schema and converter/editor identities. A child
edit reconstructs every typed ancestor and passes through
the owning registered setter. Nullable text/finite-choice values round-trip an
explicit `(none)` representation; image/resource nulls without a specialized
editor remain read-only.
Commits normalize from
the getter, expose typed change/error events, and preserve the complete parent
on invalid conversion. Each resettable parent owns a real retained Reset button
whose enabled state follows `ShouldSerialize`; activation uses the real reset
contract and resynchronizes every child. Truthfully observable properties
refresh automatically;
`refresh_properties()` is explicit for the rest. Selection is weak and
callback-time target or grid disposal is contained. Multiple selected controls
project their common schema; edits preflight every owner, roll changed owners
back if any setter rejects, and publish one logical change after success. This is a native partial
tooling control. ABI 0.22 and the generated System.Windows.Forms façade project
the real native PropertyGrid for managed GUI.Forms Control selections, including
SelectedObject(s), PropertySort, refresh, and selection/sort events. ABI 0.23
adds nonvisual foreign-object proxies for ordinary managed `TypeDescriptor` and
`ICustomTypeDescriptor` metadata. Nullable scalar/text/Color/enum values,
categories, descriptions, browsability, finite standards/exclusivity,
current-culture TypeConverter display/parse, reset/ShouldSerialize, and
supported change events cross synchronous bounded callbacks into the same
native editor and atomic multiple-owner transaction. Unsupported writable
types fail before replacing selection; descriptors and standard values are
deep-copied and callback strings use caller-owned buffers. Managed
`UITypeEditor` editors now cross ABI 0.24 through a real retained editor button.
The adapter supplies `ITypeDescriptorContext` and
`IWindowsFormsEditorService`; synchronous drop-down Control hosting,
`CloseDropDown`, owned modal Forms, typed return, failure containment, and the
same atomic multiple-owner commit are implemented. Evidence:
`experiments/M12P17_METADATA_DRIVEN_PROPERTY_GRID.md` and
`experiments/M12P18_EXPANDABLE_COMPOUND_PROPERTIES.md` and
`experiments/M12P19_NESTED_PROPERTY_VALUES_AND_COLLECTIONS.md` and
`experiments/M12P20_PROPERTY_CONVERTER_AND_EDITOR_SERVICES.md` and
`experiments/M12P21_SPECIALIZED_FLAGS_AND_COLOR_EDITORS.md` and
`experiments/M12P22_NULLABLE_CULTURE_NESTED_AND_ATOMIC_PROPERTIES.md` and
`experiments/M12P23_MANAGED_PROPERTY_GRID_ABI_0_22.md` and
`experiments/M12P24_MANAGED_TYPE_DESCRIPTOR_PROXY_ABI_0_23.md` and
`experiments/M12P25_MANAGED_UI_TYPE_EDITOR_ABI_0_24.md`.

`PropertyValueConverterRegistry` supplies canonical named format/parse services
plus optional kind mappings. `PropertyEditorRegistry` supplies named factories
which donate one unattached retained control, a non-emitting synchronization
operation, a tokenized typed commit connector, and an optional tokenized input
failure connector. Both registries are
instance-owned by PropertyGrid rather than mutable process globals. A bounded
`PropertyConversionContext` supplies per-registry culture identity and decimal/
group separators; default numeric conversion never mutates the process locale.
Consumers
may replace or deliberately share them. `PropertyList::replace_editor` retains
the ordinary row ownership, scroll/focus layout, semantics, and disposal laws;
invalid factories do not acquire ownership. Defaults map number to
NumericUpDown, flags enums to FlagsValueEditor, and Color to
ColorValueEditor/`color-hex`, while explicit descriptor service names permit specialized
editors without adding a switch case to PropertyGrid.
OnValidation subscribes to its target's cancellable validation event, commits
before focus loss, and cancels prevent-mode focus when parse/transfer fails.
Nested data members, sort/filter, arbitrary culture providers,
BindingNavigator, and DataGridView remain open. Evidence:
`experiments/M12P5_BINDING_CURRENCY_KERNEL.md` and
`experiments/M12P6_VALIDATION_AND_BOUND_ERRORS.md`. Heterogeneous dictionary,
date/duration/path and modal-editor values, framework-wide stock registration
and change events, dynamic standard-value providers, mixed-value editor visuals,
DML, component-editor/designer services, and nested arbitrary managed
object projection remain open; see
`experiments/M12P15_PROPERTY_METADATA_CENTER.md`,
`experiments/M12P16_COMPOUND_PROPERTY_VALUES.md`,
`experiments/M12P17_METADATA_DRIVEN_PROPERTY_GRID.md`, and
`experiments/M12P18_EXPANDABLE_COMPOUND_PROPERTIES.md`, and
`experiments/M12P19_NESTED_PROPERTY_VALUES_AND_COLLECTIONS.md`, and
`experiments/M12P20_PROPERTY_CONVERTER_AND_EDITOR_SERVICES.md`, and
`experiments/M12P21_SPECIALIZED_FLAGS_AND_COLOR_EDITORS.md`, and
`experiments/M12P22_NULLABLE_CULTURE_NESTED_AND_ATOMIC_PROPERTIES.md`, and
`experiments/M12P24_MANAGED_TYPE_DESCRIPTOR_PROXY_ABI_0_23.md`, and
`experiments/M12P25_MANAGED_UI_TYPE_EDITOR_ABI_0_24.md`.

## Nonvisual providers

### `ToolTip`

- tokenized `set_tool_tip`, lookup, removal, and clear mappings per attached
  Control;
- initial, reshow, and auto-pop delay policies plus hover/focus enablement and
  `show_always`;
- explicit persistent or duration-bounded show and immediate hide;
- target-anchored or pointer-adjacent client-edge placement and live
  repositioning when the owner moves;
- input-transparent overlay-plane presentation and a stable semantic
  `tool_tip` node; and
- synchronous popup/timer cleanup when a mapping, target, provider, or Window
  goes away. Accessible described-by relations and title/icon/balloon variants
  remain open.

### `ErrorProvider`

- tokenized per-control error text, alignment, signed bounded padding, clear,
  `HasErrors`, provider `Tag`, portable `ImageId` icon substitution, and
  container-root lookup;
- six target-relative icon alignments with provider-level right-to-left
  mirroring and deterministic repositioning after retained layout changes;
- one independently owned popup-root glyph per available target, passive
  ownership for disabled controls, target/ancestor visibility cleanup, and
  automatic restoration when availability returns;
- `NeverBlink`, bounded six-transition `BlinkIfDifferentError`, and
  `AlwaysBlink` using active-surface deadlines. Occlusion suppresses wakes and
  reduced motion settles the icon visible without a provider timer loop;
- error text appears through the ordinary public `ToolTip` provider, while the
  final semantic projection marks the target `invalid` and appends a stable
  error description even when a control subclass does not call its base
  descriptor; and
- `ErrorProviderSnapshot` reports each target, retained glyph geometry,
  presentation, blink phase, and active work;
- `DataSource`, `DataMember`, `BindToDataAndErrors`, and `UpdateBinding` attach
  a same-Window `BindingSource`, project current record-wide/field errors by
  real binding target, aggregate multiple errors, surface `BindingComplete`
  failures, refresh on list/currency movement, and clear synchronously when
  the source retires. Arbitrary managed `IDataErrorInfo` objects and true
  nested object traversal remain open.

### `HelpProvider`

- per-control help string, keyword, navigator, explicit/automatic `ShowHelp`,
  `ResetShowHelp`, provider namespace, clear, and `Tag`;
- F1 resolves the focused control then its retained ancestors and dispatches
  one mutable `HelpRequestEvent` to the mapped Control before provider policy;
  a handled control event terminates the route;
- requests retain the target ID, logical position, namespace, string, keyword,
  navigator, keyboard origin, and handled result, with requested/handled
  counters in `HelpProviderSnapshot`; and
- authored help enriches the target semantic description without adding a
  redundant label prefix. A mapped otherwise
  unexposed container becomes an accessible group so guidance is not dropped.
  The provider deliberately does not open a browser, network location, or help
  file; that external action belongs to a consumer or capability-gated plugin.
  TopicId/raw managed-enum projection and accessibility described-by relations
  remain open.

## `Window`

`Window` owns one retained root and one UI thread. It provides:

- resize and logical scale;
- nested update scopes, explicit layout, flush, paint, and per-plane damage;
- retained lookup by stable ID and recursive hit testing;
- focus, pointer capture, pressed state, and routed input dispatch;
- retained accept/cancel targets, exact retained `DialogResult`, stable
  duplicate-mnemonic/dialog-key routing, validation-aware programmatic
  commands, and queryable `DialogKeySnapshot`
  attempt/activation/rejection/candidate/collision/cycle counters;
- nested focus scopes and owner-tokenized retained popup attachment;
- deterministic control-availability publication for retained extender
  providers and overlays;
- frame scheduling, active surfaces, occlusion, and wake deadlines;
- UI timer callbacks plus `check_access`/`verify_access` thread guards;
- validated PNG load/replace/remove through the resource registry;
- renderer-free semantic snapshots and stable-ID action routing;
- structured metrics and activity reset.

### Paint leases and coherent release

`PaintLeaseSnapshot` exposes `content_revision`, `rendered_revision`,
`presented_revision`, `surface_epoch`, lease counters, deferred reentry, and the
clean/dirty/rendering/ready/occluded-dirty state. A paint pass holds one
exclusive UI-thread lease. Nested paint calls are coalesced into later damage;
owner callbacks rebuild retained chunks into a transaction recorder, and no
candidate command reaches the host painter until every callback succeeds.

A callback failure restores the prior retained chunks, abandons the candidate,
and republishes damage. Mutation during paint increments the content revision
and survives as one later pass. `paint` returns an exact `PaintReceipt` only
after complete backend replay and a second owner/surface-epoch validation.
Native hosts present only a valid receipt, then
`notify_presented(PaintReceipt, duration)` advances that exact revision after a
successful platform copy. Duplicate, backward, forged-future, and
replaced-epoch receipts are rejected and counted. The duration-only overload
remains for synchronous inspection code; asynchronous/native hosts must use the
exact receipt. Surface resize/scale increments the epoch so an obsolete
candidate cannot release into a replacement surface.

`set_paint_wake_handler` is the renderer-neutral host notification seam for
model-originated damage. One wake is coalesced until the host consumes all
damage, then rearmed for the next independent mutation. Occlusion suppresses
the wake without discarding dirtiness; exposure requests one wake. This lets a
controller Window mutate another Window and present the result immediately,
without requiring input to be dispatched through the changed Window first.

Generated managed compatibility input and portable retained pointer/key/text,
drag, and semantic input now queue during active paint leases. Physical
native-host reentry probes, raster double-surface swap after a backend replay
fault, and complete multi-rectangle native presentation remain open.

The paint diagnostics deliberately separate state, backing, and presentation
commit through `content_revision`, `rendered_revision`, and
`presented_revision`. `PaintLeaseState` exposes `dirty_queued`, `rendering`,
`rendering_dirty`, and `ready`; `PaintLeaseSnapshot` also reports whether one
render wake is queued and how many requests were queued or merged. Intermediate
revisions are not stored as render jobs. The normative implementation/closure
matrix is `planning/PAINT_PIPELINE_AVAILABILITY.md`.

The generated Win32 compatibility bridge applies the same shape to admitted
direct-HWND controls. `Graphics.FromHwnd` owns one private GUI.Drawing bitmap;
Flush executes only new commands into it, updates the isolated offscreen HWND,
and marks a revision rather than synchronously importing the surface. Flush,
ReleaseHdc, EndPaint, relevant callback return, resize, visibility, and
Invalidate merge behind one owner-thread import drain. `Update`/`Refresh` may
drain their target synchronously, but a call from OnPaint becomes one deferred
pass. Unknown raw GetDC writers use an adaptive 33–250 ms hash fallback that
stops once explicit boundaries are observed. The private compatibility
diagnostic reports content/captured revisions, epoch, queue/active/deferred
bits, and drain counters.

Generated managed owner paint has a parallel bounded surface contract.
Protected `DoubleBuffered`, `GetStyle`, `SetStyle`, `OnPaintBackground`, and
`InvokePaintBackground` are available to subclasses. Enabling buffering reuses
one size-matched PArgb bitmap; background and foreground paint through the same
`Graphics`. Resize/dispose/style retirement invalidates the surface epoch, so a
stale callback cannot publish into its replacement. A touch during paint posts
one follow-up; a callback exception abandons the candidate, preserves the last
published raster, and waits for a later real mutation rather than self-retrying.
Disabling buffering retires persistence but retains coherent ephemeral owner
paint. The internal deterministic snapshot exists for conformance telemetry;
native presentation remains authoritative for presented state.

All nominal `Invalidate` overloads are present, including rectangle, Region,
and recursive-child forms. Rectangle damage clips to the client and unions
behind the single queued lease. `NotifyInvalidate` synchronously enters the
protected `OnInvalidated` hook and public `Invalidated` event but does not itself
schedule painting. Recursive invalidation intersects parent damage and converts
it to child-local coordinates. Buffered owner paint exposes the captured union
through both `PaintEventArgs.ClipRectangle` and the Graphics clip; a failed
lease restores that union for a later real touch. Region input currently uses
an outward conservative GUI.Drawing `Region.GetBounds` rectangle, and native
raster upload/presentation is still full-surface. Unbuffered ephemeral paint
uses a full clip because no prior private bitmap exists to preserve untouched
pixels.

Generated pointer, key, and text ingress is deferred whenever any managed paint
lease is active on the UI thread. One thread-local queue is bounded at 1,024
entries, preserves arrival order, and posts its drain only when the outermost
lease has released. Disposed targets are removed without invoking application
code. Input-caused invalidation then follows the ordinary callback-boundary
paint path. The internal conformance injectors prove pointer-before-key delivery,
no input callback inside `OnPaint`, one localized follow-up, and disposal from
`OnPaint` without querying a retired native peer under physical Wine. Complete
key-preview return-value parity and native pressure/stall breadth remain open.

Portable `Window::dispatch_pointer`, `dispatch_key`, `dispatch_text`,
`dispatch_drag`, and `perform_semantic_action` share a renderer-neutral
lease-time queue. `maximum_deferred_inputs` is 1,024. Consecutive moves from
the same pointer and consecutive overs from the same drag session compact to
their latest event; down/up/wheel, drop/leave, key, text, and semantic actions
retain causal order. Deferred drag returns only a previously negotiated effect
from the same active session that remains allowed; new sessions return none.
Semantic requests copy stable ID, action, and value, then resolve against the
live tree after release. One posted dispatcher drain runs after the outermost
lease; root retirement and dispatcher shutdown explicitly abandon pending
input. `DeferredInputSnapshot` reports pending/capacity, deferred, delivered,
move/drag compaction, capacity rejection, abandonment, faults, and drain state.
A 100-move pressure probe, 32-over drag probe, and semantic Press probe return
to zero work after ordinary later dispatch/paint in normal and renderer-free
builds.

`HostDispatchResult.input_deferred` distinguishes accepted lease-time
retention from an immediately handled route.
`input_capacity_rejected` distinguishes a fixed-bound rejection from an
ordinary unhandled route. The headless deterministic trace publishes both
fields. `DragDispatchResult` carries equivalent `deferred` and
`capacity_rejected` facts alongside the prior valid effect. Normalized-host
conformance forces pointer and drag events through the boundary from inside
application paint.

Frame and UI-timer callback faults are isolated per request. The failing lease
disconnects, `FramePollResult::callback_faults` and
`MetricsSnapshot::frame_callback_faults` count it, and healthy scheduled
surfaces continue. Native scheduler callbacks contain any residual C++
exception instead of allowing it to cross an AppKit block or Win32 timer
callback.

One outer `poll_frame_schedule` owns a strong, fixed due set. Stop, restart,
peer cancellation, owner disposal, and callback-created requests are safe;
new requests are admitted on the next host poll even when already due. A
recursive poll delivers no callbacks and reports
`FramePollResult::reentrant_poll_deferred`; the current next wake remains
visible. `MetricsSnapshot::reentrant_frame_polls_deferred` counts this bounded
deferral for diagnostics and future host availability projection.

`Window` is not yet a reusable `Form` control or public top-level-window facade.
Platform window creation lives in private host adapters.

## Portable host protocol

Protocol version 5 declares capabilities for lifecycle, scale, monitors,
occlusion, scheduled wake, input, text composition, capture, cursor, clipboard,
typed drag destination, dialogs, menus, font discovery, accessibility, and drag
source.

`HostSessionSnapshot.phase` is the portable lifecycle authority:
`constructed → attached → close_authorized → closed → shutdown`. A cancelled
close remains attached; a forced native close may move directly from attached
to closed. Pre-attach input, duplicate attach, input after close authorization,
and non-shutdown work after closed are rejected as `invalid_lifecycle` without
mutating retained state. Backend-private native creation and handle binding must
complete before the single attach event. See [LIFECYCLE_CONTRACT.md](LIFECYCLE_CONTRACT.md)
for the initialization transaction and compatibility-handle rules.

The current headless/AppKit work implements a bounded subset including monitor
records, cursor, capture, clipboard text, typed inbound drag data, and five
dialog request/result families. Capability discovery is mandatory; absence is
reported as `unsupported`, not simulated silently.

The bounded Windows adapter implements native window lifecycle, translated
pointer/keyboard input, CPU DIB presentation, stable-ID automation, capture,
clipboard, monitor geometry, sound cues, five common-dialog families, and close
under both Wine and the cross-compiled PE64 lane. Physical-Windows dogfood and
the Wayland/X11 adapters are not implemented yet.

## Experimental C ABI 0.x

Include:

```c
#include <gui_forms/c_api.h>
```

`gf_get_api_v0` negotiates a size-prefixed `gf_api_v0` table. ABI 0.21 preserves
the 0.1 prefix: generational handles, errors, generic creation, lifetime,
component state, stable ID, visibility, bounds, child add/remove, and generic
state-change subscription. Its appended operations add kinded creation,
length-delimited UTF-8 name/text, enabled state, blocking top-level host entry,
and a length-delimited final host trace. ABI 0.4 additionally provides typed
tokenized click/form-lifecycle subscriptions, queued host dispatch, requested
close, and callback-fault counts. `run_window` accepts explicit automation-
close, force-headless, and automation-activate flags; ordinary callers use the
default. Additive 0.5–0.19 operations cover retained raster leaves, pointer,
choice/range state, keys/composed text, capture, host dialogs/tooltips,
selection/caret/edit/history, clipboard, unencoded paint surfaces,
renderer-authoritative bounds, form key preview, and cursor roles. These raster
operations do not constitute GUI.Drawing.

ABI 0.20 adds renderer-neutral retained scrolling: `Control.AutoScrollOffset`,
automatic viewport/margin/minimum-size/position mutation, complete axis state,
manual axis projection, and `ScrollControlIntoView`. The returned snapshot
contains positive internal position plus negative-origin display and clipped
viewport rectangles, along with the last real scroll event and its monotonic
revision; generated WinForms properties and `Scroll` delivery consume that
state rather than maintaining a second managed layout engine.

ABI 0.21 adds renderer-neutral per-control layout transactions:
`suspend_layout`, `resume_layout`, `perform_control_layout`, and
`get_layout_state`. The snapshot reports nested depth, deferred state, and
monotonic requested/committed revisions. A suspended subtree retains its last
committed geometry and does not block runnable siblings; final resume or a
later explicit/read-barrier flush commits through the same bounded scheduler.
The generated facade uses this state for exact `SuspendLayout`, `ResumeLayout`,
`PerformLayout`, and `LayoutEventArgs` behavior rather than a managed-only
counter.

Custom native layout controls use `snapshot_layout_children()` and
`is_current_layout_child()` around application-overridable measurement. The
framework applies the same snapshot/revalidation law in Window recursion,
AutoSize/Dock/Anchor, FlowLayoutPanel, TableLayoutPanel, scrolling extent,
Card, and MasterDetailView. Callback-time removal, reparenting, disposal, or
addition therefore cannot invalidate a traversal or assign a slot through a
former parent; additions remain dirty for a following bounded pass.

Callback-bearing paint, hit-test, semantic, semantic-action, popup, and bulk
validation traversals use the same strong-identity arbitration law. Semantic
snapshots retry after a declared generation change and reject recursive or
four-pass nonconvergent construction. Hit testing also retries at most four
times and never returns a target disposed by its own local hit callback.
`MetricsSnapshot::callback_arbitration_retries` and
`callback_arbitration_limit_hits` make stabilization and pathological callback
churn visible to diagnostics and future host availability projection.

Typed callbacks return continue, cancel, or faulted. `FormClosing` cancellation
prevents native close; faults are counted and do not cross the ABI. Dispatch
callbacks run on the owning host thread or receive `cancelled=1` during host
shutdown. The current typed event set is `Clicked`, `FormClosing`, `FormClosed`,
range value/scroll, form bounds change, and retained-container `Scroll`; it is
not the final 1.0 event record.

The C++ `gui_forms::abi0` wrapper demonstrates table negotiation, RAII handles,
copy retain/release, moves, disposal, and status-to-exception translation.

ABI 0.x remains intentionally incomplete and is not a binary compatibility
promise. C11, C++20, generated-managed, host-.NET, and Wine smoke lanes now
exercise its high-frequency control spine.

### Independent GUI.Drawing ABI 0.2

```c
#include <gui_forms/drawing_c_api.h>
```

`gd_get_api_v0` negotiates a separate size-prefixed table. Generational handles
cover brushes, pens, fonts, formats, recorders, paths, regions, logical and
owned images, and image attributes. The table also exposes renderer-service
attachment and D8 opaque native-handle operations. Windows implements HBITMAP
import/export, HDC/HWND capture/present, and tokenized bitmap-backed HDC leases;
other hosts return explicit unsupported results. The table exposes distinct
stale-handle, wrong-kind, wrong-thread, disposed, buffer, version, and limit
results; no exception crosses the boundary. The shared library exports no
other symbol. This ABI is experimental and deliberately not merged with the
GUI.Forms ABI 0.24 table.

ABI 0.2 appends bounded bitmap edit begin/commit/cancel and multi-consumer
damage queries. ABI 0.1 remains negotiable through its size-prefixed prefix;
the new table reports the version requested by the caller. C11 tests cover
byte restoration, exact generation changes, stale token rejection, local
rectangles, buffer sizing, and the legacy negotiation prefix.

## Compatibility laboratory

The opt-in Capture-1 tooling classifies all 1,952 authoritative retired compatibility specimen evidence
rows with zero unclassified entries. Static IL evidence currently yields 797
required GUI.Forms-facade rows. The same catalogue assigns 353 drawing rows to
`gui_drawing_compat_facade`: 307 are required and 46 metadata-only rows are
deferred. Component-model rows retain separate runtime ownership.

A generated replacement surface now resolves 1,104/1,104 required identities:
797 Forms rows and 307 Drawing rows. The generated `Control`
hot path uses the ABI 0.x prefix for construction, name/text, enabled/visible, bounds, parenting,
deterministic subtree disposal, callbacks, UI dispatch, and top-level host
projection. Generated `Application.Run(Form)` and `Run(ApplicationContext)`
enter the deterministic headless host or the compiled platform host. A
165-control unchanged-specimen surface now projects retained controls, fields,
resource images, raster leaves, pointer activation, and disposal under Wine.
The focused Drawing facade smoke uses native GUI.Drawing with no Microsoft
drawing call. The unchanged Wine specimen still awaits the Windows x64 private
Skia raster DLL, so its prior intermediate-PNG paint remains passthrough rather
than closure. See `../planning/GUI_DRAWING_REVISION_PLAN.md`.

## Deliberately absent today

- production-complete multi-form/application-context semantics, scrolling and
  scrollbars, Dock/Anchor/table/flow layout;
- grapheme-aware editing, shaping/fallback, IME, selection, clipboard commands,
  and undo;
- grid/complete-toolstrip/background-worker
  control families;
- arbitrary object/collection property metadata, complete stock change events,
  DML/managed descriptor projection, custom property editors, and atomic
  multi-property rollback;
- accessibility publisher and complete semantic tree;
- DML parser/compiler/designer;
- stable ABI 1.0, behavior-complete generated C# assembly, analyzer, and NuGet packages;
- production Windows host services and Wayland/X11 hosts.

See the completeness matrix before depending on an unlisted WinForms-shaped
member.
