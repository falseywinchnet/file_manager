# TextStyleId

- Status: **OBSERVED: bundle 009 text style identity review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `TextStyleId`
- Declaration: `include/gui_forms/text/types/text_types.hpp:65`
- Definition: `inline/header-only`

TextStyleId is the ordered compact identity stored by style spans.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const TextStyleId&, const TextStyleId&) = default
```

Provides value ordering and equality.
