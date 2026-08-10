# RasterCanvas

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Control → RasterCanvas`  
Declaration: `include/gui_forms/canvas.hpp:15`  
Definition: `src/controls/canvas.cpp`

RasterCanvas is a visual retained control declared in include/gui_forms/canvas.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `RasterCanvas`

```cpp
explicit RasterCanvas(StableId stable_id)
```

Constructs or tears down the retained RasterCanvas object according to its ownership contract.

### `bitmap`

```cpp
[[nodiscard]] const std::shared_ptr<gui_drawing::Bitmap>& bitmap() const noexcept
```

Reports the current bitmap value without mutation.

### `set_bitmap`

```cpp
void set_bitmap(std::shared_ptr<gui_drawing::Bitmap> bitmap)
```

Synchronously updates the retained bitmap property. Validation, typed invalidation, and notifications are defined by the implementation.

### `clear_bitmap`

```cpp
void clear_bitmap()
```

Removes the explicit bitmap value and restores fallback behavior.

### `zoom`

```cpp
[[nodiscard]] double zoom() const noexcept
```

Reports the current zoom value without mutation.

### `set_zoom`

```cpp
void set_zoom(double zoom)
```

Synchronously updates the retained zoom property. Validation, typed invalidation, and notifications are defined by the implementation.

### `view_origin`

```cpp
[[nodiscard]] gui_drawing::PointF view_origin() const noexcept
```

Reports the current view origin value without mutation.

### `set_view_origin`

```cpp
void set_view_origin(gui_drawing::PointF origin)
```

Synchronously updates the retained view origin property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_view`

```cpp
void set_view(double zoom, gui_drawing::PointF origin)
```

Synchronously updates the retained view property. Validation, typed invalidation, and notifications are defined by the implementation.

### `sampling`

```cpp
[[nodiscard]] ImageSampling sampling() const noexcept
```

Reports the current sampling value without mutation.

### `set_sampling`

```cpp
void set_sampling(ImageSampling sampling)
```

Synchronously updates the retained sampling property. Validation, typed invalidation, and notifications are defined by the implementation.

### `transparency_grid`

```cpp
[[nodiscard]] bool transparency_grid() const noexcept
```

Reports the current transparency grid value without mutation.

### `set_transparency_grid`

```cpp
void set_transparency_grid(bool visible)
```

Synchronously updates the retained transparency grid property. Validation, typed invalidation, and notifications are defined by the implementation.

### `canvas_background`

```cpp
[[nodiscard]] Color canvas_background() const noexcept
```

Reports the current canvas background value without mutation.

### `set_canvas_background`

```cpp
void set_canvas_background(Color color)
```

Synchronously updates the retained canvas background property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_transparency_colors`

```cpp
void set_transparency_colors(Color first, Color second)
```

Synchronously updates the retained transparency colors property. Validation, typed invalidation, and notifications are defined by the implementation.

### `presented_generation`

```cpp
[[nodiscard]] std::uint64_t presented_generation() const noexcept
```

Reports the current presented generation value without mutation.

### `last_resource_error`

```cpp
[[nodiscard]] ImageResourceError last_resource_error() const noexcept
```

Reports the current last resource error value without mutation.

### `synchronize_bitmap`

```cpp
[[nodiscard]] bool synchronize_bitmap()
```

Public RasterCanvas operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `bitmap_to_client`

```cpp
[[nodiscard]] Rect bitmap_to_client(gui_drawing::RectI pixels) const noexcept
```

Reports the current bitmap to client value without mutation.

### `client_to_bitmap`

```cpp
[[nodiscard]] gui_drawing::PointF client_to_bitmap(Point client) const noexcept
```

Reports the current client to bitmap value without mutation.

### `visible_bitmap_bounds`

```cpp
[[nodiscard]] gui_drawing::RectF visible_bitmap_bounds() const
```

Reports the current visible bitmap bounds value without mutation.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
