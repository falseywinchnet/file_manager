# Migration skeleton

Status: **CANDIDATE sequence only; no implementation permission**.

This sequence exists to structure discussion. Exact phases change after the
decision ledger closes.

## Phase 0 — freeze meaning, not code

- Record the exact rewrite source snapshot.
- Reconcile concurrent user work before touching overlapping files.
- Freeze behavioral oracles and ABI fixtures.
- Close R-001 through R-021 or explicitly defer nonblocking items.
- Define directory/layer profiles and exemptions.
- Approve the first constrained build target.

Exit: accepted rewrite ADR(s), accepted house grammar, source inventory, and
architect implementation direction.

## Phase 1 — enforcement in observation mode

- Add AST inventory and include/symbol dependency reports.
- Generate violations without failing existing code.
- Check reports into the rewrite evidence area, not generated documentation.
- Establish ratchets so new violations cannot appear after a layer enters
  migration.

Exit: complete classified inventory with no unknown high-risk callback,
ownership, or public-container sites.

## Phase 2 — base and allocation laboratory

- Implement only the accepted checked arithmetic, status, allocator, view, and
  heap-array primitives.
- Prove them independently under normal and injected failure.
- Compare code size and performance against the standard controls.
- Reject or revise primitives that merely imitate STL badly.

Exit: primitive contracts accepted for production use.

## Phase 3 — one vertical retained slice

Migrate a deliberately small but semantically rich path, likely:

- stable ID/value geometry;
- one retained container and child collection;
- one event;
- one dispatcher work item;
- one simple control;
- headless host trace;
- C ABI projection.

The exact slice is a decision. It must exercise ownership, allocation,
callbacks, lifecycle, and public seams without involving every renderer.

Exit: old/new behavior matches and the house primitives survive real use.

## Phase 4 — container waves

Migrate by semantic category, not global token replacement:

1. immutable/fixed snapshots and byte buffers;
2. private growable collections;
3. child/component/registry ownership collections;
4. public return/argument types with compatibility bridges;
5. associative collections after their lookup/order requirements are measured;
6. render/text/image planes and high-volume buffers.

Each wave removes its bridge or records why it remains.

## Phase 5 — callback waves

Migrate separately:

1. synchronous visitors/predicates;
2. C ABI callbacks;
3. retained events/subscriptions;
4. host service tables;
5. dispatcher work/cancellation;
6. cross-thread tasks;
7. paint/animation hot paths;
8. inspector/converter extension points.

Do not introduce one universal callable merely to finish the wave faster.

## Phase 6 — explicit types and lambda grammar

Once house containers and callbacks expose stable named types:

- replace `auto` with meaningful names;
- convert disallowed lambdas to named functions, functors, listeners, or work
  records;
- retain only approved category- and layer-specific exemptions;
- enable enforcement ratchets per migrated directory.

Doing this earlier would spell unstable implementation types and create churn.

## Phase 7 — hosted-runtime extraction

- move filesystem, streams, locale, formatting, dynamic loading, clocks,
  scheduling, synchronization, and platform services behind accepted seams;
- implement the normal desktop backend first;
- build the constrained nucleus against the selected runtime/shim;
- prove actual link and execution, not header-only compilation.

## Phase 8 — subsystem closure

Migrate core, controls, drawing, render adapters, text engine, hosts, and ABI in
bounded batches. Preserve independent buildability and run the full applicable
test matrix after every batch.

## Phase 9 — compatibility and cleanup

- remove expired standard-container/callback bridges;
- close or explicitly retain exemptions;
- regenerate library documentation only after public declarations stabilize;
- validate managed facade and installed consumer projections;
- run native desktop, headless, ABI, renderer, and constrained-runtime gates;
- record negative results and residual hosted dependencies.

## Phase 10 — completion audit

Evaluate the accepted R-021 criteria. A successful desktop rewrite does not
automatically satisfy the later UEFI/framebuffer milestone.

