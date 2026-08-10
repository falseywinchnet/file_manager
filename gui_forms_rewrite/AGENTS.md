# GUI.Forms rewrite planning guardrails

This is an interview-first planning and handoff directory. It does not authorize
edits to `../gui_forms/`.

## Absolute boundary

- Do not edit, format, regenerate, or mechanically rewrite `../gui_forms/` while
  working from this planning directory.
- Preserve the current dirty worktree.
- Read the parent `AGENTS.md`, `../gui_forms/AGENTS.md`, parent decision protocol,
  and every file here before proposing implementation.
- `ARCHITECT_SELECTIONS.md` closes the older `OPEN` items in
  `DECISION_LEDGER.md` and is the later authority.
- Minor native C++ API changes germane to the rewrite are authorized and must
  be reported per batch. Bring any C ABI, file-layout, platform-object,
  dependent-library, broad API, or behavioral change directly to the architect.

## Rewrite identity

This is an exact-behavior in-place rewrite of production `include/` and `src/`
first, followed by necessary supporting-source repair. It is not:

- an STL purge;
- a portability or firmware project;
- a private standard library;
- an allocator-propagation project;
- an ownership redesign;
- a public API or ABI redesign;
- a dependency-boundary rewrite.

## House rule

Minimize epistemic cost. Prefer explicit named machinery when a construct hides
type, control flow, lifetime, allocation, or execution beyond what the problem
justifies. Keep normal C++ and STL machinery when it remains the clearest known
choice and its implementation details do not matter to GUI.Forms.

## Required implementation method after opening

1. Freeze exact behavior with existing and added characterization tests.
2. Inventory banned constructs semantically, excluding experiment folders.
3. Replace constructs in small compilable batches without moving files;
   record every germane minor native C++ API delta.
4. Separate syntax cleanup from ownership, dispatch, allocation, and lifecycle
   changes.
5. Implement only the small approved toolbox: event/delegate, binary search,
   measured sorting choices, and later concrete facilities that pass their own
   gate.
6. Preserve negative results and compare house algorithms against standard
   controls.
7. Do not cite hypothetical UEFI/bare-metal work to broaden this rewrite.

## Implementation gate

The decision interview is closed. Implementation is **BLOCKED** until the
architect explicitly directs a sibling to begin, the starting snapshot/overlap
is frozen, semantic inventories and equivalence oracles are refreshed, and the
first batch is reviewed.
