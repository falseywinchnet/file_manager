# ToolTipBubble

- Status: **OBSERVED: bundle 005 source-private visual split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → ToolTipBubble`
- Declaration: `src/controls/tool_tip/tool_tip_bubble/tool_tip_bubble.hpp:7`
- Definition: `src/controls/tool_tip/tool_tip_bubble/tool_tip_bubble.cpp`

ToolTipBubble is ToolTip's source-private retained visual. It owns a label child, theme-derived frame and padding, exact measured content size, input transparency, and tooltip semantics; scheduling and popup lifetime remain in ToolTip.

## Visual evidence

![ToolTipBubble](../captures/tool_tip_error_provider.png)

## Declared methods

### `ToolTipBubble` (public)

```cpp
ToolTipBubble(StableId stable_id, std::string text)
```

Constructs an input-transparent bubble with retained text, wrapping width, and keyboard-origin metadata.

### `initialize_control_tree` (public)

```cpp
void initialize_control_tree()
```

Idempotently creates and attaches the internal label role after shared ownership exists.

### `set_content_size` (public)

```cpp
void set_content_size(Size size)
```

Commits measured bubble geometry and arranges the label inside theme-scaled padding.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records the theme-derived tooltip background and border behind the label child.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(Point point) const override
```

Rejects hit testing so the bubble never becomes an input target.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects tooltip role, text, and keyboard/pointer origin.
