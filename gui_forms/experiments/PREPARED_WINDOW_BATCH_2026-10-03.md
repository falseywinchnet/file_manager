# Private prepared-window aggregate admission

Status: private candidate with Windows source review and five focused passing
tests; native macOS/Linux checks pending. No performance result is claimed.
The coordinator authorized
this private scope after reviewing SwiftEdit's PREPARED_BATCH_REVIEW.md and
restored the original input proof from 6ffc81c. Its historical receipt and
negative evidence remain unchanged. CMake, Git, public headers, existing A2
storage/service, native hosts, render commands and SDKs were not edited here.

## Implementation

Input now has a movable value owner with a private reservation, originating
ledger, and unique immutable data allocation. The data owns the original four
const arrays and all validation results. There is no const_cast or second input
format. The populated input still requests five allocations: data record plus
four arrays. Its charged metadata sizeof now names the allocated data record;
the caller's value owner is not a heap allocation. Reset and assignment destroy
old arrays before releasing their charge. Default move construction empties
the former unique/shared owners and reservation without copying arrays.

The private batch admission takes the full expected key, authority, originating
ledger, retained font-bank owner, input owner and empty unique output. It checks
executor, full identity, ledger and font generation, reserves one existing A2
8 MiB payload generation, then allocates one storage record and one row-index
array. The immutable input is transferred only after a second authority/closing
check. Its input-slot reservation is released after transfer, allowing the next
input while old payload generations remain retained. Descriptor/storage/input
requested bytes are checked using subtraction before multiplication.

Rows contain only paragraph indices. There is no geometry, metrics, worker,
native shaping, output-completion state or renderable type. Font bytes retain
their independent existing ledger charge; this admission verifies identity and
generation, not actual coverage or font compatibility with a native shaper.
The private test font owner is explicitly a storage fixture, not a real font.

Desire fixes the controller/session after its first key and requires strictly
increasing epochs. It checks exhaustion before an epoch can be reused. Current
batch observation rejects changed full keys and closing state without freeing
retained storage. No public capability or SDK export follows from this source.

## Ownership, locking and failure review

Input and unique output are confined to the caller executor. Authority mutable
state is mutex protected in operations; fixture mutations occur serially with
no worker. Lock order is authority then ledger. Initial payload acquisition
holds only the ledger. Commit transfers arrays while both gates are held;
input reservation release occurs after the ledger guard ends because release
locks the ledger itself. No allocation, callbacks or native calls occur in the
commit region. Failure unwinds guards before reservation-owning storage, so
retirement does not recursively acquire a held ledger mutex.

Failed allocations and pre-commit revocation preserve input/output. Actual
controlled allocation bytes exclude allocator overhead and shared_ptr control
blocks. A unique admitted owner can later become a shared immutable owner for
retirement testing, but that conversion is outside admission and does not claim
render publication. Reservation is destroyed after input and row allocations.

## Test coverage

The original input tests retain malformed/context/separator/profile/budget/key
coverage and allocation injection, updated for the data/owner split. Move tests
check allocation identity and retained charge. New batch tests cover empty
input, wrong ledger, stale full key and desire, font generation, executor,
authority/ledger closing, each of two allocation failures, closing injected
between allocation and commit, occupied output, exact requested byte accounting,
zero-copy input transfer, next-input admission, three retained generations,
fourth-generation refusal, final-lease retirement and epoch exhaustion.

The existing input limits cap this storage-only aggregate far below 8 MiB;
an exact 8 MiB live output cannot be constructed through certified input and
row-index descriptors alone. Do not fabricate mutable certified data to claim
such a test. Exact remaining-output capacity becomes a shaping-stage test when
geometry allocations exist. Original input tests cover row/source/display and
combined metadata limits. Additional platform and boundary verification remain
required before accepting this stage.

## Coordinator-owned build registration

Register `src/core/text/prepared_window/prepared_window_input.cpp` and
`src/core/text/prepared_window/prepared_window_batch.cpp` alongside the existing
private prepared storage in gui_forms_core. Add separate executables for
`tests/prepared_window_input_tests.cpp` and `tests/prepared_window_batch_tests.cpp`,
each linked to gui_forms_core, and separate CTests. Their replacement allocation
functions are executable-local and must not be combined into one executable.
No renderer, host, SDK or external dependency registration is needed.

## Coordinator review and Windows verification

**OBSERVED and repaired:** the initial authority implementation stored epoch
history only in optional `desired`. Clearing desire could therefore revive an
old epoch or replace the controller/session. The final private state retains
controller, session and highest epoch independently of active desire. Regression
tests clear desire and verify same-epoch, foreign-controller, foreign-session
and exhausted-epoch refusal. Revocation does not release retained payload bytes;
final owner retirement does. Closing remains terminal.

**MEASURED:** Windows MinGW Release, one compiler job, existing renderer-neutral
prepared-text configuration. After the amendment, both private window suites
and the existing prepared-text service, raster and display suites passed:
5/5, 0.78 seconds CTest total. Commands from repository root after loading
`tools/Enter-WindowsToolchain.ps1`:

```text
cmake -S gui_forms -B .build/prepared-window-input
cmake --build .build/prepared-window-input --target gui_forms_prepared_window_input_tests gui_forms_prepared_window_batch_tests gui_forms_prepared_text_service_tests gui_forms_prepared_text_raster_tests gui_forms_prepared_display_tests --parallel 1
ctest --test-dir .build/prepared-window-input -R "gui_forms_prepared_(window|text_service|text_raster|display)" --output-on-failure --parallel 1
cmake -S gui_forms -B .build/prepared-window-off
```

The OFF configuration's build.ninja contains neither prepared-window source nor
test target. This is a configuration check, not another SDK installation audit.
Raw focused output is retained in `prepared-window-batch-2026-10-03/CTest.log`;
local configure/build logs are `.build/prepared-window-batch-*.log`. CI now builds
and runs both private suites on all three supported platforms after the ordinary
OFF SDK export. No current dogfood executable enables this candidate.

Root reviewed the complete four prepared-window C++ source/header files, both
test files, the optional CMake registrations and added CI commands against
`planning/PROGRAMMING_HOUSE_STYLE.md`. Explicit types/initialization, named
behavior, checked dimensions, immutable borrows, allocation rollback, transfer
order, lock order, authority history, reservation retirement and per-row work
were inspected. The spelling scanner reports six files and zero findings;
that is supplemental evidence, not the semantic review. Existing A2 provider,
renderer and native host code were not rewritten or certified by this review.

This accepts a private ownership experiment for cross-platform testing only.
Shaping, per-paragraph workspace reuse, geometry/output limits, rendering,
consumer admission and measured File Manager responsiveness remain unproven.

## Next shaping constraint from source review

The sibling's read-only follow-up was checked against
`src/render/text/harfbuzz/harfbuzz_font_engine.cpp` (SHA-256
`ecb78ab3ba1d9226acd47cb294f5770e491ffaa93e8c65abd29ab91aaf01571d`).
`shape_bounded` currently allocates caller-selected run/glyph capacities before
shaping, including an empty paragraph, and separately allocates segmentation
scratch on each call. Keeping one font engine does not establish workspace reuse.
A simple loop with default capacities cannot deliver a useful 512-row window
inside this aggregate allowance. Smaller remaining output bytes alone cause
refusal rather than smaller output arrays.

Before implementing that stage, compare a reusable bounded workspace with
actual-output allocation against a bounded-capacity retry approach. Preserve
complete independent bidi paragraphs, full script coverage and existing A2
correctness; do not assume a glyph expansion ratio or concatenate paragraphs.
Measure allocation counts and peak bytes on empty, short, mixed-script and
512-row windows. Neither candidate is selected here.

## Native integration checkpoint

**MEASURED 2026-10-03:** PR 19 push run 37106083278 and pull-request run
37106091606 both completed successfully on Windows, macOS and Linux, with
the development input/batch tests enabled. Tested source
`3ae551d30c9e0e5165d34896ca9b17a53facae7f` and rebase merge
`3d1aedecad7a854dd6933322e664366cb4898eb4` have the identical complete tree
`033b1c5406f6d68c8a20e816d05a9d297ff368dc`. This closes native integration
of the private ownership stage; it adds no shaping/rendering or responsiveness
claim. Draft PR 14's input stage is included and superseded by PR 19.
