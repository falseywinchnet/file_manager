# PropertyConversionContext

- Status: **OBSERVED: bundle 006 conversion-policy split; M4 build and focused tests pass**
- Kind: **struct**
- Hierarchy: `PropertyConversionContext`
- Declaration: `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:14`
- Definition: `inline/header-only`

PropertyConversionContext is an inspector-local culture label, decimal/group separator policy, and grouping flag. It never mutates process locale, so two grids can format independently and deterministic tests remain stable.

## Visual evidence

![PropertyConversionContext](../captures/property_grid.png)

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const PropertyConversionContext&, const PropertyConversionContext&) = default
```

Compares culture name, separators, and grouping policy exactly.
