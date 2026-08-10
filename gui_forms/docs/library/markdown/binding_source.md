# BindingSource

- Status: **OBSERVED: bundle 010 source/edit state-machine split; focused M4 binding and inspection tests pass**
- Kind: **class**
- Hierarchy: `Component → enable_shared_from_this → BindingSource`
- Declaration: `include/gui_forms/binding/binding_source/binding_source.hpp:26`
- Definition: `src/core/binding/binding_source/binding_source.cpp`

BindingSource owns stable rows, currency, edit snapshots, list/error events, suspension coalescing, active bindings, and exact owner-Window affinity.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `BindingSource` (public)

```cpp
explicit BindingSource(Window& window)
```

Binds Window lifetime, verifies affinity, and creates its sole CurrencyManager.

### `~BindingSource` (public)

```cpp
~BindingSource() override
```

Disposes while its owner remains valid and contains destructor exceptions.

### `set_records` (public)

```cpp
void set_records(std::vector<BindingRecord> records, bool metadata_changed = false)
```

Normalizes and validates a complete candidate list before atomically replacing rows, currency, edit state, and publication.

### `records` (public)

```cpp
[[nodiscard]] std::span<const BindingRecord> records() const noexcept
```

Returns the retained ordered row view.

### `count` (public)

```cpp
[[nodiscard]] std::size_t count() const noexcept
```

Returns row cardinality.

### `position` (public)

```cpp
[[nodiscard]] std::ptrdiff_t position() const noexcept
```

Returns currency or -1.

### `set_position` (public)

```cpp
bool set_position(std::ptrdiff_t position)
```

Validates owner access and admitted range, commits currency, and publishes ordered transitions.

### `move_first` (public)

```cpp
bool move_first()
```

Moves currency to the first row when present.

### `move_last` (public)

```cpp
bool move_last()
```

Moves currency to the last row when present.

### `move_next` (public)

```cpp
bool move_next()
```

Moves currency forward without passing the last row.

### `move_previous` (public)

```cpp
bool move_previous()
```

Moves currency backward without passing the first row.

### `current` (public)

```cpp
[[nodiscard]] const BindingRecord* current() const noexcept
```

Returns the current row or null for empty/invalid currency.

### `current_field` (public)

```cpp
[[nodiscard]] std::optional<BindingValue> current_field( std::string_view field) const
```

Canonicalizes a field name and returns the current value when present.

### `current_error` (public)

```cpp
[[nodiscard]] std::string current_error(std::string_view field) const
```

Returns field-specific error, falling back to record-wide error.

### `bindings` (public)

```cpp
[[nodiscard]] std::vector<std::shared_ptr<Binding>> bindings() const
```

Locks and returns the currently live registered bindings.

### `set_current_field` (public)

```cpp
bool set_current_field(std::string_view field, BindingValue value)
```

Validates edit policy and recursive value shape, commits one changed field, and publishes row/error transfer state.

### `add` (public)

```cpp
std::size_t add(BindingRecord record)
```

Appends one normalized admitted row.

### `insert` (public)

```cpp
std::size_t insert(std::size_t index, BindingRecord record)
```

Normalizes and validates one row, preserves stable identity uniqueness, adjusts currency, and publishes addition.

### `remove_at` (public)

```cpp
bool remove_at(std::size_t index)
```

Enforces remove policy, updates currency/edit state, and publishes removal plus current transition.

### `remove_current` (public)

```cpp
bool remove_current()
```

Removes current currency when present.

### `clear` (public)

```cpp
void clear()
```

Clears rows, currency, and edit state and publishes one reset/current transition.

### `find` (public)

```cpp
[[nodiscard]] std::optional<std::size_t> find( std::string_view field, const BindingValue& value) const
```

Returns the first row whose canonical field equals the requested value.

### `allow_edit` (public)

```cpp
[[nodiscard]] bool allow_edit() const noexcept
```

Returns the source-wide edit gate.

### `set_allow_edit` (public)

```cpp
void set_allow_edit(bool allow)
```

Commits edit policy and cancels an active edit when disabled.

### `allow_new` (public)

```cpp
[[nodiscard]] bool allow_new() const noexcept
```

Returns the source-wide insertion gate.

### `set_allow_new` (public)

```cpp
void set_allow_new(bool allow)
```

Commits insertion policy.

### `allow_remove` (public)

```cpp
[[nodiscard]] bool allow_remove() const noexcept
```

Returns the source-wide removal gate.

### `set_allow_remove` (public)

```cpp
void set_allow_remove(bool allow)
```

Commits removal policy.

### `begin_edit` (public)

```cpp
bool begin_edit()
```

Captures the current stable row once when row and policy permit.

### `cancel_edit` (public)

```cpp
void cancel_edit()
```

Restores the captured stable row even if currency moved, then publishes the rollback.

### `end_edit` (public)

```cpp
void end_edit()
```

Discards the edit snapshot as committed.

### `binding_suspended` (public)

```cpp
[[nodiscard]] bool binding_suspended() const noexcept
```

Reports automatic source publication suspension.

### `suspend_binding` (public)

```cpp
void suspend_binding()
```

Begins suspension without nesting side effects.

### `resume_binding` (public)

```cpp
void resume_binding()
```

Ends suspension and emits one coalesced pending reset.

### `raise_list_changed_events` (public)

```cpp
[[nodiscard]] bool raise_list_changed_events() const noexcept
```

Returns whether public list events are enabled.

### `set_raise_list_changed_events` (public)

```cpp
void set_raise_list_changed_events(bool raise) noexcept
```

Changes list-event publication policy without mutating data.

### `reset_bindings` (public)

```cpp
void reset_bindings(bool metadata_changed = false)
```

Publishes a reset with explicit metadata-change classification.

### `reset_current_item` (public)

```cpp
void reset_current_item()
```

Publishes a changed event for current currency.

### `reset_item` (public)

```cpp
bool reset_item(std::size_t index)
```

Publishes a changed event for one admitted row index.

### `data_member` (public)

```cpp
[[nodiscard]] const std::string& data_member() const noexcept
```

Returns the canonical source member name.

### `set_data_member` (public)

```cpp
void set_data_member(std::string member)
```

Canonicalizes changed member identity and publishes source/member/reset events.

### `currency_manager` (public)

```cpp
[[nodiscard]] CurrencyManager& currency_manager() noexcept
```

Returns the source-owned currency facade.

### `currency_manager` (public)

```cpp
[[nodiscard]] const CurrencyManager& currency_manager() const noexcept
```

Returns the source-owned currency facade.

### `snapshot` (public)

```cpp
[[nodiscard]] BindingSourceSnapshot snapshot() const
```

Copies currency, revision, activity, suspension, edit, and publication telemetry.

### `list_changed` (public)

```cpp
[[nodiscard]] Event<const BindingListChange&>& list_changed() noexcept
```

Returns public list-change observation.

### `current_changed` (public)

```cpp
[[nodiscard]] Event<>& current_changed() noexcept
```

Returns current-record observation.

### `current_item_changed` (public)

```cpp
[[nodiscard]] Event<>& current_item_changed() noexcept
```

Returns current-record content observation.

### `position_changed` (public)

```cpp
[[nodiscard]] Event<std::ptrdiff_t>& position_changed() noexcept
```

Returns numeric currency observation.

### `data_error` (public)

```cpp
[[nodiscard]] Event<const std::string&>& data_error() noexcept
```

Returns source error observation.

### `data_source_changed` (public)

```cpp
[[nodiscard]] Event<>& data_source_changed() noexcept
```

Returns full-source identity observation.

### `data_member_changed` (public)

```cpp
[[nodiscard]] Event<const std::string&>& data_member_changed() noexcept
```

Returns member-name observation.

### `binding_complete` (public)

```cpp
[[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept
```

Returns aggregate binding completion observation.

### `disposed_event` (public)

```cpp
[[nodiscard]] Event<>& disposed_event() noexcept
```

Returns the retirement observation used by BindingContext and bindings.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Enforces Window affinity when available.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Retires bindings, events, records, manager, edit state, and Window lifetime without callbacks into a disposed owner.

### `require_access` (private)

```cpp
void require_access(std::string_view operation) const
```

Rejects disposed, retired-Window, or wrong-thread operations.

### `normalize_record` (private)

```cpp
static void normalize_record(BindingRecord& record)
```

Canonicalizes field/error names and validates identity, recursive values, and error UTF-8.

### `validate_records` (private)

```cpp
static void validate_records(std::vector<BindingRecord>& records)
```

Normalizes a complete candidate list and rejects duplicate stable identities.

### `publish_model_change` (private)

```cpp
void publish_model_change(const BindingListChange& change)
```

Advances revision and either defers/coalesces or emits model/list changes according to suspension and publication policy.

### `publish_current_transition` (private)

```cpp
void publish_current_transition(std::ptrdiff_t old_position)
```

Emits position/current events only after a real currency transition.

### `register_binding` (private)

```cpp
void register_binding(const std::shared_ptr<Binding>& binding)
```

Adds one live binding weakly without duplicate identity.

### `unregister_binding` (private)

```cpp
void unregister_binding(const Binding* binding) noexcept
```

Removes one binding identity and expired registrations.

### `transfer_bindings` (private)

```cpp
bool transfer_bindings(bool source_to_control)
```

Snapshots registrations, performs every live directional transfer, aggregates acceptance, and prunes expiration.

### `bound_window` (private)

```cpp
[[nodiscard]] Window* bound_window() const noexcept
```

Resolves weak Window lifetime without extending ownership.
