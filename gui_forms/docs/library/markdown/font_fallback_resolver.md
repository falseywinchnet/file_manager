# FontFallbackResolver

- Status: **OBSERVED: bundle 009 fallback interface split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `FontFallbackResolver`
- Declaration: `include/gui_forms/text_shaping/font_fallback_resolver/font_fallback_resolver.hpp:9`
- Definition: `inline/header-only`

FontFallbackResolver is the policy boundary that selects bounded coverage for a missing scalar cluster without consulting the host implicitly.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `~FontFallbackResolver` (public)

```cpp
virtual ~FontFallbackResolver() = default
```

Provides polymorphic destruction.

### `resolve` (public)

```cpp
[[nodiscard]] virtual std::optional<FontFallbackMatch> resolve( const FontFallbackRequest& request) = 0
```

Returns an admitted face and coverage count, or no fallback.
