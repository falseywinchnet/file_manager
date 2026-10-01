# Games window cursor interaction — proposal 001

Status: **consumer-confirmed development proposal; first Windows slice assigned
after stable audio handoff; API/status/identity semantics reconciled in reply 003;
no runtime availability**.

## Request and observed source

**OBSERVED** — The coordinator routed Games Switchbox's owner-authorized remake
need for balanced cursor hide/restore and client-coordinate warp. The consumer
chat is `01a0f6b6-22cd-7b33-9b8f-293c735cf84f`; Switchbox source is reported at
`de23ab8` in its Games integration. This is separate from audio and text masks.

`gui_forms/include/gui_forms/host/services/host_services.hpp` exposes cursor kind,
custom cursor and pointer capture through a UI-thread host-service boundary. It
does not declare a public warp operation. The coordinator reports its host/core
audit found no `SetCursorPos` route. A custom transparent cursor is image-based
appearance, not evidence of native visibility-state ownership or pointer warp.

## Proposed minimal semantics

Names describe operations, not frozen C++ spelling or ABI. All operations use
one owning UI executor and an exact live window instance/generation. No global
desktop target, process-wide visibility counter API or game-local OS handle is
exposed. Host capability reports hiding and warping independently per backend.

### Scoped visibility

`begin_cursor_hidden(window)` returns one unique lease or a typed refusal.
Only one lease is active per window; a second independent acquisition returns
busy rather than nesting hidden counters. The host checks visible live client,
foreground/focus authority and modal suppression before changing native state.
Failure returns no lease and preserves applicable cursor behavior.

The lease hides only within its admitted client interaction scope. It is not
pointer capture or confinement and cannot hide an unrelated application's
cursor. Leaving scope restores normal behavior. Releasing is idempotent and
restores the currently applicable cursor rule without this lease, not a stale
saved shape that overrides later theme/control changes. Repeated requests must
not increment/decrement an uncontrolled native visibility counter.

Focus/foreground loss, capture loss if capture is part of the interaction,
window close/detach, host shutdown and explicit cancellation revoke the lease
and restore visibility. Regaining focus never silently reactivates it; a new
interaction must acquire a new generation. Native cleanup precedes owner/window
release. Lease destruction is on the owning executor; no deferred callback may
borrow a destroyed window. Native restoration failure remains inspectable and
must not be reported as successful restoration.

### Client-coordinate warp

`warp_cursor(window, client_metrics_generation, target_dip)` validates finite
coordinates, a nonempty client extent, current metrics/DPI and the same focus/
foreground authority. It rejects stale metrics, negative/out-of-client points,
overflow, nonfinite scale and unsupported backend before attempting native work.
Coordinates are window-client DIPs, never screen pixels or document offsets.

Candidate conversion: use current physical client bounds and positive scale;
floor each nonnegative DIP coordinate times scale to a client pixel, check its
integer representation and half-open client bounds, then use the native client-
to-screen transform. Validate resulting screen representation before the native
call. No implicit clamping, guessed DPI or overflow narrowing. Screen coordinates
remain private. Resize/DPI changes invalidate the metrics generation.

Warp does not imply capture, confinement, focus acquisition or clicking. A
successful result means the native placement request succeeded at that instant;
later user movement is unconstrained. Focus authority is rechecked immediately
before execution. No queued warp survives focus loss or close. The minimum
operation is synchronous and retains no target; any later coalesced dispatch
would require explicit semantics rather than an unbounded command queue.

### Motion and results

Warp may cause ordinary native pointer-motion delivery, coalescing or no distinct
event. Success is not a fabricated pointer event or evidence of human movement.
The minimum contract promises neither suppression nor reliable causal tagging
of subsequent OS motion. Games must avoid a warp/motion feedback loop and must
not interpret completion as a click or new physical gesture. If origin tagging
is later added, unknown origin must remain distinct from hardware or injected.

Applicable typed outcomes: success, unsupported, denied, wrong-thread,
invalid-coordinate, stale-window, stale-metrics, busy, revoked, closing and
native-failure. Expected refusal does not silently fall back to a transparent
cursor or successful no-op. Hide lease lifetime is independent of warp success;
failed warp never strands an unowned hidden state. Cleanup statuses and current
lease authority are inspectable without allocating on motion hot paths.

## Replies, ownership and validation

Games owns its vendor core/tests and confirms holding/drop/cancel behavior,
capture-loss policy and exact client target. Orchestrator reconciles semantics.
GUI.Forms provider must confirm native hide/restore feasibility and coordinate
exact HostServices/helper/host hunks with the coordinator before implementation.
Windows host is currently owned by the provider's A1 frame-transaction work;
this intake assigns no overlapping edits. No game-local Win32/AppKit code.
Any necessary Apple adapter follows the existing Games owner-reporting route.

Required focused cases: repeated release, duplicate acquisition, focus/capture/
close restoration, stale generations, invalid and edge coordinates at fractional
DPI, resize between target capture/use, native refusal and warp-induced motion.
Synthetic fixtures cannot establish native OS visibility, actual focus, event
delivery or cross-backend support. Matched SDK and independent consumer/native
evidence are required before advertising either capability. Full exact-scope
source review follows `planning/PROGRAMMING_HOUSE_STYLE.md`.

## Consumer reply 001

**OBSERVED** — Games confirms hiding only while Switchbox holds/carries an item
inside a focused visible client. Drop, cancellation, focus loss, capture loss
and close restore visibility; capture loss also cancels the game's hold state.
There is no confinement request. Foreground loss stops carrying before any warp.

The drop target is the game's `Stage::to_screen` result in game pixels multiplied
by `pixel_` (currently two DIPs per game pixel), then offset by the control's
absolute bounds into window-content DIPs. That game method name does not make
the values native screen coordinates. The host still validates finite sums,
current client metrics and bounds. Outside/stale targets refuse as proposed.

Only one actor `take_drop_cursor` event requests a warp; subsequent pointer
motion updates ordinary hover rather than retriggering a warp. On denied or
unsupported placement the game restores the native cursor and reports the
limitation explicitly; it cannot claim the pointer moved. These consumer
semantics accept the lease proposal, not its native implementation. Provider
source feasibility and coordinated host scope remain the next steps.

## Coordinator development assignment — reply 002 pending

**OBSERVED** — After inspecting actual HostServices/WindowsHostServices and this
proposal, the coordinator assigned Games chat the first cursor development slice
after its stable audio handoff. This supersedes the earlier unassigned/A1-held
status above: A1 is now committed and frozen for this work.

The assigned scope is a new shared `cursor_interaction` lease/policy, cursor-only
HostServices changes, Window metrics/revocation, WindowsHostServices and Windows
host message routing, plus focused tests. Raster/text paths are excluded. Root
retains CMake integration; the existing provider has been notified of ownership.
Games must name the exact new files and existing cursor-only hunks and reconcile
public types/statuses before the adapter freezes. This is development authority,
not a frozen API or installed capability.

The concrete reply must map lease acquisition/release and warp outcomes to
typed public results, preserve inspectable native restoration failure, and
specify window/metrics/lease generations, checked finite-DIP conversion and
immediate focus/foreground authority validation. The consumer's drop/capture/
focus/close policy and no-confinement behavior remain authoritative requirements
for this slice. Motion generated by warp remains subject to the earlier native
event/coalescing limits; no fabricated hardware-origin claim is admitted.

Other backends default to explicit unsupported. No XFixes, Apple or Linux hook
adoption is authorized by this Windows slice. Native validation is restricted
to owned windows and current input permissions; no permission bypass or external
window testing is implied. Games owns implementation and its thin consumer;
Orchestrator owns this semantic reconciliation. Exact-scope house-style review,
focused tests and new matched SDK/native evidence remain separate requirements.

## Provider API reply 002 — pending identity/lifecycle corrections

Games proposes `CursorError` with the precise outcomes already listed above,
`CursorStatus::accepted()` and an explicitly lossy `host_status()` projection.
Success/unsupported/wrong-thread retain corresponding host outcomes; invalid
coordinate/stale identities project to invalid-argument; closing to after-shutdown;
denied/busy/revoked/native-failure to backend-failure. The precise cursor error
must remain available; callers cannot recover these distinctions from the host
projection alone. No existing HostServiceStatus enum or global capability enum
change is proposed. `CursorCapabilities` independently reports hide and warp.

Proposed public operations are Window metrics query, begin-hidden returning a
move-only lease plus status, and synchronous warp using a metrics snapshot and
Point. HostServices owns per-window policy and default-unsupported native hooks.
Shared terminal lease state may outlive the native window without retaining a
live native observer. A narrowly required HostSession detach hunk is under root
review; this record assigns no extra host edits.

Before adapter freeze, Orchestrator requests these concrete corrections:

- Metrics bind exact window-instance/lifetime identity as well as metrics
  generation. Validate the whole current extent/scale snapshot, not a caller-
  changed value under a matching generation. Same-shaped windows are distinct.
- Lease state explicitly distinguishes active, released and revoked;
  `accepted()==true` cannot alone imply currently hidden after release.
- Idempotent release preserves recorded native restoration failure and terminal
  revocation cause. Cleanup cannot silently overwrite failure with success.
- Restore/revoke before clearing the native observer; post-detach query/release
  uses terminal state only and cannot dereference the destroyed host.
- Wrong-thread explicit release returns wrong-thread without native calls or
  ownership transfer. Destruction's owner-thread precondition and cleanup/failure
  behavior must be explicit, not an implied cross-thread restoration guarantee.

Required focused cases add cross-window and forged same-generation metrics,
repeated failed restoration, retained terminal lease after detach and wrong-thread
explicit release. These are pending semantic corrections, not claims of tested
implementation. Consumer behavior and unsupported-platform scope are unchanged.

## Provider counterreply 003 — semantic corrections accepted

**OBSERVED** — Games accepts a unique `window_id` plus metrics generation,
extent and scale, all matched against current Window values. Lease snapshots
explicitly distinguish empty, active, released and revoked phases, with terminal
cause and restoration result stored separately. Repeated release preserves native
restoration failure. Wrong-thread explicit release returns wrong-thread and
retains authority; the destructor requires the owning UI executor and promises
no cross-thread native cleanup.

Host retains the lease state and revokes/restores before clearing observers.
An outstanding lease after detach cannot access a dead Window or HostServices.
The coordinator reserved only the HostSession attach/detach generation and
pre-clear revocation hunks to Games, not unrelated HostSession changes.

The proposed Windows adapter uses `SetCursor(nullptr)` only while the admitted
focused client owns the pointer; WM_SETCURSOR/mouse routing maintains the scoped
state. It uses neither the global ShowCursor counter nor a transparent bitmap.
Restoration queries current control policy; leaving the admitted scope ends the
lease. This records the adapter plan, not independently tested native behavior.

These replies reconcile the API/status/identity/lifecycle semantics for the
assigned development slice. Focused implementation and owned-window validation
remain required before adapter/SDK availability. No global desktop warp was
performed or authorized; native validation stays within owned windows and
current permissions. Other backends remain explicitly unsupported.
