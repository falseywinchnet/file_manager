# Windows raster-cache backport for the existing Games toolkit

**CANDIDATE:** This branch starts at
`7b260cfb9f3267392e1470b0fcf4cd2497437819`, the Games toolkit pin. It applies
the private Windows cache implementation and exact-pixel fixture from mainline
candidate `7fb8d9dd1fa76ab7a424af9d8074050513a66ad7` (PR 34), without later
public API changes. No GUI.Forms public header, CMake prerequisite, or other
renderer source is changed. Games must still rebuild and validate its package.

The fixture is byte-identical to the mainline candidate. Comparing the host
against that candidate shows only the pre-existing absence of its newer title
callback: the cache implementation is unchanged. Three-way patch application
resolved the surrounding context without importing the title callback.

The workflow adds the same Windows private lifecycle/frame-store coverage after
the ordinary SDK export. The exported SDK retains transactional DIB OFF.

Mainline evidence: its exact shadow/gradient oracle, scale reuse, entry/byte
eviction and allocation fallback passed locally; ordinary production host
compiled; independent house-style/correctness review accepted the scoped code.
Its two paired warm medians were about 0.85 ms for 42 gradients (baseline
25.3 ms), and 0.92 ms for two shadows (baseline 5.2 ms). This is Windows
component evidence, not a backport execution result or Mac/application claim.
Source/evidence are preserved on the mainline branch under
`gui_forms/results/2026-10-03-raster-cache-checkpoint/`.

## Independent local backport execution

**MEASURED:** Release, Shadow Windows, shared Plan Paint MSYS MinGW GCC 16.2,
Skia OFF. Games confirmed its previous build/tests/profile terminal and kept
the local host quiet during this build and measurement interval. Maximum two
compiler jobs. Exact commands:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake --build gui_forms/.build/windows-prepared-dev --target gui_forms_windows_dib_frame_store_tests gui_forms_windows_dib_lifecycle_tests --parallel 2
ctest --test-dir gui_forms/.build/windows-prepared-dev -R '^gui_forms_windows_dib_(frame_store|lifecycle)_tests$' --output-on-failure
cmake --build gui_forms/.build/shadow-windows --target gui_forms_application gui_forms_application_native_tests --parallel 2
ctest --test-dir gui_forms/.build/shadow-windows -R '^gui_forms_application_native_tests$' --output-on-failure
$env:GUI_FORMS_BENCH_BRUSH_CACHE = '1'
& gui_forms/.build/windows-prepared-dev/gui_forms_windows_dib_lifecycle_tests.exe
Remove-Item Env:\GUI_FORMS_BENCH_BRUSH_CACHE
```

DIB tests passed 2/2 in 3.35 s; the ordinary transactional-DIB-OFF application
DLL built and native application test passed 1/1 in 0.24 s. The respective
LastTest logs are preserved here. A single 40-sample component repeat gave:

| Scene | p50 ms | p95 ms | p99 / max ms |
|---|---:|---:|---:|
| 42 gradients warm | 0.8528 | 0.8889 | 0.9783 |
| Shadows warm | 0.9288 | 1.0258 | 1.1451 |
| 42 gradients forced cold | 25.4889 | 28.2501 | 30.4357 |
| Shadows forced cold | 5.5200 | 7.7863 | 8.2725 |

The raw brush-backport.log includes successful exact-pixel/lifecycle checks.
This single repeat has noisier cold tails than the mainline pair; no separately
paired old-base baseline was run, so it does not establish a cold-performance
equivalence claim. Cache clearing/background/comparison remain outside timing.

Root reviewed the backport diff against the accepted mainline implementation;
only pre-existing title-callback differences remain between host files, and the
fixture is identical. The full host and a .cpp copy of the fixture passed the
spelling scanner (2 files, 0 findings); scoped semantic review remains the
mainline review plus root's unchanged-implementation comparison.

Native CI and Games consumer package validation remain pending. This revision
is ready for the sibling's development rebuild and measurement, not declared a
cross-platform released toolkit. Never substitute the mainline DLL into the
old SDK: unrelated public headers differ there.
