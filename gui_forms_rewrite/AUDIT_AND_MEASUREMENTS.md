# Rewrite audit and measurement program

Status: **active implementation audit; first native baseline and semantic
inventory captured on 2026-08-10**.

## 0. Captured implementation baseline

- **OBSERVED:** authoritative branch `main`, starting commit
  `8c6806bab7faddf942b2b02e64ee54a47dc3c7cc`. Unrelated dirty work under
  `orchestrator/` remains untouched.
- **MEASURED:** the mandated M4 host completed the pre-rewrite GUI.Forms build
  with Apple Clang 21.0.0 and passed 64 of 64 configured tests.
- **MEASURED:** after the first binary-search proof, the complete build passed
  and all 11 affected binary-search, text, list, and ABI tests passed. The new
  equivalence test compares lower bound, upper bound, and exact membership with
  the standard-library controls across empty, singleton, duplicate,
  heterogeneous-key, edge, constexpr, and deterministic generated inputs.
- **OBSERVED:** `HOUSE_POLICY_AUDIT.json` is the first Clang LibTooling ledger,
  captured after the binary-search proof and before the Delegate/Event proof.
  LLVM/Clang LibTooling 22.1.8 traversed 182 production translation units using
  the exact native compilation database, Homebrew Clang resource directory,
  and active Xcode SDK.

The first semantic ledger contains 8,404 deduplicated findings: 7,866 current
violations and 538 admitted/review findings. Its construct counts are:

| Construct | Findings | Current violations |
|---|---:|---:|
| pointer member access spelled with `->` | 5,506 | 5,506 |
| `auto` / deduced source type | 1,467 | 1,467 |
| lambda | 767 | 767 |
| `std::span` | 311 | 0 |
| `if constexpr` | 117 | 0 |
| designated initialization requiring target classification | 82 | 0 |
| defaulted comparison | 72 | 72 |
| structured binding | 33 | 33 |
| `std::any` source locations | 25 | 1 before checker namespace correction |
| `decltype(expression)` | 19 | 19 |
| `has_single_bit` | 4 | 0 |
| requires-expression | 1 | 1 |

The single reported `std::any` violation is the already-approved
`gui_forms::showcase::initialize_showcase_runtime` Tag dogfood path. The checker
originally matched the function without its `showcase` namespace; that checker
classification is corrected for the next ledger. It is not a production-code
policy violation.

**OBSERVED warning inventory:** parsing the macOS host with the active macOS 26
SDK reports seven deprecation diagnostics across the existing CoreVideo display
link create/callback/release/running/start/stop calls. They are retained and
routed as platform-host warning work; this syntax rewrite does not silently
change frame scheduling APIs or behavior.

### First explicit-language waves

- **MEASURED:** the Delegate/Event proof and the existing lifecycle, retained
  lifetime, dispatcher, timer, frame-scheduler, and binding controls passed 7
  of 7 affected tests. Delegate binding is two pointers and trivially copyable;
  it allocates no storage merely to bind. Event slot/state allocation and the
  legacy owning `std::function` overload remain deliberately unchanged.
- **MEASURED:** the requires-expression replacement and direct ASCII-lowercase
  loop passed the full build and 9 of 9 affected construction/layout/Skia
  controls.
- **OBSERVED:** the native AST rewrite plan covered all 5,487 member-arrow
  findings then visible in that configuration, across 114 files. The local
  applier verified every captured file size and every operator token before
  rewriting it as an explicit dereference.
- **MEASURED:** after that pointer-plumbing wave, the complete native build
  succeeded and all 66 configured tests passed. This proves the active native
  branches only; no-HarfBuzz and MinGW/Windows compilation databases remain
  separate required surfaces.

## 1. Source snapshot and scope

The current worktree contains substantial pre-existing GUI.Forms changes.
Before implementation the sibling must record branch, commit, status, compiler,
toolchain, build options, and exact in-scope file list without cleaning or
overwriting user work.

Normative scope is production `include/` and `src/`, rewritten first. Tests,
demo, tools, compatibility sources, and first-party generated output are updated
afterward where required to compile, exercise, or accurately represent the
production contract. Generated changes begin at the first-party generator or
template. Experiments and third-party source remain excluded.

## 2. Banned-construct inventory

Use the compilation database and Clang AST as authority. Report every:

- `auto` and `decltype(auto)`;
- trailing return;
- pointer-member arrow expression;
- lambda and generic lambda;
- structured binding;
- coroutine construct;
- `std::any` outside the existing WinForms-compatible Tag surface/backing/
  disposal/compatibility exception;
- defaulted spaceship/comparison convenience;
- `decltype(expression)` outside named private type-trait/detection plumbing;
- ranges/views and other unapproved C++20-added feature uses;
- the one requires-expression, which becomes the explicit initialization trait.

Emit machine-readable records with file, line, enclosing symbol, construct
kind, and replacement category. Regex is an orientation/control only.

Remain in C++20 mode and use current toolchains. Inventory actual C++20-added
features rather than admitting features by standard-version badge. Supplement
the semantic checker with relevant tightened compiler warnings where useful.

## 3. Replacement classification

Do not mechanically replace syntax without classifying its architectural role.

For each `auto`, record the explicit intended type and whether a stable alias is
needed. For each pointer arrow, distinguish non-owning plumbing, ownership,
platform opaque state, array traversal, and optional access. For each lambda,
classify named comparator, local helper, event callback, stored callback,
dispatcher task, cross-thread task, rollback action, visitor, or platform
trampoline.

The replacement must preserve execution and lifetime:

- named free/member function;
- named functor with explicit state;
- named context/trampoline;
- Delegate/Event binding;
- retained `std::function` constructed from named machinery;
- explicit helper algorithm;
- direct loop where clearest.

## 4. Event audit

Characterize current Event behavior before replacement:

- subscription order;
- self-disconnect and peer-disconnect during emission;
- subscription during emission;
- owner disposal and token revocation;
- nested/reentrant emission;
- exception behavior;
- callback copying and snapshot semantics;
- heap allocations per event, subscription, bind, emission, and removal;
- strong/weak lifetime edges;
- thread-affinity enforcement.

The approved Delegate/Event implementation must bind named member targets
without heap allocation solely for binding. It adds a Delegate-first path while
retaining the legacy `std::function` overload. Measure object size, invocation
cost, subscription cost, and emission cost against the existing implementation.
Do not apply event representation to dispatch automatically.

## 5. `std::function` focused audit

`std::function` remains allowed. Inventory by semantic role and identify only
conspicuous cases where it creates avoidable allocation, copying, opacity, or
lifetime uncertainty. Record callable size/alignment, bind/copy/move frequency,
allocation behavior, escape lifetime, cancellation/revocation, thread transfer,
and exception containment.

Retain ordinary justified uses. Do not create a universal replacement.

## 6. Sort and search audit

Follow `SORT_AND_SEARCH_AUDIT.md`. Instrument actual counts/order/duplicates,
stability, movement, comparator cost, call frequency, scratch allocation, and
cross-toolchain behavior.

The laboratory candidates are stable insertion, run-aware stable
merge/adaptive, and explicit-stack unstable partition/introspective sorting.
They must be optimal, small, named, and measured against
`std::sort`/`std::stable_sort` on representative and adversarial controls. Keep
negative results and retain the standard operation where no candidate earns
admission.

House binary-search templates require exhaustive boundary tests, property/fuzz
equivalence, explicit comparator types, and no overflow.

## 7. Specialized-storage admission

`std::vector` remains. A BFFT-style heap array enters only for a presently
large, non-growing heap allocation edited in place. A later stable pool remains
outside this rewrite. Record:

- fixed versus growable behavior;
- alignment;
- length/capacity and maximum size;
- construction, relocation, and destruction requirements;
- OOM/failure path;
- address-stability requirement;
- performance/code-size comparison;
- why `std::array`, `std::vector`, or direct ownership is insufficient.

## 8. Exceptions, ownership, threading, and allocators

These are preservation audits, not automatic rewrite programs.

- Verify exceptions remain exceptional and contained at ABI/failure boundaries.
- Do not change shared/weak retained ownership without a separately approved
  lifecycle decision and complete disposal/event-order evidence.
- Regenerate `OWNERSHIP_AUDIT.md` with `tools/trace_ownership.py` at the exact
  start snapshot. Use it only to bank evidence for the future lifecycle round.
- Preserve dispatcher, thread, timer, atomics, frame, cancellation, and shutdown
  behavior.
- Do not propagate allocators through object architecture. Any allocator or
  process-allocator benchmark is a separately named experiment.

## 9. Exact-behavior gates

Every batch must preserve applicable:

- ADR-014 lifecycle traces;
- retained attachment/disposal order;
- event, command, and callback order;
- focus, capture, input, close, modal, and dispatcher semantics;
- C ABI tables, handles, caller buffers, callbacks, and error containment;
- headless traces;
- rendering command/damage output;
- accessibility semantics and native publication behavior;
- public declarations and file organization;
- dependency and link boundaries.

Minor native C++ API deltas germane to the rewrite are permitted, so the gate
records and reviews them rather than assuming byte-for-byte declaration
identity. C ABI ownership, dependent APIs/boundaries, behavior, and existing
paths remain fixed unless separately approved.

Screenshots are supplementary, never sole equivalence evidence.

## 10. Performance and build controls

Record before/after where affected:

- clean/incremental build time and peak compiler memory;
- binary size;
- allocation counts and live bytes;
- retained tree construction/destruction;
- event subscription/emission;
- dispatcher latency;
- sort/search call-site distributions;
- existing layout, damage, render, and interaction benchmarks;
- native and headless test results.

No claim of simpler, faster, lighter, or more portable is accepted without the
named source/binary/workload evidence that supports it.
