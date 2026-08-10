# GUI.Forms rewrite decision ledger

Status: **interview ledger; no entry below is accepted unless marked DECIDED
with owner approval**.

The questions are ordered by dependency. Later answers must not be finalized
when an earlier answer changes their meaning.

## Cluster A — target and language boundary

### R-001 — What does “someday UEFI/bare metal” require from this rewrite?

Status: **OPEN**.

The phrase can mean three materially different things:

1. **Portable nucleus:** geometry, retained tree, layout, input normalization,
   damage, display-list construction, and headless execution can compile without
   desktop APIs. Desktop rendering/text/accessibility remain hosted adapters.
2. **Useful firmware GUI:** nucleus plus a simple framebuffer renderer, bitmap
   font/text fallback, allocator, clock, and firmware input host can run as an
   actual application.
3. **Full framework parity:** most controls and services work in firmware.

Candidate 1 is the smallest credible first contract. Candidate 2 is a later
proof target. Candidate 3 would distort desktop work unless a concrete firmware
product demands it.

Questions for the architect:

- Must this rewrite produce only the seams needed by candidate 1, or must it
  include a later framebuffer proof in its acceptance plan?
- Is “bare metal” broader than UEFI—meaning no firmware allocator, filesystem,
  threads, locale, or C runtime—or is UEFI the first honest constrained host?
- Which behavior must exist in the nucleus: retained controls only, or also
  Unicode text editing, PNG resources, timers, drag/drop, and accessibility
  semantics?

Failure mode: calling the whole library portable because a few value types
compile freestanding while ownership, callbacks, errors, and control trees still
require a hosted C++ runtime.

### R-002 — What is the rewrite boundary?

Status: **OPEN**.

Candidate boundaries:

- portable core only;
- portable core plus reusable controls;
- all first-party production source, while host/render adapters retain approved
  platform and third-party facilities;
- every file including tests, demos, tools, compatibility facades, and generated
  sources.

Recommended discussion direction: production source should share one readable
house style, but portability restrictions need layer-specific profiles.
Objective-C++ host code cannot obey the same dependency ceiling as a geometry
type. Tests should migrate after the primitives stabilize, not be allowed to
dictate production containers.

We must name:

- directories in each profile;
- whether public C++ headers have the strictest profile;
- whether `src/render/*` and `src/host/*` may use hosted containers internally;
- whether managed-facade generator sources are out of scope;
- how provisional/gallery code is treated.

Failure mode: a universal ban causes awkward wrappers around APIs that already
require hosted platform objects, while a vague “core only” boundary leaves
standard-library types embedded in public control contracts.

### R-003 — What C++ language level and subset are normative?

Status: **OPEN**.

Candidates:

- keep C++20 but impose the house grammar;
- return to C++17 for the portable nucleus, allowing C++20 in hosted adapters;
- compile both a strict C++17 nucleus profile and the normal C++20 desktop
  profile;
- define a feature list independent of the version number.

This decision must cover concepts, ranges, coroutines, designated
initialization, spaceship operators, `consteval`, `std::span`, `std::bit_cast`,
and compiler extensions individually. A version badge is not a sufficient
language policy.

Failure mode: lowering the language flag while recreating equally hosted or
compiler-heavy machinery in templates, or retaining C++20 syntax in public
headers that prevents the constrained build.

### R-004 — Exact `auto` prohibition

Status: **GIVEN direction; edge grammar OPEN**.

The ordinary cases are clear: local variables, range-for variables, iterator
results, pointers, factory results, and lambda variables receive explicit
types.

We must explicitly decide:

- whether trailing return syntax with a named return type is allowed;
- how comparison operators spell `std::strong_ordering` or another result;
- whether structured bindings are prohibited because their component types are
  implicit even when the binding uses no `auto` token;
- whether generic lambdas are independently prohibited;
- whether template metaprogramming may use `decltype` for type computation;
- whether `decltype(auto)` is absolutely forbidden;
- whether generated or third-party code is checked.

Proposed strong grammar for discussion: no `auto`, `decltype(auto)`, structured
bindings, or generic-lambda parameters in first-party production code;
`decltype(expression)` allowed only in type traits and adapter detection, never
to avoid naming an ordinary runtime type.

Failure mode: replacing `auto` with unreadable iterator implementation types
without first giving containers stable named iterator aliases.

Existing occurrences in broader BFFT subsystems are migration debt and do not
form an exemption or candidate policy.

## Cluster B — allocation, ranges, and ownership

### R-005 — GUI.Forms heap-array family

Status: **GIVEN replacement direction; design OPEN**.

`std::vector` currently serves several different jobs that must not be forced
into one replacement:

- fixed-size owned heap buffer after construction;
- growable contiguous sequence;
- output accumulator with reserve/append;
- byte buffer;
- small temporary list;
- public returned snapshot;
- child ownership collection;
- free-list or slot registry;
- matrix/image plane;
- transfer across ABI or platform adapters.

Candidate house types:

- `HeapArray<T>` — move-only, fixed length after successful allocation;
- `ArrayView<T>` / `ConstArrayView<T>` — pointer plus count, never owning;
- `ArrayList<T>` — explicitly growable contiguous collection with capacity;
- `InlineArray<T, N>` — compile-time fixed inline storage;
- `SmallArray<T, N>` — optional inline capacity plus heap spill;
- `ByteBuffer` — byte ownership with alignment and exact used/capacity fields;
- `ArenaArray<T>` — borrowed lifetime from an explicit arena;
- specialized structures where semantics matter more than generality.

Decisions required:

- Does `HeapArray` default-construct elements, leave trivial storage
  uninitialized, or offer separate factories?
- Are allocation failures returned, propagated through a status object, or
  fatal under the desktop profile?
- Is growth geometric, exact, caller-selected, or prohibited in the nucleus?
- What are maximum counts and overflow checks?
- Which types are copyable?
- Does element relocation require nothrow move construction?
- Are iterators provided, or only indexes and views?
- How is alignment requested and reported?
- Does every allocation carry an allocator identity?

Failure mode: cloning `std::vector` incompletely, then accumulating undefined
behavior around construction, relocation, over-alignment, and allocation
failure.

### R-006 — Allocation authority

Status: **OPEN**.

Candidates:

- global `new/delete` hidden behind house containers;
- explicit `Allocator` function table passed to application/core construction;
- per-subsystem arenas plus a default allocator;
- no dynamic allocation after initialization for the minimal nucleus profile.

The allocator contract must define alignment, zero-size behavior, resize,
deallocation, failure, thread safety, telemetry, and ownership across module/ABI
boundaries. It must not let memory allocated by one CRT be silently freed by
another.

Failure mode: replacing container names while leaving allocator and OOM behavior
implicit.

### R-007 — Smart-pointer and retained identity policy

Status: **OPEN; high reversal cost**.

GUI.Forms relies heavily on `shared_ptr`, `weak_ptr`, and `unique_ptr`. Options
include:

- retain standard smart pointers initially;
- move unique ownership to a house `Owned<T>` while retaining shared ownership;
- use intrusive strong/weak reference counts for retained controls;
- use generation-checked handles and owner registries;
- split control identity from allocation entirely.

This decision affects cycles, destruction order, weak observation, thread
safety, ABI exposure, allocation count, object size, and lifecycle conformance.
It must be backed by retained-tree and callback-lifetime tests before migration.

Failure mode: changing ownership representation during a syntax rewrite and
silently changing disposal/event order.

### R-008 — Strings and text buffers

Status: **OPEN**.

`std::string` is more pervasive than `std::vector` and often owns UTF-8. We must
decide whether the first rewrite:

- retains `std::string` behind explicit UTF-8 contracts;
- introduces `Utf8String`, `Utf8View`, and `StringBuilder` using the house
  allocator;
- replaces only public/nucleus strings first;
- separates immutable interned IDs from user text and formatting buffers.

The design must address validation, embedded NUL, capacity, formatting,
comparison, hashing, conversion, and OOM. Locale-sensitive APIs must not enter
portable semantics accidentally.

Failure mode: removing `vector` while leaving equivalent hidden allocation and
locale dependencies in strings and streams.

## Cluster C — lambdas and callback machinery

### R-009 — Lambda taxonomy

Status: **OPEN by architect direction**.

Every lambda use must be classified into one of these roles:

1. captureless, non-escaping algorithm predicate;
2. capturing, non-escaping algorithm predicate;
3. captureless callback converted to a function pointer;
4. capturing callback stored for later dispatch;
5. one-shot posted work item;
6. cross-thread task;
7. scope guard/rollback action;
8. recursive local algorithm;
9. test-only fixture convenience;
10. platform callback trampoline.

Candidate policy spectrum:

- allow categories 1 and 3 only;
- allow 1–3 plus tightly bounded category 2 when the closure never escapes and
  creates no type erasure or allocation;
- prohibit all lambdas in the portable nucleus but allow selected categories in
  hosted adapters;
- replace every production lambda with named functions/functors/context
  structures.

Questions:

- Is implicit closure state itself undesirable, or primarily escaping/captured
  lifetime and hidden allocation?
- Are `[&]` and `[=]` always forbidden even if specific captures may be allowed?
- May a captureless lambda be used solely as a C callback trampoline?
- Must every stored action have a stable inspectable name/type?
- Do local comparators merit named functors when used once?

Proposed discussion position: prohibit default captures and all escaping
capturing lambdas; consider allowing captureless non-escaping predicates and
explicitly captured, non-escaping predicates only if their generated closure
never crosses a type-erased or thread boundary. The stricter nucleus profile
may still prohibit all lambdas.

Failure mode: permitting “small lambdas” without a mechanical escape rule, or
prohibiting local predicates while leaving far more consequential opaque
callbacks elsewhere.

Current BFFT capturing lambdas are evidence of unfinished cleanup, not an
approved lambda category. Any category admitted here requires an affirmative
GUI.Forms decision on its own merits.

### R-010 — Which `std::function` uses remain?

Status: **OPEN; reduction GIVEN**.

Current roles include events, dispatch work, host wake/close/dialog services,
paint callbacks, animation callbacks, inspectors, converters, property
accessors, semantic feedback clocks, and platform glue.

Candidate house representations:

- `FunctionRef<Signature>` — non-owning synchronous call, no escape;
- `Callback<Signature>` — function pointer plus `void*` context and explicit
  lifetime contract;
- `Delegate<Owner, Signature>` — object plus member-function binding;
- `InlineFunction<Signature, N>` — owning type erasure with fixed inline
  storage and no heap fallback;
- `OwnedFunction<Signature>` — explicit allocator-backed owning callable;
- typed listener interfaces for semantically important long-lived roles;
- command/work records with an operation enum and data instead of arbitrary
  executable closure state.

We must decide per role, not globally. A provisional matrix to complete:

| Role | Likely direction | Reason still open |
|---|---|---|
| synchronous algorithm visitor | `FunctionRef` or template | lifetime bounded |
| C ABI callback | function pointer + context | already natural at ABI |
| retained UI event | typed callback/delegate or listener | removal and owner lifetime |
| posted UI work | owned work record or bounded callable | escape and cancellation |
| cross-thread task | named task object/message | transfer and shutdown |
| host service | typed interface/function table | capability and lifetime |
| paint callback | typed interface or bounded callback | hot path and state |
| test hook | potentially `std::function` | not product runtime |
| platform-only adapter glue | potentially retained | hosted boundary |

Questions:

- Is heap allocation inside callback binding forbidden, observable, or merely
  discouraged?
- What callable size/alignment is worth inline storage?
- Must callbacks be trivially movable?
- Are exceptions allowed through the callback boundary?
- What invalidation token revokes a callback?
- Which adapter-only uses may retain `std::function` permanently?

Failure mode: writing a home-grown `std::function` with worse lifetime,
exception, alignment, or move behavior and no measurable portability gain.

### R-011 — Events, commands, and dispatch as separate mechanisms

Status: **OPEN**.

The current generic callable vocabulary may obscure three different systems:

- multicast synchronous events with removable subscriptions;
- named commands with state and invocation results;
- queued work with ownership, cancellation, and shutdown behavior.

The rewrite should decide whether these receive separate concrete storage and
execution models. Doing so may remove more closure machinery than a generic
callable replacement while improving inspection and lifecycle behavior.

Failure mode: forcing all three through one clever delegate abstraction.

## Cluster D — hosted runtime and library heavy lifting

### R-012 — Exceptions and RTTI

Status: **OPEN**.

The current source throws in many subsystems. Candidate profiles:

- retain exceptions on desktop; status/result in the constrained nucleus;
- remove exceptions from all production GUI.Forms code;
- allow exceptions only for programmer-contract failures, never allocation or
  runtime service failure;
- compile both exception and no-exception variants from shared sources.

This decision must define constructor failure, allocation failure, callback
fault containment, ABI translation, destructor behavior, and test assertions.
RTTI and exceptions are separate decisions even when compiler flags often pair
them.

Failure mode: nominal `-fno-exceptions` support that aborts unpredictably in
ordinary low-memory or invalid-input paths.

### R-013 — Standard algorithms

Status: **AUDIT REQUIRED**.

The code uses searching, sorting, copying, erasure, transformation, and numeric
accumulation. For each algorithm family, decide among:

- retain the standard algorithm in hosted and nucleus code;
- wrap it behind a house name but keep the implementation initially;
- implement a small verified house algorithm over `ArrayView`;
- use a specialized data structure or algorithm at the call site.

Audit questions:

- Does the algorithm allocate?
- Does it depend on locale, exceptions, execution policies, or hidden global
  state?
- What iterator machinery does it instantiate?
- Is stable ordering required?
- Can the workload be bounded?
- Is the operation hot enough to measure?
- Is code-size or toolchain variance material?

Failure mode: replacing mature `sort`/`lower_bound` implementations without a
correctness corpus and code-size/performance comparison.

### R-014 — libc surface and first constrained runtime

Status: **OPEN; research required**.

Candidates mentioned by the architect include picolibc and klibc. They must be
evaluated as real environments rather than interchangeable labels. The audit
must separately inventory:

- memory and byte operations;
- allocation;
- formatting and numeric conversion;
- math functions and floating-point environment;
- time and clocks;
- file and stream I/O;
- locale and character classification;
- process environment and dynamic loading;
- threads, atomics, and synchronization;
- startup, static initialization, and termination.

The first conformance target might instead be a purpose-built minimal shim or a
UEFI toolchain profile. No target is selected yet.

Failure mode: claiming klibc/picolibc compatibility from header compilation
without linking and executing the portable nucleus under the actual runtime.

### R-015 — Threads, atomics, clocks, and scheduling

Status: **OPEN**.

The portable core currently touches `thread`, `mutex`, `condition_variable`,
`atomic`, and `chrono`. Decide which layer owns concurrency:

- core contains portable synchronization interfaces;
- core is single-thread-affine and hosts provide queues/clocks;
- desktop build uses standard primitives behind interfaces;
- constrained profile supplies cooperative or interrupt-safe implementations.

The UI thread contract, dispatcher ordering, timers, frame scheduling, and
shutdown semantics must remain behaviorally identical.

Failure mode: a “single-threaded” firmware profile that leaves atomics or
thread-safe reference counts embedded in every retained object.

### R-016 — Filesystem, streams, formatting, locale, and dynamic loading

Status: **OPEN**.

These are low-frequency by source count but high-impact hosted dependencies.
Each should move to an adapter or explicit service where possible. Decide
whether `charconv` remains the portable numeric conversion base, whether stream
formatting is eliminated from the nucleus, and whether dynamic loading exists
only in platform hosts.

Failure mode: concentrating hosted dependencies into a “utility” library that
the nucleus still links unconditionally.

## Cluster E — architecture preservation and migration

### R-017 — Public API and ABI compatibility

Status: **OPEN; Orchestrator negotiation applies**.

The C ABI, public C++ API, managed compatibility facade, and internal object
layout have different stability costs. Decide:

- C ABI semantics and tables remain stable throughout;
- C++ source compatibility is preserved with adapters/deprecated overloads;
- C++ source compatibility may break at a named major boundary;
- internal object layout is never stable ABI;
- container-returning public APIs migrate to views, caller-provided buffers, or
  house snapshots.

Failure mode: an internal `vector` replacement accidentally changes a
registered cross-project contract or allocator ownership across modules.

### R-018 — Layering of the future house runtime

Status: **OPEN**.

Candidate layers:

1. `base`: integers, status, checked arithmetic, traits, move/forward, views;
2. `memory`: allocator, heap array, arena, ownership primitives;
3. `text`: UTF-8 string/view/conversion;
4. `algorithm`: bounded algorithms over house views;
5. `callback`: references, delegates, owned work;
6. `concurrency`: clock, atomics, locks, queue interfaces;
7. retained GUI core;
8. controls;
9. render/text/platform adapters.

We must decide whether this is private to GUI.Forms or a separately reusable
library. Creating a general private standard library too early could consume the
program without proving the UI.

Failure mode: designing a comprehensive STL replacement before the call-site
inventory tells us what GUI.Forms actually needs.

### R-019 — Migration order and coexistence

Status: **OPEN**.

Candidate strategies:

- bottom-up primitives, then one vertical control slice, then subsystem waves;
- public-seam-first adapters, then internals;
- leaf-file mechanical conversion followed by architectural replacements;
- parallel old/new cores with differential traces.

The plan should prefer compilable, bisectable waves and exact behavioral
oracles. A compatibility bridge may temporarily convert between house and
standard containers, but every bridge must have an owner and removal gate.

Failure mode: a repository-wide mechanical replacement that is unreviewable,
unbisectable, and conflates syntax with lifecycle changes.

### R-020 — Enforcement

Status: **OPEN**.

Possible gates:

- Clang AST-based policy checker for `auto`, lambda categories, standard
  containers, and `std::function`;
- include allowlists by layer;
- forbidden-symbol link scan for constrained artifacts;
- C++17/C++20 and no-exception/no-RTTI build matrix as selected;
- allocator/failure injection;
- code-size and undefined-symbol budget;
- freestanding or constrained-runtime link-and-run smoke;
- ABI table and headless trace equivalence;
- source-level temporary-exemption registry with expiry.

Regex alone is insufficient for lambda escape, implicit allocations, template
instantiation, or transitive link dependencies.

Failure mode: a style document that immediately drifts because no build gate
knows which profile applies to which file.

### R-021 — Completion definition

Status: **OPEN**.

Completion may require:

- zero forbidden constructs in the selected scope;
- zero `std::vector` in first-party production code;
- only enumerated `std::function` exemptions;
- a closed libc/std dependency manifest;
- preserved tests and exact lifecycle/headless traces;
- desktop performance and memory within accepted bounds;
- a constrained-runtime artifact that links and executes a meaningful retained
  headless or framebuffer workload;
- migration shims removed or explicitly retained.

The architect must decide which of these are rewrite completion versus later
portability milestones.
