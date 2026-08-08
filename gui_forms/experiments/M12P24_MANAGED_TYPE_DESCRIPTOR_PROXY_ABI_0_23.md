# M12-P24 managed TypeDescriptor proxy and ABI 0.23

Status: **MEASURED PARTIAL**. Date: 2026-08-07.

## Question

Can the generated `PropertyGrid` inspect and edit ordinary managed objects
through the native property engine without flattening values to diagnostic
strings, retaining managed objects in native metadata, or weakening atomic
multiple-owner commit?

## Implemented contract

- Experimental ABI 0.23 adds the nonvisual
  `GF_CONTROL_PROPERTY_OBJECT_PROXY`, size-prefixed inert descriptor and
  callback records, scalar/null/text/color/enum value snapshots, deep-copied
  enum and standard-value metadata, external change notification, converted
  text commit, and reset operations. ABI 0.22 remains an unchanged prefix.
- Callback text uses caller-owned bounded buffers. Native code never retains a
  managed string pointer. Descriptor strings, enum choices, and standards are
  copied during definition; callback context remains caller-owned until the
  proxy is disposed.
- Each proxy registers ordinary native `PropertyRegistration` objects. Getter,
  setter, reset, `ShouldSerialize`, change events, converter formatting, and
  parsing therefore pass through the same schema validation and PropertyGrid
  transaction used by native controls.
- The proxy clears `Control`'s stock visual-property registrations before
  installing foreign metadata. `Name`, `Bounds`, `Visible`, and other adapter
  implementation details cannot leak into the inspected object schema.
- PropertyGrid installs proxy converters in a fresh instance-owned registry.
  Selection failure restores the previous converter registry and selected
  objects before reporting failure.
- The generated adapter uses `TypeDescriptor.GetProperties`, so ordinary
  descriptors and `ICustomTypeDescriptor` participate. It projects nullable
  Boolean/integer/number/text, `Color`, enums/flags, and types with truthful
  string round-trip converters. Read-only one-way string display is admitted.
  It carries category/description, browsability, finite standard values,
  exclusivity, reset/serialization policy, current-culture TypeConverter calls,
  and `SupportsChangeEvents` notifications.
- Managed adapters own their native proxy, descriptor event hooks, and callback
  roots. A successful selection swap installs the new proxies before retiring
  the old ones. Failed construction or selection disposes only the candidates
  and leaves the prior selection live.

## Deterministic evidence

- The C11 ABI fixture deep-copies one typed numeric descriptor, invokes its
  getter/formatter/serialization callbacks, commits converter text, rejects a
  value outside exclusive standards without calling the setter, resets through
  the callback, refreshes after external notification, rejects wrong handles,
  and disposes deterministically.
- The managed behavior fixture selects two ordinary CLR objects containing a
  nullable integer, enum, custom standard-values TypeConverter, read-only text,
  and hostile setter. It proves converter display, converted two-owner commit,
  exact current-culture propagation, proxy-schema isolation, nullable commit,
  reset, rollback after the second owner rejects, rejected
  unsupported-type selection without replacement, clearing, and exact event
  counts.

## Gates

```text
normal configured native suite:       63/63 passed
renderer-free C/C++ ABI tests:         2/2 passed
generated surface:                     1104/1104 verified; 0 build warnings
managed TypeDescriptor behavior:       passed on host .NET 10
Win64:                                 ABI 0.23 PE32+ built
Wine:                                  identical managed behavior smoke passed
```

## Honest remaining edge

The proxy is a runtime property-inspection/editing channel, not a complete
design-time service container. Managed `UITypeEditor` drop-down/modal hosting
and `IWindowsFormsEditorService` are implemented by M12-P25. Component editors, dynamic standard-value
providers, mixed-value visuals, nested arbitrary managed objects/collections,
date/duration/path/resource specializations, and public `GridItem`/
`PropertyValueChanged` projection remain open. Unsupported writable types fail
before selection instead of becoming misleading strings.
