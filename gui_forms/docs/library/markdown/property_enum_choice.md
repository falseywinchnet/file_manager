# PropertyEnumChoice

- Status: **OBSERVED: bundle 010 enum choice review; focused M4 binding tests pass**
- Kind: **struct**
- Hierarchy: `PropertyEnumChoice`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:176`
- Definition: `inline/header-only`

PropertyEnumChoice maps one canonical display/name token to a signed enum value.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const PropertyEnumChoice&, const PropertyEnumChoice&) = default
```

Compares name and numeric value.
