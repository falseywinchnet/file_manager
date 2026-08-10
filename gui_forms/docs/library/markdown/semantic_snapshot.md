# SemanticSnapshot

- Status: **OBSERVED: bundle 009 semantic projection split; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `SemanticSnapshot`
- Declaration: `include/gui_forms/semantics/types/semantic_types.hpp:88`
- Definition: `src/core/semantics/snapshot/semantic_snapshot.cpp`

SemanticSnapshot is an immutable generation-tagged forest of exact retained accessibility nodes with counted topology.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `to_json` (public)

```cpp
[[nodiscard]] std::string to_json() const
```

Serializes roles, states, actions, values, bounds, identities, and child topology with stable vocabulary.
