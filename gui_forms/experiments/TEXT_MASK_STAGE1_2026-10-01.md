# Logical wrapped mask development: Stage 1 ownership and lifecycle

Status: **OBSERVED source-only implementation; private fixture verification**.
The coordinator assigned implementation after Games accepted the reconciled
profile in `orchestrator/negotiations/GAMES_PORTABLE_CAPABILITIES_2026-10-01.md`.
This receipt covers the finite ownership/queue slice. It does not claim native
wrapping, bidi, monochrome rendering, SDK availability, goldens or latency.

## Implemented boundary

Four `text_mask` public development headers expose immutable leases, normalized
requests, typed failures, nine-slot snapshots and service-lifetime budgets. The
new private core implementation owns the ledger, exact-source keys, fixed cache,
font-byte copies, session worker and retirement. Existing prepared-text files,
Painter ABI and A2 budgets are unchanged. CMake/install wiring belongs to the
coordinator and is not part of this source manifest.

The default worker returns `unsupported_profile`. Tests inject a private named
backend that produces deterministic zero-ink or one-MiB masks. Font registration
in this stage validates structural bounds and copies bytes; native font validity
and actual shaping/raster success remain Stage 2 work. No consumer may treat a
passing lifecycle test as rendering availability.

`MaskAllocator` charges actual requested allocation bytes, including STL rebound
shared-owner control blocks, before allocation and releases them after
deallocation. Vectors charge actual allocation capacity rather than nominal
element counts. Bootstrap ledger allocation is measured by its own one-use
allocator. Font copies reserve their aggregate bytes before allocating; the
aliasing `EncodedFontLease` retains their accounting owner. Object/key counts are
reserved before their shared allocation. The session's unique public handle is
also charged. The limits are not an RSS quota: host allocator headers and opaque
standard thread/native runtime allocations are outside requested first-party
payload accounting. No native workspace allocation exists in this stage.

The fixed 700-entry cache compares exact normalized options, font-owner identity
and exact UTF-8 bytes. Bounded ranks implement LRU without an auxiliary index or
unbounded timestamp. Eviction removes only cache ownership. Copying a lease adds
no allocation. Source, line records, coverage and encoded fonts survive through
the final owner, including after session/service destruction.

Nine slots include assigned, queued, completed and retiring requests. Admission
reserves input and slot resources before publishing an ID. Failed admission
preserves the ID. Completed slots remain occupied until consumption/discard;
failed `take` preserves the caller's prior mask. Queued/completed cancellation
retires synchronously; a running cancellation keeps its charge until the worker
returns, drops every candidate/key borrow and retires under the session mutex.
Cancelled output never enters cache. Completion and cancellation linearize under
that same mutex.

Session and service begin-close are nonblocking; join is explicit and waits for
the worker and wake borrow. This refines the earlier proposal's service-level
join-on-begin-close spelling in response to coordinator review. Service
destruction still joins. Cached submit completions request the same coalesced
payload-free worker wake as native completions; they never invoke the wake
target on the opening executor or under the session mutex.
A payload-free wake already selected before close can still be posted, but cannot
deliver an adopted result after revocation. Consumers must keep the target alive
through join. Snapshot acknowledges a coalesced wake; retirement can wake a
consumer waiting for capacity. Replacement is refused until the prior worker
joins. `open_session` also refuses an unjoined output handle, avoiding an implicit
join caused by replacing an unrelated owner. Destructor cleanup joins as well.

## Meaningful fixture checks

`tests/text_mask_lifecycle_tests.cpp` uses the repository-approved Carlito bytes
and a private barrier-controlled backend. Its eleven groups cover:

1. One blocked running request plus eight queued, busy output preservation,
   synchronous queued cancellation, pending running retirement, stale cancelled
   identity, no cancelled cache insertion and completed consumption.
2. Thirty-two one-MiB live masks, eviction with frame-held leases, typed late
   coverage failure preserving previous output, true retirement and reopening
   without resetting the lifetime budget.
3. 709 distinct zero-ink owners, 700-record LRU eviction, object-bound refusal,
   and last-owner release permitting replacement.
4. Quantization and exact cache equality, allocation-free lease reuse, invalid
   UTF-8/NaN/width, tabs, fractional mono, explicit empty-line limit and foreign
   service font rejection.
5. Nonblocking close during a deterministic blocked native-work surrogate,
   refusal to reopen before join, zero post-close completion delivery in that
   ordering, and complete cancellation resource retirement.
6. Exactly two MiB of retained source capacity, source retention after cache
   clear/close, admission rollback at the byte bound and last-owner reclamation.
7. Metadata exhaustion before a shared control-block allocation, complete
   reservation/count rollback, continued service use, and nine completed cached
   requests retaining all slots until cancelled/discarded/consumed.
8. Wrong-executor refusal and explicit unsupported completion from the unfinished
   default native backend.
9. Exact-limit reservation, limit-plus-one refusal, reserved-to-live transfer and
   restoration to baseline for all eleven byte/count resources.
10. Service destruction while a lease survives, readable immutable source and
    coverage afterward, retained font/source/mask charges, and exact metadata
    return to the ledger bootstrap baseline after final release.
11. A cached completion posts a worker-side wake without caller-executor
    invocation; service begin-close returns while the deterministic backend is
    still blocked, revokes admission and permits explicit session join afterward.

## Verification and retained failures

Host: Shadow Windows; borrowed read-only Plan Paint MinGW toolchain selected by
`tools/Enter-WindowsToolchain.ps1`; GCC 16.2.0, C++20. Outputs are under
`gui_forms/.build/`. Direct executable links the previously built
`gui_forms/.build/house-style-text/libgui_forms_core.a`; this is not a fresh full
native application build or a cross-platform test claim.

The initial compile rejected `final` on the private allocator because libstdc++
derives its storage wrapper from the allocator. Both allocators were corrected.
The same strict compile exposed the unchanged Painter header's sign conversion
at `include/gui_forms/types/painter/painter.hpp:74`. That legacy header was not
rewritten. A `-Wall -Wextra -Wconversion -Werror` compile passed. A subsequent
compile restored `-Wsign-conversion -Werror`, using `-isystem gui_forms/include`
to exclude existing public-header diagnostics; authored public headers were
reviewed directly. This is not a claim that all existing public headers pass
those stricter warnings.

Direct compile command (PowerShell, after toolchain setup):

```text
g++ -std=c++20 -Wall -Wextra -Wconversion -Wsign-conversion -Werror
  -isystem gui_forms/include -pthread
  gui_forms/src/core/text/text_mask/text_mask_budget.cpp
  gui_forms/src/core/text/text_mask/text_mask_storage.cpp
  gui_forms/src/core/text/text_mask/text_mask_queue.cpp
  gui_forms/src/core/text/text_mask/text_mask_service.cpp
  gui_forms/tests/text_mask_lifecycle_tests.cpp
  gui_forms/.build/house-style-text/libgui_forms_core.a
  -o gui_forms/.build/text_mask_lifecycle_tests.exe
gui_forms/.build/text_mask_lifecycle_tests.exe gui_forms/assets/fonts
```

Ten groups passed in **14.048 seconds** after the explicit-span spelling
correction; this preceded the coordinator's two lifecycle corrections and their
new regression group. Final verification is recorded below after rebuilding.
Earlier five/eight/nine-group runs also passed; they are superseded by the full
scope. Compile and final result logs are in `.build/text_mask_compile.log` and
`.build/text_mask_lifecycle_result.txt`. No native font benchmark was run.

## House-style source review

Exact reviewed scope: the eleven files in
`TEXT_MASK_STAGE1_HASHES_2026-10-01.csv`: four public headers, six new private
source/header files and the lifecycle test. Reviewed against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`, independently of test results.

Reviewed explicit types and read-only inputs; initialized states and typed unit
fields; named worker/backend/barrier/executor behavior; raw-address and borrowed
wake/backend lifetimes; ownership and destruction ordering; allocation before
publication; checked extent/identifier conversions; rollback and unchanged
outputs; fixed queue/cache traversal; capacity accounting and reuse; lock order
(session before ledger; no ledger-to-session callbacks); and service/lease
lifetimes. Corrected executable return expressions to named results, made borrowed
span conversion explicit, and separated pending slot charges from live slots.
No known house-style violation remains in this authored scope. Existing
prepared-text/typography/test-support code and vendored implementations are not
newly certified. The count-allocation adapter is confined to this actual budget
boundary, not a general allocator architecture.

Stage 2 remains responsible for native font checks, logical wrapping,
paragraph/line bidi and contextual shaping, finite native-work/workspace limits,
gray and true FreeType monochrome rasterization, plus renderer fixtures.

## Final provider verification after coordinator corrections

The final strict compile succeeded and **all eleven lifecycle groups passed in
13.864 seconds** on Shadow Windows. The worker handles pending cached-result
notifications even when no shaping job is queued. Close clears unsent wake
requests; join waits for any wake already in flight before clearing the target.
The amended service close, cached-wake branch, named wake fixture, new regression
and explicit limit enum additions were included in the final semantic house-style
review. `git diff --check` passed. The eleven source hashes are frozen for
coordinator integration review. This is provider verification, not yet an
independent coordinator or native-platform rendering acceptance.

## Independent coordinator integration

The coordinator verified all eleven manifest hashes and independently reviewed
the complete authored files against the house style. Review traced allocator
reservation/rollback and destruction order, aliasing font ownership, exact-key
cache identity/LRU bounds, slot publication, running cancellation, callback
borrows through join, close/reopen and lease survival. Two findings were resolved
before the frozen manifest: nonblocking service begin-close is an explicitly
reconciled refinement of the original proposal; cache-hit completions now wake
on the worker even without queued native work. No remaining violation was found
in this authored scope; legacy dependencies were not certified.

**MEASURED:** a fresh CMake Release build at
`gui_forms/.build/text-mask-stage1`, GCC 16.2, with prepared text OFF, text masks
ON and native hosts OFF compiled Core, this service and its fixture. The focused
CTest passed all eleven groups in **13.82 seconds** (13.83 seconds total). This
independently establishes the source service's separation from the optional A2
Painter ABI. The eleven-file spelling check reported zero findings.

**MEASURED installation boundary:** installing this ON build refused before
creating `.build/text-mask-on-install-check`. The existing normal Windows build
was reconfigured with text masks OFF and successfully installed to the fresh
`.build/text-mask-off-install-check`; neither the umbrella nor directory of mask
headers appeared. This check reused existing normal SDK binaries; it is an
export-boundary check, not a new application distribution measurement.

The coordinator's CMake option defaults OFF and requires the admitted text stack.
The lifecycle CTest has a 30-second timeout. Native CI now exports the ordinary
OFF SDK first, then enables/builds/runs the source-only lifecycle test on all
three platforms. That subsequent test configuration has no installed development
headers or changed Core/Painter ABI. Native execution of this new CI step is
pending; the private fixture still supplies masks and production requests still
complete as `unsupported_profile` until Stage 2.
