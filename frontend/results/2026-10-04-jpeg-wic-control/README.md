# JPEG portable versus WIC fresh-call comparison

Status: **MEASURED generated-fixture control; not a shipped provider**.

The owner goal requires useful cross-platform previews. This slice recovers
prior research without merging its branch, corrects one house-style test-storage
issue, and adds a Windows WIC comparison using the same encoded bytes and
requested output extents. Product UTF-8/PNG admission and the published package
are unchanged. No decoder, supervisor or color policy is selected here.

## Preserved work

`7e40e111073a0734945ec0e98b3314f74dc32cc4` is the original JPEG/EXIF checkpoint.
Root verified exact-head push run `37111495248` and PR run `37111498441` were
successful on Windows, macOS and Linux. `Original*LastTest.log` below comes from
the push artifacts, whose artifact suffix is that original SHA. These historical
passes do not validate the new WIC source. The earlier branch and stash remain
untouched. Its original result records were restored alongside the experiment;
their older pending statements and measurements remain historical evidence.

The retained original tests allocate a fixed marker fixture inside an append
loop without reserving. The resumed source allocates all 8,194 bytes first and
fills indexed positions, closing the independently identified house-style issue.
No production parser bounds or duplicate policy was silently widened.

## Current correctness

Shadow Windows, adjacent read-only MinGW GCC 16.2, Release, exact pkg-config
libturbojpeg 3.2.0. Both compiler slots were occupied by a separate project
before this build; root waited until no compiler processes were observed.
This experiment uses at most two build jobs and no personal documents.

Host reports Windows 11 Home 10.0.22621, NTFS, AMD EPYC 9354 with four exposed
cores/eight logical processors. Shared-host scheduling and power policy were
not fixed. The final correctness executable SHA-256 is
`7BBC6CA7B3723DC938DB1A53DA8C0AB4EA1304B81B18BDDFCDB2512DF5152936`;
it includes the final extra refusal tests, while measured function bytes are
unchanged from the timing review below.

The final three suites passed in 0.55 seconds: EXIF parser, embedded-orientation
portable decoder and WIC control. `WindowsLastTest.log` preserves that run.
The WIC path validates source/output extents, acquires the named JPEG COM
decoder, scales/converts to owned BGRA and releases its entire COM graph before
the apartment ends. Generated baseline/progressive images pass independent
quadrant and opaque-alpha checks at matched dimensions. Grayscale and empty,
oversize, malformed-header and enlargement refusals pass. A valid decode after
COM/frame-acquisition failures yields the original grayscale output again.

WIC orientations here are applied by the shared first-party pixel transform;
they are not evidence of WIC EXIF extraction. WIC output dimensions come from
the portable candidate's factor selection outside timing. Admission of arbitrary
ICC, CMYK, high-precision or lossless input is not equivalent between wrappers.
No pixel-byte equivalence is required between distinct IDCT/resampling methods.

## Paired elapsed-call measurements

Run order was WIC, portable, portable, WIC. No compiler processes were observed
before or after; no system-wide idle/power-state guarantee is claimed. Each group
has one first invocation plus 30 warm samples, using the same generated JPEGs.
The output schema retains encoded/output extents, every sample and first call.
The supplied rotation values are 1 and 6; EXIF parsing is outside this comparison.

| Generated fixture | WIC first p50/p95 ms | Portable first p50/p95 ms | Portable second p50/p95 ms | WIC last p50/p95 ms |
|---|---:|---:|---:|---:|
| Baseline JPEG 6000x4000, orientation 1 | 20.299 / 20.812 | 14.278 / 20.950 | 14.603 / 17.526 | 20.429 / 21.712 |
| Baseline JPEG 6000x4000, orientation 6 | 20.807 / 21.176 | 14.902 / 17.889 | 15.092 / 16.978 | 23.009 / 37.240 |
| Progressive JPEG 2048x1536, orientation 1 | 37.351 / 65.798 | 14.421 / 16.984 | 14.403 / 15.753 | 36.431 / 43.121 |
| Progressive JPEG 2048x1536, orientation 6 | 38.492 / 47.041 | 15.915 / 17.317 | 16.018 / 19.901 | 37.877 / 38.900 |

The portable path has lower medians for these fixtures. Tail variability prevents
a uniform tail-improvement claim: the first baseline portable p95 slightly
exceeds its first WIC control. With 30 warm observations, nearest-rank p99 is
the observed maximum; JSON summaries retain both names and all CSV samples.
The largest observed WIC warm sample was 75.690 ms; portable was 22.949 ms.
These are not product latency percentiles or cold-process startup results.

The interval includes fresh decoder/header/setup, output allocation, decode,
scaling and the shared optional orientation transform. WIC additionally includes
COM setup/teardown and a copied input stream; portable borrows encoded bytes.
Therefore this is an implementation-call comparison, not isolated codec speed.
The WIC geometry reference retains an extra portable raster outside timing;
resident-memory baselines are unequal and RSS was not measured. Transform source
destruction is inside timing; final result destruction is outside. Encoding,
checks, CSV output, file I/O, IPC, process startup and native paint are excluded.

Generated constant quadrants are correctness anchors, not a photographic corpus
or perceptual-quality measurement. The term baseline in the fixture name means
baseline JPEG encoding. No production throughput, file I/O, CPU/RSS, whole-process
resource ceiling or cross-platform performance claim follows from these times.

Measurements precede the final added malformed-header/enlargement/recovery
assertions; measured functions and generated input construction are unchanged.
The final correctness suite includes those added assertions. The archived source
review records the timing-method hash separately from the final-source hash.

## Reproduce

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake -S frontend/experiments/jpeg_decode -B .build/jpeg-wic-control -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .build/jpeg-wic-control --parallel 2
ctest --test-dir .build/jpeg-wic-control --output-on-failure --timeout 60
.build/jpeg-wic-control/file_manager_jpeg_experiment.exe --measure-wic > .build/jpeg-wic-control/wic-1.csv
.build/jpeg-wic-control/file_manager_jpeg_experiment.exe --measure > .build/jpeg-wic-control/portable-1.csv
.build/jpeg-wic-control/file_manager_jpeg_experiment.exe --measure > .build/jpeg-wic-control/portable-2.csv
.build/jpeg-wic-control/file_manager_jpeg_experiment.exe --measure-wic > .build/jpeg-wic-control/wic-2.csv
python -B frontend/experiments/jpeg_decode/summarize.py .build/jpeg-wic-control/wic-1.csv
```

Check each exit code and summarize each CSV independently. The dedicated
research workflow builds the exact scalar static codec independently on all
three OSes; it adds the WIC control only on Windows. No install into the product
or ordinary SDK occurs. New-head native results remain pending until recorded.

## House-style scope and next gate

Independent review covered the restored authored experiment, the WIC COM
owners/acquisition/cleanup, shared pixel record, CMake control, corrected fixed
fixture, new verification and timing dispatch. It found no blocking defect;
the extra resident geometry reference was explicitly retained as a measurement
limitation. Root reviewed the final two failure/recovery cases and the typed
summarizer's p99 field. Six C++ files have zero spelling candidates. Exact
hashes and review scope are retained separately. Vendored codec and unchanged
product source are not certified by this experiment.

Further work is color/admission policy, real-corpus evidence, current source
identity and scoped input, killable process limits, cancellation and validated
pixel delivery to the application. PDF comparison and thumbnail ownership remain
independent. The canonical candidate intake remains an outline.
