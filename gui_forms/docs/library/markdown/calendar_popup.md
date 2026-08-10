# CalendarPopup

- Status: **OBSERVED: bundle 004 source-private calendar state-machine split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → CalendarPopup`
- Declaration: `src/controls/panel/date_time_picker/calendar_popup/calendar_popup.hpp:19`
- Definition: `src/controls/panel/date_time_picker/calendar_popup/calendar_popup.cpp`

CalendarPopup is DateTimePicker's source-private retained month grid. It owns displayed month, constrained candidate selection, pointer/keyboard navigation, commit/cancel events, stable virtual day identities, and calendar semantics while the picker retains final value authority.

## Visual evidence

![CalendarPopup](../captures/date_time_picker.png)

## Declared methods

### `~CalendarPopup` (public)

```cpp
~CalendarPopup() override
```

Releases the source-private popup through its isolated translation-unit boundary.

### `CalendarPopup` (public)

```cpp
CalendarPopup(StableId stable_id, DateTimeValue selected, DateTimeValue minimum, DateTimeValue maximum, DateTimeFormatProvider provider, BasicControlStyle style) : Control(std::move(stable_id)), selected_(selected), minimum_(minimum), maximum_(maximum), provider_(std::move(provider)), style_(style), display_year_(selected.year), display_month_(selected.month)
```

Constructs a focusable month grid from current value, admitted range, format provider, font, and style.

### `committed` (public)

```cpp
[[nodiscard]] Event<DateTimeValue>& committed() noexcept
```

Returns the event carrying the selected DateTimeValue after qualified day activation.

### `cancelled` (public)

```cpp
[[nodiscard]] Event<>& cancelled() noexcept
```

Returns the event raised by Escape without modifying picker value.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect) override
```

Records calendar frame, month navigation, weekday headings, 42 day cells, range-disabled states, selection, hover, and focus.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Handles month buttons, day selection, hover, capture qualification, and same-cell commit.

### `on_key` (public)

```cpp
void on_key(KeyEvent& event) override
```

Implements day/week/month/year navigation, home/end boundaries, commit, and cancellation under range constraints.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Commits popup focus state and invalidates the selected-day cue.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a calendar role with displayed month and selected ISO date.

### `semantic_virtual_children` (public)

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Exposes each visible day as a stable selectable virtual cell with selected/disabled metadata.

### `on_semantic_child_action` (public)

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view) override
```

Maps virtual day selection or press to constrained selection and commit.

### `cell_dates` (private)

```cpp
[[nodiscard]] std::array<DateTimeValue, 42> cell_dates() const noexcept
```

Reports the current cell dates value without mutation.

### `cell_at` (private)

```cpp
[[nodiscard]] std::optional<std::size_t> cell_at(Point local) const noexcept
```

Reports the current cell at value without mutation.

### `move_selection` (private)

```cpp
void move_selection(int days)
```

Executes CalendarPopup's move selection operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `month_candidate` (private)

```cpp
[[nodiscard]] DateTimeValue month_candidate(int direction) const noexcept
```

Reports the current month candidate value without mutation.

### `can_change_month` (private)

```cpp
[[nodiscard]] bool can_change_month(int direction) const noexcept
```

Reports the current can change month value without mutation.

### `change_month` (private)

```cpp
void change_month(int direction)
```

Executes CalendarPopup's change month operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `date_stable_id` (private)

```cpp
[[nodiscard]] std::string date_stable_id(DateTimeValue date) const
```

Reports the current date stable id value without mutation.

### `iso_date` (private)

```cpp
[[nodiscard]] static std::string iso_date(DateTimeValue date)
```

Executes CalendarPopup's iso date operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
