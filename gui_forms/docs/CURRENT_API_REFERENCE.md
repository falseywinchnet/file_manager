# Current GUI.Forms API reference

Status: **OBSERVED C++/C ABI 0.x surface as of 2026-08-06**. This is a concise
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
recorders. Brushes include solid, hatch, linear-gradient, and path-gradient
resources.

The recorder owns a bounded state stack and typed snapshot commands for clear,
rectangle, line, string, ellipse, polygon, path, and logical image operations.
`gui.drawing.trace/v1` includes resource values, transforms, clip,
quality/compositing state, UTF-8 text, command count, and close state. A
consumer-supplied `TextMetricsProvider` keeps measurement policy outside the
core until the shared text service is connected.

The core remains renderer-free. A separately packaged private raster service
executes retained commands and bounded PNG through CPU-only Skia. The current
archive is measured on macOS arm64; Windows x64 packaging remains open.

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

- requested, arranged, committed-arranged, and absolute bounds;
- retained `visible`, `enabled`, `focusable`, `allow_drop`, and inherited
  cursor state;
- application-owned `Tag` storage with exact `std::any` type retention and
  synchronous release during disposal;
- effective visibility/enabled/input eligibility queries;
- `PaintPlane` selection and display-chunk information.

Reading arranged or absolute geometry may execute the declared layout read
barrier. It is not a raw field read.

### Invalidation

- `invalidate(Dirty)` marks the affected control/path;
- `invalidate_subtree(Dirty)` explicitly marks every descendant;
- `invalidate_declared(Dirty)` rejects undeclared effects in development and
  conservatively invalidates in production;
- dirty vocabulary distinguishes measure, arrange, paint, hit test, text,
  style, resource, semantics, and accessibility.

### Initialization lifecycle

- `begin_init()` and `end_init()` nest;
- property events remain synchronous during initialization;
- dirty effects coalesce until the outer `end_init()`;
- `initialization_completed()` emits the combined dirty flags and whether the
  operation required a subtree mark;
- unmatched `end_init`, disposed mutation, and wrong-thread mutation are
  rejected.

Subclass hooks `on_attached_to_window`, `on_attachment_committed`, and
`on_detached_from_window` define the current deterministic lifecycle. Attachment
observes a fully bound subtree parent-first. Detachment observes a fully unbound
subtree child-first. The commit hook is nonthrowing and runs only after all
throwing attach hooks succeed.

`pointer_observed`, `focus_observed`, and `arranged_bounds_changed` provide
tokenized observation for nonvisual providers without subclassing a target.

### Custom-control overrides

`measure`, `arrange`, `on_paint`, `hit_test_local`, pointer/key/text/drag route
hooks, focus notification, and activation are available. Drawing uses the
renderer-neutral `Painter` vocabulary.

## Containers

### `ContainerControl`

- `contains_descendant` tests retained logical containment;
- `active_control` returns the focused descendant when present;
- `request_active_control` and `clear_active_control` use the owning window's
  focus contract.

Validation, scaling, scrolling, and dialog-key routing are not implemented.

### `UserControl`

- `loaded()` is a tokenized one-shot lifetime event;
- `is_loaded()` means load notification has begun at least once;
- `is_attached()` exposes current attachment;
- `attachment_count()` counts successful whole-subtree attachment commits.

An attach attempt that later rolls back cannot erase an already observed
`loaded` callback, but it does not increment `attachment_count`.

## Basic controls

### `Panel`

Properties: `BorderStyle`, background, and provisional `BasicControlStyle`.
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

Text, font, foreground, horizontal alignment, synchronous `text_changed`,
measurement, and nonintercepting hit testing. Ellipsis, multiple link spans,
mnemonics, and the final text engine are incomplete.

### `ButtonBase` and `Button`

Text/font/style, press/focus visual state, synchronous `clicked` and
`text_changed`, pointer activation, normalized keyboard activation, and default
button cue. Complete form accept/cancel routing and mnemonics are open.

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
marquee and pulse animation through active-surface deadlines. It is
nonfocusable, does not intercept hit testing, becomes scheduler-quiescent when
paused, disabled by application policy, or effectively hidden, and publishes
numeric or busy semantics. Reduced motion remains busy and visibly animated at
the calmer public policy cadence/speed/excursion without resetting phase. The
older pause and reduced setters delegate to the same atomic policy transaction.

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
  goes away. Accessible described-by relations, title/icon/balloon variants,
  HelpProvider, and ErrorProvider remain open.

## `Window`

`Window` owns one retained root and one UI thread. It provides:

- resize and logical scale;
- nested update scopes, explicit layout, flush, paint, and per-plane damage;
- retained lookup by stable ID and recursive hit testing;
- focus, pointer capture, pressed state, and routed input dispatch;
- nested focus scopes and owner-tokenized retained popup attachment;
- frame scheduling, active surfaces, occlusion, and wake deadlines;
- UI timer callbacks plus `check_access`/`verify_access` thread guards;
- validated PNG load/replace/remove through the resource registry;
- renderer-free semantic snapshots and stable-ID action routing;
- structured metrics and activity reset.

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

`gf_get_api_v0` negotiates a size-prefixed `gf_api_v0` table. ABI 0.19 preserves
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

Typed callbacks return continue, cancel, or faulted. `FormClosing` cancellation
prevents native close; faults are counted and do not cross the ABI. Dispatch
callbacks run on the owning host thread or receive `cancelled=1` during host
shutdown. The current typed event set is `Clicked`, `FormClosing`, and
`FormClosed`; it is not the final 1.0 event record.

The C++ `gui_forms::abi0` wrapper demonstrates table negotiation, RAII handles,
copy retain/release, moves, disposal, and status-to-exception translation.

ABI 0.x remains intentionally incomplete and is not a binary compatibility
promise. C11, C++20, generated-managed, host-.NET, and Wine smoke lanes now
exercise its high-frequency control spine.

### Independent GUI.Drawing ABI 0.1

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
GUI.Forms ABI 0.19 table.

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
- date/grid/menu/toolstrip/help/error-provider/background-worker
  control families;
- property metadata/default/reset/serialization registry;
- accessibility publisher and complete semantic tree;
- DML parser/compiler/designer;
- stable ABI 1.0, behavior-complete generated C# assembly, analyzer, and NuGet packages;
- production Windows host services and Wayland/X11 hosts.

See the completeness matrix before depending on an unlisted WinForms-shaped
member.
