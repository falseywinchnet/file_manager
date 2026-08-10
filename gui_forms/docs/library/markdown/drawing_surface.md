# DrawingSurface

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Control → DrawingSurface`  
Declaration: `include/gui_forms/diagnostic_controls.hpp:14`  
Definition: `src/controls/diagnostic_controls.cpp`

DrawingSurface is a visual retained control declared in include/gui_forms/diagnostic_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `DrawingSurface`

```cpp
explicit DrawingSurface(StableId stable_id)
```

Constructs or tears down the retained DrawingSurface object according to its ownership contract.

### `set_paint_callback`

```cpp
void set_paint_callback(PaintCallback callback)
```

Synchronously updates the retained paint callback property. Validation, typed invalidation, and notifications are defined by the implementation.

### `has_paint_callback`

```cpp
[[nodiscard]] bool has_paint_callback() const noexcept
```

Reports the current has paint callback value without mutation.

### `hit_test_visible`

```cpp
[[nodiscard]] bool hit_test_visible() const noexcept
```

Reports the current hit test visible value without mutation.

### `set_hit_test_visible`

```cpp
void set_hit_test_visible(bool visible)
```

Synchronously updates the retained hit test visible property. Validation, typed invalidation, and notifications are defined by the implementation.

### `semantic_role`

```cpp
[[nodiscard]] SemanticRole semantic_role() const noexcept
```

Reports the current semantic role value without mutation.

### `set_semantic_role`

```cpp
void set_semantic_role(SemanticRole role)
```

Synchronously updates the retained semantic role property. Validation, typed invalidation, and notifications are defined by the implementation.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `hit_test_local`

```cpp
[[nodiscard]] bool hit_test_local(Point local_point) const override
```

Reports the current hit test local value without mutation.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
