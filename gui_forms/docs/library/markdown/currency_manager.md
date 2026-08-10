# CurrencyManager

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `BindingManagerBase → CurrencyManager`  
Declaration: `include/gui_forms/binding.hpp:154`  
Definition: `src/core/binding.cpp`

CurrencyManager is a class declared in include/gui_forms/binding.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `count`

```cpp
[[nodiscard]] std::size_t count() const noexcept override
```

Reports the current count value without mutation.

### `current`

```cpp
[[nodiscard]] const BindingRecord* current() const noexcept override
```

Reports the current current value without mutation.

### `position`

```cpp
[[nodiscard]] std::ptrdiff_t position() const noexcept override
```

Reports the current position value without mutation.

### `binding_suspended`

```cpp
[[nodiscard]] bool binding_suspended() const noexcept override
```

Reports the current binding suspended value without mutation.

### `set_position`

```cpp
bool set_position(std::ptrdiff_t position) override
```

Synchronously updates the retained position property. Validation, typed invalidation, and notifications are defined by the implementation.

### `cancel_current_edit`

```cpp
void cancel_current_edit() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_current_edit`

```cpp
void end_current_edit() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_at`

```cpp
bool remove_at(std::size_t index) override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `suspend_binding`

```cpp
void suspend_binding() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resume_binding`

```cpp
void resume_binding() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pull_data`

```cpp
bool pull_data() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `push_data`

```cpp
bool push_data() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `binding_complete`

```cpp
[[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_changed`

```cpp
[[nodiscard]] Event<>& current_changed() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_item_changed`

```cpp
[[nodiscard]] Event<>& current_item_changed() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `position_changed`

```cpp
[[nodiscard]] Event<std::ptrdiff_t>& position_changed() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_error`

```cpp
[[nodiscard]] Event<const std::string&>& data_error() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `list`

```cpp
[[nodiscard]] std::span<const BindingRecord> list() const noexcept
```

Reports the current list value without mutation.

### `list_changed`

```cpp
[[nodiscard]] Event<const BindingListChange&>& list_changed() noexcept
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `refresh`

```cpp
void refresh()
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `source`

```cpp
[[nodiscard]] BindingSource& source() const noexcept
```

Reports the current source value without mutation.
