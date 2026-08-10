# FormControl

- Status: **generated inventory; detailed review pending**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → FormControl`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:99`
- Definition: `inline/header-only`

FormControl is a visual retained control declared in src/abi/control_adapters/abi_control_adapters.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `FormControl` (public)

```cpp
explicit FormControl(StableId stable_id) : Panel(std::move(stable_id))
```

Constructs or tears down the retained FormControl object according to its ownership contract.

### `key_preview` (public)

```cpp
[[nodiscard]] gui_forms::Event<RasterKeySample&>& key_preview() noexcept
```

Public FormControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_key_preview` (public)

```cpp
void on_key_preview(gui_forms::KeyEvent& event) override
```

Public FormControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
