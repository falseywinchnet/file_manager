# Native shaping phase attribution — 2026-10-01

## Scope and method

**OBSERVED:** the coordinating chat authorized private phase attribution after
the native baseline exposed long mixed-direction and fallback calls. This is
diagnostic evidence, not a selected public layout contract or a performance fix.
`GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS` defaults OFF. When enabled, each private
HarfBuzz engine owns a fixed counter snapshot, used only on its executor.
Macro-OFF preprocessing removes the timers, counters and snapshot accessor.
No public header, SDK checkpoint, application stage or dependency was changed.

Environment, pinned libraries, four exact font files, seven generated inputs,
31 warm samples, admission bounds and timing exclusions are those recorded in
`TEXT_LAYOUT_NATIVE_PROBE_2026-10-01.md`. The separate Release build is
`.build/text-layout-attribution`, with HarfBuzz ON and Skia/Windows host OFF.
The external 60-second process deadline completed normally; stderr was empty.
Raw output is retained in `TEXT_LAYOUT_PHASE_ATTRIBUTION_2026-10-01.csv`.
The initial vendored HarfBuzz build emitted a GCC `memcmp` bound warning; no
vendored source was changed. Background development load was not isolated.

Timers use steady-clock nanoseconds without logging or allocation. Phase lines
are arithmetic means over the 31 warm calls; main rows report percentiles.
Counts describe the final warm call. Do not compare phase means directly with
main-row medians. The instrumentation has overhead, especially for thousands of
runs, so these totals do not replace the two uninstrumented baseline records.

## Measured result

**MEASURED:** representative phase means, in milliseconds:

| Case | Run calls | Intersection pairs | Intersections | Append total | HB shape | Glyph output |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Mixed bidi, 16 KiB | 3,073 | 3,149,313 | 3.572 | 368.534 | 245.441 | 96.632 |
| Emoji fallback, 16 KiB | 2,341 | 2,341 | 0.424 | 205.633 | 148.270 | 37.811 |
| ASCII, 16 KiB | 1 | 1 | 0.00034 | 3.339 | 2.280 | 0.324 |

Instrumented mixed-bidi median/worst were 370.843/481.343 ms; emoji were
201.884/327.219 ms. The one-grapheme case still reports one missing cluster.
The existing finite geometry, cluster-boundary and repeated-signature checks
passed. Those checks are not a full geometry parity oracle.

`append_total` contains its subphases and native owner destruction. It must not
be added to those subphases. `glyph_output` includes glyph-vector construction,
fallback `hb_font_get_glyph_extents` calls and result metric/run updates.
Native font/buffer destruction occurs after that subphase timer stops and before
the append timer stops. Residual append time is not attributed to `hb_shape`.

**OBSERVED:** the measured intersection phase is a small contributor in these
inputs despite its nested traversal. **HYPOTHESIS:** repeated native shaping and
fallback extent work, with freshly constructed per-run HB objects, are better
optimization candidates. This experiment does not isolate library internals,
prove that reuse improves speed, or establish any latency deadline.

## Validation and authored review

The diagnostic build and existing HarfBuzz test passed: initial CTest 1/1 in
0.15 seconds total; after review corrections, 1/1 in 0.12 seconds. Strict probe
syntax checking with `-Wall -Wextra -Wconversion -Wsign-conversion -Werror`
passed using the existing public headers as system includes, as in the baseline.
The ordinary `.build/house-style-text` build was rebuilt with diagnostics OFF;
its existing HarfBuzz test passed 1/1 in 0.10 seconds total. Parent verified that
removing diagnostic blocks reproduces every nonblank pre-change production
engine source line. This is narrow source equivalence, not binary identity.
The spelling scan covered the new diagnostic header and probe: two files, zero
findings. Parent independently reviewed the engine diff, header, probe and CMake.

Semantic source review against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md` covers the new diagnostic header, the
conditional additions to the private engine header/implementation, the probe
additions and the narrow CMake option/definition. Types and conversions are
explicit; callbacks are absent; timer references remain within the engine call;
owner cleanup runs on early returns; snapshots reset before empty/missing-face
returns; counters have fixed storage; output and aggregation happen outside
timed calls. Timers stop once. Counts are bounded by this probe's admitted inputs
and 31 samples, not presented as an unrestricted long-running telemetry API.
Review corrections name converted nanoseconds before accumulation, hoist the
fixed phase count and use the recorded segment count in the direction loop.
No remaining violation was identified in this authored scope. Unchanged engine,
Painter and vendored code were not globally certified for house style.

Parent independently confirmed `GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS:BOOL=OFF`
in the ordinary text build, built the existing shaping target (up to date), and
reran its CTest: 1/1 passed in 0.07 seconds total. The parent two-file spelling
scan also had zero findings. Removing diagnostic-only blocks reproduced every
nonblank pre-change engine source line; the initial byte-exact comparison found
one extra blank line. This is source equivalence evidence, not a binary identity
or full renderer validation claim.

Final reviewed source SHA-256 values (small review corrections after sampling
do not change shaping or the phase boundaries):

- Engine: `83929d6c4428e6d81935e6143074cd703581019872648cad5070370d18329e24`
- Diagnostic header: `416421c04799e8d1e789404af5ba769ab6e67f12dfc2d0c3ae48920358546d36`
- Probe: `7b029ac89948fec3bca8fa1af26734638865e8c826fa05ef6129c0a6261d4881`

Native/opaque allocation usage and independent paint-side raster cache memory
remain unknown. The experiment adds no hard quota, cancellation mechanism,
fault-injection result or public prepared-layout seam. A full outside-timing
geometry comparator remains prerequisite to any shaping optimization.
