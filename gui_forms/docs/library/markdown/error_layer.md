# ErrorLayer

- Status: **OBSERVED: bundle 005 source-private overlay split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → ErrorLayer`
- Declaration: `src/controls/guidance/error_layer/error_layer.hpp:7`
- Definition: `src/controls/guidance/error_layer/error_layer.cpp`

ErrorLayer is ErrorProvider's source-private full-client overlay. It hosts one ErrorGlyph at provider-resolved bounds and rejects hit testing so adornment cannot steal control input.

## Visual evidence

![ErrorLayer](../captures/tool_tip_error_provider.png)

## Declared methods

### `ErrorLayer` (public)

```cpp
explicit ErrorLayer(StableId stable_id)
```

Constructs an input-transparent full-client panel for a target-owned error glyph.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(Point point) const override
```

Always rejects local hit testing so the adorned control remains the input authority.
