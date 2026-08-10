# TextMetricsProvider

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `TextMetricsProvider`  
Declaration: `include/gui_forms/drawing.hpp:796`  
Definition: `inline/header-only`

TextMetricsProvider is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `~TextMetricsProvider`

```cpp
virtual ~TextMetricsProvider() = default
```

Constructs or tears down the retained TextMetricsProvider object according to its ownership contract.

### `measure`

```cpp
[[nodiscard]] virtual SizeF measure(std::string_view utf8, const FontSnapshot& font, const StringFormatSnapshot& format) const = 0
```

Computes desired size from the available constraint without arranging children.
