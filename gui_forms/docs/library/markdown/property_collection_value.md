# PropertyCollectionValue

- Status: **OBSERVED: bundle 012 isolated property-collection handle; native and MinGW M4 builds pass**
- Kind: **class**
- Hierarchy: `PropertyCollectionValue`
- Declaration: `include/gui_forms/binding/value/property_collection_value/property_collection_value.hpp:15`
- Definition: `src/core/binding/value/property_collection_value/property_collection_value.cpp`

PropertyCollectionValue is a cheap immutable shared handle over a validated bounded homogeneous property sequence.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `PropertyCollectionValue` (public)

```cpp
PropertyCollectionValue() = default
```

Default construction is empty; private factory construction admits validated shared data.

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports whether immutable data is present.

### `item_type_name` (public)

```cpp
[[nodiscard]] std::string_view item_type_name() const noexcept
```

Returns retained item type identity or an empty view.

### `item_kind` (public)

```cpp
[[nodiscard]] BindingValueKind item_kind() const noexcept
```

Returns the declared non-null item kind.

### `data` (public)

```cpp
[[nodiscard]] const PropertyCollectionData* data() const noexcept
```

Returns the immutable backing record pointer.

### `operator==` (public)

```cpp
friend bool operator==(const PropertyCollectionValue& left, const PropertyCollectionValue& right) noexcept
```

Compares structural collection data rather than shared-pointer identity.

### `PropertyCollectionValue` (private)

```cpp
explicit PropertyCollectionValue( std::shared_ptr<const PropertyCollectionData> authored_data) : data_(std::move(authored_data))
```

Default construction is empty; private factory construction admits validated shared data.
