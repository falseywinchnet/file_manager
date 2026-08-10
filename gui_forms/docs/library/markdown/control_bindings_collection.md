# ControlBindingsCollection

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `ControlBindingsCollection`
- Declaration: `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:17`
- Definition: `src/core/binding/control_bindings_collection/control_bindings_collection.cpp`

ControlBindingsCollection is a class declared in include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `ControlBindingsCollection` (public)

```cpp
explicit ControlBindingsCollection(Control& target) : target_(&target)
```

Constructs or tears down the retained ControlBindingsCollection object according to its ownership contract.

### `~ControlBindingsCollection` (public)

```cpp
~ControlBindingsCollection()
```

Constructs or tears down the retained ControlBindingsCollection object according to its ownership contract.

### `ControlBindingsCollection` (public)

```cpp
ControlBindingsCollection(const ControlBindingsCollection&) = delete
```

Constructs or tears down the retained ControlBindingsCollection object according to its ownership contract.

### `operator=` (public)

```cpp
ControlBindingsCollection& operator=(const ControlBindingsCollection&) = delete
```

Public ControlBindingsCollection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add` (public)

```cpp
std::shared_ptr<Binding> add( std::string property_name, std::shared_ptr<BindingSource> source, std::string data_member)
```

Public ControlBindingsCollection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add` (public)

```cpp
std::shared_ptr<Binding> add( std::string property_name, std::shared_ptr<BindingSource> source, std::string data_member, BindingOptions options)
```

Public ControlBindingsCollection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add` (public)

```cpp
void add(std::shared_ptr<Binding> binding)
```

Public ControlBindingsCollection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove` (public)

```cpp
bool remove(const Binding& binding)
```

Public ControlBindingsCollection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear` (public)

```cpp
void clear() noexcept
```

Public ControlBindingsCollection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `find` (public)

```cpp
[[nodiscard]] std::shared_ptr<Binding> find( std::string_view property_name) const
```

Reports the current find value without mutation.

### `items` (public)

```cpp
[[nodiscard]] std::span<const std::shared_ptr<Binding>> items() const noexcept
```

Reports the current items value without mutation.

### `size` (public)

```cpp
[[nodiscard]] std::size_t size() const noexcept
```

Reports the current size value without mutation.

### `empty` (public)

```cpp
[[nodiscard]] bool empty() const noexcept
```

Reports the current empty value without mutation.

### `default_data_source_update_mode` (public)

```cpp
[[nodiscard]] DataSourceUpdateMode default_data_source_update_mode() const noexcept
```

Reports the current default data source update mode value without mutation.

### `set_default_data_source_update_mode` (public)

```cpp
void set_default_data_source_update_mode( DataSourceUpdateMode mode) noexcept
```

Synchronously updates the retained default data source update mode property. Validation, typed invalidation, and notifications are defined by the implementation.
