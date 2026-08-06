# GUI.Forms lifecycle contract

Status: **DECIDED** by ADR-014; implemented for the portable core, headless,
AppKit, Win32, the C ABI host bridge, and the generated Forms facade.

## 1. The identities are deliberately separate

| Identity | Created | Authority | Lifetime |
|---|---|---|---|
| retained control identity | control construction | properties, tree membership, events, rendering state | construction to disposal |
| retained window/session | a root is run | portable geometry, input, presentation and close order | one host run |
| native host identity | inside the platform adapter | OS presentation and platform services | native create to native destruction |
| compatibility handle lease | a compatible consumer reads `Control.Handle` | bounded native/GDI interoperability only | first acquisition to disposal |

A retained identity is never cast or returned as a native handle. A compatibility
lease never becomes the visual/input authority for the retained tree.

## 2. Portable host state machine

`HostSessionSnapshot.phase` is authoritative.

| Current phase | Accepted transition/event | Next phase |
|---|---|---|
| `constructed` | valid `HostAttachEvent` | `attached` |
| `constructed` | `HostShutdownEvent` | `shutdown` |
| `attached` | resize, scale, display, activation, occlusion, input, drag | `attached` |
| `attached` | cancelled close request | `attached` |
| `attached` | allowed close request | `close_authorized` |
| `attached` | forced/native `HostClosedEvent` | `closed` |
| `close_authorized` | deactivation or becoming occluded | `close_authorized` |
| `close_authorized` | `HostClosedEvent` | `closed` |
| `close_authorized` | `HostShutdownEvent` | `shutdown` |
| `closed` | `HostShutdownEvent` | `shutdown` |

All other transitions return `HostDispatchError::invalid_lifecycle`. Events
after `shutdown` return `after_shutdown`. Rejected events do not advance the
accepted sequence or mutate the retained window.

`closed` synchronously revokes dispatcher work, frame requests, pointer capture,
and drag state. `shutdown` is terminal and detaches host services.

## 3. Initialization transaction

Every presenting backend follows this order:

1. Construct retained controls and subscribe callbacks.
2. Construct the retained `Window`; its subtree attachment transaction completes.
3. Begin native surface creation.
4. Secure/bind the native surface identity privately.
5. Establish ownership, client geometry, scale, renderer resources, and service
   ownership.
6. Emit exactly one accepted `HostAttachEvent`.
7. Publish wake, close, dialog, tooltip, and clipboard services.
8. Drain one FIFO snapshot of already-posted initialization work. The generated
   Forms facade raises `Load` here. Work posted by `Load` remains deferred to an
   ordinary next dispatch turn.
9. Commit the adapter as ready and request native presentation.
10. Accept activation, input, and paint arising from the presented surface.

Properties and child controls may be mutated during step 8. Their damage is
coalesced into the first presentation; no intermediate half-initialized frame is
contractual. The single-turn rule prevents recursive `BeginInvoke` chains from
starving first presentation and preserves the dispatcher snapshot contract.

## 4. Native adapter rules

### Win32

`CreateWindowExW` reenters the WndProc. `WM_NCCREATE` extracts the adapter state
from `CREATESTRUCT::lpCreateParams`, stores it in `GWLP_USERDATA`, and binds the
HWND. Creation-time messages are fenced from portable dispatch until
initialization accepts `HostAttachEvent`. `ShowWindow` occurs only after the
managed initialization queue has drained.

### AppKit

The `NSWindow` owns its `GUIFormsView` before `initializeHost` emits attach.
Host services and managed initialization are completed before
`makeKeyAndOrderFront:`. View notifications received before attach do not enter
the portable session.

### Headless

The deterministic adapter emits attach, drains one initialization turn, then
delivers configured synthetic activation/input unless initialization requested
close. This is the reference ordering used by conformance tests.

## 5. Retained attachment is a separate axis

Controls use `detached → attaching → attached → detaching → detached`, with
`disposed` terminal. Attachment callbacks are transactional: structural
mutation during notification is rejected; a failed attach rolls the complete
subtree back before the exception escapes. A control may be reparented without
creating or recreating a platform window.

## 6. Managed Form presentation

The generated facade uses:

```text
constructed/closed -> initializing -> ready -> closing -> closed
                                      ^          |
                                      +--cancel--+
```

`Application.Run`, `ShowDialog`, native popups, and hosted secondary forms all
attach/publish their host or retained parent before running `Load`. `Load` is
idempotent for a retained instance. Input cannot precede it. `FormClosed` is
terminal for that presentation; a later admitted presentation starts a new
initializing phase without repeating the instance's one-time load callback.

## 7. Compatibility handle lease

On Windows, accessing `Control.Handle` may allocate an invisible, disabled,
offscreen native surface so unchanged code can safely call native/GDI APIs. It
is not parented, shown, focused, hit-tested, or composited as a native child.
Explicitly admitted direct-GDI controls may have its pixels sampled into the
retained raster.

The lease state is:

```text
absent -> leased -> released
```

First successful acquisition raises `HandleCreated`. Re-reading is idempotent.
Disposal raises `HandleDestroyed` once before releasing the native surface.
`IsHandleCreated` reports this lease only; it does not report retained attachment
or existence of the top-level host window.

## 8. Required conformance cases

- activation/input before attach is rejected;
- duplicate attach is rejected;
- cancelled close returns to attached;
- allowed close suppresses input;
- closed accepts only shutdown;
- initialization callback order is `Load → input → closing → closed`;
- a close requested by `Load` suppresses synthetic/user input before terminal
  close processing;
- pre-host posted callbacks run after attach and before presentation;
- nested posted callbacks remain separate dispatch turns;
- Windows handle acquisition/destruction raises exactly one matching event;
- every admitted native backend compiles and produces the same portable phase
  trace for equivalent events.
