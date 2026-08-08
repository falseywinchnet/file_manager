# M12-P23 managed PropertyGrid and ABI 0.22

Status: **MEASURED PARTIAL**. Date: 2026-08-07.

## Question

Can the generated System.Windows.Forms façade create and operate the real
native PropertyGrid instead of emitting a nominal stub, while retaining weak
native selection, common-schema multiple selection, and failure preservation?

## Implemented contract

- Experimental ABI 0.22 adds `GF_CONTROL_PROPERTY_GRID` plus bounded selection,
  PropertySort, and refresh operations. The 0.21 table remains an unchanged
  prefix.
- The ABI creates and initializes public native `gui_forms::PropertyGrid`.
  Selection accepts at most 1,024 unique, live, same-thread control handles and
  delegates common-schema/atomic behavior to the native implementation.
- The façade generator explicitly emits the bounded runtime `PropertyGrid`,
  `PropertySort`, default constructor, SelectedObject(s), sort, refresh, and
  selection/sort events. It does not admit design-time tabs or command services.
- `NativeControlBridge` maps PropertyGrid to control kind 21, negotiates ABI
  0.22, and projects cloned managed Control arrays into native handles.
- Unsupported arbitrary managed objects fail before replacing the prior
  selection. This is intentional until the TypeDescriptor callback/value
  channel can preserve converters, standard values, reset, and editor services
  truthfully.

## Deterministic evidence

- ABI C11 creates a native PropertyGrid, selects two Labels, round-trips sort,
  refreshes, rejects wrong-handle/sort calls, clears selection, and disposes.
- Generated surface resolves all 1,104 compatibility-catalogue rows and its
  verifier reports zero missing rows; the additive PropertyGrid family also
  compiles with zero warnings.
- Managed behavior smoke proves native creation, two-object selection, cloned
  SelectedObjects arrays, sort propagation, exactly two selection events and
  one sort event, unsupported-object rejection without selection loss, and
  clearing.

## Gates

```text
native C/C++ ABI focused tests:    2/2 passed
generated surface:                build 0 warnings; 1104/1104 verified
managed PropertyGrid smoke:        passed
normal configured native suite:   63/63 passed
Win64:                            ABI 0.22 PE32+ built
Wine:                             managed PropertyGrid smoke passed
```

## Honest remaining edge

This slice projects managed GUI.Forms Controls because they already possess a
native descriptor identity. Arbitrary managed objects, `ICustomTypeDescriptor`,
TypeConverter culture/standard-value callbacks, UITypeEditor drop-down/modal
services, PropertyValueChanged/GridItem, and mixed-value visuals remain open.
They require an explicit bounded descriptor
callback channel; silently reflecting them into strings would weaken the native
typed/atomic contract and is rejected.
