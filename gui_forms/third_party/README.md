# Private third-party dependencies

Skia is fetched locally into `skia/` and is deliberately not committed as a
source copy. GUI.Forms pins the following upstream revisions:

- Skia `chrome/m152`: `2a9b593bab4b2fd019fa494c8d401ff1fab0b883`
- Skia DEPS libpng: `d5515b5b8be3901aac04e5bd8bd5c89f287bcd33`
- Skia DEPS zlib: `646b7f569718921d7d4b5b8e22572ff6c76f2596`
- GN fetched by that Skia checkout: `2175 (b2afae122eeb)`

`skia_cpu_args.gn` is the audited build profile. It compiles the CPU raster
library and CoreText font host, retains PNG decode/encode, and explicitly
excludes Ganesh, Graphite, every graphics-device API, non-PNG decoders, PDF,
SVG, Skottie, SkParagraph dependencies, tracing, tools, and Rust targets.

Upstream's main `:skia` target unconditionally compiles legacy BMP/WBMP and
folds ICO into its libpng target. `skia_png_only.patch` reproducibly removes
those three decoder objects and their public feature defines. This is a local
policy patch, not a claim about an upstream-supported preset.

The fetched tree may contain excluded source. The build must not compile, link,
initialize, probe, or expose it. No Skia type may leave `src/render/skia/`.
