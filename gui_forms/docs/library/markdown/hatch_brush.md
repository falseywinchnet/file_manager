# HatchBrush

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Brush → HatchBrush`  
Declaration: `include/gui_forms/drawing.hpp:375`  
Definition: `src/core/drawing.cpp`

HatchBrush is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `HatchBrush`

```cpp
HatchBrush(HatchStyle style, Color foreground, Color background = Color::from_argb(0U, 0U, 0U, 0U))
```

Constructs or tears down the retained HatchBrush object according to its ownership contract.

### `snapshot`

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Reports the current snapshot value without mutation.
