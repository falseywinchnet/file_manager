# GUI.Forms rewrite open-item decision queue

Status: **RESOLVED by `ARCHITECT_SELECTIONS.md`; options retained as reversal
evidence**.

This queue reconciles every `OPEN` or `AUDIT REQUIRED` item in
`DECISION_LEDGER.md` with concrete source observations from the dirty
2026-08-10 tree. It also records conflicts exposed by the audit that were not
visible in the abstract policy discussion.

The option labels were handles for discussion. The recommendations below remain
reasoned proposals, while the architect's actual selections are recorded in
`ARCHITECT_SELECTIONS.md` and now control.

## How the queue is divided

- **Choose now** means the sibling otherwise cannot know the governing source
  grammar, API boundary, or completion gate.
- **Choose a preservation rule now** means the rewrite should explicitly keep
  the current facility and stop treating a hypothetical redesign as a blocker.
- **Select from evidence later** means the architect can approve the candidate
  set and acceptance rule now, but the final choice must follow measurements.
- **Deferred** means the subject is deliberately outside rewrite completion. It
  is not a reason to leave implementation permanently blocked.

## Compact queue

| ID | Decision | Timing | Reasoned recommendation |
|---|---|---|---|
| O-001 | exact source scope and generated output | choose now | A: all first-party source; fix generators rather than hand-edit output |
| O-002 | compiler mode versus house subset | choose now | A: retain C++20 mode; govern features independently |
| O-003 | concepts/requires-expression | choose now | B: explicit specialized trait for the one optional control hook |
| O-004 | `if constexpr` and pack machinery | choose now | A: permit in named templates/functors |
| O-005 | `decltype(expression)` | choose now | B: permit only in named private type-trait plumbing |
| O-006 | ranges | choose now | B: not admitted without a concrete call site |
| O-007 | designated initializers | choose now | B: keep for plain option records; named factories for stateful results |
| O-008 | `<bit>` facilities | choose now | B: keep clear predicates; require a concrete case for representation casts |
| O-009 | unlisted features and compiler extensions | choose now | B: preserve concrete necessities; review material inference case by case |
| O-010 | comparison surface | choose now | A: preserve public operators explicitly; remove unneeded private ones |
| O-011 | `std::any` and public `Tag` | choose now | C, conditional on compatibility review: replace native Tag with explicit ownership; keep managed compatibility in its own layer |
| O-012 | permission for minor API changes | choose now | B: exception ledger, not general permission |
| O-013 | Event public API versus Delegate | choose now | B: Delegate-first additive bridge; retain legacy `std::function` overload |
| O-014 | Delegate representation | choose now | A: non-owning context plus compile-time thunk, with explicit ownership elsewhere |
| O-015 | Event storage and mutation semantics | choose now | A: preserve current semantics first; optimize slot storage only after measurement |
| O-016 | commands/dispatch/timers | preservation rule | A: preserve and exclude redesign from completion |
| O-017 | focused `std::function` audit | choose now | B: quantified audit, replacement only when a concrete cost is found |
| O-018 | allocator authority | preservation rule | A: no allocator program in this rewrite |
| O-019 | shared/weak retained ownership | preservation rule | A: preserve; lifecycle redesign is separate |
| O-020 | strings and text buffers | preservation rule | A: preserve; admit only call-site-specific improvements |
| O-021 | exceptions and RTTI | preservation rule | A: preservation audit with explicit failure boundaries |
| O-022 | threads, atomics, clocks, scheduling | preservation rule | A: preserve exact behavior; no house replacement without measured need |
| O-023 | filesystem, streams, locale, dynamic loading | preservation rule | A: retain concrete hosted uses |
| O-024 | `transform` and unlisted named algorithms | choose now | B: direct loop at the one current transform; default-keep other clear algorithms |
| O-025 | house binary-search API | choose now | B: contiguous range/index core plus small named lower/upper/exact operations |
| O-026 | house-sort admission rule | choose now | B: house sort must win a declared dimension without material regression |
| O-027 | sort candidates per current call site | evidence later | bounded candidate table below; standard sort remains the control |
| O-028 | BFFT-style heap array | evidence later | B: one narrow toolbox type only after a concrete call site earns it |
| O-029 | new files and private toolbox location | choose now | B: preserve existing paths; allow a few purpose-named private files |
| O-030 | migration order | choose now | A: checker observation, binary search, Event proof, then syntax waves |
| O-031 | enforcement mechanism | choose now | B: narrow standalone Clang LibTooling checker plus cheap textual controls |
| O-032 | rewrite completion | choose now | A: exact-behavior closure list below |
| O-033 | performance acceptance | choose now | B: affected-workload non-regression plus declared wins and retained negatives |

Ledger crosswalk:

- R-003/R-004 map to O-002 through O-010.
- R-006, R-007, and R-008 map to O-018, O-019, and O-020.
- R-011 maps to O-013 through O-016; R-010's focused audit maps to O-017.
- R-012 maps to O-021.
- R-013 maps to O-024 through O-027.
- R-015 and R-016 map to O-022 and O-023.
- R-019, R-020, and R-021 map to O-030 through O-033.
- O-001, O-011/O-012, and O-029 make concrete the generated-source,
  API-conflict, and file-preservation edges exposed by R-002/R-017/R-018.
- O-028 is the still-evidence-bound admission step beneath the already-decided
  R-005 specialized-storage policy.

---

## Language, scope, and API choices

### O-001 — Exact source scope and generated output

**Observed:** the architect said “all first-party C++ outside experiment
folders.” The tree also contains generated first-party files, tests, a demo,
tools, compatibility code, and third-party source. Two generated data headers
are visible in the ordinary source/test tree, while `gui_forms/generated/` is a
separate generated surface.

Options:

- **O-001-A — One first-party rule.** Include production, tests, demo, tools,
  compatibility, and checked-in generated first-party C++. Exclude experiments
  and third-party source. When output is generated, change the first-party
  generator/template and regenerate; do not maintain output by hand.
- **O-001-B — Human-authored rule.** Apply the grammar to all human-authored
  first-party C++, but exempt generated output. Require only its API/ABI and
  behavioral equivalence.
- **O-001-C — Production-only rule.** Enforce the house grammar only in
  `include/` and production `src/`; let tests, demo, tools, and generated code
  continue using the broader language.

**Recommendation: O-001-A.** It matches the stated boundary and prevents tests
or compatibility code from becoming a reservoir of the constructs being
removed. Generated output needs a generator-first repair rule so the sibling
does not edit disposable artifacts. A concrete third-party generator whose
output cannot obey the rule remains a stop-and-ask case, not a blanket
exception.

### O-002 — Compiler mode versus house subset

**Observed:** GUI.Forms currently compiles in C++20 mode. Public source uses
`std::span`; the approved policy permits `consteval`; neither is an ordinary
C++17 standard-library surface. The initial “C++17-compatible orthodox subset”
description and the later approved feature list are therefore not literally
identical.

Options:

- **O-002-A — Keep C++20 compiler mode.** Define the house language by the
  explicit allow/ban list, not the version badge. C++20 mode supplies `span`
  and permitted compile-time machinery without admitting every C++20 feature.
- **O-002-B — Require C++17 source and library compatibility.** Replace or
  backport `span`, decline `consteval`, and audit every exposed C++20 type. This
  expands the rewrite into API/toolchain work.
- **O-002-C — Move to a newer compiler mode.** Keep the same house subset while
  using C++23/26 toolchains. No current source requirement justifies this.

**Recommendation: O-002-A.** It preserves APIs and keeps the language decision
where it belongs: in a feature policy. Choosing B would be a separate
compatibility objective, not a style cleanup.

### O-003 — Concepts and the current requires-expression

**Observed:** there are no named concepts and one actual requires-expression.
`make_control` detects `initialize_control_tree()` and invokes the hook after
construction. The optional hook is intentional; the opt-in mechanism is
implicit structural detection.

Options:

- **O-003-A — Permit this narrow requires-expression.** The architecture has
  already selected the hook, so the compiler merely detects its presence.
- **O-003-B — Use an explicit specialized trait.** A named
  `ControlInitializationTraits<ControlType>` or equivalent records opt-in in a
  searchable specialization and invokes the hook.
- **O-003-C — Use a base/interface marker.** Compound controls implement an
  initialization interface. This makes the relationship runtime-visible but
  changes hierarchy and possibly object layout.
- **O-003-D — Split factories.** Ordinary and compound controls use distinct
  factory functions. This is maximally explicit at each call site but creates
  a second construction vocabulary.

**Recommendation: O-003-B.** It preserves the current compile-time mechanism
while making the opt-in named and reviewable. C spends runtime/OOP vocabulary
on a compile-time construction fact. D invites wrong-factory call sites.

### O-004 — `if constexpr`, parameter packs, and compile-time branching

**Observed:** approximately 130 `if constexpr` occurrences exist in the
surveyed source. Most occur in generic-lambda visitors that must be replaced;
others are ordinary template implementation, including numeric conversion.
Parameter packs are already part of Event and factory machinery.

Options:

- **O-004-A — Permit named compile-time branching.** Allow `if constexpr`, pack
  expansion, and fold expressions inside named templates and named functors
  when template parameters and branch meanings are explicit.
- **O-004-B — Permit only specialization/overload dispatch.** Replace every
  compile-time branch with named overloads or specializations.
- **O-004-C — Ban all of it.** This would contradict the approved use of
  templates and substantially multiply boilerplate.

**Recommendation: O-004-A.** This is the good kind of compiler machinery under
the governing rule: ordinary code chooses a named template, and the compiler
instantiates already-declared branches. Lambda closure inference remains
banned. A branch that selects architecture rather than implementation must
still be named or specialized.

### O-005 — `decltype(expression)`

**Observed:** 21 current occurrences are type recovery inside generic lambdas.
They should disappear with the lambdas. No current ordinary call site proves a
need for expression-derived variable or return types.

Options:

- **O-005-A — Ban it completely.** Require template parameters, explicit
  aliases, or specialized traits in every case.
- **O-005-B — Narrow trait-only permission.** Permit it only in unevaluated
  operands used to define a named private type trait/detection result. Do not
  permit it for ordinary variables, public signatures, return inference, or
  `decltype(auto)`.
- **O-005-C — Permit it anywhere an alias names the result.** This is easier but
  can reintroduce inferred program types behind aliases.

**Recommendation: O-005-B.** It permits the small amount of template plumbing
that sometimes cannot be stated honestly otherwise, while forcing the derived
type to cross a named trait boundary before ordinary code sees it.

### O-006 — Ranges and views

**Observed:** no `<ranges>`, `std::ranges`, or view pipeline is present in the
current production survey.

Options:

- **O-006-A — Ban ranges for the entire rewrite.** Any occurrence is a policy
  failure.
- **O-006-B — Do not admit them without a concrete proposal.** The current
  rewrite uses none; a later call site must show its types, lifetimes, and
  execution before approval.
- **O-006-C — Permit non-lazy range algorithms but ban views.** This creates a
  policy category without a current need.

**Recommendation: O-006-B.** It avoids speculative policy. Lazy views are
especially relevant to lifetime and actual-execution visibility, so they
should not enter incidentally.

### O-007 — Designated initializers

**Observed:** roughly 80 lines across five files initialize popup options,
text-shaping validation limits, Skia raster results, and image-registry
success/failure results. Field names are visible, but omitted fields silently
take defaults. Result records can therefore encode an incomplete semantic
state.

Options:

- **O-007-A — Keep all designated initializers.** Named fields usually improve
  readability and aggregate layout is already explicit.
- **O-007-B — Distinguish records from state machines.** Permit designated
  initialization for plain option/configuration records with independent
  fields and valid defaults. Use named `Success(...)`, `Failure(...)`, or other
  factories where fields jointly define one semantic result state.
- **O-007-C — Ban them all.** Use constructors, factories, or stepwise explicit
  assignment everywhere.

**Recommendation: O-007-B.** Popup options and fixed validation limits are
honest records. Image/raster result values are closer to sum states: named
factories state which alternative exists and can enforce complete fields.

### O-008 — `<bit>` facilities

**Observed:** no `std::bit_cast` exists. Four `std::has_single_bit` calls
validate flag values.

Options:

- **O-008-A — Keep every clear `<bit>` operation.** Review only its call site.
- **O-008-B — Keep numeric predicates; gate representation conversion.**
  `has_single_bit` remains. `bit_cast` requires a concrete representation,
  aliasing, endianness, and size discussion.
- **O-008-C — House-own all bit operations.** This creates names for facilities
  whose current standard behavior is already direct and inspectable.

**Recommendation: O-008-B.** A named power-of-two predicate carries very low
epistemic cost. A representation cast can carry much more and should be
justified where it appears.

### O-009 — Unlisted features and compiler extensions

Options:

- **O-009-A — Closed allow-list.** Anything not explicitly approved is banned
  until the architect adds it.
- **O-009-B — Governing-test default.** Preserve an existing construct when it
  does not hide a material choice; bring forward concepts, extensions,
  inference, or lifetime/control-flow machinery when they do. New compiler
  extensions require a named platform/toolchain necessity.
- **O-009-C — Standard-mode default.** Permit everything supported by the
  selected C++ mode unless specifically banned.

**Recommendation: O-009-B.** A full language allow-list turns this rewrite into
language-lawyer maintenance. C is too permissive for the stated goal. The AST
inventory should still report unclassified modern/extension constructs once so
the sibling is not relying on accidental ignorance.

### O-010 — Comparison operators and public compatibility

**Observed:** the initial survey found 11 defaulted spaceship operators and 20
defaulted equality operators. The ban targets convenience-defaulted machinery,
but deleting or changing a public comparison operator also changes source API.

Options:

- **O-010-A — Preserve public comparison signatures explicitly.** Hand-write
  the exact public operator set and semantics. For source-private types, retain
  only operators proven by call sites; prefer named comparators for local sort
  orders.
- **O-010-B — Reduce every type to internally observed comparisons.** Remove
  public operators not used by first-party code. This is a broad source break.
- **O-010-C — Replace spaceship with all six relational operators.** This can
  create more surface than the type actually needs.

**Recommendation: O-010-A.** Existing public exposure is itself evidence that
the signature belongs to the API-preservation oracle. “Only where needed”
should remove private convenience, not silently amputate public vocabulary. An
explicitly implemented spaceship is not convenience-defaulted machinery; if
the architect means to ban spaceship syntax itself, that needs a separate
ruling.

### O-011 — `std::any`, `Tag`, and what Tag actually does

**Observed public surface:** four native C++ classes expose `const std::any&
tag()` and `set_tag(std::any)`:

- `Control`;
- `ImageList`;
- `ErrorProvider`;
- `HelpProvider`.

`Control` describes Tag as inert application-owned metadata and a lifetime
anchor. It does not affect rendering, layout, semantics, binding,
serialization, the C ABI, or host publication. Every owner resets the value on
disposal, so Tag can synchronously release an application-owned object.

**Observed first-party uses:**

1. The Complete Showcase stores `std::shared_ptr<ShowcaseContext>` on its root
   control. Later initialization casts it back. This is the only production use
   found. The tag keeps the context—and its subscriptions, timers, provider
   components, and worker thread—alive for the Window lifetime.
2. One Control test stores `std::shared_ptr<int>` and proves that disposal
   releases the retained lifetime.
3. ErrorProvider and HelpProvider tests store a `std::string` and prove inert
   exact-type retention.
4. No first-party ImageList tag consumer was found.

The feature was deliberately documented as WinForms-compatible `Tag`; it is not
an accidental private field. But it is also exactly the kind of open-ended
runtime type/storage choice that the `std::any` ban targets. Merely spelling a
house `Any` or `TagValue` would recreate the same epistemic cost under a new
name.

Options:

- **O-011-A — Narrow `std::any` compatibility exception.** Keep these four
  signatures and backing values, forbid `std::any` everywhere else, and record
  Tag as the one intentional open-ended metadata boundary. This best preserves
  native C++ compatibility but weakens the literal ban.
- **O-011-B — Retain Tag through a smaller owned-erasure contract.** Replace
  the native value with an explicitly owning type such as
  `std::shared_ptr<void>` under a more honest name like `application_state` or
  `lifetime_anchor`. This preserves the only production need—ownership—but
  loses arbitrary by-value metadata and checked runtime type identity. A
  managed facade can keep managed `object Tag` separately.
- **O-011-C — Remove native Tag and give each real need named ownership.** Give
  the Showcase an explicit lifetime owner; delete unused native tags from
  ImageList/ErrorProvider/HelpProvider; keep WinForms `Tag` only in a
  compatibility facade that naturally owns managed objects. This is the most
  epistemically disciplined option, but it is a public C++ source break and the
  Showcase owner needs a concrete lifetime design.
- **O-011-D — Build a house type-erased Tag value.** It could use a vtable,
  inline storage, heap storage, type tokens, and clone/destroy operations. This
  is a private `std::any` and conflicts with both the ban and the no-private-STL
  rule.

**Recommendation: O-011-C if the compatibility review confirms that native C++
Tag is not a committed consumer boundary; otherwise O-011-A.** B is a coherent
middle course only if a generic lifetime anchor is itself an intended public
feature. D should be rejected.

If C is chosen, the Showcase must not simply trade Tag for a global map, an
untyped side table, or a hidden strong callback cycle. Candidate explicit owner
designs are:

- a source-private Showcase application/window owner that retains both Window
  and `ShowcaseContext`;
- a named Showcase runtime component owned through an already-authoritative
  component lifetime boundary;
- an architect-approved typed owner field at an existing appropriate boundary.

The existing “no local Showcase control subclasses” rule means inventing a
`ShowcaseRoot` subclass is not a silent solution.

### O-012 — How minor API changes are authorized

Options:

- **O-012-A — Literal preservation.** No public additions, removals, aliases,
  or signature changes. This forces exceptions for Tag and makes a Delegate-
  first Event difficult.
- **O-012-B — Explicit exception ledger.** APIs remain fixed by default. Each
  necessary minor change names the old surface, new surface, compatibility
  effect, dependent audit, and architect approval. Tag and Event are the first
  two candidates.
- **O-012-C — General minor-change permission.** The sibling may make any
  source-compatible or judged-small break that improves the house style.

**Recommendation: O-012-B.** “Minor changes are possible” should unlock real
contradictions without dissolving the rewrite's strongest boundary. C would
allow hundreds of local cleanups to become unreviewed API design.

---

## Callback, Event, and execution choices

### O-013 — Public Event API versus a Delegate-first Event

**Observed:** public `Event<Arguments...>` currently declares
`using Callback = std::function<void(Arguments...)>` and exposes
`subscribe(Callback)` plus `subscribe(Component&, Callback)`. Replacing the
alias or parameter type with `Delegate` is a public source change. Leaving it
unchanged cannot guarantee zero allocation when a Delegate is converted into
`std::function`.

Options:

- **O-013-A — Approved breaking replacement.** Make Delegate the Event callback
  type and migrate all first-party callers. External C++ callers must update.
- **O-013-B — Additive compatibility bridge.** Add Delegate overloads as the
  primary first-party path while retaining the existing `Callback` alias and
  `std::function` subscribe overloads. Event slots can distinguish a Delegate
  from a legacy owning callable. The legacy path remains allocation-capable.
- **O-013-C — Preserve the exact API and wrap Delegate in `std::function`.**
  Binding the Delegate object itself is allocation-free, but subscription may
  allocate merely because of erasure, so the intended gain is not guaranteed.
- **O-013-D — Leave public Event unchanged and create a second private event
  type.** Controls already publicly expose Event, so this duplicates semantics
  without solving the main surface.

**Recommendation: O-013-B.** It permits all rewritten first-party code to use
the disciplined path without immediately breaking external consumers. The
bridge must be explicit and measured; it must not turn the Event slot into a
universal callable framework. If exact API shape forbids even an overload,
choose A or accept that the Delegate goal cannot be literal.

### O-014 — Delegate representation and binding contract

Options:

- **O-014-A — Compile-time thunk delegate.** Store a non-owning context/object
  pointer and one invocation thunk. The target free/member function is a
  template argument, so no member-function pointer or closure state is stored
  at runtime. Delegate is small, explicitly empty or bound, and allocation-free.
- **O-014-B — Runtime member-pointer inline storage.** Store a member-function
  pointer in a fixed inline buffer plus context and thunk. This permits
  `Bind(object, method)` with a runtime method value but increases size,
  alignment/representation rules, and compiler-ABI sensitivity.
- **O-014-C — General small-buffer callable.** Accept arbitrary named functors
  and place small ones inline, allocating large ones. This recreates a partial
  `std::function` and blurs Delegate's purpose.

**Recommendation: O-014-A.** It gives the narrowest honest contract. The
binding syntax may be slightly more explicit, for example a target as a
template argument. Delegate itself should not own the subscriber. Lifetime is
provided by Event's owner subscription, a SubscriptionToken, or another named
owner; this keeps invocation separate from lifetime as requested.

Contract details to approve with A:

- explicit empty state and `operator bool` or named `bound()` query;
- copy/move are value operations over the stored context and thunk;
- equality, if needed, compares exactly the bound context and thunk;
- const and non-const member binding are separate named template overloads;
- invocation does not catch, translate, or suppress exceptions;
- no hidden weak/strong ownership and no allocation during binding;
- no attempt to bind arbitrary captures—state lives in a named object.

### O-015 — Event storage, subscription, and mutation during emission

**Observed current behavior:** registration-order snapshot; handlers added
during an emission wait until the next emission; a handler disconnected before
its turn is skipped; a snapshot member runs at most once; the callable is copied
locally so self-disconnect/disposal cannot destroy it while executing; tokens
are move-only and disconnect on destruction; component-owned revocables
disconnect with the owner; Event destruction disconnects all; statistics are
retained.

Options:

- **O-015-A — Behavior-first hybrid slots.** Preserve the current state/slot and
  snapshot model initially. Slots hold either the new Delegate or a legacy
  `std::function` callback. Measure subscription allocation separately after
  exact behavior is proven.
- **O-015-B — Generation-indexed slot vector immediately.** Token stores an
  event-state reference plus slot index/generation. Emission snapshots handles
  or records. This can remove separately allocated slots but changes token and
  mutation machinery at the same time as callback representation.
- **O-015-C — Intrusive subscriber nodes.** SubscriptionToken/subscriber owns a
  linked node. This can provide address stability but makes subscriber lifetime
  and reentrancy more coupled.
- **O-015-D — Flat Delegate vector with deferred removals.** Smallest storage,
  but owner revocation, token survival, nested emission, and legacy callbacks
  all become harder to preserve.

**Recommendation: O-015-A for the first proof.** “Zero heap merely to bind” does
not require “zero heap per subscription.” Separating those questions is the
cleanest behavioral experiment. B may become a second measured optimization if
slot allocation is material; it should then use generation checks and retain
the exact trace oracle.

### O-016 — Commands, dispatch, timers, host services, and paint callbacks

Options:

- **O-016-A — Preserve and exclude redesign from completion.** Replace banned
  lambdas with named work objects/functors while retaining `std::function` and
  current ownership, cancellation, queue, wait, exception, and shutdown rules.
- **O-016-B — Audit each mechanism during the rewrite and redesign obvious
  cases.** This increases the number of simultaneous lifecycle projects.
- **O-016-C — Unify them under Delegate/Event.** This erases meaningful
  ownership-across-time and concurrency differences.

**Recommendation: O-016-A.** A focused `std::function` finding can still return
as its own approved case. C should be rejected.

### O-017 — What counts as an obvious `std::function` cleanup?

Options:

- **O-017-A — Events only.** Everything outside Event is retained without
  measurement.
- **O-017-B — Quantified focused audit.** Inventory semantic role, callable
  size/alignment, measured allocation, copy/move frequency, escape lifetime,
  thread transfer, cancellation, and exception boundary. Replace only when a
  named mechanism reduces a concrete cost or lifetime ambiguity.
- **O-017-C — Replace all local/synchronous uses.** Keep `std::function` only at
  public or asynchronous boundaries.

**Recommendation: O-017-B.** It implements “audit the big dumb cases” without
turning a phrase into a stylistic purge. A finding is not actionable merely
because SBO is implementation-dependent; it must matter at that call site.

---

## Preservation choices that should cease being blockers

### O-018 — Allocator authority

Options:

- **O-018-A — No allocator program.** House facilities use ordinary allocation
  directly or existing standard ownership. A process allocator benchmark is a
  separate experiment.
- **O-018-B — Minimal internal allocation capability.** Introduce allocate/free
  with size/alignment only for one proven house facility. Do not propagate an
  allocator identity through existing objects or containers.
- **O-018-C — Full allocator table.** Define allocate, free, reallocate,
  alignment, telemetry, thread policy, and module-boundary ownership now.
- **O-018-D — Select an alternative global/process allocator.** Benchmark and
  change the process allocator as part of the rewrite.

**Recommendation: choose O-018-A for rewrite completion.** B can reopen only
when a concrete facility cannot state its contract honestly with ordinary
allocation. C and D are separate architecture/performance projects. The
already-approved content soft-failure versus infrastructure OOM distinction is
the governing behavior.

### O-019 — Shared/weak retained ownership

Options:

- **O-019-A — Preserve exactly.** Syntax work may make pointer types explicit
  but cannot change ownership edges.
- **O-019-B — Audit non-tree shared ownership and replace obvious unique owners.**
  This can be useful but risks mixing lifecycle changes into grammar batches.
- **O-019-C — Replace retained identity with generation handles/stable pools.**
  This is a separate retained-engine lifecycle design.

**Recommendation: O-019-A.** Close R-007 for this rewrite. Any B case should be
submitted separately with disposal/event-order evidence; C is deferred.

### O-020 — Strings and text buffers

Options:

- **O-020-A — Preserve ordinary `std::string`.** Improve only a concrete call
  site with measured allocation, ownership, validation, or performance need.
- **O-020-B — Add a bounded string/builder for content paths.** Keep public
  strings but house-own selected internals.
- **O-020-C — Build a house string family.** This is a private standard-library
  project.

**Recommendation: O-020-A.** Close R-008 as a preservation rule. Existing UTF-8
validation and content bounds already express the important domain policy.

### O-021 — Exceptions and RTTI

**Observed:** exceptions are used at argument/invariant failures, content and
host failure boundaries, disposal cleanup, dispatch fault capture, and ABI
containment. RTTI is used in real control/adaptation paths.

Options:

- **O-021-A — Preservation audit.** Keep current externally observable throw,
  catch, translation, and RTTI behavior. Flag routine retry/branching through
  exceptions and exception work inside measured hot loops, but do not redesign
  failure contracts merely for style.
- **O-021-B — Make internals mostly exception-free.** Convert broad internal
  paths to result codes while retaining exceptions only at public adapters.
- **O-021-C — Permit existing and new exception use without audit.** This loses
  the approved “exceptional, not routine” discipline.

**Recommendation: O-021-A.** ABI containment and non-throwing disposal remain
hard requirements. Catch-all blocks require classification, not automatic
deletion: several intentionally protect cleanup/ABI boundaries.

### O-022 — Threads, atomics, clocks, and scheduling

Options:

- **O-022-A — Preserve exact machinery and behavior.** Audit only to record the
  oracle for ordering, cancellation, timers, frame scheduling, waiting, and
  shutdown.
- **O-022-B — Replace a measured primitive with copied BFFT machinery.** Open
  only for a concrete contention, determinism, portability, or code-size need.
- **O-022-C — Introduce a house concurrency layer now.** This is a broad runtime
  rewrite without evidence.

**Recommendation: O-022-A.** R-015 should be `AUDIT/PRESERVE`, not an unresolved
architecture choice. B is an experiment trigger; C is outside scope.

### O-023 — Filesystem, streams, formatting, locale, and dynamic loading

**Observed:**

- `std::filesystem` is confined to Skia module/asset path resolution in the
  surveyed production source;
- file streams read fonts/assets and test data;
- string streams perform bounded formatting/parsing in controls, snapshots,
  host commands, metrics, semantics, and binding;
- `std::locale::classic()` is deliberately used for deterministic numeric text;
- dynamic loading is a hosted Win32/ABI responsibility;
- no `std::format` use was found.

Options:

- **O-023-A — Retain concrete hosted facilities.** Replace only a call site that
  proves opacity, cost, or nondeterminism. Keep platform loading in the platform
  boundary.
- **O-023-B — Wrap every facility in private utilities.** This adds indirection
  while preserving the same hosted dependency.
- **O-023-C — House-own file/path/format/locale/loading operations.** This is the
  portability/private-runtime project already rejected.

**Recommendation: O-023-A.** `locale::classic()` improves, rather than harms,
determinism. The current stream formatting sites may be profiled, but their
existence is not a rewrite blocker.

---

## Algorithms and specialized storage

### O-024 — `std::transform` and other unlisted named algorithms

**Observed:** one production `std::transform` lowercases ASCII characters in a
string using a lambda. The transformation is a direct per-byte operation.

Options:

- **O-024-A — Retain transform with a named unary functor.** Clear, but creates
  a named type for one three-line local operation.
- **O-024-B — Use an explicit indexed/range loop at this site.** The data,
  mutation, and ASCII conversion are directly visible.
- **O-024-C — Add a house transform.** This changes a standard name without
  changing behavior or inspectability.

**Recommendation: O-024-B for the observed site.** For future unlisted named
algorithms, default to keeping the standard operation when its behavior is
clear and implementation choice is immaterial. A direct loop can win when the
operation is more legible than a new functor. C should be rejected.

### O-025 — House binary-search API

**Observed:** current lower/upper/exact searches are all over contiguous arrays
or vectors: selected indexes, layout offsets/positions, generated Unicode
ranges, style spans, and grapheme boundaries.

Options:

- **O-025-A — Generic iterator algorithms.** Closely mirror the standard API
  with explicit iterator and comparator template parameters.
- **O-025-B — Contiguous range/index core.** Accept `std::span` or explicit
  pointer/count and return an index/insertion position. Provide named lower,
  upper, and exact operations, with comparator/value types explicit.
- **O-025-C — Container-specific overloads.** Add vector/array overloads for
  convenience. This grows a small algorithm into a facade family.
- **O-025-D — Direct binary-search loops at each call site.** Maximum locality,
  but duplicates boundary logic and tests.

**Recommendation: O-025-B.** It matches every current workload, makes midpoint
and bounds visible, avoids iterator-type noise under the no-`auto` rule, and
keeps the contract deliberately smaller than `<algorithm>`. Heterogeneous
comparison should be supported by a named comparator, not a lambda. If a future
non-contiguous use appears, it can justify a second algorithm rather than
pre-generalizing this one.

### O-026 — Admission rule for house sorting

Options:

- **O-026-A — Always replace `sort`/`stable_sort`.** Standard implementations
  are only benchmark controls.
- **O-026-B — Declared-dimension admission.** A house candidate must preserve
  exact ordering/stability, avoid material regression on representative and
  adversarial inputs, and win at least one declared dimension that matters:
  call-site performance, scratch allocation, deterministic behavior, code size,
  or inspectability of a bounded operation. Otherwise retain the standard sort.
- **O-026-C — Performance-only admission.** Replace only with a statistically
  significant speed win.
- **O-026-D — Inspectability-only admission.** A simple house algorithm may
  replace the standard even when materially slower.

**Recommendation: O-026-B.** It captures the architect's pragmatic standard.
Speed is not the only relevant result, but “house” is not a license for an
unbounded loss. The benchmark record must preserve cases where the standard
implementation wins.

### O-027 — Candidate choices at each current sort site

These are evidence-bound options, not selections. `std::sort` or
`std::stable_sort` remains the control in every row.

| Call site | Bounded candidates to measure | Provisional recommendation before data |
|---|---|---|
| ErrorProvider snapshot IDs | standard sort; insertion sort; adaptive small sort | standard or insertion, selected by attached-error counts and string comparator cost |
| ListBox stable-ID duplicate validation | standard sort; house partition sort; adaptive sort | retain standard unless large-list distribution or cross-STL behavior earns ownership |
| ListBox selection normalization | insertion sort; adaptive small sort; standard sort | insertion/adaptive is plausible because selections are often small/nearly ordered |
| PropertyGrid visible descriptors | standard sort; adaptive sort; precomputed-key sort | do not add key caching/invalidation in this rewrite unless comparator cost is measured material |
| CorrespondenceView expanded indexes | direct ordered insertion; sorting network/tiny insertion; standard sort | direct ordered insertion is the leading candidate because the set is bounded to about three roles |
| TextStore style-span normalization | insertion/adaptive sort; standard sort; explicit-stack partition sort | measure authored-order frequency and span counts; preserve exact comparator order |
| DamageRegion x edges | tiny numeric insertion; explicit-stack numeric sort; standard sort | select by damage-count distribution and frame-path hotness |
| DamageRegion y intervals | insertion/adaptive pair sort; explicit-stack partition sort; standard sort | highest-priority performance measurement; do not smuggle in a sweep redesign |
| HarfBuzz face tiers | stable insertion; run-aware stable merge; standard stable sort | stable insertion is the leading small-set candidate; registration order must remain exact |
| Control tab traversal | stable insertion; run-aware stable sort; standard stable sort | measure child counts and already-sorted rate; no caching/invalidation redesign by default |
| Control mnemonic traversal | same stable candidates | consider one shared named traversal operation only if semantics are truly identical |
| Window traversal | same stable candidates | preserve null ordering and retained tree order exactly |
| Window mnemonic traversal | same stable candidates | preserve reentrancy/focus traversal behavior; no speculative cache |
| Window focus candidates | same stable candidates | measure input-path frequency and equal-TabIndex density |

Candidate collection options:

- **O-027-A — Minimal two-sort set:** stable insertion sort plus one explicit-
  stack unstable partition/introspective sort.
- **O-027-B — Adaptive three-sort set:** stable insertion, run-aware stable
  merge/adaptive sort, and explicit-stack unstable partition/introspective
  sort.
- **O-027-C — Site-specialized routines only:** direct tiny ordering for bounded
  sites and retain standard sorts elsewhere.

**Recommendation: begin measurements with O-027-B as a laboratory candidate
set, but ship the smallest subset admitted by O-026-B.** C may be the final
shipping result if only the bounded/tiny sites earn house code.

### O-028 — BFFT-style heap array

Options:

- **O-028-A — No use in the rewrite.** Keep the reference only as style
  evidence.
- **O-028-B — One narrow toolbox type after admission.** Use it for an aligned,
  move-only, explicit-capacity workspace where failure and relocation
  requirements are concrete. Reuse it at a second site only if the contract is
  truly identical.
- **O-028-C — General house dynamic array.** Expand it toward vector semantics
  and migrate multiple collections.
- **O-028-D — Inline a bespoke allocation at each earned site.** Avoids a
  general type but duplicates construction/destruction/failure machinery.

**Recommendation: O-028-B, evidence-bound.** Admission requires element
construction/destruction rules, alignment, maximum length, OOM behavior,
relocation/exception requirements, address stability, and a comparison with
`std::array`, `std::vector`, or direct ownership. C conflicts with the vector
decision. D is preferable only when contracts differ materially.

---

## Migration, enforcement, and completion

### O-029 — New files and private toolbox location

**Conflict:** “preserve files” can mean either do not move/delete existing files
or literally add no files. Delegate, binary search, and measured sort routines
need an inspectable home.

Options:

- **O-029-A — No new files.** Place machinery in existing headers/sources. This
  can hide reusable contracts in unrelated files.
- **O-029-B — Preserve every existing path; permit a few new purpose-named
  private files.** Examples are one Delegate/Event location and one small
  algorithm location. Do not create a complete `base/memory/text/...` tree.
- **O-029-C — Reorganize existing source around the new toolbox.** This violates
  the in-place file-preservation goal and makes behavioral review harder.

**Recommendation: O-029-B.** Record new files in the API exception/batch ledger
even when private. “Preserve files” should mean no incidental moves, deletions,
or reshaping—not that new approved machinery must be buried.

### O-030 — Migration order

Options:

- **O-030-A — Small proof before broad syntax:** observation-mode checker;
  binary-search toolbox; one behavior-rich Delegate/Event proof; explicit-
  language waves; sort laboratory; focused `std::function` audit; closure.
- **O-030-B — Event first:** observation checker, then full Delegate/Event
  migration before any binary-search or syntax waves.
- **O-030-C — Syntax first:** mechanically remove `auto`, arrows, lambdas, and
  comparisons across directories, then design house facilities.
- **O-030-D — Layer-by-layer rewrite:** complete core, controls, rendering, and
  hosts in sequence, including every concern found in each layer.

**Recommendation: O-030-A.** Binary search proves the private-toolbox grammar
with minimal lifetime risk. Event then proves the difficult ownership/control-
flow part before hundreds of lambda replacements choose ad hoc callback forms.
C risks redoing callback work; D combines unrelated semantic risks.

### O-031 — Enforcement mechanism

Options:

- **O-031-A — Custom clang-tidy checks.** Integrate with the compilation
  database and diagnostics. Strong ecosystem fit, but plugin/version/build
  maintenance may be disproportionate.
- **O-031-B — Narrow standalone Clang LibTooling checker.** Match the banned AST
  nodes, emit file/line/kind, honor the explicit scope, and run from CMake/CI.
  Use `rg` only as a cheap control for obvious tokens.
- **O-031-C — Compiler warnings plus regex.** Cheap, but cannot reliably
  distinguish trailing-return arrows, pointer arrows, comments/macros, defaulted
  comparisons, inference, or AST context.
- **O-031-D — Parse serialized AST dumps with scripts.** Avoids a compiled tool
  but couples policy to unstable dump formatting.

**Recommendation: O-031-B.** It is the narrowest authoritative gate. Begin in
inventory mode; ratchet touched files; then make the whole selected scope fatal
at closure. Diagnostic records should include enclosing symbol and replacement
category. C remains a fast presubmit supplement, never the authority.

### O-032 — Definition of rewrite completion

Options:

- **O-032-A — Exact-behavior closure:**
  1. zero unapproved banned constructs in the O-001 scope;
  2. every approved API exception recorded and dependent consumers updated;
  3. existing files remain at their paths and only O-029-approved private files
     are added;
  4. public C++/C ABI, retained identity, lifecycle, ordering, rendering, host,
     accessibility, managed-facade, and dependent-library oracles pass except
     for explicitly approved API deltas;
  5. first-party event use is Delegate-first, with exact Event traces and
     measured allocation/invocation evidence;
  6. retained `std::function` uses are classified and justified under O-017;
  7. every current sort/stable-sort site has workload evidence and either an
     admitted house choice or an explicit standard-library retention result;
  8. current binary-search-family uses are migrated to the approved tiny house
     API with equivalence/fuzz tests;
  9. specialized storage exists only where O-028 admission evidence exists;
  10. no BFFT link dependency, allocator propagation, ownership redesign,
      concurrency layer, firmware target, or fake portability layer was added;
  11. native/headless/Windows cross-build and relevant consumer gates pass;
  12. negative benchmark and migration results are retained.
- **O-032-B — Grammar closure only.** Completion means banned syntax is gone and
  tests pass; algorithm/event audits may remain later work.
- **O-032-C — Expanded runtime closure.** Also require allocator, stable pool,
  constrained runtime, and concurrency replacements.

**Recommendation: O-032-A.** B leaves the principal house-abstraction and
algorithm decisions unfinished. C reintroduces explicitly rejected projects.

### O-033 — Performance and build acceptance

Options:

- **O-033-A — Behavior only.** Record performance but impose no regression
  constraint.
- **O-033-B — Affected-workload acceptance.** For each batch, preserve behavior
  and record build/binary/allocation/runtime measures relevant to the changed
  mechanism. A house replacement may ship only when it meets its declared
  O-026 dimension and causes no material regression in representative or
  adversarial controls. Define “material” in the benchmark record before using
  results to select.
- **O-033-C — Global fixed percentage gate.** Impose one build/runtime/binary
  threshold across all batches.

**Recommendation: O-033-B.** One global percentage is false precision across
very different workloads. The sibling must predeclare sample size, environment,
noise treatment, and acceptance margin for each decision rather than moving the
goalposts after results.

---

## Subjects that should be explicitly deferred, not left ambiguously open

Unless the architect pulls one back into scope with a concrete workload, the
following have no option to select for rewrite completion:

- process/global allocator replacement or allocator propagation;
- retained-control ownership replacement;
- hive/stable-pool adoption;
- general string/container/standard-library replacement;
- dispatch, command, timer, or host-service unification with Event;
- custom threading/atomic/clock layer;
- filesystem/stream/locale/dynamic-loading portability facade;
- Skia, text-stack, or platform-boundary restructuring;
- UEFI, bare-metal, picolibc, klibc, framebuffer, or freestanding proof;
- separate reusable GUI.Forms runtime or Mini-STL.

Each may reopen later through a named decision with an actual requirement,
baseline, workload, failure modes, and reversal path. Their deferral is part of
the rewrite boundary, not unfinished rewrite work.

## Suggested discussion order

The highest-leverage sequence is:

1. O-011/O-012: decide whether the native Tag API is an approved exception,
   reduced ownership surface, or removal.
2. O-013/O-014/O-015: resolve how Delegate enters the public Event surface and
   how little of Event storage changes in the first proof.
3. O-002 through O-010: freeze the remaining language edges.
4. O-018 through O-024: close preservation audits so they stop masquerading as
   blocking architecture work.
5. O-025 through O-028: approve evidence rules and candidate laboratories, not
   benchmark outcomes.
6. O-029 through O-033: approve migration, enforcement, and completion.
