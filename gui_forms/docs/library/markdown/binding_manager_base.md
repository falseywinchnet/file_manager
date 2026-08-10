# BindingManagerBase

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `BindingManagerBase`
- Declaration: `include/gui_forms/binding/binding_manager_base/binding_manager_base.hpp:12`
- Definition: `inline/header-only`

BindingManagerBase is a class declared in include/gui_forms/binding/binding_manager_base/binding_manager_base.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `~BindingManagerBase` (public)

```cpp
virtual ~BindingManagerBase() = default
```

Constructs or tears down the retained BindingManagerBase object according to its ownership contract.

### `count` (public)

```cpp
[[nodiscard]] virtual std::size_t count() const noexcept = 0
```

Reports the current count value without mutation.

### `current` (public)

```cpp
[[nodiscard]] virtual const BindingRecord* current() const noexcept = 0
```

Reports the current current value without mutation.

### `position` (public)

```cpp
[[nodiscard]] virtual std::ptrdiff_t position() const noexcept = 0
```

Reports the current position value without mutation.

### `binding_suspended` (public)

```cpp
[[nodiscard]] virtual bool binding_suspended() const noexcept = 0
```

Reports the current binding suspended value without mutation.

### `set_position` (public)

```cpp
virtual bool set_position(std::ptrdiff_t position) = 0
```

Synchronously updates the retained position property. Validation, typed invalidation, and notifications are defined by the implementation.

### `cancel_current_edit` (public)

```cpp
virtual void cancel_current_edit() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_current_edit` (public)

```cpp
virtual void end_current_edit() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_at` (public)

```cpp
virtual bool remove_at(std::size_t index) = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `suspend_binding` (public)

```cpp
virtual void suspend_binding() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resume_binding` (public)

```cpp
virtual void resume_binding() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pull_data` (public)

```cpp
virtual bool pull_data() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `push_data` (public)

```cpp
virtual bool push_data() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `binding_complete` (public)

```cpp
[[nodiscard]] virtual Event<BindingCompleteEvent&>& binding_complete() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_changed` (public)

```cpp
[[nodiscard]] virtual Event<>& current_changed() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_item_changed` (public)

```cpp
[[nodiscard]] virtual Event<>& current_item_changed() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `position_changed` (public)

```cpp
[[nodiscard]] virtual Event<std::ptrdiff_t>& position_changed() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_error` (public)

```cpp
[[nodiscard]] virtual Event<const std::string&>& data_error() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
