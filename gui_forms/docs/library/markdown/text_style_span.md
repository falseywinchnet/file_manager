# TextStyleSpan

- Status: **OBSERVED: bundle 009 style span review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `TextStyleSpan`
- Declaration: `include/gui_forms/text/types/text_types.hpp:71`
- Definition: `inline/header-only`

TextStyleSpan maps one UTF-8 half-open range to a compact style identity.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const TextStyleSpan&, const TextStyleSpan&) = default
```

Compares range and style.
