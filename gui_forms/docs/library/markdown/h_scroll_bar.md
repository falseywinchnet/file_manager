# HScrollBar

- Status: **OBSERVED: bundle 004 leaf-type split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `ScrollBar → HScrollBar`
- Declaration: `include/gui_forms/controls/range_control/scroll_bar/h_scroll_bar/h_scroll_bar.hpp:7`
- Definition: `src/controls/range_control/scroll_bar/h_scroll_bar/h_scroll_bar.cpp`

HScrollBar is the explicit horizontal leaf type for reflection, serialization, toolbox discovery, and conventional API compatibility.

## Visual evidence

![HScrollBar](../captures/range_controls.png)

## Declared methods

### `HScrollBar` (public)

```cpp
explicit HScrollBar(StableId stable_id)
```

Constructs ScrollBar with horizontal orientation.
