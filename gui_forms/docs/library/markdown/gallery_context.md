# GalleryContext

- Status: **OBSERVED: bundle 012 hierarchy-isolated demo declaration; retained in one source-private composition unit**
- Kind: **class**
- Hierarchy: `GalleryContext`
- Declaration: `src/controls/gallery/context/gallery_context.hpp:11`
- Definition: `src/controls/gallery_controls.cpp`

GalleryContext synchronizes shared sample state across the native gallery's source-private demonstration controls.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `synchronize` (public)

```cpp
void synchronize(std::string_view cause)
```

Executes GalleryContext's synchronize operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
