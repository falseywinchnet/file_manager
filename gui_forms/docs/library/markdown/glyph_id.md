# GlyphId

- Status: **OBSERVED: bundle 009 glyph identity review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `GlyphId`
- Declaration: `include/gui_forms/text_shaping/types/text_shaping_types.hpp:19`
- Definition: `inline/header-only`

GlyphId is the ordered font-local glyph identity.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const GlyphId&, const GlyphId&) = default
```

Provides value ordering and equality.
