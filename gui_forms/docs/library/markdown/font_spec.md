# FontSpec

- Status: **OBSERVED: bundle 011 typography-value review**
- Kind: **struct**
- Hierarchy: `FontSpec`
- Declaration: `include/gui_forms/types/paint_types/paint_types.hpp:68`
- Definition: `inline/header-only`

FontSpec carries logical role, size, weight, italic, and bounded letter-spacing inputs shared by measurement and painting.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const FontSpec&, const FontSpec&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
