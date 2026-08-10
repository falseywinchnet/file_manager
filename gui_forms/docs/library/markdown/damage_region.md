# DamageRegion

- Status: **OBSERVED: bundle 011 bounded-damage state-machine review**
- Kind: **class**
- Hierarchy: `DamageRegion`
- Declaration: `include/gui_forms/types/damage_region/damage_region.hpp:12`
- Definition: `src/core/types/damage_region/damage_region.cpp`

DamageRegion incrementally merges exact rectangular unions, preserves disjoint damage up to 64 rectangles, then collapses to one bound with explicit compaction/collapse telemetry.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `add` (public)

```cpp
void add(Rect rect)
```

Executes DamageRegion's add operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `clear` (public)

```cpp
void clear() noexcept
```

Executes DamageRegion's clear operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

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
