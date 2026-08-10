# BindingManagerBase

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `BindingManagerBase`  
Declaration: `include/gui_forms/binding.hpp:131`  
Definition: `inline/header-only`

BindingManagerBase is a class declared in include/gui_forms/binding.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `~BindingManagerBase`

```cpp
virtual ~BindingManagerBase() = default
```

Constructs or tears down the retained BindingManagerBase object according to its ownership contract.

### `count`

```cpp
[[nodiscard]] virtual std::size_t count() const noexcept = 0
```

Reports the current count value without mutation.

### `current`

```cpp
[[nodiscard]] virtual const BindingRecord* current() const noexcept = 0
```

Reports the current current value without mutation.

### `position`

```cpp
[[nodiscard]] virtual std::ptrdiff_t position() const noexcept = 0
```

Reports the current position value without mutation.

### `binding_suspended`

```cpp
[[nodiscard]] virtual bool binding_suspended() const noexcept = 0
```

Reports the current binding suspended value without mutation.

### `set_position`

```cpp
virtual bool set_position(std::ptrdiff_t position) = 0
```

Synchronously updates the retained position property. Validation, typed invalidation, and notifications are defined by the implementation.

### `cancel_current_edit`

```cpp
virtual void cancel_current_edit() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_current_edit`

```cpp
virtual void end_current_edit() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_at`

```cpp
virtual bool remove_at(std::size_t index) = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `suspend_binding`

```cpp
virtual void suspend_binding() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resume_binding`

```cpp
virtual void resume_binding() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pull_data`

```cpp
virtual bool pull_data() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `push_data`

```cpp
virtual bool push_data() = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `binding_complete`

```cpp
[[nodiscard]] virtual Event<BindingCompleteEvent&>& binding_complete() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_changed`

```cpp
[[nodiscard]] virtual Event<>& current_changed() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_item_changed`

```cpp
[[nodiscard]] virtual Event<>& current_item_changed() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `position_changed`

```cpp
[[nodiscard]] virtual Event<std::ptrdiff_t>& position_changed() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_error`

```cpp
[[nodiscard]] virtual Event<const std::string&>& data_error() noexcept = 0
```

Public BindingManagerBase operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
