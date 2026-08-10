# TrackBar

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `RangeControl → TrackBar`  
Declaration: `include/gui_forms/range_controls.hpp:127`  
Definition: `src/controls/range_controls.cpp`

TrackBar is a visual retained control declared in include/gui_forms/range_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `TrackBar`

```cpp
explicit TrackBar(StableId stable_id)
```

Constructs or tears down the retained TrackBar object according to its ownership contract.

### `tick_frequency`

```cpp
[[nodiscard]] double tick_frequency() const noexcept
```

Reports the current tick frequency value without mutation.

### `set_tick_frequency`

```cpp
void set_tick_frequency(double frequency)
```

Synchronously updates the retained tick frequency property. Validation, typed invalidation, and notifications are defined by the implementation.

### `show_ticks`

```cpp
[[nodiscard]] bool show_ticks() const noexcept
```

Reports the current show ticks value without mutation.

### `set_show_ticks`

```cpp
void set_show_ticks(bool show)
```

Synchronously updates the retained show ticks property. Validation, typed invalidation, and notifications are defined by the implementation.

### `visual_style`

```cpp
[[nodiscard]] TrackBarVisualStyle visual_style() const noexcept
```

Reports the current visual style value without mutation.

### `set_visual_style`

```cpp
void set_visual_style(TrackBarVisualStyle style)
```

Synchronously updates the retained visual style property. Validation, typed invalidation, and notifications are defined by the implementation.

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

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.

### `on_semantic_action`

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Public TrackBar operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
