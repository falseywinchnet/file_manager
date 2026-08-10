# DateTimeFormatProvider

- Status: **OBSERVED: bundle 004 value-provider split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **struct**
- Hierarchy: `DateTimeFormatProvider`
- Declaration: `include/gui_forms/controls/panel/date_time_picker/date_time_format_provider/date_time_format_provider.hpp:10`
- Definition: `src/controls/panel/date_time_picker/date_time_format_provider/date_time_format_provider.cpp`

DateTimeFormatProvider is an immutable caller-substitutable vocabulary and pattern bundle for DateTimePicker formatting. It separates locale-shaped presentation from civil-value authority without introducing a platform locale runtime dependency.

## Visual evidence

![DateTimeFormatProvider](../captures/date_time_picker.png)

## Declared methods

### `english_united_states` (public)

```cpp
[[nodiscard]] static DateTimeFormatProvider english_united_states()
```

Returns the library's canonical English month/day names and short, long, time, and month-year patterns.
