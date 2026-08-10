# ThemeSpacingTokens

- Status: **OBSERVED: bundle 009 spacing token review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ThemeSpacingTokens`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:76`
- Definition: `inline/header-only`

ThemeSpacingTokens names the shared micro-through-section logical spacing scale.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ThemeSpacingTokens&, const ThemeSpacingTokens&) = default
```

Compares the complete spacing scale.
