# PropertyEnumValue

- Status: **OBSERVED: bundle 010 binding value review; focused M4 binding tests pass**
- Kind: **struct**
- Hierarchy: `PropertyEnumValue`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:24`
- Definition: `inline/header-only`

PropertyEnumValue preserves declared enum type, canonical symbolic name, and signed numeric representation.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const PropertyEnumValue&, const PropertyEnumValue&) = default
```

Compares type, name, and value.
