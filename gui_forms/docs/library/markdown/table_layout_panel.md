# TableLayoutPanel

- Status: **OBSERVED: bundle 003 split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `ContainerControl → TableLayoutPanel`
- Declaration: `include/gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp:47`
- Definition: `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp`

TableLayoutPanel is a retained grid solver with bounded rows/columns, fixed/add-row/add-column growth, absolute/percent/auto tracks, explicit or automatic placement, row/column spans, cell borders, overflow inspection, and content-driven sizing.

## Visual evidence

![TableLayoutPanel](../captures/layout_panels.png)

## Declared methods

### `TableLayoutPanel` (public)

```cpp
explicit TableLayoutPanel(StableId stable_id)
```

Constructs a one-by-one table that may add rows during automatic placement.

### `column_count` (public)

```cpp
[[nodiscard]] std::size_t column_count() const noexcept
```

Returns the authored base column count.

### `set_column_count` (public)

```cpp
void set_column_count(std::size_t count)
```

Validates the bounded nonzero count, resizes style storage, reconciles placement, and invalidates layout.

### `row_count` (public)

```cpp
[[nodiscard]] std::size_t row_count() const noexcept
```

Returns the authored base row count.

### `set_row_count` (public)

```cpp
void set_row_count(std::size_t count)
```

Validates the bounded nonzero count, resizes style storage, reconciles placement, and invalidates layout.

### `grow_style` (public)

```cpp
[[nodiscard]] TableLayoutGrowStyle grow_style() const noexcept
```

Returns fixed-size, add-rows, or add-columns automatic placement policy.

### `set_grow_style` (public)

```cpp
void set_grow_style(TableLayoutGrowStyle style)
```

Validates growth vocabulary and invalidates the table solution.

### `auto_size` (public)

```cpp
[[nodiscard]] bool auto_size() const noexcept override
```

Returns whether desired size follows the resolved grid extent.

### `set_auto_size` (public)

```cpp
void set_auto_size(bool auto_size) override
```

Commits content-driven sizing and invalidates table measurement/arrangement.

### `cell_border_style` (public)

```cpp
[[nodiscard]] TableCellBorderStyle cell_border_style() const noexcept
```

Returns none, single, inset, or outset cell-frame policy.

### `set_cell_border_style` (public)

```cpp
void set_cell_border_style(TableCellBorderStyle style)
```

Validates the closed border vocabulary and invalidates painting and semantics.

### `column_styles` (public)

```cpp
[[nodiscard]] std::span<const TableLayoutStyle> column_styles() const noexcept
```

Returns a read-only span over current authored column track policies.

### `row_styles` (public)

```cpp
[[nodiscard]] std::span<const TableLayoutStyle> row_styles() const noexcept
```

Returns a read-only span over current authored row track policies.

### `set_column_style` (public)

```cpp
void set_column_style(std::size_t column, TableLayoutStyle style)
```

Validates column index and finite nonnegative absolute/percent/auto track data before invalidating the solution.

### `set_row_style` (public)

```cpp
void set_row_style(std::size_t row, TableLayoutStyle style)
```

Validates row index and finite nonnegative absolute/percent/auto track data before invalidating the solution.

### `set_cell_position` (public)

```cpp
void set_cell_position(const Control& child, TableLayoutCellPosition position)
```

Requires a direct child, validates explicit row/column coordinates, and records stable placement metadata.

### `clear_cell_position` (public)

```cpp
void clear_cell_position(const Control& child)
```

Removes explicit placement so the direct child returns to deterministic automatic allocation.

### `cell_position` (public)

```cpp
[[nodiscard]] std::optional<TableLayoutCellPosition> cell_position( const Control& child) const
```

Returns the explicit authored cell, not the transient automatically resolved cell.

### `set_column_span` (public)

```cpp
void set_column_span(const Control& child, std::size_t span)
```

Requires a direct child and records a bounded positive column span.

### `column_span` (public)

```cpp
[[nodiscard]] std::size_t column_span(const Control& child) const
```

Returns the retained column span or one when no metadata exists.

### `set_row_span` (public)

```cpp
void set_row_span(const Control& child, std::size_t span)
```

Requires a direct child and records a bounded positive row span.

### `row_span` (public)

```cpp
[[nodiscard]] std::size_t row_span(const Control& child) const
```

Returns the retained row span or one when no metadata exists.

### `control_from_position` (public)

```cpp
[[nodiscard]] Control::Ptr control_from_position(std::size_t column, std::size_t row) const
```

Returns the live control whose resolved cell/span covers the requested coordinate.

### `column_widths` (public)

```cpp
[[nodiscard]] std::span<const double> column_widths() const noexcept
```

Returns the most recently resolved logical track widths for inspection.

### `row_heights` (public)

```cpp
[[nodiscard]] std::span<const double> row_heights() const noexcept
```

Returns the most recently resolved logical track heights for inspection.

### `layout_overflowed` (public)

```cpp
[[nodiscard]] bool layout_overflowed() const noexcept
```

Reports that fixed growth policy could not place every child without mutating the authored table shape.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Resolves placement and track demands without assignment and returns content extent when auto-sized.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Solves tracks against final bounds, commits resolved cells/track extents, and assigns span-aware margin-reduced child rectangles.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Paints the inherited container surface and records resolved cell borders with the selected frame policy.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a named table as a group and reports overflow state without inventing a platform-native grid identity.

### `metadata_for` (private)

```cpp
[[nodiscard]] CellMetadata& metadata_for(const Control& child)
```

Executes TableLayoutPanel's metadata for operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `metadata_for` (private)

```cpp
[[nodiscard]] const CellMetadata* metadata_for(const Control& child) const
```

Reports the current metadata for value without mutation.

### `reconcile_metadata` (private)

```cpp
void reconcile_metadata()
```

Executes TableLayoutPanel's reconcile metadata operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `layout_children` (private)

```cpp
[[nodiscard]] Size layout_children(Size available, bool assign)
```

Executes TableLayoutPanel's layout children operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `validate_style` (private)

```cpp
static void validate_style(TableLayoutStyle style)
```

Executes TableLayoutPanel's validate style operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
