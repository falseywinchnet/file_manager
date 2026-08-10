# CorrespondenceView

- Status: **OBSERVED: bundle 005 split and status-rail enhancement; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → CorrespondenceView`
- Declaration: `include/gui_forms/controls/panel/correspondence_view/correspondence_view.hpp:74`
- Definition: `src/controls/panel/correspondence_view/correspondence_view.cpp`

CorrespondenceView is a virtualized search/result correspondence surface whose stable rows can expand in response to explicit pinning or hover intent. It separates focus, selection, hover, and pin authority; preserves scroll anchors while variable row heights change; bounds realization; and exposes exact virtual semantics.

## Visual evidence

![CorrespondenceView](../captures/correspondence_view.png)

## Declared methods

### `CorrespondenceView` (public)

```cpp
explicit CorrespondenceView(StableId stable_id)
```

Constructs a focusable variable-height result surface with hover-intent scheduling and stable anchoring.

### `items` (public)

```cpp
[[nodiscard]] std::span<const CorrespondenceItem> items() const noexcept
```

Returns the authored correspondence records in ranking order.

### `set_items` (public)

```cpp
void set_items(std::vector<CorrespondenceItem> items)
```

Validates stable identities, replaces the result model, and reconciles selection, focus, hover, pinning, and scroll anchoring.

### `selected_id` (public)

```cpp
[[nodiscard]] std::string_view selected_id() const noexcept
```

Returns the authoritative selected result identity.

### `set_selected_id` (public)

```cpp
void set_selected_id(std::string_view stable_id)
```

Selects an admitted result, ensures visibility, and publishes only a real transition.

### `focused_id` (public)

```cpp
[[nodiscard]] std::string_view focused_id() const noexcept
```

Returns the keyboard-focused result independently of selection.

### `hovered_id` (public)

```cpp
[[nodiscard]] std::string_view hovered_id() const noexcept
```

Returns the result currently under hover intent, if any.

### `pinned_id` (public)

```cpp
[[nodiscard]] std::string_view pinned_id() const noexcept
```

Returns the result explicitly retained in expanded presentation.

### `set_pinned_id` (public)

```cpp
void set_pinned_id(std::string_view stable_id)
```

Commits or clears the pinned result, preserves the viewport anchor, and publishes expansion deltas.

### `expanded` (public)

```cpp
[[nodiscard]] bool expanded(std::string_view stable_id) const
```

Reports whether a result is expanded by pin or active hover intent.

### `compact_height` (public)

```cpp
[[nodiscard]] double compact_height() const noexcept
```

Returns logical height of a collapsed result row.

### `set_compact_height` (public)

```cpp
void set_compact_height(double height)
```

Validates compact geometry and restores the viewport anchor after recomputation.

### `expanded_height` (public)

```cpp
[[nodiscard]] double expanded_height() const noexcept
```

Returns logical height of an expanded correspondence row.

### `set_expanded_height` (public)

```cpp
void set_expanded_height(double height)
```

Validates expanded geometry and preserves visible content across the change.

### `status_rail_width` (public)

```cpp
[[nodiscard]] double status_rail_width() const noexcept
```

Returns the logical width of the per-result status rail.

### `set_status_rail_width` (public)

```cpp
void set_status_rail_width(double width)
```

Validates bounded rail width and refreshes the correspondence visual without changing result state.

### `hover_intent_delay` (public)

```cpp
[[nodiscard]] std::chrono::milliseconds hover_intent_delay() const noexcept
```

Returns the delay before a hovered row gains transient expansion.

### `set_hover_intent_delay` (public)

```cpp
void set_hover_intent_delay(std::chrono::milliseconds delay)
```

Validates nonnegative timing and safely re-arms or cancels pending hover work.

### `scroll_offset` (public)

```cpp
[[nodiscard]] double scroll_offset() const noexcept
```

Returns the logical pixel offset into variable-height content.

### `set_scroll_offset` (public)

```cpp
void set_scroll_offset(double offset)
```

Clamps and commits scrolling against computed content height.

### `content_height` (public)

```cpp
[[nodiscard]] double content_height() const noexcept
```

Returns total logical height of compact and expanded rows.

### `realized_count` (public)

```cpp
[[nodiscard]] std::size_t realized_count() const noexcept
```

Returns the number of rows intersecting the current virtualization window.

### `item_bounds` (public)

```cpp
[[nodiscard]] std::optional<Rect> item_bounds( std::string_view stable_id) const noexcept
```

Returns a result's logical viewport-relative bounds when its identity exists.

### `font` (public)

```cpp
[[nodiscard]] FontSpec font() const noexcept
```

Returns the retained typography for titles, paths, and correspondence text.

### `set_font` (public)

```cpp
void set_font(FontSpec font)
```

Validates typography and refreshes paint and semantics.

### `selection_changed` (public)

```cpp
[[nodiscard]] Event<const CorrespondenceSelectionChange&>& selection_changed() noexcept
```

Returns the event published after authoritative result selection changes.

### `pin_changed` (public)

```cpp
[[nodiscard]] Event<const CorrespondencePinChange&>& pin_changed() noexcept
```

Returns the event carrying the old and new explicitly pinned identities.

### `expansion_changed` (public)

```cpp
[[nodiscard]] Event<const CorrespondenceExpansionChange&>& expansion_changed() noexcept
```

Returns the event describing rows entering or leaving expanded projection.

### `item_activated` (public)

```cpp
[[nodiscard]] Event<const std::string&>& item_activated() noexcept
```

Returns the event carrying the qualified activated result.

### `context_requested` (public)

```cpp
[[nodiscard]] Event<const ObjectContextRequest&>& context_requested() noexcept
```

Returns the event carrying result identity and position for owned context UI.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Commits viewport size, clamps offset, and maintains the selected/focused result's reachability.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records only realized compact or expanded rows, status rails, matched excerpts, hover, selection, and focus.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Handles hover intent, selection, pin toggling, context requests, capture qualification, and activation.

### `on_key` (public)

```cpp
void on_key(KeyEvent& event) override
```

Implements variable-height navigation, paging, expansion/pinning, activation, and context request.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Commits focus appearance and restores a usable focused result on entry.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a result list with total and realized counts plus current selection.

### `semantic_virtual_children` (public)

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Exposes stable result nodes with correspondence descriptions, expanded/selected state, and actions.

### `on_semantic_child_action` (public)

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value) override
```

Routes virtual selection, expansion, collapse, and press through ordinary retained transitions.

### `on_detached_from_window` (protected)

```cpp
void on_detached_from_window() noexcept override
```

Executes CorrespondenceView's on detached from window operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `item_index` (private)

```cpp
[[nodiscard]] std::optional<std::size_t> item_index( std::string_view stable_id) const noexcept
```

Reports the current item index value without mutation.

### `expanded_indices` (private)

```cpp
[[nodiscard]] std::vector<std::size_t> expanded_indices() const
```

Reports the current expanded indices value without mutation.

### `expanded_index` (private)

```cpp
[[nodiscard]] bool expanded_index(std::size_t index) const noexcept
```

Reports the current expanded index value without mutation.

### `scaled_compact_height` (private)

```cpp
[[nodiscard]] double scaled_compact_height() const noexcept
```

Reports the current scaled compact height value without mutation.

### `scaled_expanded_height` (private)

```cpp
[[nodiscard]] double scaled_expanded_height() const noexcept
```

Reports the current scaled expanded height value without mutation.

### `row_height` (private)

```cpp
[[nodiscard]] double row_height(std::size_t index) const noexcept
```

Reports the current row height value without mutation.

### `row_top` (private)

```cpp
[[nodiscard]] double row_top(std::size_t index) const noexcept
```

Reports the current row top value without mutation.

### `viewport_height` (private)

```cpp
[[nodiscard]] double viewport_height() const noexcept
```

Reports the current viewport height value without mutation.

### `maximum_scroll_offset` (private)

```cpp
[[nodiscard]] double maximum_scroll_offset() const noexcept
```

Reports the current maximum scroll offset value without mutation.

### `first_visible_index` (private)

```cpp
[[nodiscard]] std::size_t first_visible_index() const noexcept
```

Reports the current first visible index value without mutation.

### `realized_range` (private)

```cpp
[[nodiscard]] std::pair<std::size_t, std::size_t> realized_range() const noexcept
```

Reports the current realized range value without mutation.

### `item_bounds` (private)

```cpp
[[nodiscard]] Rect item_bounds(std::size_t index) const noexcept
```

Returns a result's logical viewport-relative bounds when its identity exists.

### `index_at` (private)

```cpp
[[nodiscard]] std::optional<std::size_t> index_at(Point absolute) const noexcept
```

Reports the current index at value without mutation.

### `capture_anchor` (private)

```cpp
[[nodiscard]] ScrollAnchor capture_anchor(std::size_t preferred) const noexcept
```

Reports the current capture anchor value without mutation.

### `restore_anchor` (private)

```cpp
void restore_anchor(ScrollAnchor anchor)
```

Executes CorrespondenceView's restore anchor operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `clamp_scroll_offset` (private)

```cpp
void clamp_scroll_offset() noexcept
```

Executes CorrespondenceView's clamp scroll offset operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `ensure_visible` (private)

```cpp
void ensure_visible(std::size_t index)
```

Executes CorrespondenceView's ensure visible operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `focus_index` (private)

```cpp
void focus_index(std::size_t index, CorrespondenceExpansionReason reason)
```

Executes CorrespondenceView's focus index operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `set_hovered_index` (private)

```cpp
void set_hovered_index(std::optional<std::size_t> index)
```

Synchronously updates the retained hovered index property. Validation, typed invalidation, and notifications are defined by the implementation.

### `schedule_hover_intent` (private)

```cpp
void schedule_hover_intent(std::size_t index)
```

Executes CorrespondenceView's schedule hover intent operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `clear_hover_intent` (private)

```cpp
void clear_hover_intent(bool clear_expansion)
```

Removes the explicit hover intent value and restores fallback behavior.

### `apply_hover_expansion` (private)

```cpp
void apply_hover_expansion(std::string stable_id)
```

Executes CorrespondenceView's apply hover expansion operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `emit_expansion_delta` (private)

```cpp
void emit_expansion_delta(std::string_view stable_id, bool before, bool after, CorrespondenceExpansionReason reason)
```

Executes CorrespondenceView's emit expansion delta operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `paint_glyph` (private)

```cpp
void paint_glyph(Painter& painter, Rect bounds, ObjectGlyph glyph, bool enabled) const
```

Reports the current paint glyph value without mutation.
