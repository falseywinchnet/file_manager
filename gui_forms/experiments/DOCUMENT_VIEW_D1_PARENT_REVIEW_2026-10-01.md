# D1 parent source review — 2026-10-01

Status: development-model review; no installed SDK, retained editor or release
acceptance is implied. The full `planning/PROGRAMMING_HOUSE_STYLE.md` was read
for this review. The canonical contract is
`orchestrator/spec/contracts/GUI_DOCUMENT_VIEW_DEVELOPMENT.md`.

## Exact scope

Reviewed the new umbrella `include/gui_forms/document_view.hpp`, the types and
state headers under `include/gui_forms/document_view/`,
`src/core/document_view/document_view_state.cpp`,
`tests/document_view_tests.cpp`, `benchmarks/document_view_benchmark.cpp`, and
the narrow source/test/benchmark additions in `CMakeLists.txt`.

Read the existing lower-bound and UTF-8 validation dependencies to check the
new caller's bounds and exception assumptions. This does not certify those
legacy files against the house style.

## Findings

- OBSERVED: publication and both mapping directions compare the complete
  request. A changed bind clears the published page and authority while keeping
  occupied producer slots until explicit acknowledgement. Invalid requests are
  checked before slot capacity and do not mutate authority.
- OBSERVED: publication validates capacity, UTF-8 and contiguous ordered source
  and display partitions before releasing the old page. Successful publication
  transfers owned payload storage through nonthrowing moves. Rejection preserves
  the incoming owner and occupied slot. The moved-from page is empty.
- OBSERVED: partition validation and admitted lookup extents establish an
  in-range lower-bound result. Identity-span length equality bounds the
  uint64-to-uint32 relative conversion. Scalar and atomic-token interiors are
  rejected rather than rounded. No floating-point source offset is used.
- OBSERVED: executable behavior is named; new state is explicitly initialized;
  mutation and test assertions are separated; no retained callback or hidden
  worker is introduced. Page construction reserves known workload storage
  before its element loop. Public observation borrows have explicit invalidation
  rules. Cross-model token routing is forbidden by the owning adapter contract;
  request records are not globally unique capabilities.
- OBSERVED: the six new C++ files passed the spelling checker with zero findings
  during review. This supplements the source review, not replaces it.

The initial benchmark claimed both-direction endpoint checking while checking
only display-to-source. Parent requested reverse checks, explicit forged-token
mapping fixtures, hoisted invariant conversions, and storage accounting for two
producer pages plus the published page. Source reread confirmed those changes.
The corrected benchmark checks every endpoint in both directions before lookup
timing. New mapping fixtures reject altered viewport, permitted interval and
revision in both directions. Storage accounting extrapolates the workload's
payload capacity; it is not measured process peak memory or allocator overhead.

The original maximum-page publication worst sample was 9.5329 ms; the reviewed
run recorded 3.9451 ms. Both results are retained. The reviewed run also had a
15.6 microsecond worst binary-mapping sample despite a 0.2 microsecond median.
These tails preclude claiming all mapping work stayed below a microsecond.

## Parent verification

After the review corrections, the parent independently ran the configured
Windows CMake build for `gui_forms_document_view_tests` and
`gui_forms_document_view_benchmark`; Ninja reported both up to date. Focused
CTest passed 1/1 in 0.07 seconds total. The six-file spelling scan again had
zero findings and `git diff --check` passed. These checks do not stand in for
the full GUI.Forms regression suite or platform-native testing.

## Remaining gates

The serial-exhaustion guard was inspected but is not fixture-driven. Do not add
a public testing seam or tiny arithmetic wrapper solely to claim that coverage.
The allocation/context tests model terminal acknowledgement; they do not verify
a real producer, source scratch ownership, oversized-grapheme context, dispatch
teardown, or backpressure integration. Those require adapter evidence.

Model-only measurements exclude file reads, projection construction, shaping,
painting and interactive input. Preserve tail samples and clock resolution in
the measurement record. Warm mapping speed is not evidence of complete editor
or File Manager responsiveness. Independent installed-consumer validation and
the later retained-control/layout contracts remain required before adoption.
