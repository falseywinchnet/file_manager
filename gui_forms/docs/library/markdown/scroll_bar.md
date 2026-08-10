# ScrollBar

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `RangeControl → ScrollBar`  
Declaration: `include/gui_forms/range_controls.hpp:253`  
Definition: `src/controls/range_controls.cpp`

ScrollBar is a visual retained control declared in include/gui_forms/range_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ScrollBar`

```cpp
explicit ScrollBar(StableId stable_id, Orientation orientation = Orientation::vertical)
```

Constructs or tears down the retained ScrollBar object according to its ownership contract.

### `button_extent`

```cpp
[[nodiscard]] double button_extent() const noexcept
```

Reports the current button extent value without mutation.

### `set_button_extent`

```cpp
void set_button_extent(double extent)
```

Synchronously updates the retained button extent property. Validation, typed invalidation, and notifications are defined by the implementation.

### `minimum_thumb_extent`

```cpp
[[nodiscard]] double minimum_thumb_extent() const noexcept
```

Reports the current minimum thumb extent value without mutation.

### `set_minimum_thumb_extent`

```cpp
void set_minimum_thumb_extent(double extent)
```

Synchronously updates the retained minimum thumb extent property. Validation, typed invalidation, and notifications are defined by the implementation.

### `initial_repeat_delay`

```cpp
[[nodiscard]] FrameInterval initial_repeat_delay() const noexcept
```

Reports the current initial repeat delay value without mutation.

### `set_initial_repeat_delay`

```cpp
void set_initial_repeat_delay(FrameInterval delay)
```

Synchronously updates the retained initial repeat delay property. Validation, typed invalidation, and notifications are defined by the implementation.

### `repeat_interval`

```cpp
[[nodiscard]] FrameInterval repeat_interval() const noexcept
```

Reports the current repeat interval value without mutation.

### `set_repeat_interval`

```cpp
void set_repeat_interval(FrameInterval interval)
```

Synchronously updates the retained repeat interval property. Validation, typed invalidation, and notifications are defined by the implementation.

### `decrement_button_bounds`

```cpp
[[nodiscard]] Rect decrement_button_bounds() const noexcept
```

Reports the current decrement button bounds value without mutation.

### `increment_button_bounds`

```cpp
[[nodiscard]] Rect increment_button_bounds() const noexcept
```

Reports the current increment button bounds value without mutation.

### `track_bounds`

```cpp
[[nodiscard]] Rect track_bounds() const noexcept
```

Reports the current track bounds value without mutation.

### `thumb_bounds`

```cpp
[[nodiscard]] Rect thumb_bounds() const noexcept
```

Reports the current thumb bounds value without mutation.

### `part_at`

```cpp
[[nodiscard]] ScrollBarPart part_at(Point local_point) const noexcept
```

Reports the current part at value without mutation.

### `measure`

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired size from the available constraint without arranging children.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `on_pointer`

```cpp
void on_pointer(PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_key`

```cpp
void on_key(KeyEvent& event) override
```

Consumes normalized keyboard input for this control's interaction contract.

### `on_focus_changed`

```cpp
void on_focus_changed(bool focused) override
```

Updates focus-dependent retained state and invalidates affected presentation/semantics.

### `on_frame`

```cpp
void on_frame(FrameTime now) override
```

Public ScrollBar operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.

### `on_semantic_action`

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Public ScrollBar operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
