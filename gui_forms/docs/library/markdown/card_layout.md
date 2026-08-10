# CardLayout

- Status: **OBSERVED: bundle 011 card value review**
- Kind: **struct**
- Hierarchy: `CardLayout`
- Declaration: `include/gui_forms/controls/panel/card/card.hpp:10`
- Definition: `inline/header-only`

CardLayout defines validated logical padding, section gaps, and footer/header allocation used by Card measurement and arrangement.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const CardLayout&, const CardLayout&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
