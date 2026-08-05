# M6c initialization and composition lifecycle evidence

Date: 2026-08-04

Status: **OBSERVED implementation evidence for the third bounded M6 control
extraction**. M6 remains open. This slice does not claim a typed property
registry, default/reset/serialization metadata, validation, dialog-key routing,
scaling, scrolling, designer serialization, or a complete WinForms event
facade.

## Admitted contract

- **GIVEN:** GUI.Forms publishes one deterministic lifecycle/event order for its
  retained model. It does not emulate alternate historical orders.
- **OBSERVED:** `Control::begin_init`/`end_init` are nested UI-thread-guarded
  scopes. Property-specific events remain synchronous and ordered. Declared
  invalidation flags accumulate until the outer `end_init`, which applies one
  control or subtree dirty operation and emits one tokenized
  `initialization_completed` notification.
- **OBSERVED:** unmatched `end_init` is rejected. Disposal clears an unfinished
  batch and does not emit a completion event. Initialization does not weaken
  disposed-control or attached-control UI-thread guards.
- **OBSERVED:** attach binds the complete subtree before parent-first lifecycle
  callbacks. Detach unbinds the complete subtree before child-first callbacks.
  Structural tree mutation and disposal are rejected while lifecycle callbacks
  execute; ordinary property mutation remains admitted.
- **OBSERVED:** a throwing attach callback rolls back window pointers, parentage
  at the public insertion boundary, lifecycle state, and stable-ID
  registration. The failed stable ID is immediately reusable.
- **OBSERVED:** `UserControl::loaded` is a one-shot lifetime notification once
  its first attachment callback begins. A later descendant failure cannot undo
  an already observed user callback, but it does not increment the separately
  committed attachment count. Retry and same-window reparenting do not repeat
  `loaded`; each successful whole-subtree attachment increments the count.

## Gallery dogfood

The authoritative Gallery DML now declares `gallery.lifecycle-card` as a
`UserControl` with two ordinary reusable `Label` children. Its first load runs a
nested initialization batch and presents `Load 1 · attach 1 · init 1`.
`COMPOSED` uses the Portsmouth Rapids control role; the status uses the Lucida
Grande content role. The card occupies the unused right side of the fourth
basic-control row and remains inside its group edge.

This is a real composition consumer: all three controls have compiled DML
records and stable IDs, participate in the retained population count, and are
verified through the normal Gallery factory. No hidden demo-only child tree is
created behind the schema.

## Verification

- Normal build with AppKit, Skia, CoreGraphics, Gallery, and benchmark audit:
  **22/22 passed**.
- Strict Release with `-Wall -Wextra -Wpedantic -Werror`: **21/21 passed**.
- No-Skia/no-native-host Gallery lane: **17/17 passed**. On macOS the optional
  CoreGraphics target is still available globally, but `gui_forms_core` is an
  independently buildable renderer-free static target and does not link it.
- AddressSanitizer: **21/21 passed** with supported Apple ASan settings. The
  earlier `detect_leaks=1` invocation is retained as a harness failure because
  this Apple runtime reports leak detection as unsupported.
- UndefinedBehaviorSanitizer: **16/16 passed**.
- ThreadSanitizer: **16/16 passed**.
- `gui_forms_lifecycle_controls_tests` covers attach/detach order and observable
  binding state, load-once reparenting, two-phase attachment counts, attach
  rollback and ID reuse, structural mutation rejection in lifecycle callbacks,
  synchronous property events, underflow, disposal during initialization, and
  wrong-thread initialization rejection.
- Gallery integration verifies type identity, load/attachment count, lifecycle
  status text, complete visible-child containment, and the control/content font
  role split.
- **OBSERVED manual AppKit smoke:** the rebuilt native Gallery displayed the
  composed card without overlap or edge escape. Its visible status was
  `Load 1 · attach 1 · init 1`. The native close path completed without the
  earlier crash; the app was then quit.

## Honest next dependency

M6 still needs typed property/default/reset metadata before row 2 of the
completeness matrix can be called supported. The next bounded extraction can
use the existing PNG registry for `PictureBox`/`ImageList` ownership, or tackle
early nonvisual components such as `Timer` only after the dispatcher contract
is sufficiently explicit. Text/numeric controls remain dependent on M4;
scrolling/list families remain dependent on M5.
