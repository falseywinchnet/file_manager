# Per-call native shaping reuse — 2026-10-01

## Status and bounded change

**OBSERVED:** after reviewing the phase attribution, the coordinating chat
authorized a private per-call reuse experiment, with exact successful geometry
comparison before timing. This record does not select a public prepared-layout
API, introduce persistent caches or install an SDK.

Each shape call now owns at most 64 lazily acquired HB fonts, matching the
existing maximum registered-face count, and one HB buffer. A used FT face is
sized once for that call; its HB font is reused for later runs in the same call.
All owners die before the synchronous call returns. Borrowed engine faces stay
stable because registration and shaping are executor-confined. Paint-side
mutable faces are not shared or introduced by this experiment.

Pinned HarfBuzz `hb_buffer_reset` restores newly-created defaults, including
Unicode functions, flags, replacement code point and segment properties. The
existing cluster policy, complete-input context, run range and direction are
then restored before guessing script/language. The call uses `hb_shape_full`
with the default shaper list, which is the same inner operation used by the
previous `hb_shape`, but checks its success result.

The original phase attribution remains evidence for the old path. Under reuse,
`append_total` excludes destruction of the call-wide native owners at shape
exit. Whole-call timing includes it. Per-phase totals across these versions are
not interchangeable; no new phase-speed claim is made here.

## Successful geometry oracle

**MEASURED:** BEFORE changing reuse behavior, the diagnostic trace build captured
nine inputs at three font configurations (27 cases). The first seven inputs are
the existing probe cases, plus empty text and a mixed tab/newline/combining-mark
fixture. Each case uses the same engine across sizes 16, 9.5 and 32, with normal,
positive and negative letter spacing, and requested weight/italic variation.
The four registered fonts remain exactly the baseline font set.

The independent `--geometry` process mode uses classic locale and lossless
hexadecimal floating-point output. It serializes explicit fields, never native
struct bytes/padding: exact font bytes and local face IDs, face index, registered
weight/style, requested font configuration, ordered source ranges and actual
run direction, every glyph ID/cluster/x/y/advance, width/height/ascent/descent,
missing-cluster count and missing-primary status. Capacities/timestamps are
excluded. The direction field exists only with the OFF-by-default private
`GUI_FORMS_TEXT_LAYOUT_GEOMETRY_TRACE` macro. It changes diagnostic run size;
all performance comparisons have both trace and timers OFF.

The baseline, initial reuse, reviewed reuse and final failure-checked reuse
outputs are byte-identical: **12,043,216 bytes**, SHA-256
`0f85952f15d310e56045b054a44a1baa19aae9b1132098ba1a68c1c634e575d6`.
Parent independently compared the baseline/reviewed files using `fc /b` and
independently decompressed and hashed the durable golden artifact. A first
parent comparison command failed path lookup with slash-style paths; the actual
comparison with resolved Windows paths passed. That command error is not a
geometry discrepancy.
Parent subsequently compared `geometry-reuse-final.txt` with the baseline using
`fc /b`: no differences after all failure-handling changes. Parent also verified
both build targets were current and independently passed ON/OFF CTest 1/1
(0.06 seconds each). The final one-grapheme worst sample, 13.1278 ms, is worse
than the comparison baseline's 2.4172 ms; this negative result is retained.

Durable golden: `TEXT_LAYOUT_REUSE_geometry_baseline_2026-10-01.txt.gz`,
2,020,250 compressed bytes, SHA-256
`fd2dfa50fa9117a49914260e80dcbbf42599cb96d1c1e1df1f46f4bb5a76e0bd`.
It includes copies of the already bundled font bytes; the applicable bundled
license notices remain `../assets/fonts/OFL-Carlito.txt`,
`../assets/fonts/OFL-Noto.txt`, and `../assets/fonts/OFL-NotoEmoji.txt`.
No new font or distribution permission is inferred. Identical candidate output
is retained locally under `.build/text-layout-attribution/`, not duplicated in
Git. The golden is evidence, not a product dependency.

To reproduce, configure the probe with HarfBuzz ON and geometry trace ON, build
`gui_forms_text_layout_probe`, then run it with `assets/fonts --geometry`,
redirecting stdout to a file. From the repository root, compare explicit bytes:

```python
from pathlib import Path
import gzip

baseline_path: Path = Path(
    "gui_forms/experiments/TEXT_LAYOUT_REUSE_geometry_baseline_2026-10-01.txt.gz"
)
candidate_path: Path = Path(
    "gui_forms/.build/text-layout-attribution/geometry-reuse-final.txt"
)
compressed: bytes = baseline_path.read_bytes()
baseline: bytes = gzip.decompress(compressed)
candidate: bytes = candidate_path.read_bytes()
if candidate != baseline:
    raise RuntimeError("Geometry differs from the recorded baseline")
print("Geometry matches the recorded baseline")
```

This establishes fixture parity on this pinned Windows build, not universal
font/shaper correctness, native paint parity or end-user visual validation.
The inspected Shadow application build profile disables HarfBuzz (also explicit
in `../../tools/Build-Windows.ps1`). These measurements concern the private text
engine being prepared for the new document view, not a measured speedup of the
currently staged File Manager executable. Matching renderer/SDK adoption and
end-to-end application measurements remain required.

## Measured timing, including negative results

Environment, dependency pins, fonts and timing exclusions are recorded in
`TEXT_LAYOUT_NATIVE_PROBE_2026-10-01.md`. Release binaries used the same build
directory/profile; the baseline executable was copied before rebuilding reuse.
Each process had an external 60-second deadline and completed successfully.
First baseline then candidate ran sequentially; development background load was
not isolated. The final candidate was measured again after failure handling.

| Case | Baseline median ms | Initial reuse median ms | Final reuse median ms | Final worst ms |
| --- | ---: | ---: | ---: | ---: |
| ASCII small | 0.1437 | 0.1402 | 0.1228 | 0.2689 |
| ASCII 4 KiB | 0.9671 | 1.0160 | 1.0259 | 1.2666 |
| ASCII 16 KiB | 3.5487 | 3.7922 | 3.8719 | 4.4887 |
| Display labels 16 KiB | 3.5514 | 3.7343 | 3.3824 | 4.1548 |
| Mixed bidi 16 KiB | 249.801 | 75.8776 | 79.2671 | 93.4156 |
| Emoji fallback 16 KiB | 130.591 | 25.2283 | 26.4464 | 65.9451 |
| One grapheme 16 KiB | 1.8684 | 1.8573 | 1.9910 | 13.1278 |

All three raw CSVs are retained as `TEXT_LAYOUT_REUSE_{baseline,reuse,final}_2026-10-01.csv`.
The small ASCII/one-grapheme regressions and final one-grapheme/emoji tail spikes
are retained. The huge grapheme still has one missing cluster. Baseline bidi
worst was 284.607 ms and emoji worst 164.048 ms. Earlier baseline records,
including the 894.725 ms bidi sample, remain unchanged.

**MEASURED:** this bounded reuse change improves the two many-run cases in this
comparison. It does not establish a UI deadline, remove the need for worker
execution, or justify optimizing the small intersection phase next. No memory
peak measurement was added: up to 64 native font caches can now coexist for one
call, and native/opaque memory remains unknown. Returned result capacities are
unchanged in macro-OFF measurements. Paint-side cache memory is separately unknown.

## Failure behavior and validation

Parent review found that the pinned HB APIs can return nonnull empty objects,
or a font without an FT binding. The candidate validates the font is nonempty
and `hb_ft_font_get_ft_face` identifies the exact requested face before caching.
Buffer allocation status is checked on acquisition, after adding input and after
shaping. These failures throw `std::bad_alloc`. FT-size setup, impossible owner
capacity exhaustion and a false shaping result with successful buffer status
throw `std::runtime_error`. Call-owned partial output and native owners unwind;
the caller receives no ordinary partial-success result through these paths.

Allocation acquisition timing and failure policy changed deliberately. This is
not failure equivalence with the old fresh-per-run path. No public ABI was added;
the existing allocation-capable synchronous boundary already permits exceptions.

Six instance-owned diagnostic-only fault cases prove caller-old-result
preservation and subsequent same-engine recovery. Three use real HB empty-font,
unbound-font and empty-buffer objects. Two simulate rejection after add/shape on
run two, after prior output exists, and verify the requested run was reached.
One supplies an unavailable shaper list and exercises a real false shape result
on run two. These tests do not simulate actual allocator exhaustion, prove all
library internal allocation paths, or measure leak counts. No global allocator
framework or vendored mutation was introduced.

Final diagnostic build/tests (including these six cases) passed 1/1 in 0.09 s;
ordinary OFF build/tests passed 1/1 in 0.08 s. The final golden comparison passed
before final timing. Spelling scan covered five touched C++ files with zero
findings; strict probe syntax checking passed with conversion warnings as errors
and public headers treated as system includes, as in the original diagnostic.

Semantic review against the full `planning/PROGRAMMING_HOUSE_STYLE.md` covers
the new ShapeCall/CallFont owners, changed acquisition/reset/failure paths and
call wiring; private trace/accessor additions; new diagnostic failure enum;
probe serializer/mode and CMake additions; and the two new focused test functions
and their invocation. Types, named execution, explicit registration results,
named failure-case fields, conversions, fixed slot bounds, move-empty ownership,
stable face borrows, cleanup order, publication after success, single-executor
state and repeated-work storage were reviewed. No remaining violation was found
in that authored scope. Unchanged legacy glyph construction, fixture loaders,
other tests, Painter and vendor code are not globally certified by this review.

Final engine SHA-256:
`583f56129ca3531430ebf63e3f8c3330f948b6133babffacecee07070f18613a`.
Probe SHA-256:
`21649e569512841b26e89748a428df53694e3ad3fbd052f22fad8452543d93a7`.
Pre-reuse engine with the trace field assignment was saved before mutation:
SHA-256 `f009d7f6aca6d65be28261e7dbdd1328228e1fcc7d1ab37af64656bdc33fd3e0`.

Next authorized proving work is the private worker-owned shape/result lifecycle.
Native shaping remains noninterruptible inside a call. Cancellation cannot release
its slot, faces or buffers until completion and owned result release; shutdown
must stop admission, drain and join. The public prepared-layout/paint contract,
paint-side compatibility refusal, resource accounting and SDK adoption remain
separate unresolved work.
