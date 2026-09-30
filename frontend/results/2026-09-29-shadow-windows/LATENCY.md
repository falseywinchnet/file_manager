# Frontend latency investigation — 2026-09-29

**GIVEN:** the owner reports severe File Manager latency. Measure the existing
implementation before changing it. The parent chat owns native Windows painter
work; this record measures frontend construction and asynchronous directory
application without a native renderer.

## Workload and environment

**MEASURED:** Shadow Windows x64, GCC 16.2.0, CMake Release build, two build jobs,
installed development GUI.Forms SDK. Baseline SDK DLL SHA-256 is
`3ad427accd2a9830764b359c15f0457deee19be5063b55ff4f47056dce5b5a48`.
The benchmark creates the real retained Application and Window, binds worker
wake/close callbacks, drains asynchronous results, then requests final layout.
It does not open a desktop window, draw through GDI, or drive desktop input.

- Read-only real `C:/Users/Shadow` Home and `C:/Users/Shadow/file_manager` folder.
- A newly created disposable temporary root containing a child with 2,000 small
  regular files; only that fixture is written and removed.
- Five alternating navigation requests after initial enumeration for each case.
- Constructor and window construction are timed separately. Each navigation
  reports end-to-end wait, total and maximum UI drain time, final layout time,
  and retained flush/measure/arrange counters.

The polling loop sleeps for one millisecond while waiting. Windows timer
granularity makes ordinary completion readings cluster near 15 milliseconds;
these are end-to-end benchmark observations, not exact filesystem service times.
Repeated runs warm filesystem caches. The 2,000-file case measures the admitted
identity-observing workload, not a raw filename-only directory iterator.

## Baseline

Exact output: `latency-baseline.txt`.

| Operation | Observed time |
|---|---:|
| Application construction | 5.988–7.315 ms |
| Window construction and resource binding | 71.240–73.982 ms |
| Home/repository navigation, end to end | 15.150–15.440 ms |
| Home/repository UI drain | 0.163–0.285 ms |
| Home/repository final layout | 0.126–0.191 ms |
| Enter 2,000-file directory, end to end | 548.924–594.427 ms |
| Enter 2,000-file directory, UI drain | 6.780–7.117 ms |

Every navigation produced **one retained flush, one measure pass and one arrange
pass** at final layout. Initial navigation produced one flush and two passes.

**REJECTED as the measured cause on this host:** redundant frontend flushes.
`drain_ui()` does not surround mutations with `begin_update()`, but the measured
path already accumulates dirty state into a single final flush. Adding a scope
would introduce a forced layout at scope exit without evidence of improvement.
No speculative batching change was made.

**OBSERVED:** bootstrap/settings and directory work share a FIFO worker queue.
The Windows unavailable service transport returns immediately in this profile;
initial Home completion was 11.983 ms. This does not establish absence of queue
contention on other platforms or during long checksum/service operations.

**OBSERVED:** directory application currently publishes ObjectView items and
then republishes their sorted order. This is redundant work, but UI application
is below 0.3 ms for Home/repository and about 7 ms at 2,000 files. It was not
selected as the severe native-rendering bottleneck.

## Reproduction

After configuring the frontend against the installed SDK:

```powershell
cmake --build frontend/.build/shadow-windows --target file_manager_application_latency_benchmark --parallel 2
& './frontend/.build/shadow-windows/file_manager_application_latency_benchmark.exe' 'C:/Users/Shadow/file_manager'
```

The benchmark is excluded from the default build and CTest because it is a
measurement tool with real read-only folder inputs, not a timing threshold test.
The currently open application must not have its executable or staged DLLs
replaced. Subsequent renderer validation will use a separate build/output
directory and preserve this baseline.

## Actual native File Manager window

The opt-in `--native` mode uses only public Application/Window APIs. It opens a
fresh test-owned 1340×850 File Manager rooted at the repository, waits for actual
asynchronous enumeration and presented frames, requests four further full-window
repaints, and self-closes through its own handle. A 180-second deadline rejects
stalls. It does not send input to or close the user's existing application.
Native metrics measure render/present time; the first-frame wall time additionally
includes model construction, native startup, enumeration, layout and scheduling.

```powershell
& './frontend/.build/shadow-windows-latency/file_manager_application_latency_benchmark.exe' 'C:/Users/Shadow/file_manager' --native
```

**MEASURED controlled comparison:** the same benchmark executable and source,
with only its isolated GUI.Forms DLL swapped. The baseline DLL was preserved as
`libgui_forms_application.baseline.dll`. The initial optimized DLL digest is
`b982414c348dc2fae30d79e918edaa232b38d1e6e2dae389eab7461f18e3fcdd`.

| Actual native window | Baseline | Initial text-cache candidate |
|---|---:|---:|
| Native ready | 5,416.685 ms | 197.984 ms |
| First presented frame observed | 9,963.455 ms | 795.212 ms |
| Five frames, total render/present | 7,150.537 ms | 2,248.453 ms |
| Repeated warm frame render/present | 1,188.555–1,231.981 ms | 387.637–471.187 ms |

Exact outputs are `native-latency-baseline.txt` and `native-latency-after.txt`.
Both runs presented five full-window frames with 140 controls and the same
retained tree. Each completed and closed its own test window. The baseline
recorded one ambient native input event; the optimized run recorded zero; the
harness synthesized no input. Cold versus warm OS/process cache effects were not
independently randomized in this first comparison.

**REJECTED as complete latency acceptance:** the initial text-cache candidate.
It substantially improves native readiness, but actual full-window repaints
remain approximately 0.4 seconds. The text-only proxy benchmark did not reveal
this remaining whole-application renderer cost. The parent is profiling painter
primitives before selecting the next fix.

The harness now additionally reports process private bytes, current working set,
and peak working set per frame on Windows. Those fields were added after the
first native comparison; no retrospective memory claim is made for those logs.

**MEASURED:** the alternate frontend output builds and all 11 CTest suites pass
in 2.59 seconds against the initial text-cache candidate. The existing user's
executable/DLL directory was not overwritten. The alternate deliverable is
`frontend/.build/shadow-windows-latency/File Manager.exe`. Final staging and
updated measurements await the next SDK candidate.


## Final native material-rendering candidate

**MEASURED:** non-profiling actual File Manager benchmark runs using renderer
DLL SHA-256 `88c804e35a89043498b76235989cd460e8b20c47751d169d50f0b2d55061b8d8`.
The logical window is 1340×850 at the host's 2× DPI scale (2680×1700 physical
pixels). Both runs have 140 controls, five presented samples, zero input events,
zero callback faults, and one successful self-close. No user window was closed.

| Metric | Full-window invalidation | Toolbar Back invalidation |
|---|---:|---:|
| Native ready | 187.235 ms | 228.690 ms |
| First presented frame observed | 350.466 ms | 399.991 ms |
| First frame render/present | 116.842 ms | 118.453 ms |
| Stable samples 2–4 render/present | 28.794–32.951 ms | 0.410–0.699 ms |
| Five frames render/present total | 251.914 ms | 179.578 ms |
| Final private bytes | 39,129,088 | 39,395,328 |
| Peak working set bytes | 53,686,272 | 54,079,488 |

The first requested local repaint still consumes pending startup damage:
59.534 ms, with almost a full-window damage area. It is retained in the log and
excluded from the **steady** partial range, not silently discarded. Subsequent
partial samples each add exactly 1,080 logical pixels of painted damage.
Final counters are five full-window paints in the full run, and one full plus
four partial paints in the local run. The timer's roughly 15 ms wall intervals
are scheduling granularity; render/present durations come from native metrics.

Exact logs: `native-latency-final-full.txt` and
`native-latency-final-partial.txt`. Process memory fields are sampled with
GetProcessMemoryInfo and are process-level observations, not isolated cache
allocation sizes or long-duration leak evidence. Full and partial runs are
fresh processes; this short comparison is not a claim of universal frame rate,
all-directory performance, or interactive workflow/visual acceptance.

This result improves steady full-window redraw from roughly 1.2 seconds to
roughly 30 milliseconds without changing the frontend's authored appearance or
adding speculative application batching. The toolkit record contains the
text/font caches, pixel-path changes, bounded material cache, and pixel
regression evidence.

## Final installed SDK and application delivery

The final installed SDK was relinked after the preceding candidate run, so its
changed digest was verified with fresh native runs. **MEASURED:** final DLL
`e89cf95f7c5599c0f3718589f456847f5bdfdaae13ba2586a7e6d42a5ff11b9b`.
Final logs are `native-latency-staged-full.txt` and
`native-latency-staged-partial.txt`; earlier candidate logs remain intact.

| Final staged SDK | Full-window invalidation | Toolbar Back invalidation |
|---|---:|---:|
| Native ready | 270.396 ms | 218.423 ms |
| First presented frame observed | 444.044 ms | 393.202 ms |
| Stable samples 2–4 render/present | 26.638–29.243 ms | 0.325–0.522 ms |
| Five-frame render/present total | 244.191 ms | 167.920 ms |
| Final private bytes | 39,108,608 | 39,370,752 |
| Peak working set bytes | 53,710,848 | 54,059,008 |

Both final runs report zero input events, zero callback faults and successful
self-close. The partial startup carry-over sample remains 46.759 ms; stable
samples paint only 1,080 logical pixels each. The full run paints five whole
windows, while the partial run records one full and four partial paints.

**MEASURED headless control after SDK replacement:** `latency-after.txt` retains
constructor, make-window and navigation measurements. Ordinary folder UI drains
remain 0.154–0.340 ms, final layout 0.127–0.179 ms, with one flush per navigation.
One ordinary end-to-end sample was 30.529 ms; the other four were 15.169–15.466 ms.
The 2,000-file observations remain 547.752–579.677 ms, with UI drain
6.707–7.547 ms. These comparable headless results locate the large improvement
in the native renderer rather than claiming a filesystem speedup.

**MEASURED regression verification:** alternate frontend build/staging passed;
all **11/11 CTest suites passed in 2.31 seconds** against the exact final SDK.
The retained test log is `latency-final-ctest.txt`. The parent reports all
64 GUI.Forms tests passed in 6.99 seconds, including scalar pixel references
and multilingual cache/lifetime checks; details live in the toolkit receipt.

Ready side-by-side application:
`frontend/.build/shadow-windows-latency/File Manager.exe`

Executable SHA-256:
`f3351042c115789a6f5fa9c25e017c9a6f7ce8e9264f3aa10e14237fa71bbb38`.
The staged DLL hash matches the installed SDK and consumption manifest.
The previously running user's application (PID 19292 when checked) and its
original output directory were left intact. No user input was synthesized.
The native benchmark establishes render timing and retained damage behavior;
it does not replace manual visual fidelity or complete interactive workflow
acceptance. No architecture or runtime scope was expanded for this repair.
