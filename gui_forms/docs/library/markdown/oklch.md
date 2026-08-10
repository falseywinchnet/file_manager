# Oklch

- Status: **OBSERVED: bundle 009 color-space value review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `Oklch`
- Declaration: `include/gui_forms/drawing/color/color.hpp:101`
- Definition: `inline/header-only`

Oklch carries perceptual lightness, nonnegative chroma, degree hue, and straight alpha.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Oklch&, const Oklch&) = default
```

Compares all components.
