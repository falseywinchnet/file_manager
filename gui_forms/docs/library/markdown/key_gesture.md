# KeyGesture

- Status: **OBSERVED: bundle 007 accelerator value isolation; M4 build and focused tests pass**
- Kind: **struct**
- Hierarchy: `KeyGesture`
- Declaration: `include/gui_forms/window/window.hpp:71`
- Definition: `inline/header-only`

KeyGesture is exact portable physical-key plus modifier identity used for deterministic accelerator comparison.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const KeyGesture&, const KeyGesture&) = default
```

Orders and compares the complete physical-key/modifier identity.
