# GradientStop

- Status: **OBSERVED: bundle 011 paint-value review**
- Kind: **struct**
- Hierarchy: `GradientStop`
- Declaration: `include/gui_forms/types/paint_types/paint_types.hpp:29`
- Definition: `inline/header-only`

GradientStop pairs a normalized monotonic offset with a copied color for retained display-list safety.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const GradientStop&, const GradientStop&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
