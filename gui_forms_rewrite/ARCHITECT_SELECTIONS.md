# GUI.Forms rewrite architect selections

Status: **DECIDED on 2026-08-10 and implemented; see
`COMPLETION_REPORT.md`**.

This record closes the option queue in `OPEN_ITEMS.md`. The detailed option
descriptions remain there as reversal evidence. Where this record and an older
`OPEN` label in `DECISION_LEDGER.md` differ, this later architect selection
controls.

## Scope and language

### O-001 — production-first scope

**Selected: O-001-C plus the O-001-A work needed after production changes.**

Interpretation of the architect's shorthand: `include/` and production `src/`
are the normative rewrite and enforcement scope. Rewrite them first. Only after
that production change, update first-party tests, demo, tools, compatibility
code, and generated output where necessary to compile, exercise, or accurately
represent the rewritten production contract. Those supporting surfaces do not
independently drive production architecture or the rewrite order.

Experiments and third-party source remain excluded. If first-party generated
output must change, change its generator/template and regenerate rather than
maintaining the output by hand.

This interpretation is deliberately written out because the selected answer
combined C with the necessary part of A.

### O-002 — compiler mode and feature age

**Selected: O-002-A, clarified.**

Keep the project in C++20 language mode. Current/latest compiler toolchains may
be used; language mode is not a demand to use an old compiler. The house subset
does not evolve merely to absorb every feature added by C++20, C++23, or later
standards.

Audit the actual uses of features newly added in C++20. Admit them under the
epistemic-cost rule rather than by version badge. `consteval` is expressly
permitted. No move to C++23 language mode or importation of C++23 features is
authorized merely because the toolchain implements them.

### O-003 through O-010 — remaining language edges

- **O-003-B:** replace the one requires-expression with an explicit specialized
  trait for compound-control initialization.
- **O-004-A:** permit `if constexpr`, pack expansion, and predictable
  compile-time folding in named templates/functors. This is compiler execution
  of an already-stated choice, not undesirable inference.
- **O-005-B:** permit `decltype(expression)` only inside named private
  type-trait/detection plumbing; never ordinary variables, public signatures,
  return inference, or `decltype(auto)`.
- **O-006-B:** ranges/views are not admitted without a concrete proposal.
- **O-007-B:** designated initializers may remain for plain option/configuration
  records; stateful success/failure results use named factories.
- **O-008-B:** retain clear bit predicates such as `has_single_bit`; require a
  concrete representation case before admitting `bit_cast`.
- **O-009-B:** unlisted features follow the governing epistemic test; concrete
  platform/compiler necessities may remain, while material inference and new
  extensions return for review.
- **O-010-A:** preserve public comparison signatures with explicit
  implementations. Source-private types retain only comparisons proven useful.
  An explicit spaceship may preserve an existing public signature; convenience-
  defaulted comparison machinery remains banned.

## API, Tag, Delegate, and Event

### O-011 — WinForms-compatible Tag

**Selected: O-011-A.**

Retain `std::any` only for the intentional WinForms-compatible native Tag
surface and the first-party compatibility/dogfooding code needed to prove it.
Ban `std::any` everywhere else in the normative production rewrite.

This is a bounded compatibility exception, not permission for type-erased
storage in ordinary architecture. The exception covers the four existing Tag
owners—Control, ImageList, ErrorProvider, and HelpProvider—their backing
storage, accessors, disposal reset, and necessary tests/facade shims. It does
not authorize new Tag-bearing types without architect approval.

### O-012 — minor API changes

**Selected: O-012-C.**

Minor native C++ API changes are permitted during the rewrite. GUI.Forms is not
presently consumed outside experiments, so preserving every source spelling is
not worth obstructing a coherent house design.

This permission does not authorize a C ABI ownership change, dependent-library
boundary change, hosted-object contract change, behavior change, file
reorganization, or broad API redesign. Changes must remain germane to the
rewrite and be reported batch by batch. Experiments and compatibility shims may
be updated after the production surface changes.

### O-013 through O-017 — callbacks and execution

- **O-013-B:** add a Delegate-first Event path while retaining the legacy
  `std::function` callback overload for compatibility. First-party production
  code migrates to Delegate.
- **O-014-A:** Delegate is a non-owning context/object pointer plus compile-time
  invocation thunk. Binding allocates nothing; lifetime remains separate and
  explicit.
- **O-015-A:** first preserve the current Event state/slot, snapshot,
  registration order, mutation-during-emission, owner revocation, token,
  exception, and statistics semantics. Slot-storage optimization is a later
  measured step.
- **O-016-A:** commands, dispatcher work, timers, host services, and paint
  callbacks preserve their existing distinct lifetime/concurrency machinery
  and are excluded from Event unification.
- **O-017-B:** audit `std::function` quantitatively and replace only concrete
  allocation-heavy, copy-heavy, opaque, or lifetime-awkward cases.

## Preservation decisions

- **O-018-A:** no allocator replacement, propagation, or process-allocator
  program in this rewrite.
- **O-019-A plus evidence tooling:** do not change shared/weak retained
  ownership in this rewrite. A Python ownership tracer must inventory and
  surface current ownership into an audit file for a future, separately opened
  lifecycle-change round. Producing that evidence is not permission to begin
  the lifecycle round.
- **O-020-A:** retain ordinary strings/text buffers; admit only concrete
  call-site improvements.
- **O-021-A:** preserve exception and RTTI behavior, audit routine/hot misuse,
  and retain ABI/failure-boundary containment.
- **O-022-A:** preserve threads, atomics, clocks, timer/frame scheduling,
  cancellation, waiting, and shutdown behavior. House concurrency is not part
  of this rewrite.
- **O-023-A:** retain concrete filesystem, stream, classic-locale, formatting,
  and hosted dynamic-loading uses.
- **O-024-B:** replace the one current ASCII-lowercase `std::transform` with a
  direct explicit loop. Other clear standard algorithms remain by default.

## Algorithms and specialized storage

- **O-025-B:** house binary search uses a contiguous range/index core with
  named lower-bound, upper-bound, and exact-search operations and explicit
  comparators.
- **O-026-B:** a house sort must preserve semantics, avoid material regression,
  and win a declared call-site dimension. Standard implementations remain the
  controls and may remain the selected result.
- **O-027-B:** the sorting laboratory includes stable insertion, a run-aware
  stable merge/adaptive sort, and an explicit-stack unstable
  partition/introspective sort. Use optimal forms of these candidates so the
  laboratory itself reflects good house style. Ship only the subset earned by
  call-site measurements.
- **O-028-B, narrowed:** a BFFT-style heap array is admitted only for presently
  large, non-growing heap allocations that are subsequently edited in place.
  It is not for small arrays, ordinary growable collections, or a vector whose
  required behavior includes growth. “Large” must be stated from the concrete
  call-site distribution and memory footprint before selection.

## Files, order, enforcement, and completion

- **O-029-B:** preserve existing paths and permit a few new purpose-named
  private toolbox files. Do not create a private-runtime hierarchy.
- **O-030-A:** observation-mode checker, binary-search proof, one behavior-rich
  Delegate/Event proof, explicit-language waves, sorting laboratory, focused
  `std::function` audit, then closure.
- **O-031-B plus warnings:** use a narrow standalone Clang LibTooling checker as
  the semantic authority, with cheap textual controls. Tightening relevant
  compiler warnings is also admitted as a supplementary ratchet; warnings do
  not replace the AST gate.
- **O-032-A:** use the exact-behavior completion definition in
  `OPEN_ITEMS.md`, interpreted with the production-first O-001 scope and the
  approved minor-API permission.
- **O-033-B:** use affected-workload performance/build acceptance. Predeclare
  measurement margins and preserve negative results; do not impose one false-
  precision percentage on unrelated mechanisms.

## Gate after these selections

Historical gate: the option interview, evidence preflight, exact snapshot, and
explicit start direction were satisfied. O-032-A closure is recorded in
`COMPLETION_REPORT.md`. Reopening a selected decision now requires a new round,
not reinterpretation of the pre-implementation gate.
