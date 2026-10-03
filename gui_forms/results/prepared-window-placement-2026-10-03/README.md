# Private prepared-window row placement

**OBSERVED prerequisite:** the private batch shaper retained independent row
metrics and glyph arrays but no cumulative row placement. Empty shaped rows had
zero ascent/descent and a font-size height. That was insufficient to place blank
lines consistently with the admitted primary font before batch rasterization.

**CANDIDATE screen-line policy, implemented privately:** query the first exact
role/weight/italic primary face once per batch at the same rounded 26.6 size and
72-DPI setup used by glyph shaping. No host font, dummy text or fallback face
supplies these metrics. Each row uses the larger primary/shaped ascent and
descent, adds the nonnegative primary line gap, and retains a larger shaped
height when present. Row top accumulates those positive heights; baseline is
top plus ascent. Original shaping metrics and glyph coordinates remain intact.
Explicit row top/baseline/height and aggregate width/height are device units.
Width is the maximum logical advance, not an ink bound or raster allocation.

The metrics helper returns copied scalars while its owned native face stays on
the shaper executor. All additions remain local until the final full authority
and ledger check. Invalid/nonfinite/nonadvancing placement refuses without
publishing partial rows or aggregate extents. Existing `sizeof`-based payload
accounting includes every added field. No per-row metrics lookup, new worker,
public header, callback, raster operation or frontend activation is introduced.

**MEASURED correctness:** Shadow Windows, Release MSYS2 GCC 16.2.0, HarfBuzz ON,
Skia OFF, at most two compiler jobs. All nine development suites passed in
2.19 s: input, batch, shape, session, A2 service, raster, display, bounded shape
and bounded workspace. The updated shape suite took 0.68 s. These durations are
not performance measurements. Native-platform validation is pending.

The oracle opens a separate FreeType library and admitted face and reads its
size metrics directly; it does not call the production metrics helper. A
separate shaping engine supplies row geometry. Mixed Latin/combining,
Arabic/Hebrew and CR/LF/CRLF fixtures compare original metrics, placement and
exact retained bytes. Five hundred twelve blank rows retain their full logical
height beyond 4096 device units. Empty EOF gets a positive line box at minimum,
fractional and maximum admitted sizes/scales. Exact-primary refusals distinguish
fallback-only and wrong-weight faces; NaN size refuses before native conversion.
Late allocation failures preserve input/reservation and unchanged zero extents.

The oracle necessarily applies the selected placement formula to independently
obtained inputs. Native sizing failure and malformed/extreme native metrics are
not injected. Overflow of the valid pinned-font profile is not reached. No ink
clipping, raster baseline application, caret/bidi affinity, wrapping, tab or
public line-layout contract is accepted by these tests. The 16 KiB/512-row
development bounds remain intermediate gaps rather than product exclusions.

Root and the implementation sibling reviewed the exact authored changes against
the complete `planning/PROGRAMMING_HOUSE_STYLE.md`: new batch fields, private
metrics record/method, lookup/placement/publication changes, fixture/oracle/test
additions, and the private FreeType test link. The Details-review sibling
independently reviewed the same source scope. The five-file spelling scan is
clean and supplemental. Review covers explicit types/units and conversions,
native borrow lifetime, executor confinement, mutation order, initialized
storage, failure ownership and repeated work. Whole-file hashes identify exact
bytes but do not certify unchanged legacy code. The independent reviewer
confirmed exact unchanged hashes for all five source/test files and CMake after
the implementation freeze, with no concrete correctness or style finding.
Root separately reviewed the added CI spelling-scope line and these records.
