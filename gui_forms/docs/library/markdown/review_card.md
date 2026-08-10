# ReviewCard

- Status: **OBSERVED: bundle 001 split; M4 build and focused tests pass**
- Kind: **class / visual retained control**
- Hierarchy: `Card → ReviewCard`
- Declaration: `include/gui_forms/controls/panel/card/review_card/review_card.hpp:38`
- Definition: `src/controls/panel/card/review_card/review_card.cpp`

ReviewCard is a typed Card specialization that owns title, summary, and verdict Labels and atomically projects a bounded UTF-8 ReviewRecord into text, visual status, and semantics.

## Visual evidence

![ReviewCard](../captures/review_card.png)

## Declared methods

### `ReviewCard` (public)

```cpp
explicit ReviewCard(StableId stable_id)
```

Constructs the three retained Label roles with theme-aware heading, body, and caption policies; attachment remains lazy and idempotent.

### `initialize_control_tree` (public)

```cpp
void initialize_control_tree()
```

Attaches the three owned Labels to Card roles exactly once, allowing construction before shared ownership is established.

### `record` (public)

```cpp
[[nodiscard]] const ReviewRecord& record() const noexcept
```

Returns the committed typed ReviewRecord.

### `set_record` (public)

```cpp
void set_record(ReviewRecord record)
```

Validates identity, UTF-8, bounded text sizes, and disposition before any mutation; then updates all labels, accessibility text, status, and publishes one record_changed event.

### `title_label` (public)

```cpp
[[nodiscard]] std::shared_ptr<Label> title_label() const noexcept
```

Returns the owned header Label for ordinary Label-level customization.

### `summary_label` (public)

```cpp
[[nodiscard]] std::shared_ptr<Label> summary_label() const noexcept
```

Returns the owned wrapping body Label for ordinary Label-level customization.

### `verdict_label` (public)

```cpp
[[nodiscard]] std::shared_ptr<Label> verdict_label() const noexcept
```

Returns the owned footer Label for ordinary Label-level customization.

### `record_changed` (public)

```cpp
[[nodiscard]] Event<const ReviewRecord&>& record_changed() noexcept
```

Returns the event published after the complete record projection commits.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Ensures the internal tree exists, then uses Card's section measurement law.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Ensures the internal tree exists, then uses Card's section arrangement law.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects title, summary, verdict, pending, and rejected state from the same committed record used by the visible Labels.
