# PropertyObjectValue

- Status: **OBSERVED: bundle 012 isolated property-object handle; native and MinGW M4 builds pass**
- Kind: **class**
- Hierarchy: `PropertyObjectValue`
- Declaration: `include/gui_forms/binding/value/property_object_value/property_object_value.hpp:15`
- Definition: `src/core/binding/value/property_object_value/property_object_value.cpp`

PropertyObjectValue is a cheap immutable shared handle over a validated recursive property object tree.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `PropertyObjectValue` (public)

```cpp
PropertyObjectValue() = default
```

Default construction is empty; private factory construction admits validated shared data.

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports whether immutable data is present.

### `type_name` (public)

```cpp
[[nodiscard]] std::string_view type_name() const noexcept
```

Returns the retained type name or an empty view.

### `members` (public)

```cpp
[[nodiscard]] std::span<const PropertyObjectMember> members() const noexcept
```

Returns retained members or an empty span.

### `data` (public)

```cpp
[[nodiscard]] const PropertyObjectData* data() const noexcept
```

Returns the immutable backing record pointer.

### `operator==` (public)

```cpp
friend bool operator==(const PropertyObjectValue& left, const PropertyObjectValue& right) noexcept
```

Compares structural property data rather than shared-pointer identity.

### `PropertyObjectValue` (private)

```cpp
explicit PropertyObjectValue( std::shared_ptr<const PropertyObjectData> authored_data) : data_(std::move(authored_data))
```

Default construction is empty; private factory construction admits validated shared data.
