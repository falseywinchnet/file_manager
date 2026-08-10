# PropertyEnumChoice

- Status: **OBSERVED: bundle 010 enum choice review; focused M4 binding tests pass**
- Kind: **struct**
- Hierarchy: `PropertyEnumChoice`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:120`
- Definition: `inline/header-only`

PropertyEnumChoice maps one canonical display/name token to a signed enum value.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const PropertyEnumChoice&, const PropertyEnumChoice&) = default
```

Compares name and numeric value.
