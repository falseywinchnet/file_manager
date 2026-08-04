# Skia CPU proving-build evidence

Status: **MEASURED** for the named local build only. This record does not select
Skia as the permanent GUI.Forms renderer.

## Environment

- Date: 2026-08-03
- Host: macOS 14.8.7 (23J520), arm64
- Compiler: Apple Clang 16.0.0 (`clang-1600.0.26.6`)
- CMake: 4.2.3
- GN: 2175 (`b2afae122eeb`)
- Ninja: 1.12.1 Chromium 4
- Skia pin and dependency pins: `third_party/README.md`
- Configuration: `third_party/skia_cpu_args.gn`

## Reproduction

```text
gui_forms/third_party/fetch_skia_cpu.sh
gui_forms/third_party/build_skia_cpu.sh
cmake -S gui_forms -B gui_forms/build -DCMAKE_BUILD_TYPE=Release
cmake --build gui_forms/build --parallel
ctest --test-dir gui_forms/build --output-on-failure
```

## Negative result retained

**OBSERVED:** with optional JPEG/WebP/AVIF flags disabled, upstream Skia's main
`:skia` target still compiled legacy BMP and WBMP sources. Its libpng decode
target also compiled ICO support. The initial build completed 597 Ninja steps
in 37.9 seconds wall time and produced a 5.6 MiB `libskia.a`, but failed the
GUI.Forms PNG-only decoder boundary.

**MEASURED correction:** `skia_png_only.patch` removes the unconditional
BMP/WBMP sources, the ICO source coupled to libpng, and their public feature
defines. The rebuilt graph contained 505 compile/link steps and took 36.5
seconds wall time after the public-definition change forced a broad rebuild.
The archive policy test finds none of the forbidden decoder symbols and no
defined Ganesh/Graphite backend implementation symbol.

This patch is a maintenance cost and must be rebased or rejected explicitly
when the Skia pin changes.

## Current artifacts and tests

- `libskia.a`: 5,758,736 bytes
- `libpng.a`: 302,600 bytes
- `libzlib.a`: 97,480 bytes
- linked gallery executable: 3,270,304 bytes before code signing/packaging
- linked dynamic frameworks: CoreFoundation, CoreText, AppKit, CoreGraphics,
  Foundation, libc++, libSystem, and libobjc; no Metal or OpenGL framework
- full CTest run: four of four tests passed in 0.63 seconds wall time
- Skia smoke output:

```json
{"renderer":"skia-cpu","width":320,"height":192,"bytes":245760,"checksum":17289292855132215351}
```

The smoke test allocates a 2x CPU raster surface, decodes a valid PNG through
the private `ImageId` registry, draws fills, relief edges, a line, control text,
and the PNG, then verifies non-empty backing pixels. It does not establish
interactive latency, visual correctness, international shaping correctness, or
long-running memory behavior.

