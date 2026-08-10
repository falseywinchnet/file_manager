# SemanticFeedbackRecord

- Status: **OBSERVED: bundle 010 semantic feedback record review; focused M4 core tests pass**
- Kind: **struct**
- Hierarchy: `SemanticFeedbackRecord`
- Declaration: `include/gui_forms/feedback/types/feedback_types.hpp:21`
- Definition: `inline/header-only`

SemanticFeedbackRecord preserves ordered kind, chosen cue, monotonic timestamp, user sound policy, host acceptance, and actual presentation.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const SemanticFeedbackRecord&, const SemanticFeedbackRecord&) = default
```

Compares the complete feedback result.
