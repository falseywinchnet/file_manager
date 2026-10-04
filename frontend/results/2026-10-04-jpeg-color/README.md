# JPEG color preparation checkpoint

2026-10-04 UTC. **CANDIDATE research only.** No shipped application format,
Orchestrator provider, GUI.Forms dependency, installer or release changes.

## Question and control

The prior JPEG specimen rejected every embedded ICC profile. Test whether
bounded RGB/grayscale input can produce useful sRGB preview pixels without
silently ignoring that metadata. Untagged JPEG remains the decoded-sample
control. Generated sRGB profiles should preserve neutral bytes; generated
linear RGB/gray profiles should follow the independent piecewise sRGB transfer
equation. Tolerance is two 8-bit steps for neutral ramps, three for JPEG color
samples. These tolerances do not establish general perceptual accuracy.

## Dependency and local environment

- LittleCMS release `lcms2.19.1`, release archive SHA-256
  `bfc54f7bab59fbc921012014a8032e4cba4abd46db47d46b76416a8c0b2815c8`.
  The digest matches the upstream GitHub release asset and the fetched bytes.
  [Official release](https://github.com/mm2/Little-CMS/releases/tag/lcms2.19.1).
- **OBSERVED:** upstream core LICENSE is MIT. Its optional GPL plugins are
  disabled explicitly, as are upstream tools. No vendored source is rewritten.
  [Pinned options](https://github.com/mm2/Little-CMS/blob/lcms2.19.1/cmake/Lcms2Options.cmake).
- Upstream CMake reports version 2.19 for that patch release. Source integrity
  comes from the archive pin, not the package version check alone.
- Shadow Windows 11, MinGW GCC 16.2 release build, libjpeg-turbo 3.2.0 from the
  read-only adjacent toolchain. LittleCMS is built/installed only below this
  repository's ignored `.build/`; no adjacent dependency installation.
- One local compiler job for the consumer tests; at most two for the earlier
  standalone dependency build. No timing, CPU/RSS or photo-corpus claim here.

## Measured correctness and retained failure

The local three-test suite passes: existing metadata tests, portable pixel/
embedded metadata/color checks, and the Windows WIC control. The latter remains
an untagged color-policy comparison; it does not exercise WIC color management.

New checks cover a 256-step neutral sRGB identity ramp, linear RGB and gray
ramps against the scalar equation, and generated baseline/progressive JPEGs
with both linear profiles through all eight supplied orientations. The JPEG
comparison decodes identical generated image samples with/without metadata;
it computes expected output from the untagged bytes without calling LittleCMS.
Existing embedded EXIF tests remain independently active. Color plus embedded
EXIF in one combined fixture is not yet asserted by this checkpoint.

Refusals cover empty/truncated/excess profile bytes, wrong ICC magic,
device-link class, JPEG/profile color-space mismatch, unequal gray channels,
invalid raster stride/alpha, incomplete APP2 sequence and invalid sequence zero.
A valid call after failures verifies that per-invocation error state does not
poison subsequent work. These checks are not exhaustive parser conformance.

**REJECTED:** the default optimized grayscale transform failed the independent
ramp: linear sample 1 produced blue 6, expected 13. The logs
`Rejected-initial-ramp-LastTest.log` and `Rejected-gray-optimized-LastTest.log`
retain the failure. The same fixture passes with `cmsFLAGS_NOOPTIMIZE` for
gray. RGB uses the default optimizer and passes the current oracles. The precise
upstream optimizer mechanism and its performance tradeoff are unmeasured; no
vendor bug classification or general accuracy claim is made.

An initial PowerShell invocation passed an unquoted `-D...` token and failed
before downloading. Quoting the argument resolved setup. This is not codec
failure evidence.

**REJECTED native setup at `2595df0a`:** exact-head research run `37172286515`
configured the color library successfully, then failed CMake generation on both
macOS and Linux. Its installed static target references `Threads::Threads`,
but the pinned `lcms2-config.cmake.in` only includes the targets file and never
discovers Threads. The consumer now calls `find_package(Threads REQUIRED)` on
non-Windows hosts before importing LittleCMS. Vendor bytes are unchanged.
Failure excerpts are retained; this build correction needs a new native run.
The root reviewed that five-line CMake correction separately from the preceding
independent review. No C++ implementation or test oracle changed.

## Bounds, ownership and outstanding admission work

Each conversion has its own LittleCMS context and named nonthrowing error
callback with stack-owned state. Profiles and transforms close before context
destruction. Borrowed profile/raster bytes remain alive for the entire call.
Input is unchanged; result storage is separate and published only on success.
Context error state is conservatively fatal. No shared scratch/global callback,
filesystem access, dynamic plugins, retained foreign callbacks or parallel
color kernels are introduced.

ICC bytes are capped at 1 MiB before the explicit extracted-profile copy and
LittleCMS open. TurboJPEG has already assembled metadata during its header
read; its allocation is still bounded only by the encoded-input/codec policies,
not by the later 1 MiB check. At most 4 MiB source and 4 MiB converted pixels
coexist, with at most 1 MiB gray plane. Vendor transform/profile scratch is
additional. Conversion output replaces the source before orientation allocates
its destination. No hard process-memory/deadline guarantee follows.

Only RGB/gray input/display profiles and relative colorimetric intent are
admitted by the specimen. CMYK/YCCK, device-link/output/abstract profiles,
high-bit-depth/lossless JPEG and unsupported transforms refuse. Output is sRGB
data, not proof of color-managed presentation on a wide-gamut monitor.

Next admission evidence remains real-photo/profile coverage, process supervision
and resource enforcement, immutable source authority/revision checks, cancellation,
bounded transfer and actual native first paint. The current portable release
still exposes UTF-8/PNG only. Native research CI and independent source review
are recorded separately when complete; local success does not imply them.

Reproduction is in `../../experiments/jpeg_decode/README.md` and the separate
`.github/workflows/jpeg-research.yml`. The complete house-style review applies
to authored conversion, fixtures, integration, CMake and workflow, not vendored
LittleCMS/libjpeg-turbo or unrelated legacy frontend code.
