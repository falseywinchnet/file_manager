# Repeated preview wrapping widths

**OBSERVED baseline:** source `95b76f6dec3f6ee436ab95acf40f629ee11d3d77`
already avoids fixed-size Label measurement. Its paint-time wrapping still
resolves identical words and candidate line prefixes repeatedly within one
paragraph. The 64 KiB repeated-word fixture resolves 113 candidates to produce
seven lines. The unbroken fixture resolves 218 candidates, including the full
source word. These calls use real HarfBuzz in the diagnostic.

## Candidate and correctness

`label_lines` now remembers up to 32 exact widths for keys up to 128 bytes during
one synchronous paragraph traversal. The approximately 4.5 KiB table is local
fixed storage. Entries own their live key bytes; mutable line buffers are never
retained by reference. Lookup compares exact bytes and length, without hashes.
The resolver remains stable for the call's font/context. The table expires
before another paragraph or later invocation can reuse a different provider.

The key ceiling is an optimization bound, not a content bound. Larger text and
evicted entries call the existing resolver. Resolution precedes replacement;
a provider exception escapes without publishing an incomplete width. No new
allocation, retained control state or public interface is introduced by the
cache. Line breaking, grapheme handling and source coverage are unchanged.

The repeated-width regression fails on baseline production and passes after
the change. It checks exact line contents, changed-resolver isolation, more
distinct words than cache capacity and keys crossing the byte ceiling. Existing
CJK, combining-mark, joined-emoji, explicit-break, line-limit and alignment
fixtures remain in the same passing basic-controls suite. Both rejected and
accepted logs are retained here. The separate installed-consumer checks and
source review are recorded below when complete.

## Measurements

The new `wrap` mode of `gui_forms_label_layout_probe` measures the actual
`label_lines` function with HarfBuzz/FreeType/SheenBidi and checked-in
Carlito-Regular at content size 13. Seven lines, 186 logical-unit width; ASCII
input has exact font coverage. Font setup and input creation are outside the
timed interval. Four initial calls precede 100 samples per workload. All samples
must reproduce the same exact vector of lines. CSVs include output count, bytes
and a diagnostic signature; signatures are supplemental, not an identity proof.

Shadow Windows 11 Home 10.0.22621, AMD EPYC 9354 virtual allocation with 4 cores
and 8 logical processors, Balanced power plan, Release MSYS2 GCC 16.2.0. No
other local builds/tests or sibling timing campaigns ran during measurement.
Storage and font caches were warm. This does not measure cold startup, physical
host frequency, rasterization, native frame presentation or complete input lag.

| Initial pair | Baseline p50 / p99 | Candidate p50 / p99 | Resolutions per call, before / after |
|---|---|---|---|
| 64 KiB repeated words | 6.232 / 9.146 ms | 0.756 / 0.942 ms | 113 / 12 |
| 64 KiB unbroken word | 15.698 / 18.881 ms | 11.671 / 14.523 ms | 218 / 32 |
| 18-byte short control | 0.366 / 0.561 ms | 0.375 / 0.449 ms | 7 / 7 |
| 3,986-byte distinct-word control | 4.183 / 5.196 ms | 4.205 / 5.355 ms | 61 / 61 |

Three further pairs reverse the initial baseline/candidate order. Raw p50,
p95, p99 and maxima are retained, including noisy unchanged-control tails.
The repeated-word improvement is attributable to fewer real engine calls. The
short/distinct controls establish matching call counts and output; their small
timing differences do not establish improvement or universal no-regression.
The unbroken source still pays for its complete initial word measurement and
grapheme segmentation. Async prepared-window work remains necessary for broader
responsiveness; this optimization is not a complete preview performance result.

```text
gui_forms/.build/windows-prepared-dev/gui_forms_label_layout_probe.exe gui_forms/assets/fonts/Carlito-Regular.ttf wrap
```

## Integration and house style

Root rebuilt the current toolkit with at most two compiler jobs, installed it
into `.build/details-sdk`, and relinked File Manager and both application test
executables. Four toolkit suites passed in 3.12 s: basic controls, core, layout
panels and native Windows text. Both frontend interaction/transfer suites passed
in 5.80 s. Logs are retained here. These are correctness durations, not timings
for the optimization. Native CI remains pending.

Root and the visible sibling “Audit File Manager Details against interviews”
reviewed the cache, private helper contract, new tests and benchmark additions
against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`. The review covered
exact comparison, explicit types/initialization, fixed storage, current-count
bounds, copied keys, provider lifetime, replacement order, errors and repeated
work. The sibling found one meaningful implicit char-to-byte conversion in the
post-timing diagnostic signature loop; root made it explicit. No functional
cache defect or other authored-scope style violation was reported. The four
C++ files pass the spelling scanner; that is supplemental evidence. Untouched
legacy code and separately owned prepared-window changes are not certified.
Exact final source hashes are in `reviewed-source-sha256.json`.
