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
.build/jpeg-decode/file_manager_jpeg_experiment.exe > .build/jpeg-decode/results.csv 2> .build/jpeg-decode/verification.txt
python -B frontend/experiments/jpeg_decode/summarize.py .build/jpeg-decode/results.csv
```

Check each exit status before using output. CMake requires exact pkg-config
`libturbojpeg=3.2.0`; it neither fetches nor installs dependencies. The adjacent
toolchain is read-only. Other platforms have not run this specimen. Compile mode
is C++20 with warnings-as-errors.

## Bounds and unverified edges

- Nonempty encoded input at most 16 MiB; source sides at most 16384 and area at
  most 64 Mi pixels. Lossy 8-bit RGB/YCbCr/grayscale only.
- Codec intermediate allocation limit 64 MiB and scan limit 100. These are not
  whole-process memory or elapsed-time limits.
- Largest non-upscaling codec factor fitting 1024-square output, otherwise
  refusal. Contiguous opaque BGRA8 is at most 4 MiB. Rotation/reflection can
  temporarily retain both source and destination allocations.
- Orientation is an argument, **not parsed EXIF**. All eight transforms have
  independent corner-permutation and non-square geometry checks. Grayscale,
  alpha, empty/truncated input and invalid argument checks also run. Independent
  fixture coverage for source/ICC/precision/scan-limit refusal is incomplete.
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
