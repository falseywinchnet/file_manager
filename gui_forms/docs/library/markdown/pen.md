# Pen

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject → Pen`  
Declaration: `include/gui_forms/drawing.hpp:435`  
Definition: `src/core/drawing.cpp`

Pen is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Pen`

```cpp
explicit Pen(Color color, double width = 1.0)
```

Constructs or tears down the retained Pen object according to its ownership contract.

### `Pen`

```cpp
explicit Pen(const Brush& brush, double width = 1.0)
```

Constructs or tears down the retained Pen object according to its ownership contract.

### `set_width`

```cpp
void set_width(double width)
```

Synchronously updates the retained width property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_dash_style`

```cpp
void set_dash_style(DashStyle style)
```

Synchronously updates the retained dash style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_dash_pattern`

```cpp
void set_dash_pattern(std::span<const double> pattern)
```

Synchronously updates the retained dash pattern property. Validation, typed invalidation, and notifications are defined by the implementation.

### `snapshot`

```cpp
[[nodiscard]] PenSnapshot snapshot() const
```

Reports the current snapshot value without mutation.
