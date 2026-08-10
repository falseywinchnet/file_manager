# DateTimePicker

- Status: **OBSERVED: bundle 004 split and drop-down-alignment enhancement; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → DateTimePicker`
- Declaration: `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:26`
- Definition: `src/controls/panel/date_time_picker/date_time_picker.cpp`

DateTimePicker is a retained civil date/time field with validated bounds, four formatting policies, caller-supplied format vocabulary, optional check state, calendar or spin presentation, configurable popup alignment, source-private transient ownership, keyboard/pointer/semantic control, and ordered typed events.

## Visual evidence

![DateTimePicker](../captures/date_time_picker.png)

## Declared methods

### `DateTimePicker` (public)

```cpp
explicit DateTimePicker(StableId stable_id)
```

Constructs a focusable picker with validated civil value/range defaults and disclosure cursor policy.

### `value` (public)

```cpp
[[nodiscard]] DateTimeValue value() const noexcept
```

Returns the authoritative constrained civil date/time value.

### `set_value` (public)

```cpp
void set_value(DateTimeValue value)
```

Validates the civil value, clamps to the admitted range, and publishes a real change.

### `minimum` (public)

```cpp
[[nodiscard]] DateTimeValue minimum() const noexcept
```

Returns the inclusive earliest admitted civil value.

### `maximum` (public)

```cpp
[[nodiscard]] DateTimeValue maximum() const noexcept
```

Returns the inclusive latest admitted civil value.

### `set_range` (public)

```cpp
void set_range(DateTimeValue minimum, DateTimeValue maximum)
```

Validates both civil values and chronological order, constrains current value, and refreshes popup/semantics coherently.

### `format` (public)

```cpp
[[nodiscard]] DateTimePickerFormat format() const noexcept
```

Returns long-date, short-date, time, or custom presentation policy.

### `set_format` (public)

```cpp
void set_format(DateTimePickerFormat format)
```

Validates the format vocabulary and invalidates measure, paint, and semantics.

### `custom_format` (public)

```cpp
[[nodiscard]] const std::string& custom_format() const noexcept
```

Returns the token pattern used when custom formatting is selected.

### `set_custom_format` (public)

```cpp
void set_custom_format(std::string format)
```

Validates and commits the caller pattern, then refreshes formatted projection.

### `format_provider` (public)

```cpp
[[nodiscard]] const DateTimeFormatProvider& format_provider() const noexcept
```

Returns the shared immutable vocabulary/pattern provider.

### `set_format_provider` (public)

```cpp
void set_format_provider(DateTimeFormatProvider provider)
```

Requires a provider, commits shared ownership, and refreshes field and open popup presentation.

### `formatted_value` (public)

```cpp
[[nodiscard]] std::string formatted_value() const
```

Formats the current civil value through active format and provider without changing authority.

### `show_check_box` (public)

```cpp
[[nodiscard]] bool show_check_box() const noexcept
```

Reports whether an enable/disable checkbox is painted and interactive.

### `set_show_check_box` (public)

```cpp
void set_show_check_box(bool show)
```

Toggles checkbox composition and invalidates measure, paint, and semantics.

### `checked` (public)

```cpp
[[nodiscard]] bool checked() const noexcept
```

Returns the retained enabled state when check-box presentation is used.

### `set_checked` (public)

```cpp
void set_checked(bool checked)
```

Commits enabled state, closes disallowed popups, and publishes a real checked transition.

### `show_up_down` (public)

```cpp
[[nodiscard]] bool show_up_down() const noexcept
```

Reports whether compact day stepping replaces calendar disclosure.

### `set_show_up_down` (public)

```cpp
void set_show_up_down(bool show)
```

Switches between spin and popup affordances, closing transient state when required.

### `dropped_down` (public)

```cpp
[[nodiscard]] bool dropped_down() const noexcept
```

Reports whether this picker currently owns its calendar transient lease.

### `set_dropped_down` (public)

```cpp
void set_dropped_down(bool dropped_down)
```

Opens or closes through the common calendar ownership state machine.

### `drop_down_alignment` (public)

```cpp
[[nodiscard]] DateTimeDropDownAlignment drop_down_alignment() const noexcept
```

Returns left-edge or right-edge popup anchoring policy.

### `set_drop_down_alignment` (public)

```cpp
void set_drop_down_alignment(DateTimeDropDownAlignment alignment)
```

Validates alignment, commits it, and repositions an open popup within client bounds.

### `font` (public)

```cpp
[[nodiscard]] FontSpec font() const noexcept
```

Returns the retained field and calendar FontSpec.

### `set_font` (public)

```cpp
void set_font(FontSpec font)
```

Validates typography, refreshes open popup presentation, and invalidates geometry and paint.

### `style` (public)

```cpp
[[nodiscard]] const BasicControlStyle& style() const noexcept
```

Returns the compatibility field/calendar BasicControlStyle.

### `set_style` (public)

```cpp
void set_style(BasicControlStyle style)
```

Commits compatibility colors and refreshes field and open popup appearance.

### `value_changed` (public)

```cpp
[[nodiscard]] Event<DateTimeValue>& value_changed() noexcept
```

Returns the event published after authoritative value commits.

### `checked_changed` (public)

```cpp
[[nodiscard]] Event<bool>& checked_changed() noexcept
```

Returns the event published after optional checked state commits.

### `drop_down_changed` (public)

```cpp
[[nodiscard]] Event<bool>& drop_down_changed() noexcept
```

Returns the event published after transient calendar ownership opens or closes.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records checkbox, formatted value, focus, and either spin or disclosure affordance from retained state.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Qualifies checkbox toggling, day stepping, field focus, and popup disclosure by local part.

### `on_key` (public)

```cpp
void on_key(KeyEvent& event) override
```

Handles open/close, day stepping, commit, cancellation, and checked-state toggling.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Commits focus appearance while transient lifetime remains governed by owner/revocation policy.

### `on_activate` (public)

```cpp
void on_activate() override
```

Invokes the primary check, spin, or calendar action through normal state transitions.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a date-time picker or spin role with formatted/civil value, checked/expanded state, range, and actions.

### `on_semantic_action` (public)

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Routes press, expand/collapse, check, increment/decrement, and set-value through ordinary validation and events.

### `on_detached_from_window` (protected)

```cpp
void on_detached_from_window() noexcept override
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `part_at` (private)

```cpp
[[nodiscard]] HitPart part_at(Point absolute) const noexcept
```

Reports the current part at value without mutation.

### `step_days` (private)

```cpp
void step_days(int days)
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `open_drop_down` (private)

```cpp
void open_drop_down()
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `close_drop_down` (private)

```cpp
void close_drop_down()
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `commit_popup_value` (private)

```cpp
void commit_popup_value(DateTimeValue value)
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_popup_revoked` (private)

```cpp
void on_popup_revoked()
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
