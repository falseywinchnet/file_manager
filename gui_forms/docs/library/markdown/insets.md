# Insets

- Status: **OBSERVED: bundle 011 geometry review**
- Kind: **struct**
- Hierarchy: `Insets`
- Declaration: `include/gui_forms/types/geometry/geometry.hpp:53`
- Definition: `inline/header-only`

Insets stores independent logical left, top, right, and bottom extents for margin, padding, damage, and visual outsets.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Insets&, const Insets&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
