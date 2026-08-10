# PropertyObjectValue

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `PropertyObjectValue`  
Declaration: `include/gui_forms/binding_types.hpp:53`  
Definition: `src/core/binding.cpp`

PropertyObjectValue is a class declared in include/gui_forms/binding_types.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `PropertyObjectValue`

```cpp
PropertyObjectValue() = default
```

Constructs or tears down the retained PropertyObjectValue object according to its ownership contract.

### `operatorbool`

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports the current operatorbool value without mutation.

### `type_name`

```cpp
[[nodiscard]] std::string_view type_name() const noexcept
```

Reports the current type name value without mutation.

### `members`

```cpp
[[nodiscard]] std::span<const PropertyObjectMember> members() const noexcept
```

Reports the current members value without mutation.

### `data`

```cpp
[[nodiscard]] const PropertyObjectData* data() const noexcept
```

Reports the current data value without mutation.

### `operator==`

```cpp
friend bool operator==(const PropertyObjectValue& left, const PropertyObjectValue& right) noexcept
```

Public PropertyObjectValue operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
