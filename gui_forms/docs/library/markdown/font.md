# Font

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject → Font`  
Declaration: `include/gui_forms/drawing.hpp:460`  
Definition: `src/core/drawing.cpp`

Font is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Font`

```cpp
Font(std::string family, double size, FontStyle style = FontStyle::regular, GraphicsUnit unit = GraphicsUnit::point, std::uint8_t charset = 1)
```

Constructs or tears down the retained Font object according to its ownership contract.

### `Font`

```cpp
Font(const Font& source, FontStyle style)
```

Constructs or tears down the retained Font object according to its ownership contract.

### `snapshot`

```cpp
[[nodiscard]] FontSnapshot snapshot() const
```

Reports the current snapshot value without mutation.

### `deterministic_height`

```cpp
[[nodiscard]] std::int32_t deterministic_height() const
```

Reports the current deterministic height value without mutation.
