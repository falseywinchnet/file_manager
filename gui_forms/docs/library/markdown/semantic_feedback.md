# SemanticFeedback

- Status: **OBSERVED: bundle 010 semantic feedback split; focused M4 core tests pass**
- Kind: **class**
- Hierarchy: `Component → SemanticFeedback`
- Declaration: `include/gui_forms/feedback/semantic_feedback/semantic_feedback.hpp:18`
- Definition: `src/core/feedback/semantic_feedback/semantic_feedback.cpp`

SemanticFeedback turns one-shot application transitions into bounded traceable host cues without treating ordinary visual interaction as semantic sound.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `SemanticFeedback` (public)

```cpp
explicit SemanticFeedback(Window& window, Clock clock =
```

Binds a Window and installs either the supplied deterministic clock or steady-clock nanoseconds.

### `emit` (public)

```cpp
[[nodiscard]] SemanticFeedbackRecord emit(SemanticFeedbackKind kind)
```

Verifies affinity, establishes a strictly increasing timestamp, asks the host to present the mapped cue, bounds history, and publishes the record.

### `records` (public)

```cpp
[[nodiscard]] std::span<const SemanticFeedbackRecord> records() const noexcept
```

Returns retained chronological feedback.

### `dropped_record_count` (public)

```cpp
[[nodiscard]] std::uint64_t dropped_record_count() const noexcept
```

Returns the number evicted by the history bound.

### `maximum_records` (public)

```cpp
[[nodiscard]] std::size_t maximum_records() const noexcept
```

Returns the active history capacity.

### `set_maximum_records` (public)

```cpp
void set_maximum_records(std::size_t maximum)
```

Validates 1 through 65536 and evicts oldest excess records.

### `clear` (public)

```cpp
void clear()
```

Clears history and the eviction count on the owner thread.

### `trace` (public)

```cpp
[[nodiscard]] std::string trace() const
```

Serializes stable line-oriented feedback diagnostics.

### `emitted` (public)

```cpp
[[nodiscard]] Event<const SemanticFeedbackRecord&>& emitted() noexcept
```

Returns the committed-record event.

### `cue_for` (public)

```cpp
[[nodiscard]] static HostSoundCue cue_for( SemanticFeedbackKind kind) noexcept
```

Maps the closed semantic-kind vocabulary to a host cue.
