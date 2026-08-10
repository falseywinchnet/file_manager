# PropertyState

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `PropertyState`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:1393`
- Definition: `inline/header-only`

PropertyState is a struct declared in src/abi/control_adapters/abi_control_adapters.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `get` (public)

```cpp
[[nodiscard]] gui_forms::BindingValue get() const
```

Reports the current get value without mutation.

### `set` (public)

```cpp
void set(const gui_forms::BindingValue& value) const
```

Reports the current set value without mutation.

### `reset` (public)

```cpp
void reset() const
```

Reports the current reset value without mutation.

### `should_serialize` (public)

```cpp
[[nodiscard]] bool should_serialize() const
```

Reports the current should serialize value without mutation.

### `format` (public)

```cpp
[[nodiscard]] std::string format( const gui_forms::BindingValue& value) const
```

Reports the current format value without mutation.

### `parse` (public)

```cpp
[[nodiscard]] std::optional<gui_forms::BindingValue> parse( std::string_view text) const
```

Reports the current parse value without mutation.

### `edit` (public)

```cpp
[[nodiscard]] gui_forms::BindingValue edit( const gui_forms::BindingValue& current) const
```

Reports the current edit value without mutation.
