# Dynamic independent document windows: provider audit

Date: 2026-10-01. **OBSERVED** source/header audit only. No host/API change or
runtime availability is authorized by this document.

## Verified current route

The source and frozen `house-style-final` installed public
`gui_forms/application/application.hpp` are byte-identical, SHA-256
`0a1f581f4e53e0b16a23534b6797d51223d911e2128acd5087d0abb97f993f50`.

The public `Application::run` consumes either one Window or a fixed vector of
`ApplicationWindow` entries. `ApplicationWindowHandle` exposes close, show,
hide and fullscreen only. No session/group handle exposes insertion of another
window during a running application.

`src/core/application/application.cpp`, `Application::validate`, permits at
most 64 initial windows and requires **exactly one** entry with no owner and
`tool_window == false`. It rejects an invisible or hide-on-close primary.
Initial owned/tool windows therefore do not supply several independent document
lifetimes. The public runtime bridge in `src/runtime/application/application.cpp`
prepares its native entries once before running the host. Its `RunningScope`
rejects a nested portable `Application::run` as `already_running`.

`src/host/windows/application/windows_host.cpp`, `run_windows_application`,
repeats the one-primary rule; allocates state/handle vectors from the initial
count; creates them before entering the message loop; marks the primary as
`quit_thread_on_close`; and destroys remaining windows after loop exit. Native
Win32 calls or low-level host wrappers are not a missing public portable dynamic
window capability.

**Conclusion within audited scope:** the consumer-reported gap is confirmed.
No current public Application route provides dynamic independent in-process
document windows. Existing owned dialogs and a separately launched executable
remain different behaviors, not evidence of this capability. No macOS/Linux
native lifecycle run was performed; portable validation alone already excludes
multiple independent roots.

## Separate candidate negotiation W1

**CANDIDATE**, not implementation permission: an additive application-session
or window-group capability with a weak UI-thread session handle and dynamic
create/close requests. Preserve the historical one-primary Application overload
unless a new explicit group mode is negotiated. Required decisions:

- Distinct document-root and owned-dialog identities with nonreused generation;
  no raw native handles in portable API. An owned dialog belongs to exactly one
  live document window. Closing one document does not close unrelated roots.
- Creation result explicitly distinguishes accepted/ready, unsupported, limit,
  wrong-thread, invalid-owner, shutdown, callback and backend failure. Decide
  whether caller retains model on rejection; do not inherit current run's
  consume-even-on-failure behavior accidentally. No partially attached model
  may escape failed native creation.
- Close intent allows owner save/cancel policy before committed teardown.
  Teardown revokes wake/dispatch, stops and drains document work, resolves owned
  dialogs exactly once and removes callbacks before releasing model storage.
  Ready/closing/closed ordering and callback reentrancy must be specified.
- Owned-modal suppression/focus restoration is scoped to its document root;
  concurrent independent documents remain operable. Owner destruction must not
  restore focus to a dead handle.
- Bound active/creating/closing window counts, queued operations and retained
  callback state. Last independent document closing initiates application exit
  only after queued creation/close policy is resolved; pending create cannot
  resurrect a quitting group. Explicit application quit and per-window close
  need separate cancellation/aggregation rules.
- Native capability reports Windows/macOS/Linux independently. Required native
  tests: create after first-ready, two roots, close either first, owned modal,
  cancelled close, create failure, close/create reentrancy, worker completion
  during close, stale handles and last-root exit.

Orchestrator registers W1 separately from D1-D4 and print P1. SwiftEdit confirms
consumer lifecycle semantics; parent coordinates runtime/host/shared build
ownership before any implementation. This audit does not authorize spawning
additional processes as a substitute or modifying a frozen SDK.
