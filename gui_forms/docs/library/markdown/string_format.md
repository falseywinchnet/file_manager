# StringFormat

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject → StringFormat`  
Declaration: `include/gui_forms/drawing.hpp:482`  
Definition: `src/core/drawing.cpp`

StringFormat is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `StringFormat`

```cpp
StringFormat() = default
```

Constructs or tears down the retained StringFormat object according to its ownership contract.

### `StringFormat`

```cpp
explicit StringFormat(std::uint32_t flags)
```

Constructs or tears down the retained StringFormat object according to its ownership contract.

### `StringFormat`

```cpp
explicit StringFormat(const StringFormat& source)
```

Constructs or tears down the retained StringFormat object according to its ownership contract.

### `set_alignment`

```cpp
void set_alignment(StringAlignment alignment)
```

Synchronously updates the retained alignment property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_line_alignment`

```cpp
void set_line_alignment(StringAlignment alignment)
```

Synchronously updates the retained line alignment property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_trimming`

```cpp
void set_trimming(StringTrimming trimming)
```

Synchronously updates the retained trimming property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_flags`

```cpp
void set_flags(std::uint32_t flags)
```

Synchronously updates the retained flags property. Validation, typed invalidation, and notifications are defined by the implementation.

### `snapshot`

```cpp
[[nodiscard]] StringFormatSnapshot snapshot() const
```

Reports the current snapshot value without mutation.
