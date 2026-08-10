# HostFileDialogFilter

- Status: **OBSERVED: bundle 007 typed-dialog value review; M4 builds and focused tests pass**
- Kind: **struct**
- Hierarchy: `HostFileDialogFilter`
- Declaration: `include/gui_forms/host/types/host_types.hpp:154`
- Definition: `inline/header-only`

HostFileDialogFilter carries a user-facing label and bounded extension vocabulary without native filter syntax.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const HostFileDialogFilter&, const HostFileDialogFilter&) = default
```

Compares label and ordered extension vocabulary.
