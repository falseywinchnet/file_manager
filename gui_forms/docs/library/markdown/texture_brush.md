# TextureBrush

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Brush → TextureBrush`  
Declaration: `include/gui_forms/drawing.hpp:719`  
Definition: `src/core/drawing.cpp`

TextureBrush is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `TextureBrush`

```cpp
explicit TextureBrush(const Bitmap& image, WrapMode wrap_mode = WrapMode::tile)
```

Constructs or tears down the retained TextureBrush object according to its ownership contract.

### `TextureBrush`

```cpp
explicit TextureBrush(const ImageReference& image, WrapMode wrap_mode = WrapMode::tile)
```

Constructs or tears down the retained TextureBrush object according to its ownership contract.

### `set_wrap_mode`

```cpp
void set_wrap_mode(WrapMode mode)
```

Synchronously updates the retained wrap mode property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_transform`

```cpp
void set_transform(Matrix transform)
```

Synchronously updates the retained transform property. Validation, typed invalidation, and notifications are defined by the implementation.

### `reset_transform`

```cpp
void reset_transform()
```

Returns transform to its inherited or default policy.

### `translate_transform`

```cpp
void translate_transform(double x, double y)
```

Public TextureBrush operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `scale_transform`

```cpp
void scale_transform(double x, double y)
```

Public TextureBrush operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `rotate_transform`

```cpp
void rotate_transform(double degrees)
```

Public TextureBrush operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clone`

```cpp
[[nodiscard]] std::unique_ptr<TextureBrush> clone() const
```

Reports the current clone value without mutation.

### `snapshot`

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Reports the current snapshot value without mutation.
