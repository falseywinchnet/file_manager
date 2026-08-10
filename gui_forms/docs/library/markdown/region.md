# Region

- Status: **OBSERVED: bundle 009 region split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → Region`
- Declaration: `include/gui_forms/drawing/region/region.hpp:13`
- Definition: `src/core/drawing/region/region.cpp`

Region is a retained union of rectangles and path snapshots with explicit rectangular exclusions, deterministic visibility, and conservative bounds.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `Region` (public)

```cpp
explicit Region(RectF rectangle)
```

Constructs from one validated rectangle or one live GraphicsPath snapshot.

### `Region` (public)

```cpp
explicit Region(const GraphicsPath& path)
```

Constructs from one validated rectangle or one live GraphicsPath snapshot.

### `unite` (public)

```cpp
void unite(RectF rectangle)
```

Adds a validated rectangle or path snapshot to the retained union.

### `unite` (public)

```cpp
void unite(const GraphicsPath& path)
```

Adds a validated rectangle or path snapshot to the retained union.

### `exclude` (public)

```cpp
void exclude(RectF rectangle)
```

Adds a validated rectangular exclusion.

### `is_visible` (public)

```cpp
[[nodiscard]] bool is_visible(PointF point) const
```

Tests union membership and then removes any exclusion membership.

### `bounds` (public)

```cpp
[[nodiscard]] RectF bounds() const
```

Returns conservative union bounds, accounting for empty retained parts.

### `snapshot` (public)

```cpp
[[nodiscard]] RegionSnapshot snapshot() const
```

Copies rectangles, path snapshots, and exclusions.
