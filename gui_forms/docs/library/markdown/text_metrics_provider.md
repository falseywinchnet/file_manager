# TextMetricsProvider

- Status: **OBSERVED: bundle 009 text-measurement interface review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `TextMetricsProvider`
- Declaration: `include/gui_forms/drawing/graphics_recorder/graphics_recorder.hpp:76`
- Definition: `inline/header-only`

TextMetricsProvider isolates renderer-specific measurement behind renderer-neutral UTF-8, font, and format snapshots.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `~TextMetricsProvider` (public)

```cpp
virtual ~TextMetricsProvider() = default
```

Provides polymorphic cleanup.

### `measure` (public)

```cpp
[[nodiscard]] virtual SizeF measure(std::string_view utf8, const FontSnapshot& font, const StringFormatSnapshot& format) const = 0
```

Measures one UTF-8 string under immutable font and format recipes.
