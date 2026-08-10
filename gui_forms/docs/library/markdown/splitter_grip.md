# SplitterGrip

- Status: **OBSERVED: bundle 003 source-private state-machine split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → SplitterGrip`
- Declaration: `src/controls/scrollable_control/container_control/split_container/splitter_grip.hpp:10`
- Definition: `src/controls/scrollable_control/container_control/split_container/splitter_grip.cpp`

SplitterGrip is SplitContainer's source-private retained seam control. It owns only visual/focus/cursor/collapse-tab state while SplitContainer remains the allocation and input-policy authority.

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

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Stores the keyboard-focus cue and invalidates seam painting.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records the centered visible seam, focused cue, and optional directional collapse tab within the enlarged control bounds.
