# PropertyCollectionValue

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `PropertyCollectionValue`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:73`
- Definition: `src/core/binding/value/binding_value.cpp`

PropertyCollectionValue is a class declared in include/gui_forms/binding/value/binding_value.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `PropertyCollectionValue` (public)

```cpp
PropertyCollectionValue() = default
```

Constructs or tears down the retained PropertyCollectionValue object according to its ownership contract.

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports the current operatorbool value without mutation.

### `item_type_name` (public)

```cpp
[[nodiscard]] std::string_view item_type_name() const noexcept
```

Reports the current item type name value without mutation.

### `item_kind` (public)

```cpp
[[nodiscard]] BindingValueKind item_kind() const noexcept
```

Reports the current item kind value without mutation.

### `data` (public)

```cpp
[[nodiscard]] const PropertyCollectionData* data() const noexcept
```

Reports the current data value without mutation.

### `operator==` (public)

```cpp
friend bool operator==(const PropertyCollectionValue& left, const PropertyCollectionValue& right) noexcept
```

Public PropertyCollectionValue operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `PropertyCollectionValue` (private)

```cpp
explicit PropertyCollectionValue( std::shared_ptr<const PropertyCollectionData> authored_data) : data_(std::move(authored_data))
```

Constructs or tears down the retained PropertyCollectionValue object according to its ownership contract.
