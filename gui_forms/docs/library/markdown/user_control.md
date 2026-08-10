# UserControl

- Status: **OBSERVED: bundle 003 split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `ContainerControl → UserControl`
- Declaration: `include/gui_forms/controls/scrollable_control/container_control/user_control/user_control.hpp:9`
- Definition: `src/controls/scrollable_control/container_control/user_control/user_control.cpp`

UserControl is the reusable authored-container lifecycle specialization: it distinguishes current attachment from first committed load and counts successful attachment epochs without conflating construction with visibility.

## Visual evidence

![UserControl](../captures/container_focus.png)

## Declared methods

### `UserControl` (public)

```cpp
explicit UserControl(StableId stable_id)
```

Constructs an unattached, not-yet-loaded reusable ContainerControl.

### `loaded` (public)

```cpp
[[nodiscard]] Event<>& loaded() noexcept
```

Returns the one-shot event published after the first Window attachment is fully committed.

### `is_loaded` (public)

```cpp
[[nodiscard]] bool is_loaded() const noexcept
```

Reports whether first committed attachment has occurred; detaching never clears this lifetime fact.

### `is_attached` (public)

```cpp
[[nodiscard]] bool is_attached() const noexcept
```

Reports current membership in an attached Window tree.

### `attachment_count` (public)

```cpp
[[nodiscard]] std::uint64_t attachment_count() const noexcept
```

Returns the number of committed attachment epochs, including reattachment after detach.

### `on_attached_to_window` (protected)

```cpp
void on_attached_to_window() override
```

Public UserControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_attachment_committed` (protected)

```cpp
void on_attachment_committed() noexcept override
```

Public UserControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_detached_from_window` (protected)

```cpp
void on_detached_from_window() noexcept override
```

Public UserControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
