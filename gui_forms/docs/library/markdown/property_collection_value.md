# PropertyCollectionValue

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `PropertyCollectionValue`  
Declaration: `include/gui_forms/binding_types.hpp:73`  
Definition: `src/core/binding.cpp`

PropertyCollectionValue is a class declared in include/gui_forms/binding_types.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `PropertyCollectionValue`

```cpp
PropertyCollectionValue() = default
```

Constructs or tears down the retained PropertyCollectionValue object according to its ownership contract.

### `operatorbool`

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports the current operatorbool value without mutation.

### `item_type_name`

```cpp
[[nodiscard]] std::string_view item_type_name() const noexcept
```

Reports the current item type name value without mutation.

### `item_kind`

```cpp
[[nodiscard]] BindingValueKind item_kind() const noexcept
```

Reports the current item kind value without mutation.

### `data`

```cpp
[[nodiscard]] const PropertyCollectionData* data() const noexcept
```

Reports the current data value without mutation.

### `operator==`

```cpp
friend bool operator==(const PropertyCollectionValue& left, const PropertyCollectionValue& right) noexcept
```

Public PropertyCollectionValue operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
