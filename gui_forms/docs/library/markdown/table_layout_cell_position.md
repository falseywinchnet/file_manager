# TableLayoutCellPosition

- Status: **OBSERVED: bundle 011 table-layout position review**
- Kind: **struct**
- Hierarchy: `TableLayoutCellPosition`
- Declaration: `include/gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp:27`
- Definition: `inline/header-only`

TableLayoutCellPosition names a zero-based row and column for exact retained child placement.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const TableLayoutCellPosition&, const TableLayoutCellPosition&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
