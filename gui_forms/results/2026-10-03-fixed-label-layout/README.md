# Fixed Label layout work

**OBSERVED:** at baseline `c5fb4c69fa27bf323ba33f100ce5b151e84b7e5c`,
`Label::measure` copies display text, wraps it and resolves line metrics even
when both requested dimensions are positive. In that case the returned size
uses only the requested and available dimensions. File Manager's seven-line
preview is one such label (190 by 108 logical units in its initial setup).

The candidate returns that same constrained size before preparing display text
when both dimensions are explicit. Either content-derived dimension retains
the existing path. Painting still resolves the complete current display text
against its arranged width; no character clipping, font substitution, cached
geometry, new thread or public interface is introduced.

## Correctness and measurement

The focused regression first failed against baseline production code; see
`WindowsRejectedLastTest.log`. The candidate passes the basic-controls suite,
including existing wrapping, CJK, combining-mark, joined-emoji, line-limit,
alignment, display transformation and theme tests. The added case checks
requested/available sizes, zero available size, AutoSize's existing explicit
bounds precedence, text scaling and both content-derived dimensions. Its
metrics provider outlives the Window's borrowed registration.

The optional `GUI_FORMS_BUILD_LABEL_LAYOUT_PROBE` diagnostic uses the actual
Label implementation with HarfBuzz, pinned FreeType/SheenBidi and the checked-in
Carlito-Regular face at content size 13. All input is ASCII with exact coverage.
It measures **layout only**, without painting, disk reads or font registration
inside the timed interval. Each workload has four initial calls and 100 timed
calls; percentiles use nearest-rank positions. All calls must return identical
dimensions within a run. Baseline/candidate CSV dimensions also match exactly.
The first pair and three sequential repeat pairs are retained. There were no
concurrent local builds, tests or sibling timing campaigns during these runs.

Environment: Shadow Windows 11 Home 10.0.22621, AMD EPYC 9354 virtual allocation
reporting 4 cores/8 logical processors, Balanced power plan; Release MSYS2
GCC 16.2.0, shared adjacent toolchain used read-only. Native dependencies/fonts
are warmed. These in-memory measurements make no cold-storage or physical-host
power/frequency claim.

| Initial pair workload | Baseline p50 / p99 | Candidate resolver calls per 100 measurements |
|---|---|---|
| 64 KiB repeated words, fixed bounds | 6.893 / 7.889 ms | 0 (baseline 12,000) |
| 64 KiB unbroken word, fixed bounds | 16.152 / 17.589 ms | 0 (baseline 22,500) |
| 18-byte text, fixed bounds | 0.475 / 0.556 ms | 0 (baseline 800) |

Fixed-bound candidate measurements are at the clock's 0.1 microsecond
granularity. Report these as below meaningful timing resolution, **not zero
latency or a speedup ratio**. The source and resolver counters establish the
eliminated work. Content-derived controls retain the same resolver counts and
dimensions; their noisy timings do not establish a speedup or a comprehensive
no-regression latency result.

Reproduction, with the configured toolchain and fetched dependencies:

```text
cmake -S gui_forms -B gui_forms/.build/windows-prepared-dev -DGUI_FORMS_BUILD_LABEL_LAYOUT_PROBE=ON
cmake --build gui_forms/.build/windows-prepared-dev --target gui_forms_label_layout_probe gui_forms_basic_controls_tests --parallel 2
ctest --test-dir gui_forms/.build/windows-prepared-dev -R ^gui_forms_basic_controls_tests$ --output-on-failure
gui_forms/.build/windows-prepared-dev/gui_forms_label_layout_probe.exe gui_forms/assets/fonts/Carlito-Regular.ttf
```

A real host uses its configured font set and text adapter. This probe does not
establish macOS/Windows/Linux end-to-end interaction latency, and the fixed
layout short-circuit does not remove the remaining paint-time shaping work.
Prepared-window rendering, asynchronous adoption, wider preview formats and
thumbnails remain separate requirements.

## Integration and review

An initial attempt to build `.build/native-windows-x64/frontend` exposed an old
SDK configuration (`gui_forms/.build/shadow-sdk`) lacking the current Details
APIs. That attempt failed and is not acceptance evidence. Root rebuilt the
current toolkit with at most two compiler jobs, installed it into
`.build/details-sdk`, and rebuilt the active `frontend/.build/shadow-windows`
application and its interaction/transfer tests. Four toolkit suites passed:
basic controls, core, layout panels and Windows native text (3.04 s total).
Both application suites passed (5.49 s total). The separate prepared-text-enabled
basic-controls build also passed. These durations describe correctness runs,
not performance. Logs are retained beside this record. Native CI remains pending.

Root source review covers the authored Label early-return block, the complete
new counting-provider/test, the complete new benchmark, and the two CMake
additions against `planning/PROGRAMMING_HOUSE_STYLE.md`. Explicit types and
initialization, named executable behavior, borrowed provider/font-owner
lifetimes, return order, conversions, failure reporting and repeated-work
storage were reviewed. The benchmark allocates sample storage before its loop;
input/font loading is outside timing and expected native failures are explicit.
The three C++ files pass the spelling scanner. This is not a compliance claim
for untouched legacy helpers or vendored dependencies. The visible sibling
“Audit File Manager Details against interviews” independently reviewed the same
four-file authored scope against the complete standard and found no concrete
behavior, lifetime, benchmark-validity or house-style issues. The review correctly
limits measured gains to direct `Label::measure` calls. Exact LF-normalized
source hashes are in `reviewed-source-sha256.json`.
