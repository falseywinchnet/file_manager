# TextSelection

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `TextSelection`  
Declaration: `include/gui_forms/input_controls.hpp:19`  
Definition: `inline/header-only`

TextSelection is a struct declared in include/gui_forms/input_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `start`

```cpp
[[nodiscard]] Utf8Offset start() const noexcept
```

Reports the current start value without mutation.

### `end`

```cpp
[[nodiscard]] Utf8Offset end() const noexcept
```

Reports the current end value without mutation.

### `length`

```cpp
[[nodiscard]] std::size_t length() const noexcept
```

Reports the current length value without mutation.

### `empty`

```cpp
[[nodiscard]] bool empty() const noexcept
```

Reports the current empty value without mutation.

### `operator==`

```cpp
friend constexpr bool operator==(const TextSelection&, const TextSelection&) = default
```

Public TextSelection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
