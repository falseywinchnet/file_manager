# ReviewRecord

- Status: **OBSERVED: bundle 011 review-card value review**
- Kind: **struct**
- Hierarchy: `ReviewRecord`
- Declaration: `include/gui_forms/controls/panel/card/review_card/review_card.hpp:21`
- Definition: `inline/header-only`

ReviewRecord owns the stable author, timestamp, score/status, title, body, and metadata projected by ReviewCard.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const ReviewRecord&, const ReviewRecord&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
