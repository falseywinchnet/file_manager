# ControlVisualRecipe

- Status: **OBSERVED: bundle 009 visual recipe review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ControlVisualRecipe`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:47`
- Definition: `inline/header-only`

ControlVisualRecipe combines a layered material with text/glyph colors, focus/default rings, and pressed content displacement.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const ControlVisualRecipe&, const ControlVisualRecipe&) = default
```

Compares the complete recipe.
