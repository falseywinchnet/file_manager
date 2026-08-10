# ToolTip

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → ToolTip`  
Declaration: `include/gui_forms/tooltip.hpp:30`  
Definition: `src/controls/tooltip.cpp`

ToolTip is a class declared in include/gui_forms/tooltip.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ToolTip`

```cpp
explicit ToolTip(Window& window)
```

Constructs or tears down the retained ToolTip object according to its ownership contract.

### `~ToolTip`

```cpp
~ToolTip() override
```

Constructs or tears down the retained ToolTip object according to its ownership contract.

### `set_tool_tip`

```cpp
void set_tool_tip(const std::shared_ptr<Control>& target, std::string text)
```

Synchronously updates the retained tool tip property. Validation, typed invalidation, and notifications are defined by the implementation.

### `tool_tip`

```cpp
[[nodiscard]] std::string tool_tip(const Control& target) const
```

Reports the current tool tip value without mutation.

### `remove_tool_tip`

```cpp
bool remove_tool_tip(const Control& target)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear`

```cpp
void clear()
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `initial_delay`

```cpp
[[nodiscard]] std::chrono::milliseconds initial_delay() const noexcept
```

Reports the current initial delay value without mutation.

### `set_initial_delay`

```cpp
void set_initial_delay(std::chrono::milliseconds delay)
```

Synchronously updates the retained initial delay property. Validation, typed invalidation, and notifications are defined by the implementation.

### `reshow_delay`

```cpp
[[nodiscard]] std::chrono::milliseconds reshow_delay() const noexcept
```

Reports the current reshow delay value without mutation.

### `set_reshow_delay`

```cpp
void set_reshow_delay(std::chrono::milliseconds delay)
```

Synchronously updates the retained reshow delay property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_pop_delay`

```cpp
[[nodiscard]] std::chrono::milliseconds auto_pop_delay() const noexcept
```

Reports the current auto pop delay value without mutation.

### `set_auto_pop_delay`

```cpp
void set_auto_pop_delay(std::chrono::milliseconds delay)
```

Synchronously updates the retained auto pop delay property. Validation, typed invalidation, and notifications are defined by the implementation.

### `show_on_hover`

```cpp
[[nodiscard]] bool show_on_hover() const noexcept
```

Reports the current show on hover value without mutation.

### `set_show_on_hover`

```cpp
void set_show_on_hover(bool value)
```

Synchronously updates the retained show on hover property. Validation, typed invalidation, and notifications are defined by the implementation.

### `show_on_focus`

```cpp
[[nodiscard]] bool show_on_focus() const noexcept
```

Reports the current show on focus value without mutation.

### `set_show_on_focus`

```cpp
void set_show_on_focus(bool value)
```

Synchronously updates the retained show on focus property. Validation, typed invalidation, and notifications are defined by the implementation.

### `show_always`

```cpp
[[nodiscard]] bool show_always() const noexcept
```

Reports the current show always value without mutation.

### `set_show_always`

```cpp
void set_show_always(bool value)
```

Synchronously updates the retained show always property. Validation, typed invalidation, and notifications are defined by the implementation.

### `show`

```cpp
void show(const std::shared_ptr<Control>& target)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `show`

```cpp
void show(const std::shared_ptr<Control>& target, std::chrono::milliseconds duration)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `hide`

```cpp
void hide()
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `visible`

```cpp
[[nodiscard]] bool visible() const noexcept
```

Reports the current visible value without mutation.

### `active_control`

```cpp
[[nodiscard]] std::shared_ptr<Control> active_control() const noexcept
```

Reports the current active control value without mutation.

### `visibility_changed`

```cpp
[[nodiscard]] Event<const ToolTipEvent&>& visibility_changed() noexcept
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
