# Logical wrapped mask development: native layout and rasterization

Status: **MEASURED on Shadow Windows in the named development fixtures**.
Stage 1 was independently accepted and committed as `4d24f43`. The coordinator
assigned Stage 2 under the existing reconciled profile. This source-only slice
does not publish an SDK, freeze an application adapter, or claim native-platform
goldens or interactive visual acceptance.

## Implementation and dependency boundaries

New private `src/render/text/text_mask/` components implement encoded-font
opening, logical shaping, paragraph/line bidi, wrapping and native rasterization.
The only Stage 1 source changes are its private backend declaration, default
backend selection and the lifecycle test's default-backend expectation. Existing
A2 font engine, prepared raster, Painter and host backends are unchanged.

The older engine's hinted device positions and float glyph records could not be
used as this profile's logical double-precision layout. The new path reuses the
existing encoded-bank owner and bounded Unicode 17 grapheme implementation,
together with pinned HarfBuzz, FreeType and SheenBidi. All those native operations
run on the session worker. The backend singleton holds no per-job mutable state.

The coordinator admitted libunibreak 8.0 at
`28a2756b864c343f438cd22537d49d394d4666a5`, recorded provenance, and added private
CMake/fetch/CI wiring. `set_linebreaks_utf8` receives a counted caller-owned
byte array and the explicit language string `-strict`; neither host locale nor
upstream's default tailoring selects policy. Soft opportunities are intersected
with Unicode grapheme and actual full-paragraph HarfBuzz cluster boundaries.
Its independent upstream Unicode 17 line-break conformance test passed
**19,338/19,338**, with no skipped cases in this run. That proves the dependency's
algorithm on its corpus, not universal correctness of the complete mask service.

Each paragraph retains its SheenBidi resolved levels while candidate lines use
`SBParagraphCreateLine`, including L1/L2 treatment. Script and font runs are
intersected with those visual runs. HarfBuzz receives only the selected line as
context, with BOT/EOT at actual line edges, explicit script and `und` language.
Thus joining and ligatures are reshaped at selected line boundaries, rather than
carried across them. The explicit primary face is tried first; remaining faces
are tried in registration order for a whole grapheme. Missing coverage refuses.

Font size remains logical scale 1 during shaping. FreeType/HarfBuzz load flags
disable hinting, autohinting and bitmaps for logical advances. Glyph positions,
advances, line metrics and accumulation remain doubles; design ascent/descent
are scaled from font units without using rounded device-size metrics. Requested
size/width and the additional interline gap use the agreed 1/64 quantization.
Device scale does not choose line breaks. Empty input is one line, LF/CRLF retain
their exact consumed byte spans, and explicit/trailing empty lines count.

Greedy soft wrapping uses legal opportunities; trailing ASCII soft-break spaces
may be consumed without advance while initial/repeated spaces retain advance.
Long words split only at intersected grapheme/shaped-cluster boundaries. An
indivisible overwide unit remains whole with an overflow flag. No replacement,
ellipsis, system-font discovery, dictionary service or automatic hyphenation
is added. Invalid/unsupported input and all finite limits remain typed failures.

Raster size is rounded once to device 26.6. Glyph origins scale once and round
to device integers with half ties away from zero. Gray uses outline coverage;
mono uses FreeType mono load/hinting/render flags and expands actual packed bits
to 0/255. It does not threshold grayscale. Native bearings determine signed ink
placement. Ink is measured first; the complete candidate then reserves metadata
and coverage before allocation/fill/publication. Native FT bitmap allocations
are opaque; bounds checked after their creation are indexing guards, not native
pre-allocation quotas.

## Bounded storage and execution

At most 2,048 native shaping calls and four MiB aggregate submitted line context
are admitted per request, before each call. One trial glyph buffer and one final
glyph buffer grow from HarfBuzz's actual returned counts, using bounded geometric
capacity growth with a 65,536-glyph cap. They retain capacity across trials within
the job. No assumed input-to-glyph expansion factor is used. Empty input allocates
no glyph buffer. Capacity and reallocation peaks participate in the existing
eight-MiB shaping budget. This corrects the initial implementation's unconditional
initialization of two 65,536-element arrays for every tiny label.

First-party variable workspace uses counted vectors. The fixed job record is
charged while alive. The existing bounded grapheme helper's reported controlled
peak stays reserved while its temporary/returned storage exists, including
simultaneous copying into counted paragraph storage. Vendor/native/runtime
allocations remain outside a process RSS or hard-latency claim. All transient
workspace is released before completion publication; immutable mask/key/font
ownership remains charged through the existing lifetime ledger.

Native font objects are still opened and closed for each uncached job. Persistent
worker font/workspace reuse is not implemented or claimed. The exact-key cache
avoids those operations on a hit. This remaining cost must be measured against
actual multi-face application workloads before further optimization.

## Fixtures, failures and correctness evidence

Initial native testing failed with `missing_font_coverage`: direct checks found
Carlito `a=45`, combining acute U+0301=0, Hebrew alef U+05D0=0 and Arabic beh
U+0628=0. This was retained as a valid coverage refusal, not hidden by `.notdef`.
Cousine supplies U+0301=690 and U+05D0=2294, but U+0628=0. The coordinator then
approved the unmodified, licensed Amiri test fixture documented under
`tests/fonts/amiri/PROVENANCE.md`; it is not an application fallback or SDK asset.

The six native-component groups cover exact logical glyph/position invariance
across scale, combining text, gray versus mono scales 1–4, malformed/missing
font refusal, native call/context limits, inherited RTL paragraph punctuation
ordering versus a separately resolved paragraph, empty-line metrics, exact
packed-FreeType mono bits/bearings, and Arabic joined beh-beh glyph forms versus
separately reshaped isolated lines with preserved source offsets.

The four public asynchronous wrapping groups cover empty/LF/CRLF/trailing lines,
exact source consumption, one gap between lines, word wrapping and soft-space
consumption, identical logical lines at gray scales 0.5/1/1.25/1.5/2/3/4 and mono
1–4, binary mono coverage, whole overwide combining clusters with registered
fallback, overwide shaped units, leading/repeated-space advance, NBSP behavior,
approved Arabic wrapped rendering, and typed axis/coverage/soft-line/native-work
failures preserving the previous mask. They verify transient workspace returns
to zero after completion. The existing eleven lifecycle groups continue to test
ownership and retirement, with a real native empty-result default-path check.

Fresh development CMake build: GCC 16.2.0 on Shadow Windows, Release, Ninja,
prepared text OFF, text masks ON, HarfBuzz ON, Skia/native hosts/gallery OFF.
The four focused CTests passed after the exact-count storage correction in
**11.50 seconds**: lifecycle 11.48 s, native 0.09 s, public wrapping 0.44 s and
line-break conformance 0.03 s (two test jobs). Raw receipt:
`TEXT_MASK_STAGE2_TESTS_2026-10-01.txt`. Earlier pre-optimization four-test run
also passed in 13.93 s. These test durations are not performance comparisons.
Final const-handle/named-return/diagnostic-label spelling corrections were
compiled; coordinator verification of the final source hashes remains separate.

Commands:

```text
cmake --build gui_forms/.build/text-mask-stage1 --parallel 2 --target
  gui_forms_text_mask_lifecycle_tests gui_forms_text_mask_native_tests
  gui_forms_text_mask_wrapping_tests gui_forms_linebreak_conformance_tests
ctest --test-dir gui_forms/.build/text-mask-stage1 --output-on-failure
  --parallel 2 -R "gui_forms_(text_mask_(lifecycle|native|wrapping)_tests|linebreak_conformance)$"
```

The native-stack build emitted an upstream HarfBuzz `memcmp` bound warning;
vendor source was not modified or certified. Authored native sources also passed
direct strict warning compilation (`-Wall -Wextra -Wconversion -Wsign-conversion
-Werror`) with existing public includes treated as system headers, as explained
in Stage 1's retained legacy Painter-header warning record. No Mac/Linux native
execution or end-user visual/golden claim is made here.

## Tiny-label measurement and reproducible control

`tests/text_mask_cost_probe.cpp` is an opt-in probe, not a CTest latency gate.
It preloads the encoded font, then measures public submit-to-worker-wake-to-take
with a named condition-variable wake target. There is **no sleep/polling loop**
in this timed interval. The first request is separated from 64 subsequent unique
`tiny label N` strings, one Cousine face, size 16, gray, no wrap. A separate
10,000-iteration exact cached lookup loop reports a mean. Reading font files and
service/bank creation are outside the timed region. All comparisons use the
same Shadow host/toolchain/Release libraries, without CPU affinity or priority
changes. Concurrent OS activity is uncontrolled; these are local observations.

The three initial fixed-buffer runs had p50 **927.4–952.1 us**, peak shaping
**5,242,880 bytes**. The three first actual-count runs had p50 **135.7–152 us**,
peak shaping **1,040 bytes**, but cold/tail noise included a 4,462.5-us first
request and a 1,414.8-us maximum, worse than some baseline samples. All these
values remain in `TEXT_MASK_COST_BASELINE_2026-10-01.txt` and
`TEXT_MASK_COST_CANDIDATE_2026-10-01.txt`.

A subsequent complete A/B/B/A sequence, preserved in
`TEXT_MASK_COST_ABBA_2026-10-01.txt`, reported:

| Variant/order | First request us | p50 us | p95 us | max/p99 us | Cached mean us | Peak glyph bytes |
|---|---:|---:|---:|---:|---:|---:|
| Fixed A1 | 1391.5 | 933.6 | 1092.0 | 1312.2 | 0.85790 | 5242880 |
| Actual-count B1 | 488.9 | 76.3 | 140.4 | 173.4 | 0.85464 | 1040 |
| Actual-count B2 | 482.6 | 74.8 | 146.0 | 169.2 | 0.82755 | 1040 |
| Fixed A2 | 1379.5 | 990.7 | 1188.3 | 1242.1 | 0.92495 | 5242880 |

For 64 samples the reported p99 is the maximum order statistic; this is not a
broad tail distribution. The result supports removing this measured fixed
allocation/initialization cost for tiny labels, not a universal latency claim.
One supplemental current-path direct backend run, on a named probe thread,
excluded admission/wake/adoption and reported p50 44.8 us, p95 56.3 us, max
136.7 us. Its matching public median was 72 us; exact cached lookup mean was
0.8384 us. This single-run decomposition is in
`TEXT_MASK_COST_DIRECT_2026-10-01.txt`, not an additional before/after comparison.

The fixed-buffer control predated a commit. To reproduce it from the final
development source, use a disposable checkout/copy and apply
`TEXT_MASK_FIXED_BUFFER_CONTROL_2026-10-01.patch`. It changes only the three
private header/shape/layout files affected by the growth correction. Each
reconstructed file was verified byte-for-byte against its SHA-256 captured
before the correction in `TEXT_MASK_COST_BASELINE_HASHES_2026-10-01.csv`.
That historical manifest also records the then-current fonts/raster files;
final raster borrowed-handle const spelling and invariant raster-operation
selection are later review corrections outside this growth control. The
historical timing samples do not measure the final operation-selection change.

Build **only** `gui_forms_text_mask_cost_probe` for the control: the final native
component test's private buffer API and the empty-allocation regression target
the corrected implementation. Use a separate build directory for each variant.
The probe's optional later direct measurement does not alter its preceding
public timing workload. Example, in the disposable copy after normal dependency
fetches and toolchain setup:

```text
git apply gui_forms/experiments/TEXT_MASK_FIXED_BUFFER_CONTROL_2026-10-01.patch
cmake -S gui_forms -B gui_forms/.build/text-mask-control -G Ninja
  -DCMAKE_BUILD_TYPE=Release -DGUI_FORMS_BUILD_TEXT_MASKS=ON
  -DGUI_FORMS_BUILD_PREPARED_TEXT=OFF -DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=ON
  -DGUI_FORMS_ENABLE_SKIA=OFF -DGUI_FORMS_BUILD_GALLERY=OFF
  -DGUI_FORMS_ENABLE_WINDOWS_HOST=OFF -DGUI_FORMS_ENABLE_MACOS_HOST=OFF
  -DGUI_FORMS_ENABLE_LINUX_HOST=OFF -DGUI_FORMS_BUILD_TESTS=ON
cmake --build gui_forms/.build/text-mask-control --parallel 2
  --target gui_forms_text_mask_cost_probe
gui_forms/.build/text-mask-control/gui_forms_text_mask_cost_probe.exe gui_forms/assets/fonts
```

The patch is a retained measurement control, not a product rollback proposal.
Run the unpatched variant in its own build with the same configuration. No
binary archive is required to reconstruct the allocation behavior.

## Exact house-style review and outstanding gates

The eleven files in `TEXT_MASK_STAGE2_HASHES_2026-10-01.csv` were reviewed against
the complete `planning/PROGRAMMING_HOUSE_STYLE.md`: three authorized Stage 1
files, five new native source/header files, native/wrapping tests and cost probe.
Reviewed explicit types and initialized values; read-only borrowed handles;
double computation and checked conversions; named native/worker/probe execution;
source/key/font/FT/HB/paragraph/bitmap borrow lifetimes; reverse destruction order;
bounded buffer growth before kernels; cluster and line source offsets; foreign
failure containment; candidate allocation/publication order; repeated-loop
storage and work; and rollback/resource release. Corrected borrowed FT handle
const spelling and the probe's executable return expression. No known house-
style violation remains in that exact authored scope. The reconstruction patch
is retained historical control data and was checked against its captured source
hashes. Unchanged Stage 1/A2/vendor/helper code is not newly certified here.

Coordinator-owned CMake, intake, CI, licenses and canonical records have their
own review scope. Source/API freeze, native platform evidence, application
adoption, visual mask goldens and actual Games help-panel inspection remain
separate. The provider is not claiming that the broader product is complete.

## Independent coordinator integration review

The coordinator reviewed the eleven manifested files against the complete house
style, including explicit types, native owner/borrow lifetimes, publication and
failure order, source offsets, checked conversions, counted storage and repeated
work. This review requested and verified the final invariant raster-configuration
hoist described below. All eleven final source hashes matched on 2026-10-01; no
remaining violation was identified in this authored scope. Unchanged legacy and
vendor code is not certified by this review.

The independent four-target Windows CTest run passed in 13.79 seconds before the
final raster-only correction. After its rebuild, the coordinator reran both
affected native/wrapping targets: 2/2 passed in 0.43 seconds. The eight-file new
source/test/probe spelling scan reported zero findings; semantic review above is
separate. The retained control patch also passed `git apply --check`.
Its five space-only unified-diff context lines are retained as patch syntax;
they are the only staged `git diff --check` whitespace findings and are not
trailing whitespace introduced into product source.

Coordinator review additionally covered `CMakeLists.txt`,
`cmake/TextMaskLineBreak.cmake` and the native workflow: private production versus
test-only vendor sources, explicit dependencies, bounded test timeouts, default
OFF development scope and ordinary SDK export before development reconfiguration.
The unchanged install guards continue to refuse text-mask ON installation and
exclude its headers when OFF, as previously verified in Stage 1. Native CI keeps
the ordinary SDK test log separately before development tests overwrite the last
test log. No public SDK admission follows from this source integration.

Windows local correctness is established only for the named fixtures. macOS and
Linux execution, native visual comparison and real consumer adoption remain
pending; cross-platform CI is the next verification step.

## Final raster review correction

**OBSERVED:** Coordinator review found a remaining house-style section 3
violation in `src/render/text/text_mask/text_mask_raster.cpp`: invariant raster
mode/load flags and the blend operation were selected for each glyph. This
supersedes the earlier no-known-violation statement for the pre-correction file.

The final correction is restricted to that source file. A named private
`RasterConfiguration` factory selects scale, FreeType load flags/render mode,
and named bitmap-validation/blending functions once before each glyph loop.
The function pointers refer to static-lifetime named functions, retain no
callback state, and allocate no storage. Each returned bitmap still receives
its own format, pitch, extent and indexing validation because those properties
can vary between glyphs. Bitmap borrows remain confined to the current glyph
before the next FreeType load. Rounding, raster arithmetic, destination bounds,
failure statuses and publication order are unchanged.

The complete raster file was reviewed against
`planning/PROGRAMMING_HOUSE_STYLE.md`, including explicit types, initialization,
named execution, borrowed lifetimes, conversions, failure states and repeated
work. No known violation remains in that reviewed file. No additional source
scope or legacy/vendor compliance claim is added. Its Stage 2 manifest hash
was refreshed; the other ten source hashes remain unchanged.

**MEASURED:** On the same Shadow Windows Release build, the native and wrapping
targets rebuilt with two compile jobs. One focused CTest run passed 2/2:
native 0.09 seconds, wrapping 0.46 seconds, total wall time 0.48 seconds.
No additional performance experiment was run; the earlier benchmark samples
retain their original scope. The raster file is refrozen for integration.
