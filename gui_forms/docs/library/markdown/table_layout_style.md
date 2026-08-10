# TableLayoutStyle

- Status: **OBSERVED: bundle 011 table-layout style review**
- Kind: **struct**
- Hierarchy: `TableLayoutStyle`
- Declaration: `include/gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp:20`
- Definition: `inline/header-only`

TableLayoutStyle pairs absolute, autosize, or percentage sizing policy with its validated authored value.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const TableLayoutStyle&, const TableLayoutStyle&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
