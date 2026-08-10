# ComponentContainer

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `ComponentContainer`  
Declaration: `include/gui_forms/component.hpp:56`  
Definition: `src/core/component.cpp`

ComponentContainer is a class declared in include/gui_forms/component.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ComponentContainer`

```cpp
ComponentContainer() = default
```

Constructs or tears down the retained ComponentContainer object according to its ownership contract.

### `~ComponentContainer`

```cpp
~ComponentContainer()
```

Constructs or tears down the retained ComponentContainer object according to its ownership contract.

### `ComponentContainer`

```cpp
ComponentContainer(const ComponentContainer&) = delete
```

Constructs or tears down the retained ComponentContainer object according to its ownership contract.

### `operator=`

```cpp
ComponentContainer& operator=(const ComponentContainer&) = delete
```

Public ComponentContainer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add`

```cpp
void add(Component::Ptr component)
```

Public ComponentContainer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove`

```cpp
[[nodiscard]] Component::Ptr remove(const Component& component)
```

Public ComponentContainer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `components`

```cpp
[[nodiscard]] std::span<const Component::Ptr> components() const noexcept
```

Reports the current components value without mutation.

### `contains`

```cpp
[[nodiscard]] bool contains(const Component& component) const noexcept
```

Reports the current contains value without mutation.

### `dispose`

```cpp
void dispose()
```

Public ComponentContainer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `is_disposed`

```cpp
[[nodiscard]] bool is_disposed() const noexcept
```

Reports the current is disposed value without mutation.
