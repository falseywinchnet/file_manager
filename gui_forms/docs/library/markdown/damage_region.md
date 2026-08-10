# DamageRegion

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `DamageRegion`
- Declaration: `include/gui_forms/types.hpp:239`
- Definition: `src/core/types.cpp`

DamageRegion is a class declared in include/gui_forms/types.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `add` (public)

```cpp
void add(Rect rect)
```

Public DamageRegion operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear` (public)

```cpp
void clear() noexcept
```

Public DamageRegion operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `empty` (public)

```cpp
[[nodiscard]] bool empty() const noexcept
```

Reports the current empty value without mutation.

### `rectangles` (public)

```cpp
[[nodiscard]] std::span<const Rect> rectangles() const noexcept
```

Reports the current rectangles value without mutation.

### `rectangle_count` (public)

```cpp
[[nodiscard]] std::size_t rectangle_count() const noexcept
```

Reports the current rectangle count value without mutation.

### `compaction_count` (public)

```cpp
[[nodiscard]] std::uint64_t compaction_count() const noexcept
```

Reports the current compaction count value without mutation.

### `collapse_count` (public)

```cpp
[[nodiscard]] std::uint64_t collapse_count() const noexcept
```

Reports the current collapse count value without mutation.

### `bounds` (public)

```cpp
[[nodiscard]] Rect bounds() const noexcept
```

Reports the current bounds value without mutation.

### `area` (public)

```cpp
[[nodiscard]] double area() const noexcept
```

Reports the current area value without mutation.
