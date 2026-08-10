# ShapedGlyph

- Status: **OBSERVED: bundle 009 HarfBuzz glyph result review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ShapedGlyph`
- Declaration: `src/render/text/harfbuzz/harfbuzz_font_engine.hpp:16`
- Definition: `inline/header-only`

ShapedGlyph carries one font-local glyph, UTF-8 cluster, absolute pen position, and two-axis advance.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ShapedGlyph&, const ShapedGlyph&) = default
```

Compares every shaped glyph field.
