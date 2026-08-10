# ThemeGeometryTokens

- Status: **OBSERVED: bundle 009 geometry token review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ThemeGeometryTokens`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:87`
- Definition: `inline/header-only`

ThemeGeometryTokens centralizes control heights, touch targets, splitter geometry, navigation extents, and compact breakpoint.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ThemeGeometryTokens&, const ThemeGeometryTokens&) = default
```

Compares every geometry token.
