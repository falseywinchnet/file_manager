# DrawingSurface

- Status: **OBSERVED: bundle 006 split and background enhancement; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → DrawingSurface`
- Declaration: `include/gui_forms/controls/drawing_surface/drawing_surface.hpp:11`
- Definition: `src/controls/drawing_surface/drawing_surface.cpp`

DrawingSurface is the public owner-draw primitive for renderer-neutral application visuals. It retains one paint callback, explicit hit-test and semantic policy, and a caller-owned background while normal Control damage, display-list retention, and lifetime rules remain authoritative.

## Visual evidence

![DrawingSurface](../captures/drawing_raster_material.png)

## Declared methods

### `DrawingSurface` (public)

```cpp
explicit DrawingSurface(StableId stable_id)
```

Constructs an owner-draw control with transparent background, image semantics, and input transparency by default.

### `set_paint_callback` (public)

```cpp
void set_paint_callback(PaintCallback callback)
```

Replaces the renderer-neutral callback and invalidates paint and semantics without creating a subclass.

### `has_paint_callback` (public)

```cpp
[[nodiscard]] bool has_paint_callback() const noexcept
```

Reports whether an owner paint callback is currently retained.

### `hit_test_visible` (public)

```cpp
[[nodiscard]] bool hit_test_visible() const noexcept
```

Reports whether local bounds participate in pointer hit testing.

### `set_hit_test_visible` (public)

```cpp
void set_hit_test_visible(bool visible)
```

Commits input participation and invalidates hit-test and semantic projections.

### `semantic_role` (public)

```cpp
[[nodiscard]] SemanticRole semantic_role() const noexcept
```

Returns the caller-selected semantic role for the owner-drawn surface.

### `set_semantic_role` (public)

```cpp
void set_semantic_role(SemanticRole role)
```

Commits semantic role and invalidates accessibility projection.

### `background` (public)

```cpp
[[nodiscard]] Color background() const noexcept
```

Returns the retained background color painted before the callback.

### `set_background` (public)

```cpp
void set_background(Color color)
```

Commits owner-selectable backplane color and invalidates paint.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Paints the backplane and invokes the callback with exact local bounds and local damage.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(Point local_point) const override
```

Combines explicit hit-test visibility with ordinary Control bounds testing.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects caller role, accessible name, and description without inventing domain meaning.
