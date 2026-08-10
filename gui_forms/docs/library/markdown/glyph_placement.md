# GlyphPlacement

- Status: **OBSERVED: bundle 009 glyph placement review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `GlyphPlacement`
- Declaration: `include/gui_forms/text_shaping/types/text_shaping_types.hpp:57`
- Definition: `inline/header-only`

GlyphPlacement correlates one glyph to a UTF-8 cluster and finite two-axis advance/offset values.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const GlyphPlacement&, const GlyphPlacement&) = default
```

Compares glyph, cluster, advances, and offsets.
