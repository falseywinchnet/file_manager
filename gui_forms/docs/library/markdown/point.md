# Point

- Status: **OBSERVED: bundle 011 geometry review**
- Kind: **struct**
- Hierarchy: `Point`
- Declaration: `include/gui_forms/types/geometry/geometry.hpp:7`
- Definition: `inline/header-only`

Point is a renderer-neutral logical-coordinate pair.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Point&, const Point&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
