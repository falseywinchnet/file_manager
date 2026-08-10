# SurfaceMaterial

- Status: **OBSERVED: bundle 009 surface-material aggregate review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `SurfaceMaterial`
- Declaration: `include/gui_forms/surface_material/types/surface_material_types.hpp:25`
- Definition: `inline/header-only`

SurfaceMaterial is a bounded ordered fill stack plus optional border, bounded shadow stack, and shared corner radius.

## Visual evidence

![SurfaceMaterial](../captures/drawing_raster_material.png)

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const SurfaceMaterial&, const SurfaceMaterial&) = default
```

Compares every fill, shadow, border, and radius property.
