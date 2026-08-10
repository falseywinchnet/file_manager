# SolidBrush

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Brush → SolidBrush`  
Declaration: `include/gui_forms/drawing.hpp:365`  
Definition: `src/core/drawing.cpp`

SolidBrush is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `SolidBrush`

```cpp
explicit SolidBrush(Color color)
```

Constructs or tears down the retained SolidBrush object according to its ownership contract.

### `color`

```cpp
[[nodiscard]] Color color() const
```

Reports the current color value without mutation.

### `snapshot`

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Reports the current snapshot value without mutation.
