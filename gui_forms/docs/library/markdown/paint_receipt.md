# PaintReceipt

- Status: **OBSERVED: bundle 008 presentation acknowledgement review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `PaintReceipt`
- Declaration: `include/gui_forms/window/presentation/presentation_types.hpp:63`
- Definition: `inline/header-only`

PaintReceipt proves that one complete retained transaction replayed into a host backing surface at a specific content revision and surface epoch.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit constexpr operator bool() const noexcept
```

Accepts a receipt as structurally present only when both its rendered revision and surface epoch are nonzero.

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const PaintReceipt&, const PaintReceipt&) = default
```

Orders and compares receipts by their exact revision-and-epoch identity.
