# ThemeStructureTokens

- Status: **OBSERVED: bundle 009 structural theme review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ThemeStructureTokens`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:119`
- Definition: `inline/header-only`

ThemeStructureTokens groups spacing, geometry, typography, and motion vocabularies into one coherent structural contract.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ThemeStructureTokens&, const ThemeStructureTokens&) = default
```

Compares all structural token groups.
