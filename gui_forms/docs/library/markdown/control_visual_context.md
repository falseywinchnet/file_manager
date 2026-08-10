# ControlVisualContext

- Status: **OBSERVED: bundle 009 visual context review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ControlVisualContext`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:38`
- Definition: `inline/header-only`

ControlVisualContext selects a role recipe from surface state plus selected, focused, defaulted, and high-contrast axes.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ControlVisualContext&, const ControlVisualContext&) = default
```

Compares every selection axis.
