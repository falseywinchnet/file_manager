# ADR-014: GUI.Forms initialization and handle lifecycle

Status: **accepted for implementation**

Date: 2026-08-06

Owner approval: grand architect direction in the GUI.Forms compatibility and lifecycle
work: correctness is the portable authority; compatibility safely projects the
behavior programs expect without importing Windows' internal ordering.

## Question

What lifecycle governs retained controls, host windows, native identities,
managed initialization callbacks, presentation, close, and compatibility
handles?

## GIVEN constraints

- GUI.Forms targets Windows, macOS, and Linux equally. Portable event order may
  not encode Win32, AppKit, Wayland, or X11 accidents.
- The core is retained and explicitly owned.
- Correct behavior is the smallest deterministic and safe lifecycle.
- WinForms-shaped consumers may obtain compatible behavior, but an opaque
  retained identity must never be passed off as a native window handle.
- Initialization must complete before input and first presentation. Close and
  shutdown must revoke further work deterministically.

## Workloads and failure modes

The contract covers ordinary windows, owned/tool windows, modal dialogs,
popups, headless tests, controls added during initialization, early close,
cancelled close, disposal, and direct-GDI compatibility consumers.

Failures to prevent:

- callbacks observing a half-created host;
- paint, resize, activation, or input entering before attachment;
- different `Load` order on Windows, macOS, dialogs, and headless runs;
- duplicate attach, close, or terminal callbacks;
- queued work surviving close;
- treating retained ABI identity as an HWND;
- tying a compatibility paint surface to the authoritative visual/input tree;
- reentrant native creation messages mutating uninitialized renderer state.

## Candidates

1. Copy Win32/WinForms handle creation and message ordering into the portable
   core.
2. Keep the existing distributed booleans and document best-effort ordering.
3. Use separate explicit state axes for retained attachment, portable host
   presentation, managed Form presentation, and optional native handle leases.

## Evidence and measurements

**OBSERVED:** Win32 calls the window procedure synchronously from
`CreateWindowExW`; `WM_NCCREATE` therefore binds identity before the create call
returns. AppKit creates the `NSWindow`, view, and content relationship through a
different sequence. The previous adapters published readiness on opposite sides
of first presentation.

**OBSERVED:** the managed facade previously created an offscreen HWND from
`Control.Handle`, while the visible retained host owned a different HWND.
`HandleCreated` and `HandleDestroyed` were declared but not raised.

**MEASURED:** the explicit host transition tests reject pre-attach activation,
duplicate attach, post-close-authorized input, and post-closed resize. Headless
and Windows managed smoke tests observe
`Load → input → FormClosing → FormClosed`; Windows also observes exactly one
compatibility-handle create and destroy notification.

## Decision

Candidate 3 is accepted.

The portable host session is authoritative:

```text
constructed -> attached
      |             |-- close(cancelled) -> attached
      |             |-- close(allowed) -> close_authorized -> closed -> shutdown
      |             +-- forced/native close -----------> closed
      +------------------------------------------------> shutdown
```

Native adapters privately perform `create → bind native identity → configure`
before emitting `attached`. They then publish host services, drain one bounded
FIFO initialization turn, and only then present the surface. Work posted by an
initialization callback is ordinary next-turn work, not recursively folded into
initialization. No portable input or paint callback may escape before attachment
and the initialization turn.

Managed Forms use a separate presentation state:

```text
constructed/closed -> initializing -> ready -> closing -> closed
                                      ^          |
                                      +--cancel--+
```

Retained ABI identities and native compatibility handles are different objects.
On Windows, `Control.Handle` is an optional offscreen paint lease for unchanged
native/GDI consumers. Its independent lifecycle is `absent → leased → released`.
It has no input, ownership, visibility, or compositor authority.

## Why the other candidates lost

Candidate 1 makes every backend emulate Win32 reentrancy and handle recreation,
even where neither is useful nor safe. Candidate 2 cannot reject illegal order
and already produced divergent first-state, theme, popup, and paint behavior.

## Consequences

- `HostSessionSnapshot.phase` is the inspectable portable authority.
- Invalid transitions return `invalid_lifecycle` without mutating retained state.
- Initialization callbacks run after host attachment/service publication and
  before the first visible presentation.
- Cancelled close returns to `attached`/`ready`; allowed close suppresses new
  input until the terminal close.
- Handle-lease events are truthful but do not imply that each retained control
  is a native child window.
- Backend-specific creation phases remain private and must satisfy the same
  boundary tests.

## Reversal and migration path

An admitted native-child control family may replace its paint lease with a real
owned native child surface behind the same explicit lease contract. It must not
change portable host ordering. Adding asynchronous presentation requires new
portable states and fixtures rather than interpreting an existing boolean.

## Unresolved edges

- Linux host adapters must demonstrate the same trace when admitted.
- Native handle recreation is not currently supported. A consumer requiring it
  needs an explicit repeatable lease-generation contract and tests.
- Async modal presentation remains part of the pending first-party application
  surface negotiation.
