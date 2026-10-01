# Document-view D1 shutdown checkpoint

Date: 2026-10-01. **OBSERVED:** coherent development source checkpoint, not
installed availability or completed D1 acceptance. Parent requested cessation
for imminent machine shutdown. No SDK, host or application stage was changed.

## Contract and source scope

D1 semantic reconciliation is canonical at
`../../orchestrator/spec/contracts/GUI_DOCUMENT_VIEW_DEVELOPMENT.md`; provider
proposal is `../docs/DOCUMENT_VIEW_PROVIDER_PROPOSAL_2026-10-01.md`.

Authored files:

- `include/gui_forms/document_view.hpp`.
- `include/gui_forms/document_view/types/document_view_types.hpp`.
- `include/gui_forms/document_view/document_view_state/document_view_state.hpp`.
- `src/core/document_view/document_view_state.cpp`.
- `tests/document_view_tests.cpp`.
- `CMakeLists.txt`: one core source and one focused test target, parent-approved.
- Provider proposal, negotiation link and this receipt.

Model implements exact typed coordinates, revision binding, two outstanding
request slots, independent viewport serials, complete-token comparison,
cancellation/release acknowledgement, unique owned pages, capacity validation,
strict scalar/atomic-token mapping and page-token stale rejection. It performs
no I/O, layout, editing, worker scheduling or source-byte interpretation.

## Measured focused validation

**MEASURED:** Windows x64, GCC 16.2.0 (MSYS2 Rev4), read-only adjacent Plan Paint
toolchain selected by `tools/Enter-WindowsToolchain.ps1`. CPU reported AMD EPYC
9354, 4 exposed cores/8 logical processors, Balanced power plan. This is test
environment metadata, not a latency measurement.

From repository root, direct focused build and execution passed:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
g++ -std=c++20 -O2 -Wall -Wextra -Werror -I gui_forms/include `
  gui_forms/tests/document_view_tests.cpp `
  gui_forms/src/core/document_view/document_view_state.cpp `
  gui_forms/src/core/text/text_store/text_store.cpp `
  gui_forms/src/core/text/unicode/unicode_grapheme.cpp `
  -o gui_forms/.build/document-view-d1/document_view_tests.exe
./gui_forms/.build/document-view-d1/document_view_tests.exe
```

Output: `document view D1 fixtures passed; map span bytes=32`.

Fixtures cover empty/EOF and uint64-max positions, exact roundtrips beyond
double precision, invalid/budget-before-busy precedence, cancellation retaining
slots, duplicate/forged acknowledgements, same-revision page A/B stale mapping,
identity changes with pending work, malformed UTF-8/map gaps, independent
display/map capacity refusal preserving the previous page, token/scalar/CRLF
interiors, literal-label collision, simulated producer allocation exception
cleanup, and three maximum-sized 65,536-token publications against an arithmetic
mapping oracle. The allocation case deliberately throws `bad_alloc` at the
producer boundary; it is not allocator fault-injection coverage.

At this build's 32 bytes per span, maximum mapping payload is 2,097,152 bytes;
display capacity adds at most 1,048,576 bytes per page. Two producer pages plus
one published page admit 9,437,184 payload bytes, plus at most two 65,536-byte
source copies, object/string terminator/allocator overhead and separately
budgeted producer workspaces. D1 does not measure allocator overhead or claim
these bounds encompass consumer source/history storage.

CMake configure/generate also succeeded:

```powershell
cmake -S gui_forms -B gui_forms/.build/document-view-d1 -G Ninja `
  -DCMAKE_BUILD_TYPE=Release -DGUI_FORMS_ENABLE_SKIA=OFF `
  -DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=OFF -DGUI_FORMS_ENABLE_WINDOWS_HOST=OFF `
  -DGUI_FORMS_BUILD_GALLERY=OFF -DGUI_FORMS_BUILD_TESTS=ON
```

The new CMake target was **not built/run through CTest** before shutdown.
`git diff --check` passed for tracked changes. No full toolkit regression,
benchmark, installed consumer, native UI or other-platform run was performed.

## House-style review status and next actions

The complete `planning/PROGRAMMING_HOUSE_STYLE.md` was read. Authored code uses
explicit types, named helpers/predicates, unique page owners with empty moved
state, operation-local results, no anonymous callbacks and no deferred borrows.
Initial ownership review checks: no allocation/copy on publication; validation
before page mutation; failed publication preserves incoming owner and occupied
slot; successful bind revokes authority without prematurely freeing workers;
uint64 differences are checked before narrowing into bounded page coordinates.

**Remaining review:** exhaustive semantic source review of all five new C++
files and the build edits is incomplete. No spelling scan was run for this
checkpoint. Do not call this source house-style accepted. Several test
construction expressions combine helper calls with aggregate/vector assignment
and need explicit operation-order review. The existing binary-search and UTF-8
helpers were reused, not globally restyled/certified.

On restart: run the focused CMake build/CTest; review every new source/test line
against the complete standard; expand negative fixtures for nonfinite/negative
viewports, all malformed map variants, same-token altered interval/revision,
actual adapter cleanup/backpressure/teardown and bounded context outcomes.
Serial-exhaustion guard exists but is not exercised by a fixture; choose a
non-invasive test strategy before claiming this gate. Producer context and
allocation tests currently model terminal acknowledgement only; no real worker
or oversized-grapheme adapter exists yet. Add reproducible request/publication/
mapping p50/p95/p99/worst measurements against a simple reference and record
retained capacities/allocation behavior. Then obtain parent source review and
matching independent consumer validation before any new coherent SDK prefix.
D2/D3 retained layout/control, D4 selection/editing and P1 printing remain open.
