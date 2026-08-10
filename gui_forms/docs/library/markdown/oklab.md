# Oklab

- Status: **OBSERVED: bundle 009 color-space value review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `Oklab`
- Declaration: `include/gui_forms/drawing/color/color.hpp:93`
- Definition: `inline/header-only`

Oklab carries perceptual lightness/opponent axes plus straight alpha.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Oklab&, const Oklab&) = default
```

Compares all components.
