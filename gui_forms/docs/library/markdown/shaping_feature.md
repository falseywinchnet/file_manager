# ShapingFeature

- Status: **OBSERVED: bundle 009 shaping feature review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ShapingFeature`
- Declaration: `include/gui_forms/text_shaping/types/text_shaping_types.hpp:40`
- Definition: `inline/header-only`

ShapingFeature applies one OpenType tag/value to a validated UTF-8 subrange.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ShapingFeature&, const ShapingFeature&) = default
```

Compares tag, value, and range.
