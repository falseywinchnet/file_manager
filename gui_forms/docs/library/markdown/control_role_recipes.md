# ControlRoleRecipes

- Status: **OBSERVED: bundle 009 role recipe matrix review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ControlRoleRecipes`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:66`
- Definition: `inline/header-only`

ControlRoleRecipes stores complete ordinary, selected, high-contrast, and high-contrast-selected recipes across the closed surface-state axis.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const ControlRoleRecipes&, const ControlRoleRecipes&) = default
```

Compares all four state matrices.
