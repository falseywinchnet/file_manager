# TableLayoutPanel

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `ContainerControl → TableLayoutPanel`  
Declaration: `include/gui_forms/container_controls.hpp:188`  
Definition: `src/controls/container_controls.cpp`

TableLayoutPanel is a visual retained control declared in include/gui_forms/container_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `TableLayoutPanel`

```cpp
explicit TableLayoutPanel(StableId stable_id)
```

Constructs or tears down the retained TableLayoutPanel object according to its ownership contract.

### `column_count`

```cpp
[[nodiscard]] std::size_t column_count() const noexcept
```

Reports the current column count value without mutation.

### `set_column_count`

```cpp
void set_column_count(std::size_t count)
```

Synchronously updates the retained column count property. Validation, typed invalidation, and notifications are defined by the implementation.

### `row_count`

```cpp
[[nodiscard]] std::size_t row_count() const noexcept
```

Reports the current row count value without mutation.

### `set_row_count`

```cpp
void set_row_count(std::size_t count)
```

Synchronously updates the retained row count property. Validation, typed invalidation, and notifications are defined by the implementation.

### `grow_style`

```cpp
[[nodiscard]] TableLayoutGrowStyle grow_style() const noexcept
```

Reports the current grow style value without mutation.

### `set_grow_style`

```cpp
void set_grow_style(TableLayoutGrowStyle style)
```

Synchronously updates the retained grow style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_size`

```cpp
[[nodiscard]] bool auto_size() const noexcept override
```

Reports the current auto size value without mutation.

### `set_auto_size`

```cpp
void set_auto_size(bool auto_size) override
```

Synchronously updates the retained auto size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `cell_border_style`

```cpp
[[nodiscard]] TableCellBorderStyle cell_border_style() const noexcept
```

Reports the current cell border style value without mutation.

### `set_cell_border_style`

```cpp
void set_cell_border_style(TableCellBorderStyle style)
```

Synchronously updates the retained cell border style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `column_styles`

```cpp
[[nodiscard]] std::span<const TableLayoutStyle> column_styles() const noexcept
```

Reports the current column styles value without mutation.

### `row_styles`

```cpp
[[nodiscard]] std::span<const TableLayoutStyle> row_styles() const noexcept
```

Reports the current row styles value without mutation.

### `set_column_style`

```cpp
void set_column_style(std::size_t column, TableLayoutStyle style)
```

Synchronously updates the retained column style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_row_style`

```cpp
void set_row_style(std::size_t row, TableLayoutStyle style)
```

Synchronously updates the retained row style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_cell_position`

```cpp
void set_cell_position(const Control& child, TableLayoutCellPosition position)
```

Synchronously updates the retained cell position property. Validation, typed invalidation, and notifications are defined by the implementation.

### `clear_cell_position`

```cpp
void clear_cell_position(const Control& child)
```

Removes the explicit cell position value and restores fallback behavior.

### `cell_position`

```cpp
[[nodiscard]] std::optional<TableLayoutCellPosition> cell_position( const Control& child) const
```

Reports the current cell position value without mutation.

### `set_column_span`

```cpp
void set_column_span(const Control& child, std::size_t span)
```

Synchronously updates the retained column span property. Validation, typed invalidation, and notifications are defined by the implementation.

### `column_span`

```cpp
[[nodiscard]] std::size_t column_span(const Control& child) const
```

Reports the current column span value without mutation.

### `set_row_span`

```cpp
void set_row_span(const Control& child, std::size_t span)
```

Synchronously updates the retained row span property. Validation, typed invalidation, and notifications are defined by the implementation.

### `row_span`

```cpp
[[nodiscard]] std::size_t row_span(const Control& child) const
```

Reports the current row span value without mutation.

### `control_from_position`

```cpp
[[nodiscard]] Control::Ptr control_from_position(std::size_t column, std::size_t row) const
```

Reports the current control from position value without mutation.

### `column_widths`

```cpp
[[nodiscard]] std::span<const double> column_widths() const noexcept
```

Reports the current column widths value without mutation.

### `row_heights`

```cpp
[[nodiscard]] std::span<const double> row_heights() const noexcept
```

Reports the current row heights value without mutation.

### `layout_overflowed`

```cpp
[[nodiscard]] bool layout_overflowed() const noexcept
```

Reports the current layout overflowed value without mutation.

### `measure`

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired size from the available constraint without arranging children.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
