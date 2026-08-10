# ToolTipLayer

- Status: **OBSERVED: bundle 005 source-private state-machine split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → ToolTipLayer`
- Declaration: `src/controls/tool_tip/tool_tip_layer/tool_tip_layer.hpp:7`
- Definition: `src/controls/tool_tip/tool_tip_layer/tool_tip_layer.cpp`

ToolTipLayer is ToolTip's source-private full-client overlay boundary. It deliberately rejects hit testing so presentation cannot intercept application input, while the provider and Window retain lifetime authority.

## Visual evidence

![ToolTipLayer](../captures/tool_tip_error_provider.png)

## Declared methods

### `ToolTipLayer` (public)

```cpp
explicit ToolTipLayer(StableId stable_id)
```

Constructs a source-private full-client panel with an input-transparent cursor contract.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(Point point) const override
```

Always rejects local hit testing so pointer routing passes through the tooltip overlay.
