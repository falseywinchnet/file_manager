# MasterDetailLayout

- Status: **OBSERVED: bundle 011 master-detail layout review**
- Kind: **struct**
- Hierarchy: `MasterDetailLayout`
- Declaration: `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:17`
- Definition: `inline/header-only`

MasterDetailLayout defines bounded navigation width and inter-pane gap for deterministic responsive projection.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const MasterDetailLayout&, const MasterDetailLayout&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
