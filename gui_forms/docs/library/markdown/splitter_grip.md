# SplitterGrip

- Status: **OBSERVED: source-private state machine; M4 focused and installed-consumer tests pass; current proximity visuals await Screen Sharing**
- Kind: **class / visual retained control**
- Hierarchy: `Control → SplitterGrip`
- Declaration: `src/controls/scrollable_control/container_control/split_container/splitter_grip.hpp:10`
- Definition: `src/controls/scrollable_control/container_control/split_container/splitter_grip.cpp`

SplitterGrip is SplitContainer's source-private retained seam control. It owns
only visual/focus/cursor/collapse-tab state while SplitContainer remains the
allocation and input-policy authority. The rest actuator is 7 x 28 logical
units; pointer proximity, active press, or keyboard-visible focus expands it to
12 x 42 with stronger contrast and depth. That decoration is independent from
the consumer-authored invisible hit strip and the <=3-unit visible seam.

## Visual evidence

![SplitterGrip](../captures/container_focus.png)

## Declared methods

### `SplitterGrip` (public)

```cpp
explicit SplitterGrip(StableId stable_id)
```

Constructs a focusable retained seam control with orientation-specific resize cursor policy.

### `set_orientation` (public)

```cpp
void set_orientation(Orientation orientation)
```

Validates orientation, updates the resize cursor, and invalidates painting.

### `set_visible_width` (public)

```cpp
void set_visible_width(double width)
```

Accepts a finite positive visible thickness independent of the Control's enlarged hit bounds.

### `set_collapse_appearance` (public)

```cpp
void set_collapse_appearance(SplitFixedPanel panel, bool collapsed)
```

Commits optional first/second collapse-tab direction and current collapsed appearance.

### `set_interaction_active` (public)

```cpp
void set_interaction_active(bool active)
```

Projects SplitContainer's retained drag/collapse press state into the private
visual state machine without transferring allocation authority.

### `actuator_bounds` (public)

```cpp
[[nodiscard]] Rect actuator_bounds() const noexcept
```

Returns the expanded 12 x 42 activation/proximity box in local coordinates so
SplitContainer can own preview routing across adjacent pane pixels.

### `visual_outsets` (public)

```cpp
[[nodiscard]] Insets visual_outsets() const noexcept override
```

Declares the union of the engaged actuator and its conservative renderer shadow
envelope for damage and display-list replay without changing layout or hit
testing.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Tracks bounded pointer proximity against the center actuator rather than the
entire pane-length splitter strip.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Stores focus state; engaged painting is gated by Window's keyboard-visible focus
cue so primary-pointer focus does not impersonate keyboard discovery.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records the centered visible seam plus the quiet or engaged directional actuator,
including bounded shadow depth and horizontal-orientation transposition.
