# TextSelection

- Status: **OBSERVED: bundle 004 split and Unicode edit-policy review; M4 build, focused tests, and Screen Sharing pass**
- Kind: **struct**
- Hierarchy: `TextSelection`
- Declaration: `include/gui_forms/controls/panel/text_box/text_box.hpp:17`
- Definition: `inline/header-only`

TextSelection is TextBox's normalized byte-boundary selection value. It keeps anchor and active endpoints distinct while exposing ordered ranges for mutation and painting.

## Visual evidence

![TextSelection](../captures/text_box.png)

## Declared methods

### `start` (public)

```cpp
[[nodiscard]] Utf8Offset start() const noexcept
```

Returns the lesser UTF-8 byte boundary regardless of selection direction.

### `end` (public)

```cpp
[[nodiscard]] Utf8Offset end() const noexcept
```

Returns the greater UTF-8 byte boundary regardless of selection direction.

### `length` (public)

```cpp
[[nodiscard]] std::size_t length() const noexcept
```

Returns the selected byte count between normalized endpoints.

### `empty` (public)

```cpp
[[nodiscard]] bool empty() const noexcept
```

Reports whether anchor and active endpoints identify the same insertion position.

### `operator==` (public)

```cpp
friend constexpr bool operator==(const TextSelection&, const TextSelection&) = default
```

Compares both directional endpoints exactly.
