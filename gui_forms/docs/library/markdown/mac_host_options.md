# MacHostOptions

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `MacHostOptions`  
Declaration: `include/gui_forms/platform/macos_host.hpp:15`  
Definition: `inline/header-only`

MacHostOptions is a struct declared in include/gui_forms/platform/macos_host.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `void`

```cpp
std::function<void(HostCloseRequest&)> close_request
```

Public MacHostOptions operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `void`

```cpp
std::function<void(std::function<void()> wake, std::function<void()> request_close, std::function<HostDialogResult(const HostDialogRequest&)> show_dialog, std::function<HostServiceStatus(const HostTooltipRequest&)> show_tooltip, std::function<void()> hide_tooltip, std::function<HostClipboardTextResult()> read_clipboard_text, std::function<HostServiceStatus(std::string_view)> write_clipboard_text)> host_ready
```

Public MacHostOptions operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `void`

```cpp
std::function<void()> dispatch_pending
```

Public MacHostOptions operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `void`

```cpp
std::function<void()> closed
```

Public MacHostOptions operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `void`

```cpp
std::function<void(std::string_view metrics_json, std::string_view host_json)> final_snapshot
```

Public MacHostOptions operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
