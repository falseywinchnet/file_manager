# AbiPropertyObjectControl

- Status: **OBSERVED: bundle 010 private ABI property-proxy split; native and MinGW ABI builds pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → AbiPropertyObjectControl`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:895`
- Definition: `inline/header-only`

AbiPropertyObjectControl is a nonvisual retained proxy that exposes explicitly registered foreign properties to native inspection without retaining managed runtime objects or executable callbacks in descriptors.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `AbiPropertyObjectControl` (public)

```cpp
explicit AbiPropertyObjectControl(StableId stable_id) : Control(std::move(stable_id))
```

Constructs a stable nonvisual proxy.

### `define` (public)

```cpp
void define(const gf_property_descriptor_v1& authored, const gf_property_callbacks_v1& callbacks)
```

Copies/validates descriptor and callback table, installs a PropertyRegistration, and retains bounded callback state.

### `notify_changed` (public)

```cpp
void notify_changed(std::string_view name)
```

Publishes an explicitly named foreign property change.

### `install_converters` (public)

```cpp
void install_converters(gui_forms::PropertyValueConverterRegistry& target)
```

Registers named formatter/parser services backed by the retained callback state.

### `install_editors` (public)

```cpp
void install_editors(gui_forms::PropertyEditorRegistry& target, std::set<std::string>& installed)
```

Registers named editor services backed by the retained callback state.

### `copy_text` (private)

```cpp
static std::string copy_text(gf_string_view view, std::size_t maximum, std::string_view field)
```

Validates ABI string views and copies UTF-8 with a field-specific diagnostic.

### `native_kind` (private)

```cpp
static gui_forms::BindingValueKind native_kind(std::uint32_t kind)
```

Maps closed ABI value kind to native BindingValueKind.

### `abi_kind` (private)

```cpp
static std::uint32_t abi_kind(gui_forms::BindingValueKind kind)
```

Maps native BindingValueKind to closed ABI kind.

### `to_abi` (private)

```cpp
static gf_property_value to_abi(const gui_forms::BindingValue& value)
```

Projects a native scalar value into an ABI record without leaking ownership.

### `from_abi` (private)

```cpp
static gui_forms::BindingValue from_abi( const gf_property_value& value, std::string text, const gui_forms::PropertyDescriptor& descriptor)
```

Validates and converts an ABI value/text pair through declared descriptor schema.

### `callback_text` (private)

```cpp
static std::string callback_text( const std::function<std::uint32_t(char*, std::uint64_t, std::uint64_t*)>& invoke, std::string_view operation)
```

Performs size-query then bounded fill for callback-owned UTF-8 output.

### `copy_descriptor` (private)

```cpp
static gui_forms::PropertyDescriptor copy_descriptor( const gf_property_descriptor_v1& authored)
```

Copies and validates the complete inert ABI descriptor and standard values.
