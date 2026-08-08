# M12-P25 managed UITypeEditor services and ABI 0.24

Status: **MEASURED PARTIAL**. Date: 2026-08-07.

## Question

Can an ordinary managed `PropertyDescriptor` activate its authored
`UITypeEditor`, receive a working `IWindowsFormsEditorService`, and commit the
returned typed value through the native PropertyGrid transaction rather than a
managed-only test hook or nominal no-op?

## Implemented contract

- Experimental ABI 0.24 appends an optional, size-checked property edit callback
  and `property_grid_activate_editor`. ABI 0.23 callback records remain valid
  prefixes when they do not declare an editor.
- A foreign descriptor with an editor identity installs a real retained Button
  through the instance-owned native `PropertyEditorRegistry`. Its visible value
  is formatted by the descriptor's converter; pointer, keyboard, semantic, and
  ABI activation all use the same Button click event.
- The edit callback receives the current typed value and returns a typed value.
  PropertyGrid then applies the existing schema validation and rollback-safe
  multiple-owner commit. The editor callback cannot bypass owner setters.
- The generated .NET 10 surface now contains `UITypeEditor`,
  `UITypeEditorEditStyle`, `PaintValueEventArgs`, and
  `IWindowsFormsEditorService` with truthful base behavior.
- `EditorAttribute` is resolved by `PropertyDescriptor.GetEditor`. The adapter
  supplies a bounded `ITypeDescriptorContext` and a per-edit service provider.
- `DropDownControl` requires an unparented managed Control, hosts it in a
  screen-anchored owned Form, blocks through the normal nested modal loop, and
  returns only after `CloseDropDown`. It restores the donated Control's parent
  and bounds on every exit path.
- `ShowDialog` uses the PropertyGrid's owning Form when available. Nested
  drop-down/modal transactions are rejected, and disposal closes any active
  drop-down.
- Editor callbacks and failure callbacks revalidate PropertyGrid lifetime after
  nested-loop reentry before touching projection state.

## Deterministic evidence

- The C11 fixture declares a converter-backed numeric property with a foreign
  editor callback. Activating the retained editor invokes it once and commits
  its result through the real setter. A record truncated immediately before the
  ABI 0.24 tail is also accepted for an editor-free property.
- The managed fixture selects two ordinary objects. Its drop-down editor hosts a
  real ListBox, posts a close after attachment, returns a new enum, and updates
  both owners. Its modal editor opens an owned Form, accepts it from the Load
  turn, returns an integer, and updates both owners. A later hostile second-owner
  setter still rolls both owners back to the editor-produced prior value.

## Gates

```text
normal configured native suite:       63/63 passed
focused inspection/C11 tests:          2/2 passed
renderer-free C/C++ ABI tests:         2/2 passed
generated surface:                     1104/1104 verified; 259 types
managed build:                         0 warnings, 0 errors
managed editor behavior:               passed on host .NET 10
Win64:                                 ABI 0.24 PE32+ built
Wine:                                  identical editor behavior passed
```

## Honest remaining edge

This tranche implements runtime `UITypeEditor` drop-down/modal service
projection, not the complete designer ecosystem. Dynamic standard-value
providers, component editors, public `GridItem`/`PropertyValueChanged`, mixed
value visuals, nested arbitrary managed objects/collections, reset context
menus, DML property schemas, and specialized date/path/image/resource editors
remain open. Drop-down hosting currently uses a border-owned Form rather than a
native in-window popup plane; its blocking, close, ownership, restoration, and
typed-commit semantics are implemented and portable.
