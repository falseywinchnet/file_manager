# ScrollBar

- Status: **OBSERVED: bundle 004 hierarchical state-machine split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `RangeControl → ScrollBar`
- Declaration: `include/gui_forms/controls/range_control/scroll_bar/scroll_bar.hpp:19`
- Definition: `src/controls/range_control/scroll_bar/scroll_bar.cpp`

ScrollBar specializes RangeControl with independently bounded button/thumb/repeat timing, inspectable part geometry, proportional thumb allocation, drag and track/button interaction, deterministic key behavior, retained auto-repeat, focus cues, and scrollbar semantics.

## Visual evidence

![ScrollBar](../captures/range_controls.png)

## Declared methods

### `ScrollBar` (public)

```cpp
explicit ScrollBar(StableId stable_id, Orientation orientation = Orientation::vertical)
```

Constructs an orientation-specific range control with scrollbar cursor and repeat state.

### `button_extent` (public)

```cpp
[[nodiscard]] double button_extent() const noexcept
```

Returns the logical decrement/increment button axis extent.

### `set_button_extent` (public)

```cpp
void set_button_extent(double extent)
```

Accepts a finite bounded extent and invalidates geometry and paint.

### `minimum_thumb_extent` (public)

```cpp
[[nodiscard]] double minimum_thumb_extent() const noexcept
```

Returns the minimum logical draggable thumb extent.

### `set_minimum_thumb_extent` (public)

```cpp
void set_minimum_thumb_extent(double extent)
```

Accepts a finite bounded positive extent and invalidates geometry.

### `initial_repeat_delay` (public)

```cpp
[[nodiscard]] FrameInterval initial_repeat_delay() const noexcept
```

Returns the delay before a held part first repeats.

### `set_initial_repeat_delay` (public)

```cpp
void set_initial_repeat_delay(FrameInterval delay)
```

Accepts a positive duration for repeat qualification.

### `repeat_interval` (public)

```cpp
[[nodiscard]] FrameInterval repeat_interval() const noexcept
```

Returns the positive interval between qualified repeats.

### `set_repeat_interval` (public)

```cpp
void set_repeat_interval(FrameInterval interval)
```

Accepts a positive repeat cadence.

### `decrement_button_bounds` (public)

```cpp
[[nodiscard]] Rect decrement_button_bounds() const noexcept
```

Returns the current local geometry of the leading decrement part.

### `increment_button_bounds` (public)

```cpp
[[nodiscard]] Rect increment_button_bounds() const noexcept
```

Returns the current local geometry of the trailing increment part.

### `track_bounds` (public)

```cpp
[[nodiscard]] Rect track_bounds() const noexcept
```

Returns the local axis track between command buttons.

### `thumb_bounds` (public)

```cpp
[[nodiscard]] Rect thumb_bounds() const noexcept
```

Returns the constrained proportional thumb geometry within the track.

### `part_at` (public)

```cpp
[[nodiscard]] ScrollBarPart part_at(Point local_point) const noexcept
```

Classifies a local point as none, decrement button/page, thumb, increment page/button.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Provides orientation-aware desired thickness and two-button axis length.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records buttons, chevrons, track, pages, thumb, interaction, focus, and disabled states.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Owns part qualification, capture, thumb dragging, page/line changes, cancellation, and repeat scheduling.

### `on_key` (public)

```cpp
void on_key(KeyEvent& event) override
```

Handles arrows, page movement, home, and end through reasoned RangeControl transitions.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Commits keyboard focus cues and stops interaction when focus policy requires.

### `on_frame` (public)

```cpp
void on_frame(FrameTime now) override
```

Executes deadline-based held-part repeats while preserving current pointer qualification.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a scrollbar role with range, value, orientation, and line/page actions.

### `on_semantic_action` (public)

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Routes increment, decrement, page, and set-value commands through ordinary constrained state.

### `on_detached_from_window` (protected)

```cpp
void on_detached_from_window() noexcept override
```

Executes ScrollBar's on detached from window operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `axis_coordinate` (private)

```cpp
[[nodiscard]] double axis_coordinate(Point local_point) const noexcept
```

Reports the current axis coordinate value without mutation.

### `value_from_thumb_coordinate` (private)

```cpp
[[nodiscard]] double value_from_thumb_coordinate(double coordinate) const noexcept
```

Reports the current value from thumb coordinate value without mutation.

### `apply_part` (private)

```cpp
bool apply_part(ScrollBarPart part)
```

Executes ScrollBar's apply part operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `begin_repeat` (private)

```cpp
void begin_repeat(ScrollBarPart part, Point pointer)
```

Executes ScrollBar's begin repeat operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `stop_interaction` (private)

```cpp
void stop_interaction() noexcept
```

Executes ScrollBar's stop interaction operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
