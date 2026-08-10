# Component

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component`  
Declaration: `include/gui_forms/component.hpp:26`  
Definition: `src/core/component.cpp`

Component is a class declared in include/gui_forms/component.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Component`

```cpp
Component() = default
```

Constructs or tears down the retained Component object according to its ownership contract.

### `~Component`

```cpp
virtual ~Component()
```

Constructs or tears down the retained Component object according to its ownership contract.

### `Component`

```cpp
Component(const Component&) = delete
```

Constructs or tears down the retained Component object according to its ownership contract.

### `operator=`

```cpp
Component& operator=(const Component&) = delete
```

Public Component operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `dispose`

```cpp
void dispose()
```

Public Component operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `component_state`

```cpp
[[nodiscard]] ComponentState component_state() const noexcept
```

Reports the current component state value without mutation.

### `is_alive`

```cpp
[[nodiscard]] bool is_alive() const noexcept
```

Reports the current is alive value without mutation.

### `is_disposed`

```cpp
[[nodiscard]] bool is_disposed() const noexcept
```

Reports the current is disposed value without mutation.

### `own_revocable`

```cpp
void own_revocable(const std::weak_ptr<detail::Revocable>& revocable)
```

Public Component operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
