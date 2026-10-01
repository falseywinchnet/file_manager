# Document-view D1 resumed review and measurements

Date: 2026-10-01. **MEASURED:** focused renderer-neutral model conformance and
microbenchmarks. **OBSERVED:** six-file semantic house-style review completed for
this bounded source slice; broader toolkit/adapters are not certified.

The shutdown stop in `DOCUMENT_VIEW_D1_CHECKPOINT_2026-10-01.md` was subsequently
revoked: the parent verified the owner's correction that the shutdown notice
was stale. That record remains historical evidence. This record supersedes its
pending focused CMake, negative-fixture and source-review entries only.

## Exact reviewed scope

Full `planning/PROGRAMMING_HOUSE_STYLE.md` source review covered:

- `include/gui_forms/document_view.hpp`.
- `include/gui_forms/document_view/types/document_view_types.hpp`.
- `include/gui_forms/document_view/document_view_state/document_view_state.hpp`.
- `src/core/document_view/document_view_state.cpp`.
- `tests/document_view_tests.cpp`.
- `benchmarks/document_view_benchmark.cpp`.
- D1 source/test and separate benchmark option/target additions in `CMakeLists.txt`.

Explicit typed source/display units, named execution and predicates, initialized
records and valid result alternatives were reviewed. There are no callbacks,
worker closures or implicit captures. Page owners are noncopyable; moves empty
the former owner. UI-model ownership is noncopyable/nonmovable. Borrow lifetime
ends at publication/bind/destruction and must not cross dispatch. The adapter
must enforce thread ownership and shutdown; the model supplies no synchronization.

Validation precedes mutation. Full tokens gate slot and mapping authority.
Failed publication preserves old page, incoming payload and occupied slot;
`finish` is an explicit external-ownership acknowledgement. Successful publication
uses nonthrowing standard-allocator swaps/moves and releases the previous page;
no payload allocation/copy occurs inside it. No input can alias the model's
immutable page through the admitted API. Size differences are ordered and
bounded before uint64-to-uint32 conversion. UTF-8 validation plus contiguous
nonempty mapping partitions establishes binary-search access bounds.

Source loops use stable counts and pre-reserved producer benchmark buffers.
Test setup allocation is explicit; reviewed changes separate string assignment,
payload replacement, capacity calculation and narrowing from unrelated effects.
Benchmark output work is outside timing loops; source-to-size conversion and
endpoint bounds are hoisted. No blanket fast-math, hidden allocation pipeline,
anonymous behavior or general type-erasure machinery was added.

Parent independently reviewed core ownership/bounds and requested reverse-map
benchmark correctness, mapping-token forgery tests, conversion hoisting and total
payload accounting. Those corrections are included. No remaining house-style
violation was identified in the listed source scope. This is source review,
not a proof and not a whole-repository compliance statement. Reused UTF-8 and
binary-search implementations were inspected as dependencies, not restyled or
claimed newly compliant.

## Correctness and build evidence

Environment: Windows 11 Home 10.0.22621, AMD EPYC 9354 (4 exposed cores/8 logical
processors), 16,757,176 KiB visible memory, Balanced power plan. GCC 16.2.0
MSYS2 Rev4 from read-only adjacent Plan Paint. CMake Release `-O3 -DNDEBUG`,
CPU/core only, Skia/HarfBuzz/Windows host disabled. Timing ran locally with other
development activity possible; no isolation or fixed-frequency claim.

Base repository at measurement: `1da25f3fdb925be9b5293f01f022aff0d84299a5` plus
the reviewed edits. SHA-256 of exact working bytes for the reviewed run:

| File | SHA-256 |
|---|---|
| `src/core/document_view/document_view_state.cpp` | `a219e49fac76056e686f78bbfe95d2ccb4649144bfdfcba56e99745164a726e0` |
| `tests/document_view_tests.cpp` | `562c34ff132f706083dd7ed16d7a7b0c5a360d8622eb604c81088665a0b202bb` |
| `benchmarks/document_view_benchmark.cpp` | `6822175138d2a52bb711b6b90d3d067c89a02b1951df688f6ad204a36b04d3a3` |

An intermediate compile failed after changing fixture map initialization to
`std::array`: its aggregate needed an additional brace level. Corrected before
the recorded passing builds; no runtime result is attributed to that attempt.

**MEASURED:** focused CMake build succeeded; CTest `gui_forms_document_view_tests`
passed 1/1 in 0.07 seconds (0.08 seconds total). Added cases cover nonfinite and
negative DIP offsets, reversed/out-of-extent requests before capacity checks,
invalid coverage/identity lengths/enum/display extent, zero/decreasing revisions,
and both mapping directions rejecting forged viewport/permitted/revision fields.
Earlier maximum mapping, uint64 edge, cancellation, stale-page and preservation
fixtures continue to pass. Strict syntax compile of all three new `.cpp` files
with `-Wall -Wextra -Wconversion -Wsign-conversion -Werror` passed. Spelling scan:
six files, zero findings. `git diff --check` passed.

Commands from repository root:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake -S gui_forms -B gui_forms/.build/document-view-d1 -G Ninja `
  -DCMAKE_BUILD_TYPE=Release -DGUI_FORMS_ENABLE_SKIA=OFF `
  -DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=OFF -DGUI_FORMS_ENABLE_WINDOWS_HOST=OFF `
  -DGUI_FORMS_BUILD_GALLERY=OFF -DGUI_FORMS_BUILD_TESTS=ON `
  -DGUI_FORMS_BUILD_DOCUMENT_VIEW_BENCHMARK=ON
cmake --build gui_forms/.build/document-view-d1 `
  --target gui_forms_document_view_tests gui_forms_document_view_benchmark --parallel 2
ctest --test-dir gui_forms/.build/document-view-d1 `
  -R '^gui_forms_document_view_tests$' --output-on-failure
./gui_forms/.build/document-view-d1/gui_forms_document_view_benchmark.exe
g++ -std=c++20 -Wall -Wextra -Wconversion -Wsign-conversion -Werror `
  -fsyntax-only -I gui_forms/include `
  gui_forms/src/core/document_view/document_view_state.cpp `
  gui_forms/tests/document_view_tests.cpp gui_forms/benchmarks/document_view_benchmark.cpp
python tools/check_house_style.py gui_forms/include/gui_forms/document_view.hpp `
  gui_forms/include/gui_forms/document_view gui_forms/src/core/document_view `
  gui_forms/tests/document_view_tests.cpp gui_forms/benchmarks/document_view_benchmark.cpp
```

## Measurement interpretation

Deterministic 64/4,096/65,536-span pages project one source byte into `[BYTE FF]`
per span. Source admission and display expansion are deliberately different.
Before timing, every endpoint round-trips in both directions; selected shuffled
endpoints also match an independent linear scan. The linear reference is a
deliberately simple baseline, not a claim about the old TextBox. Test correctness
has no timing threshold.

Each size records 101 warm request/replacement samples and 1,001 warm mapping
samples; p50/p95/p99 use the sorted floor index of `(n-1)*percent/100`, with worst
reported separately. Timers include clock overhead and were quantized at about
100 ns; zero samples mean below this measurement resolution, not zero work.
Producer page construction is excluded. Publication includes complete payload
validation and destruction of the old page. First publication is reported
separately but follows page construction; it is **not cold OS-cache evidence**.
Payload pointer identity before/after publication checks ownership transfer;
allocator overhead and whole-program allocation counts are not instrumented.

Reviewed run (nanoseconds in raw CSV; milliseconds below for publication):

| Span count | Publication p50 / p95 / p99 / worst (ms) | Binary map p50 / p95 / p99 / worst (ns) | Linear reference p50 / worst (ns) |
|---|---|---|---|
| 64 | 0.0010 / 0.0010 / 0.0013 / 0.0015 | 0 / 100 / 100 / 100 | 100 / 100 |
| 4,096 | 0.0593 / 0.0634 / 0.0809 / 0.1180 | 100 / 200 / 200 / 200 | 1,400 / 21,500 |
| 65,536 | 0.7923 / 1.3517 / 2.4499 / 3.9451 | 200 / 400 / 500 / 15,600 | 20,600 / 189,900 |

Preserve the first run too: maximum-page publication worst was **9.5329 ms**,
p99 2.9377 ms. The later smaller worst sample does not erase this tail. These
bounded-model measurements do not establish end-to-end editor responsiveness.
No file I/O, projection construction, shaping, painting, accessibility or edit
history is included. A later integrated lag scan must include those stages.

Raw outputs:
`DOCUMENT_VIEW_D1_BENCHMARK_2026-10-01.csv` (initial run) and
`DOCUMENT_VIEW_D1_BENCHMARK_REVIEWED_2026-10-01.csv` (corrected harness).

In this build `DocumentMapSpan` is 32 bytes and `DocumentViewState` is 320 bytes.
For the maximum measured workload one page has 589,824 display bytes and
2,097,152 map bytes. Two full producers plus one published payload total
8,060,928 bytes; two source pages add 131,072 assuming each source owner's
capacity equals the workload count (not measured producer allocations).
Counted model/two producer-page/
two source-string/coalesced viewport+range metadata adds 672 bytes: **8,192,672
accounted bytes**, excluding string terminators, allocator overhead, worker and
dispatch envelopes and separately bounded indexing/shaping workspace. The
hard-profile formula is `3 * (display_capacity + mapping_capacity * sizeof(span))
+ 2 * source_bytes + known_metadata`: maximum display capacity of 1 MiB gives
9,568,928 accounted bytes with those same map/source/metadata bounds. These are accounted bounds,
not process RSS or measured peak live allocation; adapter receipts owe their
remaining bounded storage.

## Remaining gates

Serial-exhaustion guard remains inline and source-reviewed but unexercised;
parent explicitly preferred this to a tiny arithmetic helper created solely
for coverage. uint64 source-position tests are separate evidence. Allocation
failure fixture deliberately throws at producer boundary; no actual allocator
fault injection, worker cancellation/teardown or oversized-grapheme adapter has
been tested. `context_required` is an admitted terminal outcome, not implemented
layout/indexing. Full toolkit regression, matching installed consumer and native
application acceptance remain separate. No installed package or manifest
availability is promoted by this record. D2/D3/D4/P1 remain separately gated.
