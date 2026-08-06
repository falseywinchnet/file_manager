# M11h retained focus scopes

Date: 2026-08-05

Status: **MEASURED PARTIAL** renderer-free focus/focus-restoration substrate.

## Claim and boundary

The bounded claim is that GUI.Forms can own nested keyboard-focus scopes in
the retained portable core, without encoding AppKit or Win32 policy and without
leaving popup focus restoration to each consumer.

This slice does not claim a popup control, click-away dismissal, screen-edge
placement, independent native modeless windows, validation, mnemonics,
accessibility publication, DML, or C ABI exposure.

## Implemented contract

- Stable `FocusScopeId` values with a maximum nesting depth of 32.
- An active contained scope rejects focus moves outside its retained subtree.
- Nested contained scopes must remain descendants of their containing scope.
- Optional preferred focus and deterministic first-focus selection.
- Tab and Shift+Tab traverse eligible focusable controls in stable retained
  pre-order, wrap within the active scope, and establish initial focus when no
  control is active.
- Explicit close restores the focus recorded at entry. Out-of-order close is
  retained as a tombstone until deeper scopes close, then restores the
  outermost surviving history rather than an already-closed intermediate
  target.
- Hide, disable, detach, and disposal revoke scopes owned by the affected
  subtree. Ordinary owner unavailability reports a typed close reason and
  restores eligible outside focus; disposal retains the existing no-throw
  cleanup boundary.
- Scope open/close/restoration/rejection/depth counters are available through
  `MetricsSnapshot` and its deterministic JSON projection.
- Scope mutation preserves the renderer-free UI-thread guard.

## Fixtures and results

`tests/focus_scope_tests.cpp` covers containment, preferred focus, forward and
backward traversal, wrap, initial focus, nested out-of-order close, owner-hide
cleanup, typed change ordering, foreign nested-scope rejection, structured
metrics, and wrong-thread rejection.

**MEASURED:**

- Full macOS CPU-renderer/native-host build: 34/34 tests pass.
- Renderer-free build (`GUI_FORMS_ENABLE_SKIA=OFF`, AppKit host off): 26/26
  tests pass.
- Address/undefined sanitizer build with `-Wall -Wextra -Wpedantic -Werror`:
  focused scope test passes.

These results establish portable retained semantics only. They do not establish
native assistive-technology focus publication or unchanged managed-facade
popup behavior.

## Refined next dependency

The next popup slice should compose this core contract with retained popup
ownership, click-away/Escape dismissal, capture transfer, bounded nesting,
screen-edge placement, and semantic parentage. The C ABI and generated binding
must then project the same scope rather than preserve the current facade-local
global-focus approximation. Independent native modeless windows remain a
separate host-session refactor and must not be faked by child-form composition.
