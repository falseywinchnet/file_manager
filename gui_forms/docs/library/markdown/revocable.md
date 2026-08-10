# Revocable

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Revocable`  
Declaration: `include/gui_forms/component.hpp:11`  
Definition: `inline/header-only`

Revocable is a class declared in include/gui_forms/component.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `~Revocable`

```cpp
virtual ~Revocable() = default
```

Constructs or tears down the retained Revocable object according to its ownership contract.

### `disconnect`

```cpp
virtual void disconnect() noexcept = 0
```

Public Revocable operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `connected`

```cpp
[[nodiscard]] virtual bool connected() const noexcept = 0
```

Reports the current connected value without mutation.
