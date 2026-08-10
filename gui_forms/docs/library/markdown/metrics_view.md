# MetricsView

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Control → MetricsView`  
Declaration: `include/gui_forms/diagnostic_controls.hpp:46`  
Definition: `src/controls/diagnostic_controls.cpp`

MetricsView is a visual retained control declared in include/gui_forms/diagnostic_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `MetricsView`

```cpp
explicit MetricsView(StableId stable_id, std::string title = "Runtime metrics")
```

Constructs or tears down the retained MetricsView object according to its ownership contract.

### `title`

```cpp
[[nodiscard]] const std::string& title() const noexcept
```

Reports the current title value without mutation.

### `set_title`

```cpp
void set_title(std::string title)
```

Synchronously updates the retained title property. Validation, typed invalidation, and notifications are defined by the implementation.

### `style`

```cpp
[[nodiscard]] const BasicControlStyle& style() const noexcept
```

Reports the current style value without mutation.

### `set_style`

```cpp
void set_style(BasicControlStyle style)
```

Synchronously updates the retained style property. Validation, typed invalidation, and notifications are defined by the implementation.

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
