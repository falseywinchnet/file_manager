# FormControl

- Status: **OBSERVED: bundle 010 private ABI form-adapter split; native and MinGW ABI builds pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → FormControl`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:99`
- Definition: `inline/header-only`

FormControl is a retained Panel with a pre-focused-child key-preview bridge kept inside the compatibility adapter.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `FormControl` (public)

```cpp
explicit FormControl(StableId stable_id) : Panel(std::move(stable_id))
```

Constructs the retained panel surface.

### `key_preview` (public)

```cpp
[[nodiscard]] gui_forms::Event<RasterKeySample&>& key_preview() noexcept
```

Returns mutable compatibility key-preview observation.

### `on_key_preview` (public)

```cpp
void on_key_preview(gui_forms::KeyEvent& event) override
```

Projects a core key event, emits preview, and copies handled state back.
