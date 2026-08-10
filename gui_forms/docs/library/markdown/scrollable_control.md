# ScrollableControl

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Control → ScrollableControl`  
Declaration: `include/gui_forms/scrolling.hpp:107`  
Definition: `src/controls/scrolling.cpp`

ScrollableControl is a visual retained control declared in include/gui_forms/scrolling.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ScrollableControl`

```cpp
explicit ScrollableControl(StableId stable_id)
```

Constructs or tears down the retained ScrollableControl object according to its ownership contract.

### `auto_scroll`

```cpp
[[nodiscard]] bool auto_scroll() const noexcept
```

Reports the current auto scroll value without mutation.

### `set_auto_scroll`

```cpp
virtual void set_auto_scroll(bool enabled)
```

Synchronously updates the retained auto scroll property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_scroll_margin`

```cpp
[[nodiscard]] Size auto_scroll_margin() const noexcept
```

Reports the current auto scroll margin value without mutation.

### `set_auto_scroll_margin`

```cpp
void set_auto_scroll_margin(Size margin)
```

Synchronously updates the retained auto scroll margin property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_auto_scroll_margin`

```cpp
void set_auto_scroll_margin(double x, double y)
```

Synchronously updates the retained auto scroll margin property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_scroll_min_size`

```cpp
[[nodiscard]] Size auto_scroll_min_size() const noexcept
```

Reports the current auto scroll min size value without mutation.

### `set_auto_scroll_min_size`

```cpp
void set_auto_scroll_min_size(Size size)
```

Synchronously updates the retained auto scroll min size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_scroll_position`

```cpp
[[nodiscard]] Point auto_scroll_position() const noexcept
```

Reports the current auto scroll position value without mutation.

### `set_auto_scroll_position`

```cpp
void set_auto_scroll_position(Point position)
```

Synchronously updates the retained auto scroll position property. Validation, typed invalidation, and notifications are defined by the implementation.

### `scroll_position`

```cpp
[[nodiscard]] Point scroll_position() const noexcept
```

Reports the current scroll position value without mutation.

### `display_rectangle`

```cpp
[[nodiscard]] Rect display_rectangle() const noexcept override
```

Reports the current display rectangle value without mutation.

### `viewport_rectangle`

```cpp
[[nodiscard]] Rect viewport_rectangle() const noexcept
```

Reports the current viewport rectangle value without mutation.

### `horizontal_scroll`

```cpp
[[nodiscard]] const ScrollProperties& horizontal_scroll() const noexcept
```

Reports the current horizontal scroll value without mutation.

### `horizontal_scroll`

```cpp
[[nodiscard]] ScrollProperties& horizontal_scroll() noexcept
```

Public ScrollableControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `vertical_scroll`

```cpp
[[nodiscard]] const ScrollProperties& vertical_scroll() const noexcept
```

Reports the current vertical scroll value without mutation.

### `vertical_scroll`

```cpp
[[nodiscard]] ScrollProperties& vertical_scroll() noexcept
```

Public ScrollableControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `hscroll`

```cpp
[[nodiscard]] bool hscroll() const noexcept
```

Reports the current hscroll value without mutation.

### `vscroll`

```cpp
[[nodiscard]] bool vscroll() const noexcept
```

Reports the current vscroll value without mutation.

### `set_hscroll`

```cpp
void set_hscroll(bool visible)
```

Synchronously updates the retained hscroll property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_vscroll`

```cpp
void set_vscroll(bool visible)
```

Synchronously updates the retained vscroll property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_scroll_state`

```cpp
[[nodiscard]] bool get_scroll_state(std::uint32_t bit) const noexcept
```

Reports the current get scroll state value without mutation.

### `set_scroll_state`

```cpp
void set_scroll_state(std::uint32_t bit, bool value)
```

Synchronously updates the retained scroll state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `scroll`

```cpp
[[nodiscard]] Event<ScrollEvent&>& scroll() noexcept
```

Public ScrollableControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `last_scroll_event`

```cpp
[[nodiscard]] std::optional<ScrollEvent> last_scroll_event() const noexcept
```

Reports the current last scroll event value without mutation.

### `scroll_event_revision`

```cpp
[[nodiscard]] std::uint64_t scroll_event_revision() const noexcept
```

Reports the current scroll event revision value without mutation.

### `scroll_snapshot`

```cpp
[[nodiscard]] ScrollSnapshot scroll_snapshot() const noexcept
```

Reports the current scroll snapshot value without mutation.

### `scroll_to`

```cpp
bool scroll_to(Point position, ScrollEventType type = ScrollEventType::thumb_position, bool notify = false)
```

Public ScrollableControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `scroll_by`

```cpp
bool scroll_by(Point delta, ScrollEventType horizontal_type = ScrollEventType::small_increment, ScrollEventType vertical_type = ScrollEventType::small_increment, bool notify = false)
```

Public ScrollableControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `scroll_control_into_view`

```cpp
void scroll_control_into_view(const Control::Ptr& control)
```

Public ScrollableControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

### `on_pointer`

```cpp
void on_pointer(PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_pointer_bubble`

```cpp
void on_pointer_bubble(PointerEvent& event) override
```

Public ScrollableControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_virtual_children`

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Reports the current semantic virtual children value without mutation.

### `on_semantic_child_action`

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value) override
```

Public ScrollableControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
