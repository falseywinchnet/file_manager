# ThemeTypographyTokens

- Status: **OBSERVED: bundle 009 typography token review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ThemeTypographyTokens`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:101`
- Definition: `inline/header-only`

ThemeTypographyTokens defines role-aware control, field, caption, heading, title, and monospace font recipes.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ThemeTypographyTokens&, const ThemeTypographyTokens&) = default
```

Compares all font recipes.
