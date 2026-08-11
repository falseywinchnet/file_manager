# GUI.Forms rewrite operational policy

Date: 2026-08-10

Authority: extracted from `DECISION_LEDGER.md` and the later closure in
`ARCHITECT_SELECTIONS.md`. The later selections control if an older `OPEN`
label differs.

## Rewrite boundary

- The exact-behavior rewrite was completed on 2026-08-10; this policy remains
  authoritative for maintenance of the rewritten source.
- Future lifecycle, allocator, constrained-runtime, and model-checking rounds
  remain separately gated.
- Rewrite `include/` and production `src/` first as the normative scope.
- After production changes, update first-party tests, demo, tools,
  compatibility code, and generated output only where needed to compile,
  exercise, or represent the new production source. Fix first-party generators
  before generated output.
- Preserve file organization, ABI ownership, semantics, behavior,
  dependent-library boundaries, and platform boundaries.
- Minor native C++ API changes germane to the rewrite are permitted and must be
  reported by batch. They do not authorize C ABI, dependency, hosted-object,
  or behavioral changes.

## Governing idea

Epistemic cost is the criterion. Remove machinery that hides type, control flow,
lifetime, allocation, or actual execution more than the problem warrants.
Compiler machinery is good when it instantiates an explicit architectural
decision; inference is bad when it makes the decision for the source.

## Banned in in-scope first-party C++

- `auto` and `decltype(auto)`;
- trailing-return `->` syntax;
- pointer member `->` syntax—dereference explicitly and use dot;
- all lambdas, including captureless and generic lambdas;
- structured bindings;
- coroutines;
- `std::any`, except the existing bounded WinForms-compatible Tag surface,
  backing storage, disposal behavior, and necessary compatibility tests/shims;
- convenience-defaulted spaceship/comparison machinery; define only required
  comparisons.

`decltype(expression)` is permitted only in named private type-trait/detection
plumbing. `decltype(auto)` remains banned.

## Permitted and encouraged where appropriate

- explicit named types and aliases;
- templates used as explicit, inspectable toolbox machinery;
- `consteval`;
- `if constexpr`, pack expansion, and predictable compile-time folding in
  named templates/functors;
- `std::vector` as the ordinary growable contiguous collection;
- `std::unique_ptr`;
- `std::shared_ptr` and `std::weak_ptr` under the current retained ownership
  model; changing them is a separate lifecycle project;
- `std::span`, `std::optional`, and `std::variant`;
- RTTI where the existing architecture requires it;
- exceptions at genuine subsystem/failure boundaries, never as routine control
  flow and never across ABI boundaries;
- `std::function` generally, subject to focused audit of conspicuously wasteful,
  opaque, allocation-heavy, or lifetime-awkward uses.

GUI.Forms remains in C++20 language mode and may use current toolchains. Audit
actual features added in C++20; the mode does not admit every C++20/C++23 idea.
Ranges/views require a concrete proposal. Designated initialization remains for
plain option records; stateful result alternatives use named factories. Clear
bit predicates remain; representation casts require a concrete case.

## Approved standard algorithms

Keep:

- `min`, `max`, `clamp`;
- `find`, `find_if`;
- erase/remove;
- `copy`, `copy_n`;
- range `move` and ordinary object `move`;
- `accumulate`, `inner_product`;
- `any_of`, `all_of`, `none_of`;
- `visit`.

Do not replace other algorithms merely because they are standard-library
facilities.

The one current ASCII-lowercase `std::transform` becomes a direct explicit
loop. Other unlisted clear standard algorithms remain by default.

## Algorithms requiring deliberate ownership

- Audit every `std::sort` and `std::stable_sort` call site against actual sizes,
  ordering, duplicates, stability, element movement, comparator cost, hotness,
  scratch tolerance, and cross-STL behavior.
- Build only a small, descriptive house sort collection selected and measured
  against representative GUI.Forms workloads and standard implementations.
- The laboratory candidate set is stable insertion, run-aware stable
  merge/adaptive sorting, and explicit-stack unstable partition/introspective
  sorting. Ship only the measured subset.
- House-own the small binary-search family with a contiguous range/index core.
- Do not casually introduce heap algorithms; choose an explicit data structure
  if priority behavior is actually needed.

## Events and callbacks

- All lambdas are replaced by named functions, functors, context structures,
  delegates, or platform trampolines.
- `std::function` is not globally replaced.
- Events are the principal approved house abstraction:
  `Delegate<Signature>` handles named binding; `Event<Signature>` handles
  ordering, subscription, revocation, removal, and emission.
- Retain a legacy `std::function` Event overload while first-party production
  code migrates to the Delegate-first path.
- Binding a delegate must not allocate merely to store the callback.
- Commands, dispatch, timers, host services, and paint callbacks remain separate
  design questions because their lifetime and concurrency semantics differ.

## Allocation and content failure

- No allocator replacement or allocator propagation is currently authorized.
- A process allocator experiment such as a mimalloc-like alternative is
  separate from object architecture.
- GUI infrastructure is not redesigned for routine OOM recovery.
- Bounded content/resource allocations may fail into an explicit safe state,
  warn usefully, and leave the application stable and closable.

## Specialized storage

- Do not replace `std::vector` wholesale.
- The pinned BFFT `heap_array` style is admitted only for a presently large,
  non-growing heap allocation that is edited in place. It is not for small
  arrays or collections whose contract includes growth.
- BFFT implementation may be copied/adapted; GUI.Forms must not link to BFFT.
- Hive-inspired stable pools are later workload experiments, preferably using
  generation handles if adopted. They are not rewrite completion criteria.

## Portability non-goal

No firmware, framebuffer, freestanding nucleus, picolibc, klibc, UEFI shim, or
constrained-runtime proof belongs to this rewrite. Future constrained-host work
must open as its own project with actual requirements.

## Current source orientation

The 2026-08-10 dirty-tree audit observed about 80,523 lines across 421 production
C/C++/Objective-C++ files, with 1,640 textual `auto` occurrences, roughly 753
lambda-like lines, 577 `std::vector` occurrences, and 181 `std::function`
occurrences. These are orientation only; the sibling must refresh them from the
implementation-start snapshot using semantic tooling.

The generated `OWNERSHIP_AUDIT.md` preserves ownership evidence for a future
lifecycle round. The current rewrite does not alter shared/weak retained
ownership.
