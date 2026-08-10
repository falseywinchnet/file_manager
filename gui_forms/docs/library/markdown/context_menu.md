# ContextMenu

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → ContextMenu`  
Declaration: `include/gui_forms/menu_controls.hpp:62`  
Definition: `src/controls/menu_controls.cpp`

ContextMenu is a class declared in include/gui_forms/menu_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ContextMenu`

```cpp
explicit ContextMenu(std::string stable_id)
```

Constructs or tears down the retained ContextMenu object according to its ownership contract.

### `~ContextMenu`

```cpp
~ContextMenu() override
```

Constructs or tears down the retained ContextMenu object according to its ownership contract.

### `stable_id`

```cpp
[[nodiscard]] const std::string& stable_id() const noexcept
```

Reports the current stable id value without mutation.

### `items`

```cpp
[[nodiscard]] const std::vector<MenuItemSpec>& items() const noexcept
```

Reports the current items value without mutation.

### `set_items`

```cpp
void set_items(std::vector<MenuItemSpec> items)
```

Synchronously updates the retained items property. Validation, typed invalidation, and notifications are defined by the implementation.

### `show`

```cpp
void show(const Control::Ptr& owner, Point window_position)
```

Public ContextMenu operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `close`

```cpp
void close() noexcept
```

Public ContextMenu operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `is_open`

```cpp
[[nodiscard]] bool is_open() const noexcept
```

Reports the current is open value without mutation.

### `item_invoked`

```cpp
[[nodiscard]] Event<const MenuItemInvocation&>& item_invoked() noexcept
```

Public ContextMenu operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `open_changed`

```cpp
[[nodiscard]] Event<bool>& open_changed() noexcept
```

Public ContextMenu operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
