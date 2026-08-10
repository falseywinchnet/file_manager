# Audit and measurement program

Status: **method draft; baseline must be refreshed before implementation**.

## 1. Source snapshot discipline

The working tree was already heavily modified when this package was created.
The implementing sibling must not use the orientation counts in `GIVENS.md` as
a frozen baseline. Before implementation it must record:

- branch and commit;
- `git status --short` without modifying or cleaning user work;
- exact source roots included;
- compiler and standard-library implementation/version;
- target OS/triple and build options;
- generated and third-party exclusions.

## 2. Syntax and semantic inventory

Use Clang's AST or a compilation-database-based tool as the authoritative
inventory. Text search is only orientation.

Required records:

- every `auto`, `decltype(auto)`, structured binding, and generic lambda;
- every lambda with capture list, escape destination, conversion/type erasure,
  allocation possibility, thread transfer, and lifetime owner;
- every `std::function`, signature, construction site, callable size if
  measurable, copy/move count, allocation count, and semantic role;
- every `std::vector`, element type, peak size/capacity, mutation operations,
  copy/move behavior, public exposure, and allocation phase;
- every standard container and smart pointer;
- every exception throw/catch boundary and RTTI use;
- every standard algorithm and its semantic requirements;
- every direct and transitive standard/C/POSIX/platform header and symbol.

Inventories should be emitted as TSV or JSON plus a concise Markdown summary.

## 3. Container workloads

Instrument representative runs to learn, per vector-like field:

- construction and destruction count;
- requested sizes and peak live count;
- capacity growth and bytes reserved/used;
- reallocation count;
- element relocation/copy/destruction count;
- allocation failure behavior under injection;
- hot-path mutation frequency;
- cross-thread or cross-module transfer;
- whether stable addresses are assumed accidentally.

Classify each use into fixed heap array, growable list, inline fixed array,
small array, byte buffer, arena allocation, associative structure, ring queue,
or specialized domain collection before replacement.

## 4. Callback workloads

For each stored callback family measure:

- callable object size/alignment distribution;
- heap allocations during binding/copy/move;
- bind/unbind frequency;
- invocation frequency and latency distribution;
- escape lifetime and revocation path;
- capture ownership, especially strong/weak retained-control references;
- cross-thread transfer;
- exception/fault behavior;
- cancellation and shutdown behavior.

This evidence chooses inline callable capacity and determines where a typed
listener is better than generic type erasure.

## 5. Standard-library and libc dependency map

Produce two maps:

1. source dependency: headers, templates, and direct symbol calls;
2. binary dependency: undefined/imported symbols, unwind/RTTI sections, static
   constructors, TLS, allocator calls, and linked libraries.

Audit at least:

- allocation and object runtime;
- byte/memory functions;
- math and floating point;
- strings, conversion, hashing, locale, and formatting;
- sorting/search/copy algorithms;
- exceptions, RTTI, guards, and static initialization;
- atomics, locks, threads, TLS, clocks, and condition variables;
- streams/filesystem/environment/dynamic loading;
- platform framework calls;
- third-party transitive requirements.

Do not count inline template names as runtime imports, and do not treat a small
dynamic-symbol list as proof of a small static runtime.

## 6. Replacement primitive proof gates

Every new primitive needs:

- specification of invariants and failure behavior;
- trivial and nontrivial element tests;
- alignment and overflow tests;
- allocation failure injection;
- move/copy/destruction oracle;
- fuzzing where input controls sizes or mutation sequences;
- sanitizer runs on hosted builds;
- code-size comparison;
- representative latency/allocation comparison;
- at least two compiler/standard-library configurations until the constrained
  target is selected.

## 7. GUI behavior preservation

The rewrite must retain:

- lifecycle traces required by ADR-014;
- retained attachment/disposal order;
- event and callback order;
- focus, capture, input, close, modal, and dispatcher semantics;
- exact C ABI table behavior and stale/wrong-kind/wrong-thread errors;
- headless trace fixtures;
- rendering command/damage equivalence where representation is not intended to
  change;
- accessibility and host capability reporting.

Representation equivalence is not inferred from screenshots alone.

## 8. Performance and footprint controls

Record before/after:

- clean build time and peak compiler memory;
- executable/library size and relevant sections;
- cold/warm launch;
- steady-state heap bytes, allocation count, and fragmentation proxy;
- retained tree construction and destruction;
- event subscription/invocation;
- dispatcher queue throughput and tail latency;
- layout/damage/frame workloads already admitted by GUI.Forms;
- constrained artifact link size and imported-symbol budget.

No claim of lightweight, portable, or faster is accepted without its named
workload.

