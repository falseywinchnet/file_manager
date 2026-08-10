# Future sibling implementation handoff

Status: **instructions prepared; implementation BLOCKED**.

## Mission after opening

Perform an exact-behavior in-place rewrite of GUI.Forms production `include/`
and `src/` first, then repair supporting first-party source as required.
Preserve paths, ABI ownership, semantics, tests, and dependent-library
boundaries; minor native C++ API changes germane to the rewrite are permitted.

## Read before any source edit

1. Parent and GUI.Forms `AGENTS.md`.
2. Parent planning and decision protocol.
3. ADR-014 and GUI.Forms lifecycle contract.
4. Orchestrator GUI.Forms negotiation and registry entries.
5. Every file in `gui_forms_rewrite/`, treating
   `ARCHITECT_SELECTIONS.md` as the later closure authority.
6. `IMPLEMENTATION_START_CHECKLIST.md` and `ENFORCEMENT_SPEC.md` immediately
   before proposing the first batch.

## Preconditions

- Explicit architect direction to begin.
- Architect selections reconciled (already recorded here).
- Exact worktree/overlap review complete.
- Semantic banned-construct inventory refreshed.
- Public/API/ABI and behavioral oracles recorded.
- First batch approved.

## Non-negotiable rewrite rules

- Minor native C++ API changes germane to the rewrite are allowed and reported
  per batch. No C ABI ownership, file-layout, host-object, dependent-library,
  broad API, or behavioral change without asking the architect directly.
- No repository-wide token replacement.
- No STL purge or private standard library.
- Keep `std::vector` as the ordinary growable contiguous collection.
- Keep permitted smart pointers and current shared/weak retained ownership.
- Use `OWNERSHIP_AUDIT.md` only as evidence for a future lifecycle round; do not
  act on it during this rewrite.
- Keep approved standard algorithms.
- Keep `std::function` unless a focused audit justifies a concrete cleanup.
- Do not merge Event and dispatch architecture.
- Do not introduce allocator propagation, firmware targets, constrained
  runtimes, or Skia/text/platform restructuring.
- Do not link GUI.Forms against BFFT; copy/adapt bounded proven machinery only
  when a concrete call site earns it.
- Do not migrate third-party source.
- Do not delete tests or negative evidence to make a batch pass.

## Required source grammar

For in-scope first-party C++:

- no `auto` or `decltype(auto)`;
- no trailing return;
- no pointer-member arrow syntax;
- no lambdas or generic lambdas;
- no structured bindings;
- no coroutines;
- no `std::any` outside the existing WinForms-compatible Tag surface, backing
  storage, disposal path, and necessary compatibility tests/shims;
- no convenience-defaulted spaceship/comparisons;
- explicit named types, functions, functors, contexts, delegates, and required
  comparisons;
- templates, `consteval`, and predictable `if constexpr`/pack folding in named
  templates/functors permitted;
- `decltype(expression)` permitted only in named private type-trait/detection
  plumbing;
- unlisted feature cases follow the ledger's epistemic-cost questions and are
  brought back to the architect when material.

## Required batch proposal

State:

- exact files and why they form one semantic batch;
- banned constructs/rewrite decisions addressed;
- explicit replacement types and control flow;
- behavior intended to remain identical;
- lifetime, allocation, exception, and threading consequences;
- minor native C++ API delta report and C ABI/dependent-boundary confirmation;
- tests and measurements before/after;
- user-work overlap and rollback boundary.

## Verification cadence

- Focused characterization/unit tests for every batch.
- ABI and headless traces for affected paths.
- Banned-construct enforcement for touched files.
- Allocation/lifetime tests for Delegate/Event or specialized storage.
- Sorting equivalence and workload benchmarks for sorting batches.
- Periodic full native suite through the recorded M4 build helper for heavy
  work.
- Documentation regeneration only at stable public-source checkpoints.

## Stop and ask when

- a concrete case conflicts with a decided rule;
- a proposed native C++ API change is broad or not germane to the rewrite;
- lambda removal exposes unclear ownership or cancellation;
- event behavior cannot be preserved by the proposed Delegate/Event design;
- a sort candidate changes ordering/stability or loses materially;
- a file/API/dependency boundary must move;
- current user work overlaps unsafely;
- an open decision becomes blocking.

## Completion report

Report each accepted O-032-A criterion separately: banned constructs, paths,
minor native API deltas, C ABI/dependent-boundary preservation, behavior gates,
Delegate/Event evidence, retained justified
`std::function` uses, sorting decisions and negative results, binary-search
closure, specialized-storage exceptions, dependency boundaries, and remaining
open or deferred work.
