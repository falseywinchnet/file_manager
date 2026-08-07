# M12-P19 nested property values and collections

Status: **MEASURED PARTIAL** on 2026-08-07.

## Question

Can the renderer-neutral property substrate represent and edit bounded nested
objects and ordered collections without exposing mutable implementation storage,
parsing diagnostic strings, or invalidating a live editor during its callback?

## Implemented contract

- `BindingValueKind::object` and `BindingValueKind::collection` carry immutable
  shared snapshots. Equality is structural, not allocation identity.
- Object type/member names, UTF-8, canonical member uniqueness, depth, member
  count, item count, total nodes, and homogeneous collection item kind are
  validated before publication. The current bounds are depth 8, 256 members per
  object, 4,096 items per collection, and 8,192 total nodes.
- `PropertyGrid` stores typed member/index/compound path segments. Paths such as
  `Settings.Endpoint.Host`, `Settings.Modes[1]`, and `Bounds.X` recursively
  reconstruct immutable ancestors and commit only through the owning registered
  setter. A read-only nested member cannot be widened by a writable parent.
- Collection insert/remove/move rebuild one homogeneous snapshot and preserve
  independent disclosure state. Invalid indices and failed conversions leave
  the owner unchanged and publish a path-specific edit error.
- Same-shape updates refresh existing retained rows in place. Only structural
  shape changes rebuild the projection; this prevents disposal of the editor
  that is still completing its own commit callback.
- Stock `ComboBox.Items` is now a truthful content-serialized, resettable,
  non-bindable collection property with tokenized `items_changed` observation.
  It is non-bindable because collection binding semantics are not yet declared.

## Dogfood

The Complete Showcase Values page can switch the public PropertyGrid between a
`NumericUpDown` and a stock `ComboBox`, expand `Items[0..n]`, edit an item, and
insert `CW` through `PropertyGrid::insert_collection_item`. Native AppKit
accessibility reports the exact index paths and values. The showcase contains no
private property-grid subclass or duplicate collection model.

The separately reported slider-to-Animation abort was sequence-dependent. Its
crash report showed a C++ exception escaping an AppKit dispatch-source callback,
not a raster fault. The current scheduler disconnects and counts a throwing
frame callback, native wake boundaries contain C++/Objective-C exceptions, and
the exact drag/release/page-swap/wake/paint sequence passes 48 deterministic
headless cycles plus 20 native Computer Use cycles. This is **not reproduced on
the fresh binary**, not claimed impossible.

## Measured gates

```text
normal focused tests
  input_controls, inspection_controls, binding, showcase_interaction,
  showcase_public_control_policy: 5/5 passed

renderer-free focused tests
  input_controls, inspection_controls, binding: 3/3 passed

ASan/UBSan warnings-as-errors focused tests
  input_controls, inspection_controls, binding: 3/3 passed

Win64-GNU cross-build
  gui_forms_core and gui_forms_controls: passed

git diff --check: passed
```

Focused oracles cover structural equality without shared storage, conversion to
homogeneous item kinds, duplicate/depth/conversion rejection, three-level object
paths, read-only members, precise edit events, insert/remove/move, disclosure
retention, structural reset, stock Items metadata/change/reset, target switching,
and collection showcase mutation.

## Open edge

Arbitrary editor/type-converter factories, heterogeneous dictionaries, nullable
specializations, modal component editors, multiple-object selection, collection
transactions spanning multiple owners, localization, compiled DML projection,
and managed PropertyGrid projection remain open. `ListBox.Items` and other stock
content collections should adopt the same contract only when their derived-type
event ordering and model identity rules are explicit.
