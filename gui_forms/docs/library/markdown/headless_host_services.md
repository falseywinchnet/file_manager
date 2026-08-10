# HeadlessHostServices

- Status: **OBSERVED: bundle 007 deterministic service adapter split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `HostServices → HeadlessHostServices`
- Declaration: `src/host/headless/services/headless_host_services.hpp:12`
- Definition: `src/host/headless/services/headless_host_services.cpp`

HeadlessHostServices is the deterministic reference adapter for portable monitor, cursor/capture, clipboard, typed dialogs, and semantic sounds; it records textual traces instead of owning native state.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `HeadlessHostServices` (public)

```cpp
HeadlessHostServices()
```

Constructs the reference capability set and one deterministic primary monitor.

### `~HeadlessHostServices` (public)

```cpp
~HeadlessHostServices() override
```

Runs portable shutdown before releasing fixtures.

### `queue_dialog_result` (public)

```cpp
void queue_dialog_result(HostDialogResult result)
```

Queues one deterministic typed result for the next dialog request.

### `set_dialog_handler` (public)

```cpp
void set_dialog_handler(DialogHandler handler)
```

Installs a programmable nested-dialog test handler.

### `dialog_trace` (public)

```cpp
[[nodiscard]] const std::string& dialog_trace() const noexcept
```

Returns stable request/outcome trace text.

### `sound_trace` (public)

```cpp
[[nodiscard]] const std::string& sound_trace() const noexcept
```

Returns stable semantic cue/time/gain trace text.

### `query_monitors_impl` (protected)

```cpp
[[nodiscard]] HostMonitorResult query_monitors_impl() override
```

Returns the deterministic monitor set.

### `set_cursor_impl` (protected)

```cpp
[[nodiscard]] HostServiceStatus set_cursor_impl(CursorKind cursor) override
```

Acknowledges portable cursor mapping without native state.

### `set_pointer_capture_impl` (protected)

```cpp
[[nodiscard]] HostServiceStatus set_pointer_capture_impl( bool captured, std::uint64_t pointer_id) override
```

Acknowledges portable capture mapping without native state.

### `read_clipboard_text_impl` (protected)

```cpp
[[nodiscard]] HostClipboardTextResult read_clipboard_text_impl() override
```

Returns deterministic clipboard text, presence, and generation.

### `write_clipboard_text_impl` (protected)

```cpp
[[nodiscard]] HostServiceStatus write_clipboard_text_impl( std::string_view text_utf8) override
```

Commits deterministic clipboard text and advances generation.

### `show_dialog_impl` (protected)

```cpp
[[nodiscard]] HostDialogResult show_dialog_impl( const HostDialogRequest& request) override
```

Uses the installed handler or queued result and records a stable typed trace.

### `play_sound_cue_impl` (protected)

```cpp
[[nodiscard]] HostServiceStatus play_sound_cue_impl( const HostSoundCueRequest& request) override
```

Records the already-qualified semantic cue request.

### `shutdown_impl` (protected)

```cpp
void shutdown_impl() noexcept override
```

Clears deterministic clipboard, dialog queue, and sound trace.
