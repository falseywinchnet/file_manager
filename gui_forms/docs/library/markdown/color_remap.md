# ColorRemap

- Status: **OBSERVED: bundle 009 color-remap value review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ColorRemap`
- Declaration: `include/gui_forms/drawing/image_attributes/image_attributes.hpp:16`
- Definition: `inline/header-only`

ColorRemap maps one exact authored color to another.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ColorRemap&, const ColorRemap&) = default
```

Compares old and new colors.
