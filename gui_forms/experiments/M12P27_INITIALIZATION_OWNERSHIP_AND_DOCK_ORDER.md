# M12-P27 — initialization ownership and Dock order

Date: 2026-08-07

## Status and question

- **GIVEN:** initialization must not publish callbacks into partially
  constructed controls.
- **OBSERVED:** native `Control::begin_init()` previously coalesced dirty state
  only. Stock property events remained synchronous, so a native subscriber
  could re-enter a setter during initialization.
- **OBSERVED:** the retained child vector is painter order (backmost first),
  while the public child index is Forms order (index zero topmost). Paint,
  hit-testing, `BringToFront`, and `SendToBack` already agree on that mapping.
- **OBSERVED:** default Dock layout alone consumed children topmost to
  backmost. This was an arbitrary exception, not a renderer or performance
  requirement.

## Implemented contract

`Control` now owns an initialization publication queue. A stock state-change
event is synchronous outside initialization. Inside nested initialization it
is retained by event identity, later writes replace the pending payload, and
the outer `end_init()` applies accumulated invalidation before publishing each
event once in first-event order. `initialization_completed` is the final
barrier. Disposal discards pending publications.

Entering the first initialization level also revokes focus, pointer capture,
press/hover state, popups, and focus scopes for the control subtree. The
control is excluded from hit-testing, focus, commands, and mnemonic activation
until commit. Popup/focus-scope closure, capture/availability events, and focus
virtual/event callbacks caused by revocation use the same deferred boundary;
`begin_init()` therefore does not synchronously enter the partial control
through cleanup observers.

The policy is implemented in the native retained core and used by base,
geometry, text, choice, range, collection, tab/split, date, material,
inspection, and property-grid state events. A managed adapter must still avoid
reflecting an API-originated setter back through its native callback, but it no
longer needs a different native initialization rule.

Default Dock layout now traverses the retained painter-order snapshot directly:
backmost to topmost, which is reverse public z-order. Edge controls placed
behind a Fill control reserve their client area before Fill. Public index zero
remains topmost; paint stays back-to-front and hit-testing front-to-back. Dock
therefore does not introduce a second z-order model or a compatibility-only
layout fork. Traversal remains linear and retains the existing mutation-safe
strong-identity snapshot.

## Correctness oracles

- nested initialization publishes no partial text event, coalesces two writes
  to the final value, commits one dirty mutation, then emits completion;
- disposal during initialization abandons completion/publication;
- initialization revokes focus/capture, blocks routed activation, defers the
  focus callback plus popup/focus-scope/capture notifications, and restores
  ordinary input eligibility at commit;
- a native custom-control specimen uses the protected publication seam and
  proves coalesced initialization plus ordinary synchronous delivery;
- five-way Dock geometry proves edge reservation before Fill when edges are
  behind it;
- two same-edge siblings prove reverse-z order and deterministic
  `set_child_index` relayout;
- existing overlap tests continue to prove index-zero-topmost hit-testing,
  paint order, `BringToFront`, and `SendToBack`.

## Measurements

On the recorded development host:

```text
ctest --test-dir build --output-on-failure -j 6
63/64 passed on the first parallel pass; the AppKit scheduled-wake timing test
failed its autonomous timing window and passed immediately in isolation.

ctest --test-dir build -R gui_forms_macos_host_close_tests --output-on-failure
1/1 passed

cmake --build build -j 6
ctest --test-dir build --output-on-failure -j 6
64/64 passed

ctest --test-dir build-renderer-free-polish --output-on-failure -j 6
49/49 passed

CMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror
  -fsanitize=address,undefined -fno-omit-frame-pointer"
gui_forms_lifecycle_controls_tests: passed
gui_forms_layout_panel_tests: passed

cmake --build build-win64 -j 6
strict Win64 cross-build completed, including the native demos and tests
```

The initial failure did not touch initialization or layout code and reproduced
the suite's timing-sensitive host-wake lane. It is retained above rather than
erased; the complete rebuilt follow-up passed in one run.

## Open edges

- Managed reflection must classify setter-originated versus native-user-
  originated notifications at its own boundary.
- Change records that expose previous/current values currently coalesce to the
  latest complete record; an aggregate first-previous/final-current policy
  needs an explicit per-event contract before it is generalized.
- Independent .NET 10 Dock ordering traces and physical Windows focus/capture
  initialization traces remain useful compatibility evidence.
