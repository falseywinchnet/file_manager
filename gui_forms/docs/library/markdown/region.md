# Region

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject → Region`  
Declaration: `include/gui_forms/drawing.hpp:565`  
Definition: `src/core/drawing.cpp`

Region is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Region`

```cpp
explicit Region(RectF rectangle)
```

Constructs or tears down the retained Region object according to its ownership contract.

### `Region`

```cpp
explicit Region(const GraphicsPath& path)
```

Constructs or tears down the retained Region object according to its ownership contract.

### `unite`

```cpp
void unite(RectF rectangle)
```

Public Region operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `unite`

```cpp
void unite(const GraphicsPath& path)
```

Public Region operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `exclude`

```cpp
void exclude(RectF rectangle)
```

Public Region operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `is_visible`

```cpp
[[nodiscard]] bool is_visible(PointF point) const
```

Reports the current is visible value without mutation.

### `bounds`

```cpp
[[nodiscard]] RectF bounds() const
```

Reports the current bounds value without mutation.

### `snapshot`

```cpp
[[nodiscard]] RegionSnapshot snapshot() const
```

Reports the current snapshot value without mutation.
