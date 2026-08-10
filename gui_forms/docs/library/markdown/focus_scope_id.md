# FocusScopeId

- Status: **OBSERVED: bundle 007 focus-scope identity review; M4 build and focused tests pass**
- Kind: **struct**
- Hierarchy: `FocusScopeId`
- Declaration: `include/gui_forms/window/window.hpp:153`
- Definition: `inline/header-only`

FocusScopeId is a nonzero monotonically assigned transient-containment identity.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit constexpr operator bool() const noexcept
```

Reports whether the scope identity is nonzero.

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const FocusScopeId&, const FocusScopeId&) = default
```

Orders and compares exact scope identity.
