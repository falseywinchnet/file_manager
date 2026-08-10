# CurrencyManager

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `BindingManagerBase → CurrencyManager`
- Declaration: `include/gui_forms/binding/currency_manager/currency_manager.hpp:11`
- Definition: `src/core/binding/currency_manager/currency_manager.cpp`

CurrencyManager is a class declared in include/gui_forms/binding/currency_manager/currency_manager.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `count` (public)

```cpp
[[nodiscard]] std::size_t count() const noexcept override
```

Reports the current count value without mutation.

### `current` (public)

```cpp
[[nodiscard]] const BindingRecord* current() const noexcept override
```

Reports the current current value without mutation.

### `position` (public)

```cpp
[[nodiscard]] std::ptrdiff_t position() const noexcept override
```

Reports the current position value without mutation.

### `binding_suspended` (public)

```cpp
[[nodiscard]] bool binding_suspended() const noexcept override
```

Reports the current binding suspended value without mutation.

### `set_position` (public)

```cpp
bool set_position(std::ptrdiff_t position) override
```

Synchronously updates the retained position property. Validation, typed invalidation, and notifications are defined by the implementation.

### `cancel_current_edit` (public)

```cpp
void cancel_current_edit() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_current_edit` (public)

```cpp
void end_current_edit() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_at` (public)

```cpp
bool remove_at(std::size_t index) override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `suspend_binding` (public)

```cpp
void suspend_binding() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resume_binding` (public)

```cpp
void resume_binding() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pull_data` (public)

```cpp
bool pull_data() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `push_data` (public)

```cpp
bool push_data() override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `binding_complete` (public)

```cpp
[[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_changed` (public)

```cpp
[[nodiscard]] Event<>& current_changed() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_item_changed` (public)

```cpp
[[nodiscard]] Event<>& current_item_changed() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `position_changed` (public)

```cpp
[[nodiscard]] Event<std::ptrdiff_t>& position_changed() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_error` (public)

```cpp
[[nodiscard]] Event<const std::string&>& data_error() noexcept override
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `list` (public)

```cpp
[[nodiscard]] std::span<const BindingRecord> list() const noexcept
```

Reports the current list value without mutation.

### `list_changed` (public)

```cpp
[[nodiscard]] Event<const BindingListChange&>& list_changed() noexcept
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `refresh` (public)

```cpp
void refresh()
```

Public CurrencyManager operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `source` (public)

```cpp
[[nodiscard]] BindingSource& source() const noexcept
```

Reports the current source value without mutation.

### `CurrencyManager` (private)

```cpp
explicit CurrencyManager(BindingSource& source) : source_(&source)
```

Constructs or tears down the retained CurrencyManager object according to its ownership contract.
