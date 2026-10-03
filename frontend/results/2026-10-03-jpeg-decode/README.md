# JPEG pixel experiment — 2026-10-03 UTC

**MEASURED, Windows only; CANDIDATE, not shipped preview support.**

Source and reproduction: `../../experiments/jpeg_decode/`. No application,
provider, package or capability was changed.

Environment: Shadow Windows 11 Home 10.0.22621; AMD EPYC 9354 presented as 4 cores /
8 logical processors; 16757176 KiB visible RAM; GNU/MinGW 16.2.0 Release; two
compile jobs; pkg-config libturbojpeg 3.2.0 borrowed read-only from the adjacent
toolchain.

| Generated fixture / supplied orientation | Output | Warm median | Warm p95 | Warm max |
|---|---|---:|---:|---:|
| Baseline JPEG 6000×4000 / 1 | 750×500 | 13.953 ms | 15.302 ms | 18.046 ms |
| Baseline JPEG 6000×4000 / 6 | 500×750 | 14.562 ms | 15.727 ms | 15.792 ms |
| Progressive JPEG 2048×1536 / 1 | 1024×768 | 14.346 ms | 16.846 ms | 16.937 ms |
| Progressive JPEG 2048×1536 / 6 | 768×1024 | 15.798 ms | 16.286 ms | 16.294 ms |

Each row contains 30 warm samples; p95 is nearest-rank sample 29 in sorted order.
`results.csv` retains all 124 observations, including first invocations;
`summary.json` comes from the checked-in typed `summarize.py`. This is one process
run, without confidence intervals or native-decoder controls. "Baseline JPEG"
is a format term. No product speedup is claimed. Four-color quadrants are highly
compressible and do not represent photographic/metadata-heavy input. Timed work
includes decoder/header/scale, allocation, decoding and rotation, but excludes
I/O, startup, IPC, upload, painting and final output destruction.

`verification.txt` records all eight supplied transforms passing independent
corner/color anchors (±8 channel units), non-square geometry, opaque alpha,
grayscale equality and empty/truncated/invalid-orientation refusal. No whole-
process RSS/deadline enforcement, EXIF parser, ICC conversion, CMYK acceptance,
perceptual resampling oracle, native picture or installed package was tested.

## Retained failure

`initial-icc-failure.txt` preserves the first rejected run: the success-only
interpretation of the ICC query refused every fixture without a profile.
Inspection of exact 3.2.0 upstream source established the absence combination
(`-1`, warning severity, zero size). The correction recognizes that combination
and preserves other failures. This does not establish malformed ICC semantics.
The recorded final build/run passed after that correction and explicit static
orientation range and consequential-result annotations.

## Integrity and house-style review

SHA-256 of measured files (source hash covers Shadow checkout bytes):

- `jpeg_experiment.cpp`: `5fd9d0b633b63918f5eaf2795009c42745937bd5b84849773446746ee8b8a341`
- executable: `72d934f04424440d98189f2d94fab0317be1edd72c1d7bf43cd394f685a038a5`
- adjacent `mingw64/bin/libturbojpeg.dll`: `f6a689c287636d394b2299b91a2afbece260081e95e125d676fe43f6a6d3129c`

Reviewed the entire new C++ specimen, CMake target and Python summarizer against
`planning/PROGRAMMING_HOUSE_STYLE.md`: explicit initialized types, named behavior,
foreign resource owners/deleted copies, synchronous spans, disjoint output,
cleanup, bounded products/conversions, allocations before pixel loops, invariant
transform selection, alpha initialization, ordered samples and named summary
fields. Each sample intentionally creates independent codec/output owners; no
reusable workspace or allocation-free claim. C++ spelling scanner: one file,
zero candidates. Vendored codec and unrelated legacy code are not certified.

Python checks group completeness, sample order, finite positive durations and
stable geometry/extents before publishing. Untested bounds/failure cases remain
listed in the experiment README; source review does not make them tested.

## EXIF and portable correctness follow-up

**OBSERVED, 2026-10-03:** the new private orientation reader accepts borrowed
encoded bytes, returns explicit absent/present/malformed/limit states, allocates
nothing and retains no input. It reads only pre-SOS markers and primary TIFF
IFD0 orientation, in either byte order. It does not follow IFD links or validate
the complete EXIF schema. The standalone decoder now has an EXIF-driven path;
missing orientation defaults to one, while malformed or ambiguous metadata
refuses before pixel publication. The original timing table above is unchanged
and does not measure this path.

**MEASURED correctness on Shadow:** both CTest cases pass with the existing
3.2.0 pkg-config dependency (0.04/0.04 s) and with independently built static
3.2.0 source, SIMD disabled (final source rerun: 0.03/0.06 s). These are test durations, not codec
performance measurements. `WindowsExifPkgConfigLastTest.log` and
`WindowsExifStaticLastTest.log` retain output. No `--measure` run was performed.

Metadata fixtures use independent literal headers for all eight orientations
in both byte orders, absence/unknown tags, every truncated prefix, duplicate
EXIF and duplicate orientation, invalid type/count/value/offset, bounded marker
and IFD work, fill bytes and standalone markers. An actual generated JPEG with
little-endian EXIF passes all eight independent corner/geometry oracles and
refuses invalid orientation. This is not a photographic corpus, complete EXIF
conformance or proof of production process containment.

The new dedicated workflow builds Windows MSVC, macOS and Linux correctness
against the exact upstream archive, SHA-256
`6f30092cef9fb839779646608f4ee14ae3cbac989c47fa05e841b0841f09878e`.
It installs the codec only into its ignored research prefix and consumes its
exported exact-version CMake package. Default pkg-config consumption remains
available. **Native results are pending.** Ordinary product CI alone does not
exercise this research target.

**Retained setup rejection:** the initial attempt to use CMake FetchContent
as an in-tree codec target failed because upstream explicitly refuses
`add_subdirectory` integration. No vendor patch bypassed that rule. The corrected
workflow downloads/verifies the archive, independently configures/builds/installs
the upstream project, then configures this research consumer. The dedicated
fetch script and static-package route both ran successfully on Shadow.
A first PowerShell fetch invocation omitted quotes around a `-D` argument
containing `.build`; the script refused the missing output argument. Quoting
that argument corrected invocation. The CI shell is Bash.

**Full house-style review:** reviewed the complete three new metadata files,
new decoder wrapper and embedded-metadata checks, explicit verification/measurement
CLI selection, CMake/fetch script and native workflow. Fixed-width field reads
have proven extents; byte order is selected before the IFD loop; count/length
bounds precede traversal; result states and conversions are explicit; borrowed
input remains synchronous; tagged fixture storage is reserved before its
orientation loop; all behavior is named. No remaining violation was identified
in this authored scope. Four C++ files produce zero spelling candidates; that
scan does not certify dependencies or legacy code. The new normalized source
receipt is `exif-reviewed-source-sha256.json`.
