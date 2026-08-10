# TextShaper

- Status: **OBSERVED: bundle 009 shaping interface split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `TextShaper`
- Declaration: `include/gui_forms/text_shaping/text_shaper/text_shaper.hpp:7`
- Definition: `inline/header-only`

TextShaper is the renderer-neutral boundary from a validated shaping request to clustered glyph placement.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `~TextShaper` (public)

```cpp
virtual ~TextShaper() = default
```

Provides polymorphic destruction.

### `shape` (public)

```cpp
[[nodiscard]] virtual GlyphRun shape(const ShapingRequest& request) = 0
```

Shapes one request into a source-correlated glyph run.
