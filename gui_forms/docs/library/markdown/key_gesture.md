# KeyGesture

- Status: **OBSERVED: bundle 007 accelerator value isolation; M4 build and focused tests pass**
- Kind: **struct**
- Hierarchy: `KeyGesture`
- Declaration: `include/gui_forms/window/window.hpp:73`
- Definition: `inline/header-only`

KeyGesture is exact portable physical-key plus modifier identity used for deterministic accelerator comparison.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const KeyGesture&, const KeyGesture&) = default
```

Orders and compares the complete physical-key/modifier identity.
