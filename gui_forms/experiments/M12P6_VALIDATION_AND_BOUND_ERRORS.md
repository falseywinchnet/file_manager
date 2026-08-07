# M12-P6 — retained validation and binding-aware errors

Status: **MEASURED PARTIAL**

## Question

Can GUI.Forms make focus validation, `OnValidation` binding, and
`ErrorProvider` one retained transaction instead of three application/demo
shortcuts, while preserving renderer-free and cross-platform ownership?

## Evidence and chosen contract

- **OBSERVED:** pinned LibreWinForms exposes exact `AutoValidate` values
  `Inherit=-1`, `Disable=0`, `EnablePreventFocusChange=1`, and
  `EnableAllowFocusChange=2`; exact `ValidationConstraints` values are
  `Selectable=1`, `Enabled=2`, `Visible=4`, `TabStop=8`, and
  `ImmediateChildren=16`.
- **OBSERVED:** its focus path emits cancellable `Validating` before
  successful `Validated`; a destination with `CausesValidation=false` skips
  automatic validation; `Binding` pulls target data from validation; and
  ErrorProvider listens to currency/binding completion.
- **DECIDED:** GUI.Forms uses that useful ordering but keeps portable C++
  event/value types, explicit binding descriptors, retained target identity,
  and one Window-owned transaction. It does not emulate arbitrary managed
  reflection.
- **DECIDED:** nested focus requests from validation callbacks are rejected and
  counted. Validation callbacks may dispose or change endpoint eligibility, so
  Window rechecks both endpoints before publishing focus loss/gain.

## Implemented center

- `Control::causes_validation`, change event, mutable
  `ControlValidationEvent`, `validating`, and `validated`;
- inherited `ContainerControl::auto_validate`, explicit `validate`, and
  constrained `validate_children`;
- pre-focus-loss previous-branch/common-ancestor traversal with
  prevent/allow/disable behavior and `ValidationSnapshot` counters;
- transactional pointer entry: a prevent-mode rejection consumes the press
  without leaking a later Button activation, while `CausesValidation=false`
  remains the explicit bypass;
- `Binding` automatically commits `OnValidation` and cancels the focus
  transaction on parse/transfer failure;
- `BindingRecord::errors`, canonical record/field lookup, and live binding
  enumeration;
- ErrorProvider `DataSource`, `DataMember`, `BindToDataAndErrors`, and
  `UpdateBinding`, including record-wide/field aggregation, BindingComplete
  failures, currency refresh, and source-disposal cleanup;
- Complete Showcase Values-page dogfood with an empty-name parse rejection;
- the existing 48-cycle slider-drag → Animation → frame service → paint loop
  remains the deterministic regression for the earlier intermittent crash.

## Focused gates

Normal native build:

```sh
cmake -S . -B build
cmake --build build --target gui_forms_validation_tests \
  gui_forms_binding_tests gui_forms_guidance_provider_tests \
  gui_forms_focus_scope_tests gui_forms_showcase_interaction_tests -j 6
./build/gui_forms_validation_tests
./build/gui_forms_binding_tests
./build/gui_forms_guidance_provider_tests
./build/gui_forms_focus_scope_tests
./build/gui_forms_showcase_interaction_tests
```

Results:

- **MEASURED:** normal validation/binding/guidance/focus/showcase suites pass;
- **MEASURED:** ASan/UBSan validation, binding, and guidance suites pass with
  leak detection disabled for the established harness policy;
- **MEASURED:** renderer-free validation/binding/guidance and strict
  renderer-free validation/showcase suites pass;
- **MEASURED:** MinGW x64 core-only, validation, binding, and guidance targets
  link. This gate caught and removed an initial core→ContainerControl link
  cycle by replacing the concrete dependency with a base-control authored
  policy hook;
- **MEASURED:** those four PE64 executables pass under Wine with the pinned
  MinGW runtime `WINEPATH`; and
- **MEASURED:** the showcase interaction suite completes all 48 deterministic
  slider-to-Animation cycles with zero frame callback faults.
- **MEASURED:** deterministic catalogue regeneration leaves 18,097 total rows,
  promotes 278 exact Forms rows to `measured_partial`, and leaves 9,973
  admitted Forms rows `missing`. The same regeneration corrects 236
  PrintDialog/preview/page-setup rows to the already-GIVEN printing exclusion.

## Honest remainder

Arbitrary managed `IDataErrorInfo`, managed delegate/CancelEventArgs identity,
true nested object `DataMember` traversal, culture providers, sort/filter,
BindingNavigator, DataGridView, C ABI/facade projection, independent native
form validation oracles, and accessibility described-by relations remain open.
