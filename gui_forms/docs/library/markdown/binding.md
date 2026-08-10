# Binding

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → enable_shared_from_this → Binding`  
Declaration: `include/gui_forms/binding.hpp:338`  
Definition: `src/core/binding.cpp`

Binding is a class declared in include/gui_forms/binding.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Binding`

```cpp
Binding(Control& target, std::string property_name, std::shared_ptr<BindingSource> source, std::string data_member, BindingOptions options =
```

Constructs or tears down the retained Binding object according to its ownership contract.

### `~Binding`

```cpp
~Binding() override
```

Constructs or tears down the retained Binding object according to its ownership contract.

### `target`

```cpp
[[nodiscard]] Control* target() const noexcept
```

Reports the current target value without mutation.

### `source`

```cpp
[[nodiscard]] std::shared_ptr<BindingSource> source() const noexcept
```

Reports the current source value without mutation.

### `property_name`

```cpp
[[nodiscard]] const std::string& property_name() const noexcept
```

Reports the current property name value without mutation.

### `data_member`

```cpp
[[nodiscard]] const std::string& data_member() const noexcept
```

Reports the current data member value without mutation.

### `options`

```cpp
[[nodiscard]] const BindingOptions& options() const noexcept
```

Reports the current options value without mutation.

### `set_options`

```cpp
void set_options(BindingOptions options)
```

Synchronously updates the retained options property. Validation, typed invalidation, and notifications are defined by the implementation.

### `active`

```cpp
[[nodiscard]] bool active() const noexcept
```

Reports the current active value without mutation.

### `read_value`

```cpp
bool read_value()
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `write_value`

```cpp
bool write_value()
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validate`

```cpp
bool validate()
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `snapshot`

```cpp
[[nodiscard]] BindingSnapshot snapshot() const
```

Reports the current snapshot value without mutation.

### `format`

```cpp
[[nodiscard]] Event<BindingConvertEvent&>& format() noexcept
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `parse`

```cpp
[[nodiscard]] Event<BindingConvertEvent&>& parse() noexcept
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `binding_complete`

```cpp
[[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
