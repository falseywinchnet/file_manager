# PropertyCollectionValue

- Status: **OBSERVED: bundle 010 property collection handle review; focused M4 binding tests pass**
- Kind: **class**
- Hierarchy: `PropertyCollectionValue`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:73`
- Definition: `src/core/binding/value/binding_value.cpp`

PropertyCollectionValue is a cheap immutable shared handle over a validated bounded homogeneous property sequence.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

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
