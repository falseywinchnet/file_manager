# PropertyObjectValue

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `PropertyObjectValue`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:53`
- Definition: `src/core/binding/value/binding_value.cpp`

PropertyObjectValue is a class declared in include/gui_forms/binding/value/binding_value.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `PropertyObjectValue` (public)

```cpp
PropertyObjectValue() = default
```

Constructs or tears down the retained PropertyObjectValue object according to its ownership contract.

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports the current operatorbool value without mutation.

### `type_name` (public)

```cpp
[[nodiscard]] std::string_view type_name() const noexcept
```

Reports the current type name value without mutation.

### `members` (public)

```cpp
[[nodiscard]] std::span<const PropertyObjectMember> members() const noexcept
```

Reports the current members value without mutation.

### `data` (public)

```cpp
[[nodiscard]] const PropertyObjectData* data() const noexcept
```

Reports the current data value without mutation.

### `operator==` (public)

```cpp
friend bool operator==(const PropertyObjectValue& left, const PropertyObjectValue& right) noexcept
```

Public PropertyObjectValue operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `PropertyObjectValue` (private)

```cpp
explicit PropertyObjectValue( std::shared_ptr<const PropertyObjectData> authored_data) : data_(std::move(authored_data))
```

Constructs or tears down the retained PropertyObjectValue object according to its ownership contract.
