# PropertyValueConverterRegistry

- Status: **OBSERVED: bundle 006 registry source split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `PropertyValueConverterRegistry`
- Declaration: `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:41`
- Definition: `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp, src/controls/panel/property_grid/property_grid.cpp`

PropertyValueConverterRegistry is an instance-owned deterministic service map from canonical names and BindingValueKind defaults to validated format/parse pairs. It bounds names and text, supports local conversion context, admits nullable/standard values, and has no process-global mutable descriptor table.

## Visual evidence

![PropertyValueConverterRegistry](../captures/property_grid.png)

## Declared methods

### `register_converter` (public)

```cpp
bool register_converter(std::string name, PropertyValueConverter converter)
```

Canonicalizes a bounded service name, requires format and parse coverage, and inserts without replacing an existing service.

### `unregister_converter` (public)

```cpp
bool unregister_converter(std::string_view name)
```

Removes a named converter and every kind mapping that referred to it.

### `map_kind` (public)

```cpp
void map_kind(BindingValueKind kind, std::string converter_name)
```

Maps a value kind to an already registered converter.

### `clear_kind` (public)

```cpp
void clear_kind(BindingValueKind kind)
```

Removes the default converter mapping for one value kind.

### `converter_for` (public)

```cpp
[[nodiscard]] std::optional<std::string> converter_for( BindingValueKind kind) const
```

Returns the canonical default converter name for a value kind.

### `find` (public)

```cpp
[[nodiscard]] const PropertyValueConverter* find( std::string_view name) const noexcept
```

Returns an immutable converter pointer for valid canonical UTF-8 names.

### `format` (public)

```cpp
[[nodiscard]] std::string format(const BindingValue& value, const PropertyDescriptor& descriptor) const
```

Selects explicit or kind-default service, formats through local context, and rejects invalid or unbounded UTF-8 output.

### `parse` (public)

```cpp
[[nodiscard]] std::optional<BindingValue> parse( std::string_view text, const BindingValue& current, const PropertyDescriptor& descriptor) const
```

Handles nullable/standard values, invokes selected parser, converts to descriptor kind, and validates the complete value tree.

### `context` (public)

```cpp
[[nodiscard]] const PropertyConversionContext& context() const noexcept
```

Returns the retained instance-local conversion context.

### `set_context` (public)

```cpp
void set_context(PropertyConversionContext context)
```

Validates distinct bounded UTF-8 separators and commits local formatting policy.

### `create_default` (public)

```cpp
[[nodiscard]] static std::shared_ptr<PropertyValueConverterRegistry> create_default()
```

Creates invariant scalar and canonical color-hex services with deterministic kind mappings.
