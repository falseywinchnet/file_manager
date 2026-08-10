# AbiPropertyObjectControl

- Status: **generated inventory; detailed review pending**
- Kind: **class / visual retained control**
- Hierarchy: `Control → AbiPropertyObjectControl`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:895`
- Definition: `inline/header-only`

AbiPropertyObjectControl is a visual retained control declared in src/abi/control_adapters/abi_control_adapters.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `AbiPropertyObjectControl` (public)

```cpp
explicit AbiPropertyObjectControl(StableId stable_id) : Control(std::move(stable_id))
```

Constructs or tears down the retained AbiPropertyObjectControl object according to its ownership contract.

### `define` (public)

```cpp
void define(const gf_property_descriptor_v1& authored, const gf_property_callbacks_v1& callbacks)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `notify_changed` (public)

```cpp
void notify_changed(std::string_view name)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `install_converters` (public)

```cpp
void install_converters(gui_forms::PropertyValueConverterRegistry& target)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `install_editors` (public)

```cpp
void install_editors(gui_forms::PropertyEditorRegistry& target, std::set<std::string>& installed)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `copy_text` (private)

```cpp
static std::string copy_text(gf_string_view view, std::size_t maximum, std::string_view field)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `native_kind` (private)

```cpp
static gui_forms::BindingValueKind native_kind(std::uint32_t kind)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `abi_kind` (private)

```cpp
static std::uint32_t abi_kind(gui_forms::BindingValueKind kind)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `to_abi` (private)

```cpp
static gf_property_value to_abi(const gui_forms::BindingValue& value)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `from_abi` (private)

```cpp
static gui_forms::BindingValue from_abi( const gf_property_value& value, std::string text, const gui_forms::PropertyDescriptor& descriptor)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `callback_text` (private)

```cpp
static std::string callback_text( const std::function<std::uint32_t(char*, std::uint64_t, std::uint64_t*)>& invoke, std::string_view operation)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `copy_descriptor` (private)

```cpp
static gui_forms::PropertyDescriptor copy_descriptor( const gf_property_descriptor_v1& authored)
```

Public AbiPropertyObjectControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
