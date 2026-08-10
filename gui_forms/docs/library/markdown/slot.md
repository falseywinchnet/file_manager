# Slot

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `Revocable → Slot`
- Declaration: `include/gui_forms/event.hpp:105`
- Definition: `inline/header-only`

Slot is a struct declared in include/gui_forms/event.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `Slot` (public)

```cpp
Slot(std::weak_ptr<State> event_state, Callback event_callback) : state(std::move(event_state)), callback(std::move(event_callback))
```

Constructs or tears down the retained Slot object according to its ownership contract.

### `disconnect` (public)

```cpp
void disconnect() noexcept override
```

Public Slot operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept override
```

Reports the current connected value without mutation.
