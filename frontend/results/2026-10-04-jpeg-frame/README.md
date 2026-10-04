# JPEG preparation through the candidate byte frame

2026-10-04 UTC. **CANDIDATE joined experiment**, no product format/provider grant.
The existing standalone JPEG specimen now emits its actual decoded pixels through
the byte frame defined by Orchestrator's `SELECTED_PREVIEW_FRAME_DRAFT.md`. An
independent Rust process receives that stream and checks dimensions and color
anchors. This connects real codec preparation and receiver validation; it does
not connect native process supervision, file authority or the frontend GUI.

## Workload and expected output

Two closed modes generate a 2048x1536 four-quadrant RGB image, JPEG-encode it,
insert a little-endian EXIF orientation marker, then run the existing bounded
JPEG/EXIF/ICC preparation code:

- `--emit-frame N`: baseline JPEG, no ICC profile, assumed sRGB.
- `--emit-color-frame N`: progressive JPEG with a linear RGB ICC profile.

`N` is one character from 1 to 8. No file path, arbitrary dimensions, provider,
script, user bytes or command can be supplied. The color mode combines embedded
EXIF and ICC in the same source, extending the earlier separately tested paths.
The codec selects half scale: 1024x768 for orientations 1–4 and 768x1024 for 5–8,
3,145,728 opaque BGRA bytes per result. Header ticket is the inert fixture 7/11.

The writer checks positive axes, tight stride, exact vector extent and every
alpha byte before emitting its 64-byte header. It writes explicit little-endian
integers, then the raster, checking both writes and flush. A failure propagates
to the specimen's existing nonzero exit/stderr boundary; truncated output cannot
be accepted by the receiver. A write failure after a complete frame still needs
the future host's independent process-status check before publication.

Rust `check_jpeg` uses the existing bounded receiver, then compares scaled/
oriented dimensions and four interior color anchors. Its orientation table and
scalar linear-to-sRGB equation are independent of the C++ EXIF/ICC functions.
Tolerance is four channel units for JPEG loss plus color rounding. Every alpha
byte is checked by the receiver. It does not compare every RGB pixel against a
photographic oracle, prove arbitrary ICC profiles, or establish monitor accuracy.

## Local result and verification scope

**MEASURED Windows Shadow:** all 16 streams passed using GCC 16.2.0 and Rust
1.98.1, one compiler job. The existing three C++ JPEG/EXIF/ICC/WIC CTest entries
passed in 1.95 seconds. Rust Clippy passes all targets/features with warnings
denied; rustfmt and the 12-file C++ spelling scan pass. Timings are test-command
observations, not selection-to-display or codec performance measurements.

Source review covers new `preview_frame.hpp/.cpp`, the generated EXIF helper and
closed emit-mode changes in `jpeg_experiment.cpp`, CMake/workflow changes and the
new Rust `check_jpeg` consumer. The full programming house style applies. Earlier
review of untouched decoder/color code is recorded separately; this change does
not declare all legacy frontend code compliant. Independent review and native
matrix results remain pending at this checkpoint.

## Remaining integration

The shell pipeline is fixed test orchestration with failure propagation; it is
not product process launching, resource confinement or deadline enforcement.
The codec generates and encodes its own input, so encoder memory must not be
mistaken for a decoder-only footprint. No claim of aggregate-copy admission,
real-file revision safety, cancellation during stream transfer, descendant
cleanup, peer authentication, source-revocation policy or visible GUI pixels is
made. The provider decision and corresponding runtime integration remain open.
