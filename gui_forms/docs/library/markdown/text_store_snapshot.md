# TextStoreSnapshot

- Status: **OBSERVED: bundle 009 text telemetry review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `TextStoreSnapshot`
- Declaration: `include/gui_forms/text/types/text_types.hpp:83`
- Definition: `inline/header-only`

TextStoreSnapshot reports coordinate counts, revision counters, rebuilds, and rejected operations without exposing storage.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const TextStoreSnapshot&, const TextStoreSnapshot&) = default
```

Compares every telemetry field.
