# CBMC verification audit

Status: **OBSERVED gap; no CBMC proof is presently claimed**.

## What exists

GUI.Forms has ordinary unit/integration tests, historical focused ASan/UBSan
runs, and an opt-in PNG mutation/libFuzzer target compiled with address and
undefined-behavior sanitizers. Those are useful execution-based controls. They
can detect bounds, pointer, overflow, shift, and related faults only on the
executions they actually run.

There is no first-party CBMC harness, property set, checked-in CBMC command,
unwind assignment, unwinding-assertion result, or CI/build gate in the current
tree. Therefore none of the following is currently a GUI.Forms model-checking
claim:

- bounds safety;
- pointer safety and object lifetime;
- division-by-zero absence;
- signed-overflow absence;
- shift validity;
- sufficient loop unwinding;
- assertion reachability over all admitted inputs.

A bounded behavioral test is not an unwinding proof. A sanitizer pass is not a
symbolic proof. A successful CBMC run with an insufficient unwind bound is also
not a proof unless its unwinding assertions pass.

## Smallest honest nominal lane

The first lane should be a feasibility experiment, not a rewrite completion
gate. It should begin with the house binary-search implementation because its
state is finite, its loops monotonically shrink, and its postconditions can be
stated without modeling a retained GUI, allocator, host callback, or thread.
Admission requires all of these conditions:

1. The harness compiles the production implementation itself. A separately
   retyped C model does not count as proof of the C++ source.
2. The input extent is bounded and nondeterministic; the harness assumes only
   the documented sorted-range precondition.
3. Lower bound, upper bound, and exact membership are checked against direct
   mathematical postconditions, including empty, duplicate, and boundary
   cases.
4. Bounds, pointer, division-by-zero, signed-overflow, and undefined-shift
   checks are named explicitly in the recorded command even when enabled by
   the installed CBMC version. This prevents defaults from silently becoming
   project policy.
5. Unwinding assertions remain enabled. Every harness/setup loop and every
   production loop either has a justified named unwind bound or is proven by a
   supported loop invariant. The result must contain no unwinding failure.
6. The exact CBMC version, target data model, command, properties checked,
   runtime, peak memory, and result are retained as evidence.

Only after that spike should a second target be selected. Good candidates are
small C ABI argument/bounds validators or fixed-capacity state transitions.
`Event`, dispatcher, live-surface, retained-tree, allocator, and host models are
poor nominal targets until a harness can preserve their actual lifetime and
concurrency decisions instead of stubbing those decisions away.

## Present decision

CBMC is not silently added to rewrite completion and no unrun harness is called
coverage. The existing rewrite remains exact-behavior and syntax-policy work.
This audit establishes the missing proof boundary and the acceptance rule for a
separate, deliberately bounded model-checking round.
