# ThemeDefinition

- Status: **OBSERVED: bundle 009 theme definition review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ThemeDefinition`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:127`
- Definition: `inline/header-only`

ThemeDefinition is one named, complete compatibility palette, role/state material matrix, and structural-token set.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const ThemeDefinition&, const ThemeDefinition&) = default
```

Compares identity and the complete theme payload.
