# CommandBinding

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `CommandBinding`  
Declaration: `include/gui_forms/commands.hpp:84`  
Definition: `src/controls/commands.cpp`

CommandBinding is a class declared in include/gui_forms/commands.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `CommandBinding`

```cpp
CommandBinding(std::shared_ptr<Command> command, std::shared_ptr<ButtonBase> button, CommandBindingOptions options =
```

Constructs or tears down the retained CommandBinding object according to its ownership contract.

### `~CommandBinding`

```cpp
~CommandBinding() = default
```

Constructs or tears down the retained CommandBinding object according to its ownership contract.

### `CommandBinding`

```cpp
CommandBinding(CommandBinding&&) noexcept = default
```

Constructs or tears down the retained CommandBinding object according to its ownership contract.

### `operator=`

```cpp
CommandBinding& operator=(CommandBinding&&) noexcept = default
```

Public CommandBinding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `CommandBinding`

```cpp
CommandBinding(const CommandBinding&) = delete
```

Constructs or tears down the retained CommandBinding object according to its ownership contract.

### `operator=`

```cpp
CommandBinding& operator=(const CommandBinding&) = delete
```

Public CommandBinding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `command`

```cpp
[[nodiscard]] const std::shared_ptr<Command>& command() const noexcept
```

Reports the current command value without mutation.

### `button`

```cpp
[[nodiscard]] const std::shared_ptr<ButtonBase>& button() const noexcept
```

Reports the current button value without mutation.
