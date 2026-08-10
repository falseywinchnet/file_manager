# Future sibling handoff

Status: **prepared instructions; implementation currently BLOCKED**.

## Mission after the gate opens

Rewrite `../gui_forms/` in place according to the accepted decisions in this
directory, preserving GUI.Forms behavior and registered contracts while
reducing its hosted C++/libc dependency surface.

## Before any source edit

1. Confirm the grand architect explicitly opened implementation.
2. Read parent and GUI.Forms `AGENTS.md`, all files in this directory, the
   decision protocol, ADR-014, lifecycle contract, and Orchestrator GUI.Forms
   negotiation.
3. Verify every phase-zero ledger item is `DECIDED` or explicitly deferred with
   a reason that does not block the first phase.
4. Inspect the dirty worktree. Do not overwrite or reformat user changes.
5. Record the exact branch, commit, status, compiler, dependencies, and source
   scope.
6. Refresh every audit count using semantic tooling.
7. Present the first implementation batch and overlap analysis before editing.

## Prohibited implementation behavior

- No repository-wide search-and-replace migration.
- No appeal to existing BFFT `auto`, lambda, or `std::vector` usage as precedent;
  the architect has classified that usage as undesirable cleanup debt.
- No `std::vector` typedef masquerading as a house container.
- No generic in-house `std::function` clone before callback roles are measured.
- No lifecycle, event-order, retained-identity, accessibility, rendering, or
  host-policy changes hidden inside language cleanup.
- No C/C++ ABI freeze inferred from internal object layout.
- No claim of freestanding, UEFI, bare-metal, lightweight, or faster without an
  accepted build/run workload.
- No migration of third-party sources.
- No deletion of negative results, compatibility fixtures, or tests to make a
  batch pass.
- No generated documentation churn during private implementation waves.
- No remote edit of the M4 mirror; local source remains authoritative.

## Required batch format

Every batch proposal must state:

- exact files and ownership layer;
- accepted decisions it implements;
- old representation and new representation;
- behavior intended to remain identical;
- allocation, OOM, lifetime, exception, and thread-affinity consequences;
- temporary bridge and removal gate;
- tests and measurements before/after;
- rollback boundary;
- known overlap with current user work.

## Verification cadence

- Focused unit/characterization tests for the batch.
- ABI/headless trace checks for any affected contract path.
- strict compile and policy checker for the migrated layer.
- sanitizer/failure-injection checks for ownership or container work.
- periodic full native GUI.Forms test pass using the repository's recorded M4
  build commands when compute-heavy.
- binary dependency and code-size comparison at each phase boundary.

## Stop conditions

Stop and return to the architect when:

- a decision is absent or contradictory;
- behavior must change to complete a representation migration;
- a public or registered ABI meaning would change;
- an allocator/callback primitive fails its proof gate;
- user changes overlap a proposed batch in a way that cannot be safely merged;
- the constrained target requires a materially broader scope than accepted;
- measurements show a significant regression without an accepted tradeoff.

## Completion handoff

Do not declare completion from construct counts alone. Report the accepted
R-021 criteria individually, remaining exemptions, dependency manifests,
behavioral evidence, performance/footprint results, constrained-runtime proof,
and reversal path.
