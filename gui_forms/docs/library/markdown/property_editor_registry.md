# PropertyEditorRegistry

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `PropertyEditorRegistry`  
Declaration: `include/gui_forms/inspection_controls.hpp:223`  
Definition: `src/controls/inspection_controls.cpp`

PropertyEditorRegistry is a class declared in include/gui_forms/inspection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `register_factory`

```cpp
bool register_factory(std::string name, PropertyEditorFactory factory)
```

Public PropertyEditorRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `unregister_factory`

```cpp
bool unregister_factory(std::string_view name)
```

Public PropertyEditorRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `map_kind`

```cpp
void map_kind(BindingValueKind kind, std::string factory_name)
```

Public PropertyEditorRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear_kind`

```cpp
void clear_kind(BindingValueKind kind)
```

Removes the explicit kind value and restores fallback behavior.

### `factory_for`

```cpp
[[nodiscard]] std::optional<std::string> factory_for( BindingValueKind kind) const
```

Reports the current factory for value without mutation.

### `create`

```cpp
[[nodiscard]] std::optional<PropertyEditorBinding> create( const PropertyEditorRequest& request) const
```

Reports the current create value without mutation.

### `create_default`

```cpp
[[nodiscard]] static std::shared_ptr<PropertyEditorRegistry> create_default()
```

Public PropertyEditorRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
