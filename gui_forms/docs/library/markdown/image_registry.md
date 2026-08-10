# ImageRegistry

- Status: **OBSERVED: bundle 009 image registry split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `ImageRegistry`
- Declaration: `include/gui_forms/resources/image_registry/image_registry.hpp:13`
- Definition: `src/core/resources/image_registry/image_registry.cpp`

ImageRegistry is the renderer-neutral generational identity store for validated PNG and premultiplied BGRA resources, with bounded per-image/global quotas, in-place live updates, rectangular patches, stale-ID rejection, and structured telemetry.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `ImageRegistry` (public)

```cpp
explicit ImageRegistry(ImageRegistryLimits limits =
```

Constructs an empty registry with caller-supplied bounded quotas; moves ownership or rejects copying.

### `~ImageRegistry` (public)

```cpp
~ImageRegistry()
```

Releases all owned encoded/raw byte vectors and slot metadata.

### `ImageRegistry` (public)

```cpp
ImageRegistry(ImageRegistry&&) noexcept
```

Constructs an empty registry with caller-supplied bounded quotas; moves ownership or rejects copying.

### `operator=` (public)

```cpp
ImageRegistry& operator=(ImageRegistry&&) noexcept
```

Moves complete registry ownership; copying is prohibited.

### `ImageRegistry` (public)

```cpp
ImageRegistry(const ImageRegistry&) = delete
```

Constructs an empty registry with caller-supplied bounded quotas; moves ownership or rejects copying.

### `operator=` (public)

```cpp
ImageRegistry& operator=(const ImageRegistry&) = delete
```

Moves complete registry ownership; copying is prohibited.

### `load_png` (public)

```cpp
[[nodiscard]] ImageLoadResult load_png(std::span<const std::byte> encoded)
```

Validates PNG structure and limits, owns encoded bytes, and allocates a generational slot only after the candidate is complete.

### `load_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] ImageLoadResult load_bgra32_premultiplied( std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Validates dimensions, stride, byte extent, and quotas before owning one raw BGRA resource.

### `replace_png` (public)

```cpp
[[nodiscard]] ImageLoadResult replace_png( ImageId image, std::span<const std::byte> encoded)
```

Validates a complete PNG candidate and current ImageId before atomically advancing slot generation and totals.

### `replace_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] ImageLoadResult replace_bgra32_premultiplied( ImageId image, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Validates a complete raw candidate and current ImageId before replacement.

### `update_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] ImageLoadResult update_bgra32_premultiplied( ImageId image, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Updates same-sized raw pixels without changing ImageId, for steady-state retained animation.

### `patch_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] ImageLoadResult patch_bgra32_premultiplied( ImageId image, std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height, std::uint64_t source_row_bytes, std::span<const std::byte> pixels)
```

Validates and copies one bounded rectangle into an existing raw image while preserving identity and exact stride rules.

### `remove` (public)

```cpp
[[nodiscard]] bool remove(ImageId image) noexcept
```

Retires a current generational ID, advances its slot generation, updates totals, and makes stale handles fail.

### `clear` (public)

```cpp
void clear() noexcept
```

Retires every occupied slot and resets byte/resource totals while preserving future generation safety.

### `find` (public)

```cpp
[[nodiscard]] std::optional<ImageResourceView> find( ImageId image) const noexcept
```

Resolves only an exact current generational ID and returns a mutation-scoped byte view.

### `image_ids` (public)

```cpp
[[nodiscard]] std::vector<ImageId> image_ids() const
```

Returns all current generational IDs in stable slot order.

### `snapshot` (public)

```cpp
[[nodiscard]] ImageRegistrySnapshot snapshot() const noexcept
```

Copies registry revision, resource count, and encoded/decoded byte totals.

### `limits` (public)

```cpp
[[nodiscard]] const ImageRegistryLimits& limits() const noexcept
```

Returns the immutable quota policy.

### `store_new` (private)

```cpp
[[nodiscard]] ImageLoadResult store_new( ImageResourceEncoding encoding, std::vector<std::byte> encoded, PngMetadata metadata, std::uint64_t row_bytes, std::uint64_t content_hash)
```

Finds or creates a slot, commits candidate ownership, updates totals/revision, and returns exact generational identity.

### `registry_quota_error` (private)

```cpp
[[nodiscard]] ImageResourceError registry_quota_error( std::uint64_t encoded_bytes, std::uint64_t decoded_bytes, std::uint64_t replaced_encoded = 0, std::uint64_t replaced_decoded = 0) const noexcept
```

Checks replacement-adjusted encoded and decoded totals without overflow and names the first exceeded quota.
