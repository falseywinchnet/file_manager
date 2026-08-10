# PropertyState

- Status: **OBSERVED: bundle 010 private ABI foreign-property state review; native and MinGW ABI builds pass**
- Kind: **struct**
- Hierarchy: `PropertyState`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:1393`
- Definition: `inline/header-only`

PropertyState owns one copied inert descriptor and bounded caller callbacks for native property access/conversion/editing.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `get` (public)

```cpp
[[nodiscard]] gui_forms::BindingValue get() const
```

Calls the foreign getter and converts its ABI value/text into declared native schema.

### `set` (public)

```cpp
void set(const gui_forms::BindingValue& value) const
```

Converts native value to ABI form and invokes the foreign setter.

### `reset` (public)

```cpp
void reset() const
```

Invokes the optional foreign reset callback.

### `should_serialize` (public)

```cpp
[[nodiscard]] bool should_serialize() const
```

Invokes optional authored serialization policy or derives it from descriptor defaults.

### `format` (public)

```cpp
[[nodiscard]] std::string format( const gui_forms::BindingValue& value) const
```

Invokes optional foreign formatter and validates returned schema.

### `parse` (public)

```cpp
[[nodiscard]] std::optional<gui_forms::BindingValue> parse( std::string_view text) const
```

Invokes optional foreign parser and treats declared callback rejection as no conversion.

### `edit` (public)

```cpp
[[nodiscard]] gui_forms::BindingValue edit( const gui_forms::BindingValue& current) const
```

Invokes optional foreign editor and validates the returned value.
