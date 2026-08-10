# HelpProvider

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → HelpProvider`  
Declaration: `include/gui_forms/guidance.hpp:211`  
Definition: `src/controls/guidance.cpp`

HelpProvider is a class declared in include/gui_forms/guidance.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `HelpProvider`

```cpp
explicit HelpProvider(Window& window)
```

Constructs or tears down the retained HelpProvider object according to its ownership contract.

### `~HelpProvider`

```cpp
~HelpProvider() override
```

Constructs or tears down the retained HelpProvider object according to its ownership contract.

### `can_extend`

```cpp
[[nodiscard]] bool can_extend(const std::shared_ptr<Control>& target) const
```

Reports the current can extend value without mutation.

### `set_help_string`

```cpp
void set_help_string(const std::shared_ptr<Control>& target, std::string text)
```

Synchronously updates the retained help string property. Validation, typed invalidation, and notifications are defined by the implementation.

### `help_string`

```cpp
[[nodiscard]] std::string help_string(const Control& target) const
```

Reports the current help string value without mutation.

### `set_help_keyword`

```cpp
void set_help_keyword(const std::shared_ptr<Control>& target, std::string keyword)
```

Synchronously updates the retained help keyword property. Validation, typed invalidation, and notifications are defined by the implementation.

### `help_keyword`

```cpp
[[nodiscard]] std::string help_keyword(const Control& target) const
```

Reports the current help keyword value without mutation.

### `set_help_navigator`

```cpp
void set_help_navigator(const std::shared_ptr<Control>& target, HelpNavigator navigator)
```

Synchronously updates the retained help navigator property. Validation, typed invalidation, and notifications are defined by the implementation.

### `help_navigator`

```cpp
[[nodiscard]] HelpNavigator help_navigator(const Control& target) const
```

Reports the current help navigator value without mutation.

### `set_show_help`

```cpp
void set_show_help(const std::shared_ptr<Control>& target, bool show)
```

Synchronously updates the retained show help property. Validation, typed invalidation, and notifications are defined by the implementation.

### `show_help`

```cpp
[[nodiscard]] bool show_help(const Control& target) const
```

Reports the current show help value without mutation.

### `reset_show_help`

```cpp
void reset_show_help(const Control& target)
```

Returns show help to its inherited or default policy.

### `clear`

```cpp
void clear()
```

Public HelpProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `help_namespace`

```cpp
[[nodiscard]] const std::string& help_namespace() const noexcept
```

Reports the current help namespace value without mutation.

### `set_help_namespace`

```cpp
void set_help_namespace(std::string value)
```

Synchronously updates the retained help namespace property. Validation, typed invalidation, and notifications are defined by the implementation.

### `tag`

```cpp
[[nodiscard]] const std::any& tag() const noexcept
```

Reports the current tag value without mutation.

### `set_tag`

```cpp
void set_tag(std::any tag)
```

Synchronously updates the retained tag property. Validation, typed invalidation, and notifications are defined by the implementation.

### `request_help`

```cpp
bool request_help(const std::shared_ptr<Control>& target, Point position, bool keyboard_initiated = false)
```

Public HelpProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `help_requested`

```cpp
[[nodiscard]] Event<HelpRequestEvent&>& help_requested() noexcept
```

Public HelpProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `snapshot`

```cpp
[[nodiscard]] HelpProviderSnapshot snapshot() const noexcept
```

Reports the current snapshot value without mutation.
