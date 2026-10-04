# Windows brush-cache follow-up validation

**OBSERVED:** The resumed change closes the earlier P2 fixture gap. Shadow
and gradient scenes are now compared independently against uncached scalar
rendering. Two transparent-background passes exercise construction and reuse;
an opaque pass verifies background replacement. The same painter survives
1.0, 1.25, then 1.0 scale. Large admitted brushes force byte-budget eviction
below the entry ceiling. Independent scoped source and house-style review
accepted these changes; see FOLLOWUP_REVIEW.md. No legacy-wide certification
or public API change is implied.

**MEASURED:** Local Shadow Windows, shared Plan Paint MSYS MinGW GCC 16.2
toolchain, Release, CPU DIB renderer, Skia disabled. The Games sibling confirmed
the exclusive two-job build slot and quiet measurement interval. The tests use
only generated brush data; no user files. Power policy and hardware scheduling
were not controlled beyond this host/quiet interval, so results remain local
component evidence.

The final lifecycle and frame-store suites passed 2/2 in 3.32 seconds. The
ordinary GUI.Forms application DLL also compiled with transactional DIB OFF.
The exact commands were:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake --build gui_forms/.build/windows-prepared-dev --target gui_forms_windows_dib_frame_store_tests gui_forms_windows_dib_lifecycle_tests --parallel 2
ctest --test-dir gui_forms/.build/windows-prepared-dev -R '^gui_forms_windows_dib_(frame_store|lifecycle)_tests$' --output-on-failure
cmake --build gui_forms/.build/shadow-windows --target gui_forms_application --parallel 2
```

WindowsLastTest.log preserves the focused execution. The workflow now repeats
the two DIB suites after normal SDK export; development configuration must not
change the exported OFF SDK. The preceding text-test log is retained separately.
Native CI execution is still pending at this source checkpoint.

## Paired measurements

The 800 x 500 workload and timed variant-zero brush sequence are unchanged from
the checkpoint. Each row has 40 samples. Background writes and pixel comparisons
are outside elapsed time. Run order was baseline, candidate, candidate,
baseline, with no build between runs. Every executable exited successfully.

| Workload | Baseline first p50/p95 ms | Candidate first p50/p95 ms | Candidate second p50/p95 ms | Baseline last p50/p95 ms |
|---|---:|---:|---:|---:|
| 42 gradients, warm calls | 25.3547 / 26.4379 | 0.8498 / 0.8727 | 0.8648 / 0.9457 | 25.2582 / 26.7469 |
| Shadows, warm calls | 5.3008 / 5.6820 | 0.9202 / 0.9851 | 0.9138 / 0.9643 | 5.2155 / 5.5047 |
| 42 gradients, forced cold | — | 25.1274 / 26.4658 | 25.3094 / 26.1606 | — |
| Shadows, forced cold | — | 5.1939 / 5.6489 | 5.1980 / 5.4098 | — |

Raw `*-final-*.log` files also retain p99/max. Baseline gradient calls miss all
42 brushes each iteration because of 32-entry FIFO churn; candidate warm calls
have zero new builds. Forced-cold candidate runs clear caches outside the timer;
sample allocation and brush generation remain inside. These are distinct cache
policies, not proof of equivalent allocation/destruction cost. No material cold
draw regression appeared in this scene. Rotating large working sets, clipped
fragments, and end-to-end application behavior are not established by it.

Replay measurements by setting GUI_FORMS_BENCH_BRUSH_CACHE=1 before invoking
gui_forms_windows_dib_lifecycle_tests.exe. The baseline executable hash is in
README.md. `baseline-instrumentation.patch` reproduces its instrumentation and
initial fixture on base 568d7fbde2cc173cb5f857431aec53400ee1b550; its application
was checked against that base using a temporary Git index and
`git apply --cached --unidiff-zero --check`. Apply with `--unidiff-zero` on that
exact base; the zero-context format avoids whitespace-only context lines in
the stored evidence. It changes no brush
algorithm. Preserve the historical initial fixture when replaying those logs.

## Remaining edges

- Native Windows/macOS/Linux CI and actual consumer adoption are pending.
- Games needs a separately tested backport onto 7b260cf to avoid unrelated
  public-header changes. No old/new SDK DLL interchangeability is claimed.
- These are Windows DIB results, not Mac Skia or whole-application latency.
- Image sampling and broader File Manager preview/thumbnail work are unchanged.
- Shadow failure fallback is tested before sample allocation. Vector insertion
  failure is covered by source ownership review, not an injected allocator.
- The inherited gradient allocation-failure/accounting limitation remains as
  recorded in README.md; it is not repaired by this cache-capacity change.
