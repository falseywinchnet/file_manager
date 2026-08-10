# StringFormat

- Status: **OBSERVED: bundle 009 string-format split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → StringFormat`
- Declaration: `include/gui_forms/drawing/string_format/string_format.hpp:15`
- Definition: `src/core/drawing/string_format/string_format.cpp`

StringFormat retains renderer-neutral alignment, line alignment, trimming, and compatibility flags.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `StringFormat` (public)

```cpp
StringFormat() = default
```

Constructs defaults, explicit flags, or a live snapshot copy.

### `StringFormat` (public)

```cpp
explicit StringFormat(std::uint32_t flags)
```

Constructs defaults, explicit flags, or a live snapshot copy.

### `StringFormat` (public)

```cpp
explicit StringFormat(const StringFormat& source)
```

Constructs defaults, explicit flags, or a live snapshot copy.

### `set_alignment` (public)

```cpp
void set_alignment(StringAlignment alignment)
```

Validates and commits horizontal alignment.

### `set_line_alignment` (public)

```cpp
void set_line_alignment(StringAlignment alignment)
```

Validates and commits line-block alignment.

### `set_trimming` (public)

```cpp
void set_trimming(StringTrimming trimming)
```

Validates and commits the closed trimming vocabulary.

### `set_flags` (public)

```cpp
void set_flags(std::uint32_t flags)
```

Commits compatibility flags under liveness/thread rules.

### `snapshot` (public)

```cpp
[[nodiscard]] StringFormatSnapshot snapshot() const
```

Requires liveness and returns the complete text-layout recipe.
