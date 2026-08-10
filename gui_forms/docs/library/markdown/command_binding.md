# CommandBinding

- Status: **OBSERVED: bundle 010 command presentation split; focused M4 collection tests pass**
- Kind: **class**
- Hierarchy: `CommandBinding`
- Declaration: `include/gui_forms/commands/command_binding/command_binding.hpp:11`
- Definition: `src/controls/commands/command_binding/command_binding.cpp`

CommandBinding owns deterministic subscriptions between one shared command and one ButtonBase while honoring independent synchronization options.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `CommandBinding` (public)

```cpp
CommandBinding(std::shared_ptr<Command> command, std::shared_ptr<ButtonBase> button, CommandBindingOptions options =
```

Validates both shared owners, applies initial state, routes clicks to execute, and observes future state; copying is prohibited and moves transfer subscriptions.

### `~CommandBinding` (public)

```cpp
~CommandBinding() = default
```

Disconnects through owned subscription-token destruction.

### `CommandBinding` (public)

```cpp
CommandBinding(CommandBinding&&) noexcept = default
```

Validates both shared owners, applies initial state, routes clicks to execute, and observes future state; copying is prohibited and moves transfer subscriptions.

### `operator=` (public)

```cpp
CommandBinding& operator=(CommandBinding&&) noexcept = default
```

Move assignment transfers authority while copy assignment is prohibited.

### `CommandBinding` (public)

```cpp
CommandBinding(const CommandBinding&) = delete
```

Validates both shared owners, applies initial state, routes clicks to execute, and observes future state; copying is prohibited and moves transfer subscriptions.

### `operator=` (public)

```cpp
CommandBinding& operator=(const CommandBinding&) = delete
```

Move assignment transfers authority while copy assignment is prohibited.

### `command` (public)

```cpp
[[nodiscard]] const std::shared_ptr<Command>& command() const noexcept
```

Returns the retained command owner.

### `button` (public)

```cpp
[[nodiscard]] const std::shared_ptr<ButtonBase>& button() const noexcept
```

Returns the retained presentation owner.

### `apply` (private)

```cpp
void apply(const CommandState& state)
```

Applies only the enabled synchronization axes to the button.
