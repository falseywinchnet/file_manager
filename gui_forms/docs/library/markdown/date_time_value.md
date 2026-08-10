# DateTimeValue

- Status: **OBSERVED: bundle 004 value-type split and calendar arithmetic review; M4 build, focused tests, and Screen Sharing pass**
- Kind: **struct**
- Hierarchy: `DateTimeValue`
- Declaration: `include/gui_forms/controls/panel/date_time_picker/date_time_value/date_time_value.hpp:8`
- Definition: `inline/header-only`

DateTimeValue is DateTimePicker's timezone-free civil date/time value. Ordering is lexicographic across validated year-through-second fields so range constraint and calendar selection remain deterministic.

## Visual evidence

![DateTimeValue](../captures/date_time_picker.png)

## Declared methods

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const DateTimeValue&, const DateTimeValue&) = default
```

Orders values chronologically by year, month, day, hour, minute, then second.
