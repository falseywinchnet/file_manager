# Bounded JPEG pixel experiment

Status: **CANDIDATE research only**. This standalone process is not linked,
installed, invoked or enabled by File Manager or GUI.Forms. It admits generated
fixture bytes only, with no document path argument. It does not implement a
preview provider or production containment. ADR-020's shipped PNG/UTF-8 lane
is unchanged.

The question is whether a portable decoder can produce bounded, correctly
oriented BGRA pixels for `Window::load_bgra32_premultiplied` at useful cost.
This specimen exercises libjpeg-turbo 3.2.0 already in the adjacent toolchain.
Windows WIC and macOS ImageIO remain comparison candidates. No production
decoder, dependency distribution profile or provider protocol is selected.

## Reproduce on Shadow

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake -S frontend/experiments/jpeg_decode -B .build/jpeg-decode -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .build/jpeg-decode --parallel 2
ctest --test-dir .build/jpeg-decode --output-on-failure
.build/jpeg-decode/file_manager_jpeg_experiment.exe --measure > .build/jpeg-decode/results.csv 2> .build/jpeg-decode/verification.txt
python -B frontend/experiments/jpeg_decode/summarize.py .build/jpeg-decode/results.csv
```

Check each exit status before using output. CMake requires exact pkg-config
`libturbojpeg=3.2.0` by default; that route neither fetches nor installs
dependencies. The adjacent toolchain is read-only. Default execution and
`--verify-only` run correctness only; timing now requires `--measure` explicitly.
Compile mode is C++20 with warnings-as-errors.

The separate `JPEG research correctness` workflow builds Windows, macOS and
Linux from the SHA-256-pinned upstream 3.2.0 archive. `fetch_codec.cmake` downloads
only into the caller's build directory. Upstream is configured, built and
installed independently, then this project consumes its exact-version CMake
package with `FILE_MANAGER_JPEG_CMAKE_PACKAGE=ON` and `CMAKE_PREFIX_PATH`.
The workflow disables SIMD and builds static libraries for correctness checks;
it supplies no comparable performance result or production packaging decision.
Native results remain pending until that workflow completes. Neither codec nor
experiment is installed into File Manager by this workflow.

## Bounds and unverified edges

- Nonempty encoded input at most 16 MiB; source sides at most 16384 and area at
  most 64 Mi pixels. Lossy 8-bit RGB/YCbCr/grayscale only.
- Codec intermediate allocation limit 64 MiB and scan limit 100. These are not
  whole-process memory or elapsed-time limits.
- Largest non-upscaling codec factor fitting 1024-square output, otherwise
  refusal. Contiguous opaque BGRA8 is at most 4 MiB. Rotation/reflection can
  temporarily retain both source and destination allocations.
- The independent, allocation-free metadata reader extracts primary IFD0
  orientation from pre-SOS EXIF APP1 data in either TIFF byte order. Missing
  orientation defaults to one. Duplicate EXIF/Orientation, invalid field type,
  count, value or extent refuse explicitly. Work stops at 1 MiB of header,
  4096 markers or 4096 IFD0 entries; encoded input is capped at 16 MiB. Other
  IFD links and unknown tag payloads are never traversed. This is a deliberately
  narrow orientation reader, not complete JPEG/EXIF conformance validation.
  The codec still validates the encoded image.
- Literal metadata fixtures cover all eight values in both byte orders,
  absence, every truncated header prefix, ambiguous/malformed cases and work
  bounds. A generated JPEG with embedded little-endian EXIF exercises all eight
  transforms against independent corner-permutation and non-square geometry
  checks. Grayscale, alpha and empty/truncated/range refusal also run.
  Independent coverage for source/ICC/precision/scan-limit refusal remains
  incomplete; no real-photo corpus is claimed.
- No ICC conversion. In the pinned
  [3.2.0 implementation](https://github.com/libjpeg-turbo/libjpeg-turbo/blob/3.2.0/src/turbojpeg.c),
  absent profile data returns `-1`, warning severity and zero size. The specimen
  recognizes this combination. Malformed/incomplete ICC behavior is unverified;
  no production color policy can be inferred from it.

Verification exercises the codec before measurement. Each group has one first
invocation and 30 warm samples; this is not a cold-process measurement. Each
sample times fresh decoder/header/scale, output allocation, decode and optional
transform. Encoding, validation, final raster destruction, process startup,
file I/O, IPC, upload and native paint are excluded. Highly compressible generated
quadrants are correctness anchors, not a photographic corpus or perceptual oracle.
"baseline-24mp" means baseline JPEG encoding, not a performance control.
The older checked-in timing results used supplied orientation arguments and
predate metadata parsing; they are not measurements of the new EXIF path.

The orientation field and default were checked against CIPA DC-008-2026,
section 4.6.5.1.6, available through the
[official Exif standards index](https://www.cipa.jp/e/std/std-sec.html).
The parser's bounds and duplicate-refusal rules are local research policy.

## Route to an actual preview

**GIVEN:** ADR-020 requires bounds, cancellation and identity/revision checks.
The native/third-party execution boundary is recorded in
`orchestrator/spec/contracts/PLUGIN_AND_CAPABILITIES.md`; ORC-PLG-003 remains
stubbed. A GUI worker thread does not supply process containment. Frontend must
not create a competing supervisor or call private plugin workers directly.

**CANDIDATE next work**, with canonical Orchestrator negotiation before adapters
freeze:

1. Compare portable and platform decoders on the same owned photo corpus:
   baseline/progressive, grayscale, EXIF orientations, ICC profiles, CMYK, large
   dimensions and ordinary refusal cases. Define admitted color/output semantics.
2. Negotiate source identity/revision, bounded input ownership, generation,
   dimensions/stride/color format, cancellation/deadline, process memory/output
   limits and failures. Decide the relation to ORC-PLG-003 explicitly.
3. Implement supervision, stale-result retirement and bounded pixel transfer to
   the existing image registry. Decoder success is not selection authority.
4. Verify real application pixels and selection-to-preview timing on all three
   platforms, rapid replacement/close behavior, and packaged dependencies/licences.

Indexed thumbnails remain a separate Engine/Orchestrator contract. This research
does not authorize background per-folder thumbnail scans. Results and source
review are in `frontend/results/2026-10-03-jpeg-decode/README.md`.
