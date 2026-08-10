# PropertyEditorRegistry

- Status: **OBSERVED: bundle 006 registry source split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `PropertyEditorRegistry`
- Declaration: `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:27`
- Definition: `src/controls/inspection/property_editor_registry/property_editor_registry.cpp, src/controls/panel/property_grid/property_grid.cpp`

PropertyEditorRegistry is an instance-owned canonical service map from explicit editor names or value-kind defaults to retained-control factories. It validates returned ownership and synchronization seams, honors read-only policy, and supplies default numeric, flags, and color editors without a global mutable table.

## Visual evidence

![PropertyEditorRegistry](../captures/property_grid.png)

## Declared methods

### `register_factory` (public)

```cpp
bool register_factory(std::string name, PropertyEditorFactory factory)
```

Canonicalizes a bounded name, requires a callback, and inserts without replacing an existing service.

### `unregister_factory` (public)

```cpp
bool unregister_factory(std::string_view name)
```

Removes a factory and every value-kind mapping that referred to it.

### `map_kind` (public)

```cpp
void map_kind(BindingValueKind kind, std::string factory_name)
```

Maps a value kind to an already registered factory.

### `clear_kind` (public)

```cpp
void clear_kind(BindingValueKind kind)
```

Removes the default factory mapping for one value kind.

### `factory_for` (public)

```cpp
[[nodiscard]] std::optional<std::string> factory_for( BindingValueKind kind) const
```

Returns the canonical default factory name for a value kind.

### `create` (public)

```cpp
[[nodiscard]] std::optional<PropertyEditorBinding> create( const PropertyEditorRequest& request) const
```

Selects explicit/top-level or kind-default service and validates the returned unparented live control and synchronization callbacks.

### `create_default` (public)

```cpp
[[nodiscard]] static std::shared_ptr<PropertyEditorRegistry> create_default()
```

Creates numeric, independent-bit flags, and canonical color editor factories with safe typed subscriptions.
