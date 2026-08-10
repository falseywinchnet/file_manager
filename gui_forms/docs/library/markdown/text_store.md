# TextStore

- Status: **OBSERVED: bundle 009 Unicode text-state split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `TextStore`
- Declaration: `include/gui_forms/text/text_store/text_store.hpp:14`
- Definition: `src/core/text/text_store/text_store.cpp`

TextStore is renderer-neutral contiguous UTF-8 storage with typed UTF-8/UTF-16/scalar/grapheme/line positions, bounded edits and style spans, Unicode grapheme metadata, exact revisions, and rejection telemetry.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `TextStore` (public)

```cpp
explicit TextStore(std::string_view text =
```

Validates limits and initial UTF-8, builds scalar, UTF-16, grapheme, and line metadata once, and starts an owned text state.

### `utf8` (public)

```cpp
[[nodiscard]] std::string_view utf8() const noexcept
```

Returns the current contiguous UTF-8 view until the next mutation.

### `utf8_size` (public)

```cpp
[[nodiscard]] Utf8Offset utf8_size() const noexcept
```

Returns byte length as a typed Utf8Offset.

### `utf16_size` (public)

```cpp
[[nodiscard]] Utf16Offset utf16_size() const noexcept
```

Returns the cached UTF-16 code-unit length.

### `scalar_count` (public)

```cpp
[[nodiscard]] ScalarIndex scalar_count() const noexcept
```

Returns cached Unicode scalar count.

### `grapheme_count` (public)

```cpp
[[nodiscard]] GraphemeIndex grapheme_count() const noexcept
```

Returns cached extended-grapheme count from boundary metadata.

### `line_count` (public)

```cpp
[[nodiscard]] std::size_t line_count() const noexcept
```

Returns the number of retained logical lines.

### `revision` (public)

```cpp
[[nodiscard]] std::uint64_t revision() const noexcept
```

Returns the monotonic committed text/style revision.

### `limits` (public)

```cpp
[[nodiscard]] const TextStoreLimits& limits() const noexcept
```

Returns immutable byte and style-span limits.

### `style_spans` (public)

```cpp
[[nodiscard]] std::span<const TextStyleSpan> style_spans() const noexcept
```

Returns normalized nonoverlapping retained style spans.

### `set_text` (public)

```cpp
void set_text(std::string_view text)
```

Replaces the full byte range through the authoritative edit transaction.

### `replace` (public)

```cpp
[[nodiscard]] TextEditResult replace( Utf8Range range, std::string_view replacement, std::optional<TextStyleId> inserted_style = std::nullopt)
```

Validates typed range and replacement UTF-8, constructs candidate text and metadata, transforms styles, then commits all state and counters atomically.

### `set_style_spans` (public)

```cpp
void set_style_spans(std::span<const TextStyleSpan> spans)
```

Normalizes and validates a complete candidate style collection before committing it.

### `style_at` (public)

```cpp
[[nodiscard]] std::optional<TextStyleId> style_at( Utf8Offset position) const
```

Validates the byte position and returns the containing style identity when present.

### `is_scalar_boundary` (public)

```cpp
[[nodiscard]] bool is_scalar_boundary(Utf8Offset position) const noexcept
```

Tests cached scalar boundaries without throwing for out-of-range input.

### `is_grapheme_boundary` (public)

```cpp
[[nodiscard]] bool is_grapheme_boundary(Utf8Offset position) const noexcept
```

Tests cached extended-grapheme boundaries without mutating text.

### `utf8_offset` (public)

```cpp
[[nodiscard]] Utf8Offset utf8_offset(Utf16Offset position) const
```

Converts typed UTF-16, scalar, or grapheme positions to an exact UTF-8 byte boundary.

### `utf8_offset` (public)

```cpp
[[nodiscard]] Utf8Offset utf8_offset(ScalarIndex position) const
```

Converts typed UTF-16, scalar, or grapheme positions to an exact UTF-8 byte boundary.

### `utf8_offset` (public)

```cpp
[[nodiscard]] Utf8Offset utf8_offset(GraphemeIndex position) const
```

Converts typed UTF-16, scalar, or grapheme positions to an exact UTF-8 byte boundary.

### `utf16_offset` (public)

```cpp
[[nodiscard]] Utf16Offset utf16_offset(Utf8Offset position) const
```

Converts a validated UTF-8 scalar boundary to UTF-16 units.

### `scalar_index` (public)

```cpp
[[nodiscard]] ScalarIndex scalar_index(Utf8Offset position) const
```

Converts a validated UTF-8 boundary to scalar index.

### `grapheme_index` (public)

```cpp
[[nodiscard]] GraphemeIndex grapheme_index(Utf8Offset position) const
```

Converts a validated grapheme boundary to grapheme index.

### `next_scalar_boundary` (public)

```cpp
[[nodiscard]] Utf8Offset next_scalar_boundary(Utf8Offset position) const
```

Returns the next scalar byte boundary after a validated position.

### `previous_scalar_boundary` (public)

```cpp
[[nodiscard]] Utf8Offset previous_scalar_boundary(Utf8Offset position) const
```

Returns the preceding scalar byte boundary.

### `next_grapheme_boundary` (public)

```cpp
[[nodiscard]] Utf8Offset next_grapheme_boundary(Utf8Offset position) const
```

Returns the next extended-grapheme byte boundary.

### `previous_grapheme_boundary` (public)

```cpp
[[nodiscard]] Utf8Offset previous_grapheme_boundary( Utf8Offset position) const
```

Returns the preceding extended-grapheme byte boundary.

### `grapheme_range` (public)

```cpp
[[nodiscard]] Utf8Range grapheme_range(GraphemeIndex grapheme) const
```

Returns the exact UTF-8 byte range of one grapheme cluster.

### `scalar_at` (public)

```cpp
[[nodiscard]] char32_t scalar_at(Utf8Offset position) const
```

Decodes the scalar beginning at a validated scalar boundary.

### `line_start` (public)

```cpp
[[nodiscard]] Utf8Offset line_start(LineIndex line) const
```

Returns the cached UTF-8 start of one retained line.

### `line_content_range` (public)

```cpp
[[nodiscard]] Utf8Range line_content_range(LineIndex line) const
```

Returns line content excluding retained line-break scalars.

### `snapshot` (public)

```cpp
[[nodiscard]] TextStoreSnapshot snapshot() const noexcept
```

Copies size, revision, edit/style/rebuild, and rejection telemetry.

### `analyze` (private)

```cpp
[[nodiscard]] static Metadata analyze(std::string_view text)
```

Validates candidate UTF-8 and builds all scalar, UTF-16, grapheme, and line metadata before commit.

### `normalize_style_spans` (private)

```cpp
[[nodiscard]] std::vector<TextStyleSpan> normalize_style_spans( std::span<const TextStyleSpan> spans, std::string_view candidate_text) const
```

Validates bounds/boundaries, orders spans, rejects overlap and count overflow, and drops empty spans.

### `transform_style_spans` (private)

```cpp
[[nodiscard]] std::vector<TextStyleSpan> transform_style_spans( Utf8Range removed, std::size_t inserted_bytes, std::optional<TextStyleId> inserted_style, std::string_view candidate_text) const
```

Maps retained spans across one replacement and admits the optional inserted style without splitting Unicode boundaries.

### `validate_position` (private)

```cpp
void validate_position(Utf8Offset position) const
```

Requires an in-range UTF-8 offset and counts rejected position queries.

### `validate_range` (private)

```cpp
void validate_range(Utf8Range range) const
```

Requires ordered in-range scalar boundaries for an edit.

### `reject_invalid_utf8` (private)

```cpp
[[noreturn]] void reject_invalid_utf8( const Utf8ValidationResult& validation)
```

Counts the rejected mutation and throws a precise validation error.
