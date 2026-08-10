# Command

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → Command`  
Declaration: `include/gui_forms/commands.hpp:39`  
Definition: `src/controls/commands.cpp`

Command is a class declared in include/gui_forms/commands.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Command`

```cpp
Command(std::string stable_id, std::string text)
```

Constructs or tears down the retained Command object according to its ownership contract.

### `stable_id`

```cpp
[[nodiscard]] const std::string& stable_id() const noexcept
```

Reports the current stable id value without mutation.

### `state`

```cpp
[[nodiscard]] const CommandState& state() const noexcept
```

Reports the current state value without mutation.

### `set_text`

```cpp
void set_text(std::string text)
```

Synchronously updates the retained text property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_description`

```cpp
void set_description(std::string description)
```

Synchronously updates the retained description property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_icon_id`

```cpp
void set_icon_id(std::string icon_id)
```

Synchronously updates the retained icon id property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_shortcut`

```cpp
void set_shortcut(std::string shortcut)
```

Synchronously updates the retained shortcut property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_mnemonic`

```cpp
void set_mnemonic(std::string mnemonic)
```

Synchronously updates the retained mnemonic property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_key_tip`

```cpp
void set_key_tip(std::string key_tip)
```

Synchronously updates the retained key tip property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_availability_reason`

```cpp
void set_availability_reason(std::string reason)
```

Synchronously updates the retained availability reason property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_enabled`

```cpp
void set_enabled(bool enabled)
```

Synchronously updates the retained enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_visible`

```cpp
void set_visible(bool visible)
```

Synchronously updates the retained visible property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_checked`

```cpp
void set_checked(bool checked)
```

Synchronously updates the retained checked property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_default_action`

```cpp
void set_default_action(bool is_default)
```

Synchronously updates the retained default action property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_destructive`

```cpp
void set_destructive(bool destructive)
```

Synchronously updates the retained destructive property. Validation, typed invalidation, and notifications are defined by the implementation.

### `execute`

```cpp
bool execute(std::string_view source_id =
```

Public Command operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `state_changed`

```cpp
[[nodiscard]] Event<const CommandState&>& state_changed() noexcept
```

Public Command operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invoked`

```cpp
[[nodiscard]] Event<const CommandInvocation&>& invoked() noexcept
```

Public Command operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
