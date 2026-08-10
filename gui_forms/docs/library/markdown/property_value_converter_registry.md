# PropertyValueConverterRegistry

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `PropertyValueConverterRegistry`  
Declaration: `include/gui_forms/inspection_controls.hpp:60`  
Definition: `src/controls/inspection_controls.cpp`

PropertyValueConverterRegistry is a class declared in include/gui_forms/inspection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `register_converter`

```cpp
bool register_converter(std::string name, PropertyValueConverter converter)
```

Public PropertyValueConverterRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `unregister_converter`

```cpp
bool unregister_converter(std::string_view name)
```

Public PropertyValueConverterRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `map_kind`

```cpp
void map_kind(BindingValueKind kind, std::string converter_name)
```

Public PropertyValueConverterRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear_kind`

```cpp
void clear_kind(BindingValueKind kind)
```

Removes the explicit kind value and restores fallback behavior.

### `converter_for`

```cpp
[[nodiscard]] std::optional<std::string> converter_for( BindingValueKind kind) const
```

Reports the current converter for value without mutation.

### `find`

```cpp
[[nodiscard]] const PropertyValueConverter* find( std::string_view name) const noexcept
```

Reports the current find value without mutation.

### `format`

```cpp
[[nodiscard]] std::string format(const BindingValue& value, const PropertyDescriptor& descriptor) const
```

Reports the current format value without mutation.

### `parse`

```cpp
[[nodiscard]] std::optional<BindingValue> parse( std::string_view text, const BindingValue& current, const PropertyDescriptor& descriptor) const
```

Reports the current parse value without mutation.

### `context`

```cpp
[[nodiscard]] const PropertyConversionContext& context() const noexcept
```

Reports the current context value without mutation.

### `set_context`

```cpp
void set_context(PropertyConversionContext context)
```

Synchronously updates the retained context property. Validation, typed invalidation, and notifications are defined by the implementation.

### `create_default`

```cpp
[[nodiscard]] static std::shared_ptr<PropertyValueConverterRegistry> create_default()
```

Public PropertyValueConverterRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
