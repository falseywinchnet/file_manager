# BindingContext

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `Component → BindingContext`
- Declaration: `include/gui_forms/binding/binding_context/binding_context.hpp:18`
- Definition: `src/core/binding/binding_context/binding_context.cpp`

BindingContext is a class declared in include/gui_forms/binding/binding_context/binding_context.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `BindingContext` (public)

```cpp
explicit BindingContext(Window& window)
```

Constructs or tears down the retained BindingContext object according to its ownership contract.

### `~BindingContext` (public)

```cpp
~BindingContext() override
```

Constructs or tears down the retained BindingContext object according to its ownership contract.

### `add` (public)

```cpp
void add(const std::shared_ptr<BindingSource>& source)
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `manager` (public)

```cpp
CurrencyManager& manager(const std::shared_ptr<BindingSource>& source)
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `contains` (public)

```cpp
[[nodiscard]] bool contains(const BindingSource& source) const noexcept
```

Reports the current contains value without mutation.

### `remove` (public)

```cpp
bool remove(const BindingSource& source)
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear` (public)

```cpp
void clear()
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `size` (public)

```cpp
[[nodiscard]] std::size_t size() const noexcept
```

Reports the current size value without mutation.

### `collection_changed` (public)

```cpp
[[nodiscard]] Event<const BindingContextChange&>& collection_changed() noexcept
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `bound_window` (private)

```cpp
[[nodiscard]] Window* bound_window() const noexcept
```

Reports the current bound window value without mutation.

### `remove_entry` (private)

```cpp
bool remove_entry(BindingSource* source, bool publish)
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
