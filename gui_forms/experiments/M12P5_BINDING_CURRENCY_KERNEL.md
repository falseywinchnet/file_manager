# M12-P5 retained binding and currency kernel

Status: **MEASURED PARTIAL**. Date: 2026-08-06.

## Claim under test

GUI.Forms needed a real renderer-neutral binding substrate rather than façade
identities or showcase-local synchronization. The gate required retained
records and currency, explicit property descriptors, two-way update policies,
format/parse completion, deterministic errors and ordering, edit/list
mutation, suspension, manager-wide transfer, and lifetime cleanup.

## Implemented native contract

- `BindingValue` is a bounded null/Boolean/signed/unsigned/number/UTF-8 union
  with strict invariant conversion.
- Stock and custom controls register named `BindableProperty` descriptors with
  explicit get, set, and tokenized change-connect behavior. No C++ RTTI,
  managed reflection, native handle, renderer type, or platform object enters
  this seam.
- `BindingSource` owns stable-ID records, current position, add/insert/remove,
  current-field mutation, find, begin/cancel/end edit, list/current/position
  events, data member state, reset operations, and coalesced binding
  suspension.
- `CurrencyManager` exposes current/count/position, edit control, suspension,
  list/current/error events, refresh, removal, and manager-wide `PullData` and
  `PushData` over weakly registered bindings.
- `Binding` implements initial and explicit source-to-control reads,
  control-to-source writes, `OnPropertyChanged`, explicit `OnValidation`, and
  `Never` modes, invariant `F0..F12`, Format/Parse hooks, null substitution,
  post-commit BindingComplete propagation, DataError, reentrancy suppression,
  and synchronous endpoint cleanup.
- `ControlBindingsCollection` owns each control's bindings, rejects duplicate
  canonical properties, distinguishes its no-options default from explicitly
  supplied `OnValidation`, and supports add/find/remove/clear.
- same-window `BindingContext` preserves manager identity and eagerly removes
  a source on disposal with exactly one collection-removal event.
- stock bindable properties currently include base `Name`, `Visible`, and
  `Enabled`; text on TextBox/Label/ButtonBase; checked state on CheckBox and
  RadioButton; value on range controls and NumericUpDown; and Text plus
  SelectedIndex on the noneditable ComboBox.

The Complete Showcase Values page dogfoods three immediate two-way controls and
currency movement across three retained records. The regression edits one
record, moves currency, verifies all projected controls, then returns and
verifies that the edit remained authoritative.

## Deterministic ordering

For currency movement, internal binding propagation runs before public
`PositionChanged`, `CurrentChanged`, and `CurrentItemChanged`, in that order.
Successful BindingComplete callbacks run after the destination has committed;
the binding callback precedes the source/manager callback. Cancellation reports
rejection to the initiating binding operation but does not pretend the already
committed destination rolled back.

## Evidence

- normal macOS build: binding, input-control, and 48-cycle Complete Showcase
  interaction tests pass;
- sanitizer build: binding, tooltip, and host-protocol tests pass with
  `ASAN_OPTIONS=detect_leaks=0`;
- strict renderer-free build with Skia, HarfBuzz, and native hosts disabled:
  binding and Complete Showcase interaction tests pass;
- MinGW x64: binding, Complete Showcase interaction, and native Win32 Showcase
  targets compile; the two test executables pass under Wine with the MinGW
  runtime on `WINEPATH`.
- live AppKit dogfood edits the bound name, advances currency, observes the
  Checkbox and TrackBar update, returns to the first record with its edit
  retained, then enters Animation and exercises pause/resume/reduced/full
  transitions without closing or stalling the application.

## Honest open surface

The native core intentionally does not inspect arbitrary managed objects. A
future façade must project their properties and list notifications into these
descriptors and records. Nested `DataMember`, sort/filter/advanced sorting,
arbitrary culture/`IFormatProvider`, DataGridView, BindingNavigator,
ErrorProvider data-source integration, and complete `CausesValidation` /
cancellable `Validating` focus semantics remain open. Current
`OnValidation` is an explicit `validate`/`WriteValue` transfer until that focus
validation substrate lands.
