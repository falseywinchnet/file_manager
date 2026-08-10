# ThemeMotionTokens

- Status: **OBSERVED: bundle 009 motion token review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ThemeMotionTokens`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:111`
- Definition: `inline/header-only`

ThemeMotionTokens defines quick, standard, emphasized, and busy-cycle durations without prescribing an animation engine.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ThemeMotionTokens&, const ThemeMotionTokens&) = default
```

Compares all durations.
