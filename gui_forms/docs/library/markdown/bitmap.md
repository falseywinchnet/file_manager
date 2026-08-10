# Bitmap

- Status: **OBSERVED: bundle 009 bitmap lease/damage state-machine split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → Bitmap`
- Declaration: `include/gui_forms/drawing/bitmap/bitmap.hpp:49`
- Definition: `src/core/drawing/bitmap/bitmap.cpp`

Bitmap owns bounded premultiplied pixel storage, stable identity and generation, exclusive read/write locks, rollback-capable rectangular edit leases, exact changed-byte damage derivation, bounded damage history, transforms, cloning, and immutable snapshots.

## Visual evidence

![Bitmap](../captures/drawing_raster_material.png)

## Declared methods

### `Bitmap` (public)

```cpp
Bitmap(std::uint32_t width, std::uint32_t height, PixelFormat pixel_format = PixelFormat::bgra32_premultiplied)
```

Validates dimensions/format/byte extent, allocates zeroed storage, and assigns nonzero stable identity.

### `width` (public)

```cpp
[[nodiscard]] std::uint32_t width() const
```

Requires liveness and returns pixel width.

### `height` (public)

```cpp
[[nodiscard]] std::uint32_t height() const
```

Requires liveness and returns pixel height.

### `pixel_format` (public)

```cpp
[[nodiscard]] PixelFormat pixel_format() const
```

Requires liveness and returns the admitted premultiplied layout.

### `generation` (public)

```cpp
[[nodiscard]] std::uint64_t generation() const
```

Requires liveness and returns the current published mutation generation.

### `get_pixel` (public)

```cpp
[[nodiscard]] Color get_pixel(std::uint32_t x, std::uint32_t y) const
```

Requires liveness, no conflicting write state, and valid coordinates, then decodes one premultiplied pixel.

### `set_pixel` (public)

```cpp
void set_pixel(std::uint32_t x, std::uint32_t y, Color color)
```

Prepares exclusive mutation, writes one pixel, and publishes exact one-pixel damage only for a changed value.

### `make_transparent` (public)

```cpp
void make_transparent(Color key)
```

Transforms matching RGB pixels to transparent and publishes conservative changed runs.

### `clone` (public)

```cpp
[[nodiscard]] std::unique_ptr<Bitmap> clone(RectI source) const
```

Validates a source rectangle and copies it into independent identity/storage.

### `thumbnail` (public)

```cpp
[[nodiscard]] std::unique_ptr<Bitmap> thumbnail(std::uint32_t width, std::uint32_t height) const
```

Validates target dimensions and creates a deterministic nearest-sampled independent bitmap.

### `adjusted` (public)

```cpp
[[nodiscard]] std::unique_ptr<Bitmap> adjusted( const ImageAttributes& attributes) const
```

Applies a retained color matrix/remap snapshot into independent storage.

### `lock` (public)

```cpp
[[nodiscard]] BitmapLockView lock(BitmapLockMode mode)
```

Rejects nested leases, creates a nonzero token, and returns read and optional mutable storage according to lock mode.

### `unlock` (public)

```cpp
void unlock(std::uint64_t token)
```

Requires the exact active token and publishes full damage only for a writable lease.

### `locked` (public)

```cpp
[[nodiscard]] bool locked() const
```

Reports whether any lock or edit lease is active.

### `begin_edit` (public)

```cpp
[[nodiscard]] BitmapEditView begin_edit(RectI bounds)
```

Validates a nonempty rectangle, snapshots its original bytes for rollback, and returns a bounded writable lease.

### `commit_edit` (public)

```cpp
[[nodiscard]] std::uint64_t commit_edit(std::uint64_t token)
```

Requires the exact edit token, compares backup/current bytes, derives bounded changed rectangles, publishes one generation, and retires the lease.

### `cancel_edit` (public)

```cpp
void cancel_edit(std::uint64_t token)
```

Requires the exact edit token, restores backup bytes exactly, and retires without publication.

### `changes_since` (public)

```cpp
[[nodiscard]] BitmapDamageSnapshot changes_since( std::uint64_t generation) const
```

Returns bounded accumulated damage after a generation and marks history incomplete when the requested baseline aged out.

### `snapshot` (public)

```cpp
[[nodiscard]] ImageSnapshot snapshot() const
```

Returns immutable shared pixel storage plus stable identity, dimensions, format, and generation.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Drops storage, backup, damage history, and active lease state without throwing.

### `require_unlocked` (private)

```cpp
void require_unlocked() const
```

Rejects operations that would violate exclusive lock/edit ownership.

### `require_coordinate` (private)

```cpp
void require_coordinate(std::uint32_t x, std::uint32_t y) const
```

Rejects out-of-bounds pixel coordinates.

### `require_edit_token` (private)

```cpp
void require_edit_token(std::uint64_t token) const
```

Requires an active edit and its exact nonzero token.

### `prepare_write` (private)

```cpp
void prepare_write()
```

Requires liveness/unlocked state and detaches shared storage before mutation.

### `publish_mutation` (private)

```cpp
void publish_mutation(std::vector<RectI> damage)
```

Coalesces bounded damage, advances nonzero generation, and appends/compacts bounded history.

### `finish_edit` (private)

```cpp
void finish_edit() noexcept
```

Clears active edit identity, bounds, and rollback bytes.
