# GUI.Forms rewrite decision ledger — architect response

> **Closure addendum, 2026-08-10:** the later architect answers in
> `ARCHITECT_SELECTIONS.md` resolve the `OPEN` and `AUDIT REQUIRED` choices
> below. Historical status labels in this response are retained to preserve the
> original interview record; the later selections control.

Status rule: only items explicitly approved below are `DECIDED`. Where the architect has approved part of an item but not the whole design space, the item remains `OPEN` or `AUDIT` and the approved constraints are recorded underneath it.

This rewrite is not a portability rewrite, STL purge, API redesign, or attempt to create a private standard library. It is an exact-behavior rewrite of first-party C++ outside experiment folders. Files and APIs remain where they are. The work changes how existing code accomplishes the same behavior.

The governing criterion is epistemic cost. Prefer explicit, named, inspectable machinery when an abstraction quickly increases uncertainty about type, lifetime, allocation, control flow, or cost. Compiler machinery is acceptable when the architecture has already made the decision and the compiler is merely instantiating it. It is undesirable when inference itself decides what the code means.

A correction to earlier discussion is important: earlier references to concern about "`std::function`" sometimes meant standard-library named functions/algorithms generally. The actual `std::function` type is not a broad rewrite target. R-013 is the important standard-library algorithm audit.

---

## Cluster A — target and language boundary

### R-001 — What does “someday UEFI/bare metal” require from this rewrite?

Status: **DECIDED**.

It does not create a firmware deliverable, framebuffer proof, freestanding nucleus contract, or dependency rewrite.

The rewrite must not restructure Skia, text, rendering, platform hosts, or dependent libraries around hypothetical UEFI or bare-metal requirements.

The useful influence of that future possibility is narrower: where GUI.Forms currently relies on costly or opaque standard-library machinery, prefer a smaller, explicit dependency when that gives a concrete benefit in inspectability, determinism, performance, or implementation control.

Do not turn this into candidate 1, 2, or 3 from the original question. There is no present firmware parity target.

A future constrained host is a separate project/milestone and must justify its own requirements.

---

### R-002 — What is the rewrite boundary?

Status: **DECIDED**.

The rewrite applies to all first-party C++ files outside experiment folders.

A rewrite is a rewrite: preserve file organization and API surface while materially changing implementation code to accomplish the same behavior.

Do not invent layer-specific language policy exceptions merely because a file is in `src/host/*` or `src/render/*`. If an actual hosted/platform object creates a conflict with the house policy, bring that exact case to the architect in chat.

Do not let tests, demos, tools, generated code, or compatibility work redefine production architecture. Whether a generated source should be exempt from a source-style rule must be decided from the concrete generator/output case, not presumed globally.

Public API changes, hosted-object API changes, or ABI changes are not authorized by this rewrite.

---

### R-003 — What C++ language level and subset are normative?

Status: **OPEN**.

The version badge is not the policy. Define an explicit feature list independent of C++17/C++20/C++26 labels.

Approved feature decisions so far:

- `auto`: banned in first-party C++ in scope.
- trailing-return `->` syntax: banned.
- structured bindings: banned.
- generic lambdas: banned.
- all lambdas: banned.
- coroutines: banned.
- `consteval`: permitted.
- templates: permitted and encouraged where they make generic machinery explicit and inspectable.
- `std::span`: permitted.
- `std::optional`: permitted.
- `std::variant`: permitted.
- `std::any`: banned.
- defaulted spaceship/comparison convenience: banned as a general style. Define only comparisons actually required.
- RTTI: permitted where the current architecture actually uses it; do not create an RTTI-removal project inside this rewrite.

Still requires explicit discussion if encountered materially:

- concepts;
- ranges;
- designated initializers;
- `std::bit_cast`;
- `decltype(expression)` in type-level plumbing;
- other compiler extensions or new C++ features not already decided.

The intended style is a blend of traditional/orthodox C++ and engine-style low-level machinery where it earns its place. Generic programming itself is not the target.

---

### R-004 — Exact `auto` prohibition and inference grammar

Status: **OPEN**, with the important cases decided.

Approved bans:

```cpp
auto control = FindControl(id);
auto iterator = controls.begin();
auto FindControl(Id id) -> Control*;
const auto [index, generation] = DecodeHandle(handle);
[](const auto& left, const auto& right) { ... }
decltype(auto)
```

Use explicit ordinary declarations instead:

```cpp
Control* control = FindControl(id);
ControlIterator iterator = controls.begin();
Control* FindControl(Id id);
DecodedHandle decoded = DecodeHandle(handle);
```

The objection is not concision. The objection is delegating architectural/type choices to compiler inference when the source can state them.

The pointer member operator `->` is also banned by house style. Pointers are plumbing for direct access and in-place work. Dereference explicitly and then use the dot operator where a pointer must be used:

```cpp
(*pointer).member
```

The preference is not anti-pointer. It is anti-sugar that obscures ordinary object semantics.

`using` aliases/imports are permitted.

Still open: the exact allowed surface for `decltype(expression)` in narrow type-trait/toolbox implementation code.

---

## Cluster B — allocation, ranges, and ownership

### R-005 — GUI.Forms heap-array family

Status: **DECIDED**.

Do not create a replacement family merely to remove `std::vector`.

Keep `std::vector` as the ordinary growable contiguous collection for now.

Arrays do not grow; vectors grow; lists grow. Do not force them into one house abstraction.

Dynamic structures such as a hive-inspired stable pool, bag, special deque/list storage, or fixed heap array may be tested later when a concrete GUI.Forms workload benefits from their semantics or performance.

A house collection must exist because its contract is materially different or because measured implementation control matters, not because `std::` appears in the name.

BFFT code may be copied into GUI.Forms when pragmatically useful, but GUI.Forms must not depend on BFFT as a library.

---

### R-006 — Allocation authority

Status: **OPEN**.

There is no decision to replace `malloc`, global `new/delete`, or the process allocator.

There is also no decision to introduce allocator identity into every container/object.

Current direction:

- investigate a small internal allocator capability only if a concrete house facility needs it;
- benchmark alternative process/global allocators such as mimalloc-like approaches separately from object architecture;
- do not turn this rewrite into allocator propagation work.

OOM policy direction is approved:

GUI infrastructure is not designed around routine OOM recovery. The interface is substantially allocated as the program runs and has bounded dynamism compared with content.

Content/resource allocation can fail. When content cannot be allocated/displayed, the program should soft-fault that operation, report a useful warning, return to a stable state, and remain closable.

GUI.Forms already has examples of this style in bounded resource/content code. Preserve that distinction rather than trying to make every allocation an explicit result type.

Still open if a house allocator is introduced:

- exact function table;
- alignment contract;
- resize/reallocation;
- telemetry;
- thread-safety requirements;
- allocator ownership across internal module boundaries.

---

### R-007 — Smart-pointer and retained identity policy

Status: **OPEN**.

Approved direction:

- `std::unique_ptr` is fine.
- `std::shared_ptr`/`std::weak_ptr` are not banned by style.
- do not change retained control ownership representation as part of a syntax cleanup.

Current GUI.Forms control identity/lifetime materially relies on shared/weak ownership. Any replacement must be justified as an ownership/lifecycle project and must preserve disposal/event order.

Generation-checked stable pools/handles are interesting for later specialized storage, not a blanket replacement for current control ownership.

---

### R-008 — Strings and text buffers

Status: **OPEN**.

No broad `std::string` removal has been approved.

Do not invent `Utf8String`, `StringBuilder`, or a house string library merely for house consistency.

Existing explicit UTF-8 contracts, text limits, and content-failure behavior are useful and should remain inspectable.

Revisit string storage only where a concrete call site shows an allocation, ownership, validation, or performance reason.

---

## Cluster C — lambdas and callback machinery

### R-009 — Lambda taxonomy

Status: **DECIDED**.

Lambdas are banned in first-party C++ in scope.

Do not create category exceptions for captureless predicates, synchronous captures, C trampolines, test convenience, or “small lambdas.”

Use named functions, named functors, named context structures, explicit delegates, or explicit platform callback trampolines.

Reason: closure syntax introduces hidden/unnamed executable types and, with captures, hidden lifetime state. Even where lifetime is safe, named machinery improves inspectability.

---

### R-010 — Which `std::function` uses remain?

Status: **DECIDED**.

`std::function` is permitted.

This is not a program to replace every `std::function` with a house type.

Audit only conspicuously wasteful, opaque, allocation-heavy, or lifetime-awkward uses.

The strongest approved house-abstraction target is events.

The intended event direction is:

```cpp
Delegate<Signature>
Event<Signature>
```

`Delegate` answers how to invoke a named target. `Event` owns subscription ordering, revocation, removal, and emission semantics.

Binding a delegate must not heap-allocate merely to store a callback. Rich member binding is desirable because it is easy to understand:

```cpp
button.Clicked().Add(
    Delegate<ButtonEvent>::Bind(dialog, Dialog::OnButtonClicked));
```

Do not build a universal replacement that simply recreates `std::function` under a house name.

Other `std::function` uses remain unless a concrete audit shows that they are “big dumb” uses worth cleaning up.

---

### R-011 — Events, commands, and dispatch as separate mechanisms

Status: **OPEN**, with event direction decided.

Events should receive a house abstraction as described in R-010.

Do not presume commands, queued dispatch, timers, host services, and paint callbacks should use the same representation.

Current dispatch has genuinely different semantics: ownership across time, cancellation, thread transfer, waiting, failure capture, shutdown, and bounded queue behavior.

Any command/dispatch redesign must be justified separately rather than forced through one clever delegate abstraction.

---

## Cluster D — hosted runtime and library heavy lifting

### R-012 — Exceptions and RTTI

Status: **OPEN**, with direction approved.

Exceptions are permitted.

The intended style is between the original options B and C:

- exceptions may wrap substructures/operations that genuinely fail;
- exceptions are not ordinary control flow;
- exception-heavy paths should not sit in typical/hot execution;
- failure boundaries may catch/translate exceptions;
- ABI boundaries must continue containing exceptions rather than allowing them to cross.

The point is not ideological exception elimination. The point is to avoid letting exception machinery dominate normal runtime behavior.

RTTI is permitted where currently required. GUI.Forms presently has real RTTI use in concrete control adaptation. Do not create an RTTI-removal project unless a specific use becomes a problem.

---

### R-013 — Standard algorithms

Status: **AUDIT REQUIRED**.

This is a primary rewrite concern.

Do not replace standard algorithms indiscriminately. The decision depends on how much the algorithm hides, how implementation-sensitive the cost is, and what GUI.Forms knows about its own workloads.

Approved KEEP decisions:

- `std::min`
- `std::max`
- `std::clamp`
- `std::find`
- `std::find_if`
- erase/remove idiom
- `std::copy`
- `std::copy_n`
- range `std::move`
- ordinary `std::move(object)`
- `std::accumulate`
- `std::inner_product`
- `std::any_of`
- `std::all_of`
- `std::none_of`
- `std::visit`

`accumulate` and `inner_product` are accepted as well-known mathematical operations and have already been dogfooded/performance-tested in BFFT/Cleanup-style work.

Sorting is the important exception.

Audit every `std::sort` and `std::stable_sort` call site based on actual workload characteristics:

- typical element count;
- maximum realistic count;
- already-sorted or nearly-sorted frequency;
- reverse-sorted frequency;
- duplicate/equal-key density;
- stability requirement;
- element size;
- cost of moving/swapping;
- comparator cost;
- hot-path frequency;
- temporary-memory tolerance;
- implementation variance across standard libraries.

The intended outcome is a small house collection of sorting routines, not one universal “best” sort and not an elaborate algorithm library.

A theoretically slower algorithm may be faster for the actual bounded/partially ordered workload. Measure representative GUI.Forms cases.

Potential house sort vocabulary should remain small and descriptive. Candidate algorithms must be chosen from call-site evidence, not fashion. Timsort-like/adaptive stable sorting is relevant where data is commonly partially ordered, but no algorithm is preselected.

`std::stable_sort` deserves the same workload audit, especially because allocation/memory behavior may matter.

Binary search family:

Prefer a tiny house binary-search implementation where used. It is simple enough to own and inspect directly.

Heap algorithms:

`std::make_heap`, `std::push_heap`, `std::pop_heap` are considered obscure. Do not introduce them casually. If a priority structure is actually required, select a named data structure/operation deliberately.

Still open if encountered:

- `std::transform`;
- other named algorithms not listed above;
- whether a particular call site is clearer as a direct loop.

The rule is: house-own an algorithm when implementation choice itself matters to performance, determinism, memory behavior, or cross-STL variance. Otherwise use the clear named standard operation.

---

### R-014 — libc surface and first constrained runtime

Status: **DECIDED for this rewrite**.

Do not select picolibc, klibc, a UEFI shim, or another constrained runtime as a rewrite acceptance target.

No constrained-runtime link-and-run proof is required by this rewrite.

If a future firmware/constrained product is opened, perform that audit against the actual runtime/toolchain then.

---

### R-015 — Threads, atomics, clocks, and scheduling

Status: **AUDIT REQUIRED**.

Do not remove current threading/atomic behavior merely for portability.

Manual/house atomic-threading machinery is possible. The architect has existing concurrency-safe BFFT code that may be audited if GUI.Forms has a concrete need for it.

Do not integrate that machinery simply because it exists.

Preserve current UI-thread, dispatcher ordering, timer, frame scheduling, cancellation, and shutdown behavior.

No “single-thread firmware nucleus” requirement exists in this rewrite.

---

### R-016 — Filesystem, streams, formatting, locale, and dynamic loading

Status: **OPEN**.

No blanket rewrite has been approved for these facilities.

Review concrete uses under the same epistemic/performance rule. Hosted/platform-required behavior may remain hosted.

Do not move facilities into a fake “portable utility” layer merely to satisfy an invented portability target.

---

## Cluster E — architecture preservation and migration

### R-017 — Public API and ABI compatibility

Status: **DECIDED**.

Files and APIs remain exactly as they are unless the architect explicitly approves a concrete exception.

The rewrite changes implementation, not the public shape of GUI.Forms.

The C ABI currently uses sensible ownership boundaries including generational handles, caller-owned buffers/views, and C callbacks with context pointers. Do not alter those contracts as incidental cleanup.

Any question about:

- changing a public C++ type;
- changing a public return/storage model;
- changing C ABI ownership;
- changing hosted/native object exposure;
- changing managed-facade assumptions;

must be brought directly to the architect in chat.

Internal implementation/layout may change as required to perform the rewrite, provided external behavior and API remain intact.

---

### R-018 — Layering of the future house runtime

Status: **DECIDED**.

Do not build a separately reusable GUI.Forms runtime or Mini-STL.

Use a small private toolbox inside GUI.Forms.

BFFT already exists as a separate library but GUI.Forms must not depend on it. Where BFFT contains useful proven machinery, copy/adapt the relevant implementation into GUI.Forms pragmatically.

Templates are particularly welcome in this toolbox when they act like explicit generic subprograms: easy to instantiate, inspect, and reason about without requiring every generic operation to become its own runtime object hierarchy.

House facilities should remain few and purpose-driven.

Currently justified or plausible toolbox items include:

- event/delegate machinery;
- tiny binary search;
- a small measured sort collection;
- later, specialized stable/churn storage if a workload proves it useful;
- later, allocator/concurrency primitives only if a concrete need appears.

Do not create layers such as `base`, `memory`, `text`, `algorithm`, `callback`, `concurrency` merely because such a hierarchy looks complete.

---

### R-019 — Migration order and coexistence

Status: **OPEN**.

No migration sequence has been architect-approved yet.

Constraints:

- preserve compilability and behavior;
- do not perform a repository-wide mechanical replacement that obscures semantic changes;
- do not combine ownership redesign with unrelated syntax cleanup;
- preserve current tests/lifecycle behavior while replacing machinery;
- temporary bridges should exist only where a concrete migration needs them.

A likely practical first architectural proof is event/delegate cleanup because it directly exercises the house-toolbox philosophy without requiring API changes, but this is not yet a mandated global migration order.

---

### R-020 — Enforcement

Status: **OPEN**.

The style rules are now specific enough to automate, but the exact gate has not been chosen.

An enforcement design should be capable of detecting, at minimum:

- `auto`;
- trailing-return `->`;
- lambdas;
- structured bindings;
- generic lambdas;
- coroutines;
- `std::any`;
- prohibited defaulted comparison/spaceship forms;
- any later explicitly forbidden constructs.

Regex alone is not sufficient for all of these.

Do not create a giant policy framework before the rewrite needs it. Prefer a narrow mechanical gate whose error points to the exact source construct and whose scope excludes experiment folders.

Algorithm choices such as sort selection are review/performance decisions, not merely forbidden-token checks.

---

### R-021 — Completion definition

Status: **OPEN**.

Rewrite completion must not be tied to UEFI, picolibc/klibc, framebuffer rendering, Skia replacement, allocator replacement, or complete STL elimination.

Likely completion dimensions, still requiring final architect approval, are:

- no banned language constructs in the selected first-party C++ scope;
- public API/file organization preserved;
- behavior and lifecycle tests preserved;
- conspicuously bad `std::function` uses cleaned up, with event machinery moved to the approved house abstraction;
- `std::sort`/`std::stable_sort` call sites audited and migrated only where the measured house choice is justified;
- binary-search uses moved to the tiny house implementation where appropriate;
- no accidental BFFT link-time dependency;
- no speculative firmware/platform architecture changes.

Anything beyond those constraints is a separate milestone unless explicitly pulled into rewrite completion.

---

## Repository facts that should constrain further proposals

These are observations about the current code, not new architecture decisions.

1. GUI.Forms currently builds as C++20 and is split into retained core, controls, GUI.Drawing, headless host, platform hosts, Skia/text adapters, and shared C ABI targets.

2. Current retained control ownership uses `std::shared_ptr` children, `std::weak_ptr` parents/observers, and raw/non-owning pointers for attached plumbing. Replacing shared ownership would therefore be a lifecycle architecture change.

3. Current `Event` machinery is unusually heavy relative to its surface: `std::function`, shared event state, separately allocated shared slots, weak back-links, vector storage, snapshot copying during emit, and callback copying for self-disconnect safety. This is why events are an approved house-abstraction target.

4. Current dispatch is materially different from events: queued `std::function`, shared work objects, `deque`, atomics, mutexes, condition variables, cancellation, cross-thread synchronous wait, and exception capture. Do not replace it merely because event callbacks changed.

5. Current C ABI already uses generation-checked handles, caller-provided buffers/views, and function-pointer-plus-context callbacks. It does not require exposing STL ownership across the ABI.

6. Current image/resource code already demonstrates the desired content-failure philosophy: explicit resource/byte limits, generational IDs, and allocation failure translated to an explicit content error.

7. Current text storage likewise has explicit content limits while retaining ordinary `std::string`/`std::vector` internally.

8. No GUI.Forms-wide custom allocator regime has been identified. Most ordinary allocation currently arrives through standard containers and smart pointers; platform adapters additionally obey their native allocation/ownership rules.

9. Current code uses RTTI in real adapter/registry paths (`dynamic_pointer_cast`-style concrete type discovery), while other subsystems deliberately avoid reflection/RTTI. RTTI removal is therefore not mechanical cleanup.

10. Current BFFT `heap_array` is evidence for the desired house style when such machinery is useful: explicit capacity/length, explicit allocation, simple move/relocation assumptions, direct pointer/index access, and no attempt to recreate the entire standard-container contract.

---

## Architect intent to use when an unlisted case appears

When a construct is not explicitly listed above, do not infer a ban from the fact that it is modern C++, standard-library code, template code, or hosted code.

Ask instead:

1. Does this construct rapidly hide what type, storage, lifetime, control flow, or work the program is actually performing?
2. Is the hidden choice one the architecture should decide explicitly?
3. Does the implementation vary materially across standard libraries/toolchains in a way GUI.Forms actually cares about?
4. Can a small named house implementation make the behavior more inspectable or more performant without creating a general-purpose replacement library?
5. Is there call-site evidence or measurement for owning the machinery?

If the answers are mostly no, keep the ordinary C++/standard-library facility.

If the answers are yes, prefer the smallest explicit house mechanism that solves the real GUI.Forms case.

Do not broaden a local cleanup into an architectural rewrite without bringing the concrete boundary case back to the architect.
