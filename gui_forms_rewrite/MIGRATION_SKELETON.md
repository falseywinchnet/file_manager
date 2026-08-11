# Exact-behavior migration skeleton

Status: **historical O-030-A sequence; completed on 2026-08-10**.

This sequence records the order used by the completed rewrite. It does not
authorize a new migration round.

## Phase 0 — close and freeze

- Treat `ARCHITECT_SELECTIONS.md` and O-032-A as the closed decision/completion
  authority.
- Record the exact dirty-tree reconciliation point.
- Refresh semantic inventories.
- Freeze public declarations, ABI fixtures, lifecycle traces, and dependent
  boundaries as equivalence oracles.
- Select enforcement tooling and temporary-exemption format.

Exit: architect implementation direction and a reviewed first batch.

## Phase 1 — policy gate in observation mode

- Add the narrow AST-backed banned-construct checker.
- Report violations without immediately failing legacy files.
- Ratchet each file/directory to zero new violations once touched.
- Keep experiment folders out of the production policy scope.

Exit: complete classified inventory with exact replacement categories.

## Phase 2 — binary-search toolbox proof

- Implement tiny named lower-bound, upper-bound, and exact-search templates.
- Use explicit comparator/functor types.
- Prove boundary, duplicate, overflow, fuzz, and standard-equivalence behavior.
- Migrate current binary-search-family call sites to the selected contiguous
  range/index core, recording any germane minor native C++ API delta.

Why early: it is small, explicitly approved, and proves toolbox style without
entangling ownership.

## Phase 3 — Delegate/Event proof

- Characterize current event semantics and allocations.
- Design named `Delegate<Signature>` binding with no heap allocation merely to
  bind a target.
- Add the Delegate-first Event path while retaining the legacy `std::function`
  overload for compatibility.
- Keep Event subscription order, token revocation, reentrancy, removal, emission,
  disposal, and exception semantics distinct from Delegate invocation.
- Migrate one behavior-rich event family and compare exact traces/allocations.
- Expand only after the proof survives real controls and C ABI-facing behavior.

Dispatch, commands, timers, host services, and paint callbacks do not join this
phase automatically.

## Phase 4 — explicit-language waves

Migrate small related production file groups while preserving paths and exact
behavior; report permitted minor native C++ API changes:

1. replace `auto`/structured bindings/trailing returns with explicit named types;
2. replace lambdas with named comparators, functors, helpers, contexts,
   delegates, or trampolines;
3. replace pointer arrow syntax with explicit dereference and dot;
4. replace banned comparison conveniences with only required named operators;
5. remove `std::any` outside the existing WinForms-compatible Tag exception and
   remove coroutine constructs where present according to concrete behavior;
6. replace the one current ASCII-lowercase `std::transform` with a direct loop;
7. replace the requires-expression with the explicit initialization trait and
   apply the selected C++20-feature policy.

Do not mix shared-ownership redesign, dispatch redesign, allocator propagation,
or dependency changes into these waves.

## Phase 5 — sorting laboratory and call-site migration

- Gather real workload distributions for every sort/stable-sort site.
- Implement the smallest justified candidate set.
- Start the laboratory with the selected stable-insertion, run-aware stable
  merge/adaptive, and explicit-stack unstable partition/introspective
  candidates.
- Benchmark against libc++ and Windows standard-library controls.
- Preserve exact ordering and stability.
- Migrate only call sites where a house choice is measured or materially more
  inspectable.
- Retain standard sorting where no candidate earns replacement, recording that
  result honestly.

## Phase 6 — focused `std::function` cleanup

- Audit semantic roles after lambdas are gone and events are migrated.
- Clean only obvious allocation-heavy, copy-heavy, opaque, or lifetime-awkward
  cases.
- Use named machinery and keep ordinary justified `std::function` uses.
- Do not force dispatch or platform services through Delegate/Event.

## Phase 7 — specialized storage experiments, only if earned

- Evaluate BFFT-style heap arrays only for presently large, non-growing heap
  allocations that are edited in place.
- Leave ordinary growable collections as `std::vector`.
- Keep hive-like stable pools, allocator work, and concurrency replacements out
  unless separately opened from measured GUI.Forms need.

## Phase 8 — supporting-source repair and closure

- After production is rewritten, update tests, demo, tools, compatibility, and
  first-party generators/output only where required to compile, exercise, or
  represent the production contract.
- Bring normative production source to zero unapproved banned constructs.
- Close or architect-approve every narrow exemption.
- Run exact lifecycle, ABI, headless, control, renderer, host, accessibility,
  managed-facade, and installed-consumer gates.
- Regenerate documentation only after declarations and source organization are
  confirmed unchanged.
- Record measured sorting choices and negative results.
- Verify no BFFT link dependency and no speculative portability restructuring.

## Batch law

Every batch remains compilable, reviewable, and reversible. It names exact
files, behavior oracle, replacement category, tests, measurements, and rollback.
A mass token replacement is never a batch.
