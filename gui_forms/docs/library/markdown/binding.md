# Binding

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `Component → enable_shared_from_this → Binding`
- Declaration: `include/gui_forms/binding/binding/binding.hpp:16`
- Definition: `src/core/binding/binding/binding.cpp`

Binding is a class declared in include/gui_forms/binding/binding/binding.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `Binding` (public)

```cpp
Binding(Control& target, std::string property_name, std::shared_ptr<BindingSource> source, std::string data_member, BindingOptions options =
```

Constructs or tears down the retained Binding object according to its ownership contract.

### `~Binding` (public)

```cpp
~Binding() override
```

Constructs or tears down the retained Binding object according to its ownership contract.

### `target` (public)

```cpp
[[nodiscard]] Control* target() const noexcept
```

Reports the current target value without mutation.

### `source` (public)

```cpp
[[nodiscard]] std::shared_ptr<BindingSource> source() const noexcept
```

Reports the current source value without mutation.

### `property_name` (public)

```cpp
[[nodiscard]] const std::string& property_name() const noexcept
```

Reports the current property name value without mutation.

### `data_member` (public)

```cpp
[[nodiscard]] const std::string& data_member() const noexcept
```

Reports the current data member value without mutation.

### `options` (public)

```cpp
[[nodiscard]] const BindingOptions& options() const noexcept
```

Reports the current options value without mutation.

### `set_options` (public)

```cpp
void set_options(BindingOptions options)
```

Synchronously updates the retained options property. Validation, typed invalidation, and notifications are defined by the implementation.

### `active` (public)

```cpp
[[nodiscard]] bool active() const noexcept
```

Reports the current active value without mutation.

### `read_value` (public)

```cpp
bool read_value()
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `write_value` (public)

```cpp
bool write_value()
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validate` (public)

```cpp
bool validate()
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `snapshot` (public)

```cpp
[[nodiscard]] BindingSnapshot snapshot() const
```

Reports the current snapshot value without mutation.

### `format` (public)

```cpp
[[nodiscard]] Event<BindingConvertEvent&>& format() noexcept
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `parse` (public)

```cpp
[[nodiscard]] Event<BindingConvertEvent&>& parse() noexcept
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `binding_complete` (public)

```cpp
[[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `start` (private)

```cpp
void start()
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `update_control` (private)

```cpp
bool update_control(bool automatic)
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `update_source` (private)

```cpp
bool update_source(bool automatic)
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `complete` (private)

```cpp
bool complete(BindingCompleteContext context, BindingCompleteState state, std::string error =
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `source_changed` (private)

```cpp
void source_changed(const BindingListChange& change)
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `target_changed` (private)

```cpp
void target_changed()
```

Public Binding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
