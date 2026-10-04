# Admission laboratory source review

Complete standard: `planning/PROGRAMMING_HOUSE_STYLE.md`. Independent visible reviewer: Audit File Manager Details against interviews, thread `01a0fb48-1437-7302-8b6f-f3bcf02e7319`. Reports are retained verbatim, newest first; the latest control update supersedes the original two-way description.

**The level-6 control and three-way sampling changes are accepted. No new correctness or house-style blockers found.**

The generator uses the same opaque RGBA rows, zero row filters, dimensions and PNG metadata for both controls; only the zlib compression level and output filename differ. The probe loads and validates both controls before timing, keeps their owning buffers alive throughout collection, and warms all three admission paths.

Each of the 61 iterations records exactly one BGRA, PNG-store and PNG-deflate sample. The order is cyclic:

- BGRA → PNG-store → PNG-deflate: **21 iterations**
- PNG-store → PNG-deflate → BGRA: **20 iterations**
- PNG-deflate → BGRA → PNG-store: **20 iterations**

That slight imbalance is explicit and acceptable for this fixture. It balances ordinal positions approximately, but does not cover all six permutations or eliminate predecessor/cache effects. Reversing geometry order in a second process remains useful without implying independent statistical trials.

Timing still surrounds only admission. Both PNG paths use the same admission helper; reads, validation outside admission, warmup, retirement, checks, sorting and reporting remain excluded. Separate labels, encoded byte counts and complete sample lists make the three results distinguishable.

The compressed control improves the comparison by exposing the effect of encoded representation size on admission work. A cheap result for this highly compressible solid image would support only that workload’s admission cost—it would not establish cheap PNG decoding, rendering, typical-image performance or end-to-end preview latency. The initial Windows numbers remain root-reported observations; I did not reproduce them.

Exact follow-up scope: the compression parameter and six fixture-generation calls in [fixture.py](C:/Users/Shadow/file_manager/frontend/experiments/preview_admission/fixture.py), and the second control’s loading/validation, warmup, sample storage, order rotation and reporting in [probe.cpp](C:/Users/Shadow/file_manager/frontend/experiments/preview_admission/probe.cpp). The full house-style assessment remains clean for these changes. No edits, builds, tests or Git commands were performed.

---

**No correctness or house-style blockers found in the admission laboratory and workflow additions.** The probe supports an admission-only comparison through the installed Core API, with no renderer attached.

The reviewed mechanics are sound:

- **Same-pixel controls:** the Python fixture emits opaque RGBA `(12, 226, 198, 255)` with filter byte zero on every row and zlib level 0. The raw input uses the corresponding BGRA `(198, 226, 12, 255)`. Fixed dimensions bound fixture generation and pixel-buffer arithmetic. Exclusive file creation prevents silent replacement of existing controls.
- **Ownership and lifetime:** the raw ownership check modifies caller storage after admission, verifies a distinct registry buffer retaining the original first byte, restores the source, checks accounting, removes the resource and rejects its stale ID. The resource view is borrowed only during the check and is gone before removal. Exceptions unwind the window and its registry.
- **Timing boundaries:** each timer surrounds only the public admission call. File reads, buffer initialization, ownership assertions, retirement, reporting and sorting are outside timing. Input buffers and sample arrays are prepared before collection. Allocations performed inside admission are appropriately included because admission is the measured operation.
- **Sampling:** 61 pairs alternate raw-first and PNG-first order. The second process reverses case-size order. The reported percentile indices implement nearest-rank percentiles correctly; with 61 observations, the reported p99 is the maximum sample.
- **Workflow and linkage:** the consumer requests only `GUIForms::Core` from the exported SDK. Build products and generated inputs stay in the separate experiment directory. `pipefail` preserves executable failures through `tee`, and both forward/reverse logs are included in artifact collection.

The interpretation needs these precise limits:

- PNG and raw admission perform different validation and storage work. The PNG control is deliberately uncompressed and nearly raw-sized; results do not describe typical compressed PNG workloads.
- The ownership-mutation assertion directly covers **raw BGRA admission**. Both timed paths check successful admission and retirement/accounting, but the probe does not perform the analogous caller-mutation check for PNG.
- The probe validates PNG structure and dimensions; same-pixel content follows from the inspected fixture generator and raw initializer. It does not independently decode the PNG and compare every pixel.
- These are warm, sequential, single-resource measurements. Alternating order and reversing case order reduce some bias but do not establish a robust tail-latency distribution. There is no native paint, decode-to-display, RSS, selection-latency or end-to-end preview measurement. Registry `decoded_bytes` is accounting, not measured resident storage.

Exact reviewed scope:

- Full [CMakeLists.txt](C:/Users/Shadow/file_manager/frontend/experiments/preview_admission/CMakeLists.txt).
- Full [fixture.py](C:/Users/Shadow/file_manager/frontend/experiments/preview_admission/fixture.py).
- Full [probe.cpp](C:/Users/Shadow/file_manager/frontend/experiments/preview_admission/probe.cpp).
- The two admission-measurement steps and admission-log artifact line in [native-builds.yml](C:/Users/Shadow/file_manager/.github/workflows/native-builds.yml).

Supporting reads covered the existing Window admission wrappers and image-registry load/store paths to check what the timer and accounting represent; this is not a new acceptance review of those implementations.

I applied the complete [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md), including types, named execution, initialization, borrows, extent/conversion checks, failure cleanup and repeated storage. No remaining violations were found in the requested scope. No edits, builds, tests or Git mutations were performed; native measurements remain outside this source-review conclusion.

