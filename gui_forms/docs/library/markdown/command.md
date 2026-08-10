# Command

- Status: **OBSERVED: bundle 010 command authority split; focused M4 collection tests pass**
- Kind: **class**
- Hierarchy: `Component → Command`
- Declaration: `include/gui_forms/commands/command/command.hpp:13`
- Definition: `src/controls/commands/command/command.cpp`

Command is the shared retained authority for multiple presentations, with validated UTF-8 state and ordered enabled-only invocation.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `Command` (public)

```cpp
Command(std::string stable_id, std::string text)
```

Validates stable identity and initial UTF-8 text.

### `stable_id` (public)

```cpp
[[nodiscard]] const std::string& stable_id() const noexcept
```

Returns immutable command identity.

### `state` (public)

```cpp
[[nodiscard]] const CommandState& state() const noexcept
```

Returns the current generation-stamped snapshot.

### `set_text` (public)

```cpp
void set_text(std::string text)
```

Validates and publishes changed display text.

### `set_description` (public)

```cpp
void set_description(std::string description)
```

Validates and publishes changed descriptive text.

### `set_icon_id` (public)

```cpp
void set_icon_id(std::string icon_id)
```

Validates and publishes changed icon identity.

### `set_shortcut` (public)

```cpp
void set_shortcut(std::string shortcut)
```

Validates and publishes changed shortcut text.

### `set_mnemonic` (public)

```cpp
void set_mnemonic(std::string mnemonic)
```

Validates and publishes changed mnemonic text.

### `set_key_tip` (public)

```cpp
void set_key_tip(std::string key_tip)
```

Validates and publishes changed key-tip text.

### `set_availability_reason` (public)

```cpp
void set_availability_reason(std::string reason)
```

Validates and publishes changed disabled-state rationale.

### `set_enabled` (public)

```cpp
void set_enabled(bool enabled)
```

Publishes a changed execution gate.

### `set_visible` (public)

```cpp
void set_visible(bool visible)
```

Publishes changed presentation visibility.

### `set_checked` (public)

```cpp
void set_checked(bool checked)
```

Publishes changed check state.

### `set_default_action` (public)

```cpp
void set_default_action(bool is_default)
```

Publishes changed default-action state.

### `set_destructive` (public)

```cpp
void set_destructive(bool destructive)
```

Publishes changed destructive-action metadata.

### `execute` (public)

```cpp
bool execute(std::string_view source_id =
```

Rejects disposed, disabled, or invalid-source execution and otherwise emits one sequenced invocation.

### `state_changed` (public)

```cpp
[[nodiscard]] Event<const CommandState&>& state_changed() noexcept
```

Returns the state observation event.

### `invoked` (public)

```cpp
[[nodiscard]] Event<const CommandInvocation&>& invoked() noexcept
```

Returns the admitted invocation event.

### `publish_state` (private)

```cpp
void publish_state()
```

Advances generation then emits the complete snapshot.
