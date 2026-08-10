# WindowsHostServices

- Status: **OBSERVED: bundle 007 Win32 service state-owner isolation; M4 MinGW build passes**
- Kind: **class**
- Hierarchy: `HostServices → WindowsHostServices`
- Declaration: `src/host/windows/services/windows_host_services.hpp:6`
- Definition: `inline/header-only`

WindowsHostServices is the source-private Win32 adapter state owner. It binds one HWND internally, maps monitor/cursor/capture/clipboard/dialog/sound operations, and leaves all validation, modal policy, accounting, and shutdown ordering in HostServices.

## Visual evidence

![WindowsHostServices](../captures/native_window_host.png)

## Declared methods

### `WindowsHostServices` (public)

```cpp
WindowsHostServices() : HostServices(windows_capabilities())
```

Constructs the Win32 portable capability set.

### `bind_owner` (public)

```cpp
void bind_owner(HWND owner) noexcept
```

Binds the private native owner handle after window creation without exposing it through portable API.

### `query_monitors_impl` (protected)

```cpp
HostMonitorResult query_monitors_impl() override
```

Enumerates Win32 monitor/work rectangles, DPI-derived scale, and primary identity.

### `set_cursor_impl` (protected)

```cpp
HostServiceStatus set_cursor_impl(CursorKind cursor) override
```

Maps a portable cursor to a shared system cursor and verifies installation.

### `set_pointer_capture_impl` (protected)

```cpp
HostServiceStatus set_pointer_capture_impl(bool captured, std::uint64_t) override
```

Synchronizes Win32 capture ownership with the retained pointer identity.

### `read_clipboard_text_impl` (protected)

```cpp
HostClipboardTextResult read_clipboard_text_impl() override
```

Reads CF_UNICODETEXT under native clipboard locking and converts it to UTF-8.

### `write_clipboard_text_impl` (protected)

```cpp
HostServiceStatus write_clipboard_text_impl(std::string_view text) override
```

Converts validated UTF-8, transfers a movable CF_UNICODETEXT allocation, and closes native ownership paths.

### `show_dialog_impl` (protected)

```cpp
HostDialogResult show_dialog_impl(const HostDialogRequest& request) override
```

Maps the typed dialog variant to Win32 message, open/save/folder, or color UI and returns the matching typed result.

### `play_sound_cue_impl` (protected)

```cpp
HostServiceStatus play_sound_cue_impl( const HostSoundCueRequest& request) override
```

Maps semantic cues to MessageBeep meanings without exposing native identifiers.

### `shutdown_impl` (protected)

```cpp
void shutdown_impl() noexcept override
```

Releases the private HWND association after portable shutdown commits.
