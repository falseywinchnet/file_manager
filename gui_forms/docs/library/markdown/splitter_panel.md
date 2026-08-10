# SplitterPanel

- Status: **OBSERVED: bundle 003 split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `ContainerControl → SplitterPanel`
- Declaration: `include/gui_forms/controls/scrollable_control/container_control/split_container/splitter_panel/splitter_panel.hpp:7`
- Definition: `src/controls/scrollable_control/container_control/split_container/splitter_panel/splitter_panel.cpp`

SplitterPanel is the allocated scroll-capable pane surface owned by SplitContainer, with explicit retained background painting so movement never exposes an invisible allocation.

## Visual evidence

![SplitterPanel](../captures/container_focus.png)

## Declared methods

### `SplitterPanel` (public)

```cpp
explicit SplitterPanel(StableId stable_id)
```

Constructs a transparent ContainerControl pane whose allocation is owned by SplitContainer.

### `background` (public)

```cpp
[[nodiscard]] Color background() const noexcept
```

Returns the explicit pane fill color.

### `set_background` (public)

```cpp
void set_background(Color color)
```

Commits a real color change and invalidates pane painting.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records the pane background from committed local bounds before descendant content paints.
