# HeadlessHostServices

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `HostServices → HeadlessHostServices`  
Declaration: `src/host/headless/headless_host.hpp:14`  
Definition: `src/host/headless/headless_host.cpp`

HeadlessHostServices is a class declared in src/host/headless/headless_host.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `HeadlessHostServices`

```cpp
HeadlessHostServices()
```

Constructs or tears down the retained HeadlessHostServices object according to its ownership contract.

### `~HeadlessHostServices`

```cpp
~HeadlessHostServices() override
```

Constructs or tears down the retained HeadlessHostServices object according to its ownership contract.

### `queue_dialog_result`

```cpp
void queue_dialog_result(HostDialogResult result)
```

Public HeadlessHostServices operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_dialog_handler`

```cpp
void set_dialog_handler(DialogHandler handler)
```

Synchronously updates the retained dialog handler property. Validation, typed invalidation, and notifications are defined by the implementation.

### `dialog_trace`

```cpp
[[nodiscard]] const std::string& dialog_trace() const noexcept
```

Reports the current dialog trace value without mutation.

### `sound_trace`

```cpp
[[nodiscard]] const std::string& sound_trace() const noexcept
```

Reports the current sound trace value without mutation.
