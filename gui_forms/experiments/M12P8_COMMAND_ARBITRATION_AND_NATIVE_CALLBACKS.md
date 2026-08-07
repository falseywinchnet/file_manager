# M12-P8 command arbitration and native callback boundaries

Status: **MEASURED PARTIAL**. Date: 2026-08-06.

## Question

Can GUI.Forms close the next dialog/menu compatibility tranche without making
callback order depend on mutable tree traversal, and can the real macOS host
survive the reported Ranges-to-Animation transition long enough to preserve an
actionable fault?

## Retained contract

- `Control::process_mnemonic` and `Window` materialize eligible mnemonic
  candidates in stable retained/tab order before invoking application code.
  Disposal or reparenting from one callback cannot rewrite the active walk.
- Window confines candidates to the active focus scope, cycles duplicate
  mnemonics after the prior winner, and reports candidates, collisions, and
  cycles separately from attempts and handled commands.
- MenuStrip and live ContextMenu rows use the same ampersand parser as stock
  controls. Markers do not leak into width, paint, or semantics. Duplicate
  top-level items cycle deterministically; popup rows activate the existing
  shared `Command` authority and submenu path.
- `DialogResult` publishes the exact WinForms numeric set, including the gaps
  before `TryAgain` and `Continue`. Button and Window reject unknown values.
  Assigning a cancel Button supplies `Cancel` only when its result is `None`.
  Button activation raises Click first and then publishes the retained Window
  result if the Button remains alive and attached.
- The portable Window retains result state but does not pretend to own a native
  modal loop. A host/facade decides whether a non-None result closes a modal
  presentation.

## Crash evidence and repair

**OBSERVED:** incident `892735B9-DF96-4F02-BAA4-D25096650999` ended in
`_objc_terminate` from the main-queue dispatch-source timer, after the user had
adjusted sliders and selected Animation. The previous host caught C++
exceptions in `collectDamage`, but an Objective-C exception could cross that
boundary and force `abort()` before its name or reason was recorded.

The dispatch-source wake is now an Objective-C exception boundary. A raised
exception disables the wake source through the existing native-callback fault
path and records its exact name/reason. This is containment and evidence
preservation, not a claim that the unrecorded historical AppKit exception has
been identified.

The reported interaction family is also deterministic in
`gui_forms_showcase_interaction_tests`: 48 alternating captured slider drags,
release, immediate Animation replacement, deadline service, and retained paint.
The rebuilt native application completed 10 additional real AppKit cycles with
live pointer dragging, continuing animation, zero recorded callback faults, and
no process exit. The historical exception could not be independently retriggered
after the repair.

## Gates

- Normal renderer build: basic controls, menus, and the 48-cycle Complete
  Showcase interaction suite pass.
- Strict ASan/UBSan renderer-free build: basic controls and menus pass.
- Independent renderer-free build: basic controls and menus pass.
- Strict-warning build: basic controls and menus compile and pass.
- MinGW x86-64: both focused executables link as PE32+.
- Wine: both focused PE32+ tests pass with the MinGW runtime on `WINEPATH`.
- Native AppKit: the rebuilt Complete Showcase remains visible on Animation
  after 10 alternating live transition cycles.

## Honest boundary

Locale-sensitive mnemonic folding, underline visibility policy, protected
managed `ProcessMnemonic` projection, arbitrary third-party `IButtonControl`
adaptation, independent native Form/modal-loop closure, and the exact historical
AppKit exception name remain open. Host containment must not be treated as a
substitute for fixing a future recorded AppKit cause.
