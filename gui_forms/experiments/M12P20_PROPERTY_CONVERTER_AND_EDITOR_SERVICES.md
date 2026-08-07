# M12-P20 property converter and editor services

Status: **MEASURED PARTIAL**. Date: 2026-08-07.

## Claim and upstream edge

**OBSERVED:** the pinned LibreWinForms oversight branch remains at commit
`1457ed5beef24a7d7e3688a4725c7487da09791e`. Its public surface includes the
WinForms `PropertyGrid`, `GridItem`, component-model converter, and editor
families. The exact member ledger remains
`planning/generated/WINFORMS_API_CATALOGUE.tsv`; this tranche does not promote
the managed `PropertyGrid` facade merely because a native service now exists.

**OBSERVED before this tranche:** native `PropertyGrid` selected editors with a
closed switch over `BindingValueKind`. Every numeric value used a generic
`TextBox`. A custom control could publish rich inert metadata but could not name
a reusable converter or retained editor without modifying PropertyGrid itself.

**HYPOTHESIS:** instance-owned converter/editor registries can provide the useful
runtime premise of `TypeConverter`/`UITypeEditor` without importing mutable
process-global reflection, executable callbacks into inert descriptors, or a
designer dependency.

## Implemented contract

- `PropertyDescriptor::converter_name` and `editor_name` are bounded UTF-8
  service identities. They contain no callbacks.
- `PropertyValueConverterRegistry` owns named format/parse pairs and optional
  kind mappings. Registration is canonical and rejects duplicates. The default
  invariant converter covers Boolean, signed/unsigned integer, number, text,
  and finite enum values through existing descriptor-aware conversion.
- `PropertyEditorRegistry` owns named retained-control factories and optional
  kind mappings. A factory returns one unattached live control, a non-emitting
  synchronization callback, and a tokenized commit connector.
- Registries are ordinary per-PropertyGrid objects. There is no global mutable
  type table; applications may share an immutable/configured registry
  deliberately.
- `PropertyList::replace_editor` attaches before retiring the stock editor, so a
  stable-ID or lifecycle failure leaves the prior row intact. The replacement
  then participates in the same scroll geometry, focus reveal, semantic tree,
  ownership, and deterministic disposal as stock editors.
- PropertyGrid installs factories only after its complete row/path projection
  exists. Factory exceptions are contained as row diagnostics. Commit uses the
  real typed path and setter; programmatic refresh calls the factory's guarded
  synchronize operation without feedback.
- The default number mapping is a retained `NumericUpDown`, including nested
  geometry fields. The Complete Showcase now dogfoods that reusable mapping for
  `NumericUpDown.Value` and `Bounds.X`.
- A test-only percent converter plus bounded percent stepper proves explicit
  descriptor service selection, canonical duplicate rejection, typed commit,
  and feedback-free refresh.

## Invariants exercised

1. A descriptor remains safe to enumerate or serialize without executable
   service access.
2. A factory cannot donate a disposed, parented, window-attached, or incomplete
   editor binding.
3. PropertyList assumes ownership only after successful attachment.
4. Factory commits are tokenized to the PropertyGrid owner and disappear with
   selection rebuild or disposal.
5. Synchronization never re-enters the target setter.
6. Text parsing validates UTF-8 and a 64 KiB bound before custom conversion;
   returned values must pass the bounded property-value-tree validator.
7. Missing explicit editor services report a diagnostic; absent kind mappings
   retain the prior truthful stock/read-only projection.

## Verification

Focused normal build and tests:

```text
cmake --build build --target gui_forms_inspection_controls_tests \
  gui_forms_showcase_interaction_tests gui_forms_showcase -j4
ctest --test-dir build --output-on-failure \
  -R 'gui_forms_(inspection_controls|showcase_interaction)_tests'
```

Result: **2/2 passed** after rebuilding the native Complete Showcase.

**MEASURED native dogfood:** on the fresh macOS app, the Values page switched
from `ComboBox.Items` inspection to the numeric specimen, exposed retained
NumericUpDown editors for `Value` and expanded `Bounds` leaves, accepted `4.5`,
committed on Return, updated the inspected numeric control from `3.75` to `4.5`,
and published `PropertyGrid committed Value · origin local`. The app remained
open and responsive.

Focused portability/robustness gates:

```text
build-renderer-free-polish: gui_forms_inspection_controls_tests  1/1 passed
build-m11e-sanitize:        gui_forms_inspection_controls_tests  1/1 passed
build-win64:                gui_forms_controls                    built
```

The sanitizer build uses the existing warnings-as-errors ASan/UBSan profile.
The Win64 result is compile/link evidence, not a physical Windows interaction
claim.

## Honest remaining edge

This is the native reusable runtime service center, not complete WinForms
component-model parity. Still open:

- specialized flags, color/resource, nullable, date/time, duration, path, and
  command editors;
- drop-down/modal editor ownership and an `IWindowsFormsEditorService`-like
  managed projection;
- converter culture/localization contexts and standard-values metadata;
- nested member-specific converter/editor identities;
- multiple selection and atomic multi-owner rollback;
- `GridItem`, managed `PropertyGrid`, DML schema, and generated facade
  projection;
- editor availability/capability reporting across hosts.
