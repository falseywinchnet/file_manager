# HostServices

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `HostServices`  
Declaration: `include/gui_forms/host.hpp:286`  
Definition: `src/core/host.cpp`

HostServices is a class declared in include/gui_forms/host.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `HostServices`

```cpp
explicit HostServices(HostCapabilities capabilities)
```

Constructs or tears down the retained HostServices object according to its ownership contract.

### `~HostServices`

```cpp
virtual ~HostServices() = default
```

Constructs or tears down the retained HostServices object according to its ownership contract.

### `HostServices`

```cpp
HostServices(const HostServices&) = delete
```

Constructs or tears down the retained HostServices object according to its ownership contract.

### `operator=`

```cpp
HostServices& operator=(const HostServices&) = delete
```

Public HostServices operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `query_monitors`

```cpp
[[nodiscard]] HostMonitorResult query_monitors()
```

Public HostServices operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_cursor`

```cpp
[[nodiscard]] HostServiceStatus set_cursor(CursorKind cursor)
```

Synchronously updates the retained cursor property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_pointer_capture`

```cpp
[[nodiscard]] HostServiceStatus set_pointer_capture(bool captured, std::uint64_t pointer_id = 1)
```

Synchronously updates the retained pointer capture property. Validation, typed invalidation, and notifications are defined by the implementation.

### `read_clipboard_text`

```cpp
[[nodiscard]] HostClipboardTextResult read_clipboard_text()
```

Public HostServices operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `write_clipboard_text`

```cpp
[[nodiscard]] HostServiceStatus write_clipboard_text(std::string_view text_utf8)
```

Public HostServices operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `show_dialog`

```cpp
[[nodiscard]] HostDialogResult show_dialog(const HostDialogRequest& request)
```

Public HostServices operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `play_sound_cue`

```cpp
[[nodiscard]] HostServiceStatus play_sound_cue( const HostSoundCueRequest& request)
```

Public HostServices operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `shutdown`

```cpp
void shutdown() noexcept
```

Public HostServices operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `snapshot`

```cpp
[[nodiscard]] HostServicesSnapshot snapshot() const
```

Reports the current snapshot value without mutation.

### `modal_changed`

```cpp
[[nodiscard]] Event<const HostModalTransition&>& modal_changed() noexcept
```

Public HostServices operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
