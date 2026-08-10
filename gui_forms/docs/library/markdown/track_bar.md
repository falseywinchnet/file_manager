# TrackBar

- Status: **OBSERVED: bundle 004 split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `RangeControl → TrackBar`
- Declaration: `include/gui_forms/controls/range_control/track_bar/track_bar.hpp:16`
- Definition: `src/controls/range_control/track_bar/track_bar.cpp`

TrackBar specializes RangeControl with bounded tick frequency, optional ticks, explicit visual style, orientation-aware measurement, absolute pointer positioning, keyboard movement, focus rendering, and slider semantics.

## Visual evidence

![TrackBar](../captures/range_controls.png)

## Declared methods

### `TrackBar` (public)

```cpp
explicit TrackBar(StableId stable_id)
```

Constructs a RangeControl with slider cursor and control-specific visual defaults.

### `tick_frequency` (public)

```cpp
[[nodiscard]] double tick_frequency() const noexcept
```

Returns the positive value interval between recorded tick marks.

### `set_tick_frequency` (public)

```cpp
void set_tick_frequency(double frequency)
```

Accepts a finite positive interval and invalidates painting.

### `show_ticks` (public)

```cpp
[[nodiscard]] bool show_ticks() const noexcept
```

Reports whether tick marks are painted beside the track.

### `set_show_ticks` (public)

```cpp
void set_show_ticks(bool show)
```

Toggles tick projection and invalidates measure and paint.

### `visual_style` (public)

```cpp
[[nodiscard]] TrackBarVisualStyle visual_style() const noexcept
```

Returns standard, compact, or accent slider appearance.

### `set_visual_style` (public)

```cpp
void set_visual_style(TrackBarVisualStyle style)
```

Validates the style vocabulary and invalidates size and appearance.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Provides orientation- and tick-aware desired cross-axis thickness.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records track, completed segment, ticks, thumb, hover/press, focus, and disabled states.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Captures primary dragging and converts window points to constrained range values.

### `on_key` (public)

```cpp
void on_key(KeyEvent& event) override
```

Handles arrows, page keys, home, and end through reasoned RangeControl transitions.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Commits the keyboard focus cue and invalidates slider painting.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a slider role with numeric range, value, orientation, and supported change actions.

### `on_semantic_action` (public)

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Routes increment, decrement, and set-value through ordinary constrained transitions.

### `value_from_window_point` (private)

```cpp
[[nodiscard]] double value_from_window_point(Point point) const noexcept
```

Reports the current value from window point value without mutation.
