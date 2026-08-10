# SemanticFeedback

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → SemanticFeedback`  
Declaration: `include/gui_forms/feedback.hpp:43`  
Definition: `src/core/feedback.cpp`

SemanticFeedback is a class declared in include/gui_forms/feedback.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `SemanticFeedback`

```cpp
explicit SemanticFeedback(Window& window, Clock clock =
```

Constructs or tears down the retained SemanticFeedback object according to its ownership contract.

### `emit`

```cpp
[[nodiscard]] SemanticFeedbackRecord emit(SemanticFeedbackKind kind)
```

Public SemanticFeedback operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `records`

```cpp
[[nodiscard]] std::span<const SemanticFeedbackRecord> records() const noexcept
```

Reports the current records value without mutation.

### `dropped_record_count`

```cpp
[[nodiscard]] std::uint64_t dropped_record_count() const noexcept
```

Reports the current dropped record count value without mutation.

### `maximum_records`

```cpp
[[nodiscard]] std::size_t maximum_records() const noexcept
```

Reports the current maximum records value without mutation.

### `set_maximum_records`

```cpp
void set_maximum_records(std::size_t maximum)
```

Synchronously updates the retained maximum records property. Validation, typed invalidation, and notifications are defined by the implementation.

### `clear`

```cpp
void clear()
```

Public SemanticFeedback operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `trace`

```cpp
[[nodiscard]] std::string trace() const
```

Reports the current trace value without mutation.

### `emitted`

```cpp
[[nodiscard]] Event<const SemanticFeedbackRecord&>& emitted() noexcept
```

Public SemanticFeedback operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `cue_for`

```cpp
[[nodiscard]] static HostSoundCue cue_for( SemanticFeedbackKind kind) noexcept
```

Public SemanticFeedback operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
