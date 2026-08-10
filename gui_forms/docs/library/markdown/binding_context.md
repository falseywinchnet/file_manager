# BindingContext

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → BindingContext`  
Declaration: `include/gui_forms/binding.hpp:448`  
Definition: `src/core/binding.cpp`

BindingContext is a class declared in include/gui_forms/binding.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `BindingContext`

```cpp
explicit BindingContext(Window& window)
```

Constructs or tears down the retained BindingContext object according to its ownership contract.

### `~BindingContext`

```cpp
~BindingContext() override
```

Constructs or tears down the retained BindingContext object according to its ownership contract.

### `add`

```cpp
void add(const std::shared_ptr<BindingSource>& source)
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `manager`

```cpp
CurrencyManager& manager(const std::shared_ptr<BindingSource>& source)
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `contains`

```cpp
[[nodiscard]] bool contains(const BindingSource& source) const noexcept
```

Reports the current contains value without mutation.

### `remove`

```cpp
bool remove(const BindingSource& source)
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear`

```cpp
void clear()
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `size`

```cpp
[[nodiscard]] std::size_t size() const noexcept
```

Reports the current size value without mutation.

### `collection_changed`

```cpp
[[nodiscard]] Event<const BindingContextChange&>& collection_changed() noexcept
```

Public BindingContext operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
