# Font

- Status: **OBSERVED: bundle 009 font value split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → Font`
- Declaration: `include/gui_forms/drawing/font/font.hpp:18`
- Definition: `src/core/drawing/font/font.cpp`

Font retains validated UTF-8 family, bounded size, style bits, unit, and charset without exposing a platform font object.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `Font` (public)

```cpp
Font(std::string family, double size, FontStyle style = FontStyle::regular, GraphicsUnit unit = GraphicsUnit::point, std::uint8_t charset = 1)
```

Validates family, size, style mask, unit, and charset, or derives a style variant from another live Font.

### `Font` (public)

```cpp
Font(const Font& source, FontStyle style)
```

Validates family, size, style mask, unit, and charset, or derives a style variant from another live Font.

### `snapshot` (public)

```cpp
[[nodiscard]] FontSnapshot snapshot() const
```

Requires liveness and returns the complete renderer-neutral font recipe.

### `deterministic_height` (public)

```cpp
[[nodiscard]] std::int32_t deterministic_height() const
```

Converts size/unit to a bounded deterministic logical height for compatibility callers.
