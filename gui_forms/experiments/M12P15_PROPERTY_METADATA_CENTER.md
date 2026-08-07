# M12-P15: retained property metadata center

Date: 2026-08-06

Status: **MEASURED PARTIAL** closure of behavior gap B02.

## Claim and boundary

GUI.Forms needs one native, renderer-neutral property definition to serve
imperative controls, binding, future DML, inspection, and designer tooling. A
managed reflection shim or a table of untyped names would not establish that
behavior.

This tranche admits public value-only `PropertyDescriptor` snapshots and keeps
executable callbacks private to `PropertyRegistration`. The same registration
now supplies:

- canonical case-insensitive lookup with authored display casing;
- scalar kind, category, description, default value, browsability, binding,
  serialization visibility, reset support, change-notification support, and
  exact declared dirty effects plus local/subtree scope;
- centralized conversion before a real retained setter runs;
- default or authored reset and authored/fallback `ShouldSerialize` policy;
- owner-revoked tokenized change observation;
- deterministic canonical descriptor order; and
- existing `BeginInit`/`EndInit` dirtiness union through the real setters.

Stock definitions cover base `Name`, `Visible`, `Enabled`, `AutoSize`, and
`CausesValidation`; text on TextBox, Label, and ButtonBase; checked state on
CheckBox and RadioButton; range/NumericUpDown values; and ComboBox text and
selected index. ComboBox text has an explicit reset so an empty item cannot
accidentally become selected.

Executable reads/writes/resets enforce live-control and UI-thread rules.
Disposed controls retain only inert descriptor inspection. Failed conversion
does not call the setter or change state. Registration rejects incompatible
defaults, unreadable getters, unwritable setters, unknown dirty bits, and
invalid serialization visibility.

## Measured gates

On the macOS arm64 development host:

```text
cmake --build build --target gui_forms_binding_tests -j 8
./build/gui_forms_binding_tests
  passed

ctest --test-dir build --output-on-failure \
  -R 'gui_forms_(core|binding|lifecycle_controls|basic_controls|input_controls|range_controls|animation|showcase_interaction)_tests'
  8/8 passed

ctest --test-dir build --output-on-failure --repeat until-fail:50 \
  -R gui_forms_showcase_interaction_tests
  50/50 passed; 18.60 s

ctest --test-dir build-renderer-free-polish --output-on-failure \
  -R 'gui_forms_(binding|animation)_tests'
  2/2 passed

ctest --test-dir build-m11e-sanitize --output-on-failure \
  -R 'gui_forms_(binding|animation)_tests'
  2/2 passed under ASan/UBSan

cmake --build build-win64 --target gui_forms_core gui_forms_controls -j 8
  passed
```

The repeated showcase gate includes captured slider movement, release,
immediate navigation to Animation, animation lease service, and retained paint.
It is useful negative evidence for the reported intermittent crash, but it does
**not** establish that the physical AppKit failure is independently reproduced
or closed.

## Open edge

B02 remains partial. Compound/enumerated/resource property value kinds,
ambient/inherited value origins, framework-wide stock-property registration,
atomic rollback transactions, DML schema/round trip, managed
`System.ComponentModel.PropertyDescriptor` projection, localization, and a
PropertyGrid consumer remain. Declared effects are inspectable and tested
against stock setters; a future development-mode checker must still detect a
custom setter that mutates undeclared state.
