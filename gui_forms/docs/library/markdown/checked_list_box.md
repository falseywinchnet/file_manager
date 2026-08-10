# CheckedListBox

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `ListBox → CheckedListBox`  
Declaration: `include/gui_forms/input_controls.hpp:253`  
Definition: `src/controls/input_controls.cpp`

CheckedListBox is a visual retained control declared in include/gui_forms/input_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `CheckedListBox`

```cpp
explicit CheckedListBox(StableId stable_id)
```

Constructs or tears down the retained CheckedListBox object according to its ownership contract.

### `set_items`

```cpp
void set_items(std::vector<std::string> items) override
```

Synchronously updates the retained items property. Validation, typed invalidation, and notifications are defined by the implementation.

### `add_item`

```cpp
void add_item(std::string item) override
```

Public CheckedListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_item`

```cpp
void add_item(std::string item, CheckState state)
```

Public CheckedListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_item`

```cpp
void remove_item(std::size_t index) override
```

Public CheckedListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear_items`

```cpp
void clear_items() override
```

Removes the explicit items value and restores fallback behavior.

### `item_check_state`

```cpp
[[nodiscard]] CheckState item_check_state(std::size_t index) const
```

Reports the current item check state value without mutation.

### `item_checked`

```cpp
[[nodiscard]] bool item_checked(std::size_t index) const
```

Reports the current item checked value without mutation.

### `set_item_check_state`

```cpp
void set_item_check_state(std::size_t index, CheckState state)
```

Synchronously updates the retained item check state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_item_checked`

```cpp
void set_item_checked(std::size_t index, bool checked)
```

Synchronously updates the retained item checked property. Validation, typed invalidation, and notifications are defined by the implementation.

### `toggle_item`

```cpp
void toggle_item(std::size_t index)
```

Public CheckedListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `checked_indices`

```cpp
[[nodiscard]] std::vector<std::size_t> checked_indices() const
```

Reports the current checked indices value without mutation.

### `check_on_click`

```cpp
[[nodiscard]] bool check_on_click() const noexcept
```

Reports the current check on click value without mutation.

### `set_check_on_click`

```cpp
void set_check_on_click(bool enabled)
```

Synchronously updates the retained check on click property. Validation, typed invalidation, and notifications are defined by the implementation.

### `item_checking`

```cpp
[[nodiscard]] Event<ItemCheckEvent&>& item_checking() noexcept
```

Public CheckedListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `item_check_state_changed`

```cpp
[[nodiscard]] Event<std::size_t, CheckState>& item_check_state_changed() noexcept
```

Public CheckedListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_pointer`

```cpp
void on_pointer(PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_key`

```cpp
void on_key(KeyEvent& event) override
```

Consumes normalized keyboard input for this control's interaction contract.

### `semantic_virtual_children`

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Reports the current semantic virtual children value without mutation.

### `on_semantic_child_action`

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value) override
```

Public CheckedListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
