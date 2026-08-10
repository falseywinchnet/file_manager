# VScrollBar

- Status: **OBSERVED: bundle 004 leaf-type split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `ScrollBar → VScrollBar`
- Declaration: `include/gui_forms/controls/range_control/scroll_bar/v_scroll_bar/v_scroll_bar.hpp:7`
- Definition: `src/controls/range_control/scroll_bar/v_scroll_bar/v_scroll_bar.cpp`

VScrollBar is the explicit vertical leaf type for reflection, serialization, toolbox discovery, and conventional API compatibility.

## Visual evidence

![VScrollBar](../captures/range_controls.png)

## Declared methods

### `VScrollBar` (public)

```cpp
explicit VScrollBar(StableId stable_id)
```

Constructs ScrollBar with vertical orientation.
