# PropertyRowSpec

- Status: **OBSERVED: bundle 012 isolated authored-row declaration/definition; native and MinGW M4 builds pass**
- Kind: **struct**
- Hierarchy: `PropertyRowSpec`
- Declaration: `include/gui_forms/inspection/property_row_spec/property_row_spec.hpp:12`
- Definition: `src/controls/inspection/property_row_spec/property_row_spec.cpp`

PropertyRowSpec is the authored settings-row model: stable identity; name/value/description; stock editor/choices; validation; parent/depth disclosure; enable/required state; and reset capability.

## Visual evidence

![PropertyRowSpec](../captures/property_grid.png)

## Declared methods

### `PropertyRowSpec` (public)

```cpp
PropertyRowSpec() = default
```

Default-constructs an empty row or constructs the complete authored row model from explicitly named fields.

### `PropertyRowSpec` (public)

```cpp
PropertyRowSpec(std::string authored_stable_id, std::string authored_name, std::string authored_value, std::string authored_description =
```

Default-constructs an empty row or constructs the complete authored row model from explicitly named fields.
