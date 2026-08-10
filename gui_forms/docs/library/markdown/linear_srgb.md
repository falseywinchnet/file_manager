# LinearSrgb

- Status: **OBSERVED: bundle 009 color-space value review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `LinearSrgb`
- Declaration: `include/gui_forms/drawing/color/color.hpp:76`
- Definition: `inline/header-only`

LinearSrgb carries normalized straight-alpha linear-light sRGB channels.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const LinearSrgb&, const LinearSrgb&) = default
```

Compares all four channels.
