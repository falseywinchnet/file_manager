# C++20 feature audit

Status: **OBSERVED and selected on the rewritten first-party source; C++20
compiler mode retained without admitting the whole C++20 language surface**.

This audit asks which facilities added in C++20 are actually used. It does not
treat `-std=c++20` as an architectural decision to consume every new feature.
The semantic ledger is authoritative for the syntax categories it recognizes;
the library additions below are supplemented by a source inventory.

## Retained features

| Feature | Observed use | Decision |
|---|---|---|
| `std::span` | 427 deduplicated semantic source locations in the MinGW first-party closure ledger | Retain. It states contiguous non-owning range access and is part of existing public and private APIs. |
| `consteval` | Two gallery DML declarations in `demo/gallery_dml.hpp` | Retain. The grammar is explicitly chosen and invalid descriptions are rejected at compilation; inference does not select architecture. |
| designated initialization | Seven locations: two popup option records, four text-store limit records, and one text-store test record | Retain only for these plain configurations with independent fields and valid defaults. Stateful image/raster results now use named success/failure factories. |
| `std::has_single_bit` | Four flag/enum validation predicates | Retain. It is a direct, named numeric predicate; no representation conversion is hidden. |
| string/string-view `starts_with` and `ends_with` | 42 first-party standard string/view call sites outside the checker, across parsing, stable-ID prefixes/suffixes, automation commands, and assertions | Retain. Each is a direct predicate with no lazy range or lifetime machinery. The checker's 11 similarly spelled calls are LLVM `StringRef`, not C++20 standard-library use. |
| associative-container `contains` | Membership tests on existing maps and sets in registries, layout metadata, retained-state tracking, image caches, host identity sets, and tests | Retain. The operation exposes intent more directly than `find(...) != end()` and does not alter allocation or iteration behavior. Geometry types' own `contains` methods are unrelated to C++20. |
| `std::erase_if` | Three sites: scaled-layout stale slots, live-surface wake pruning, and one instrument test | Retain. The predicate type is named and the operation is the standard erase/remove combination stated directly. |
| explicit three-way comparison | Eleven explicitly authored `operator<=>` declarations for identity/value records and ordered date/presentation keys | Retain where ordering is consumed. No comparison is compiler-defaulted; multi-field order remains written in source. |

`if constexpr` is admitted named template machinery, but it was added in C++17
and is not evidence of C++20 feature adoption. Likewise `std::any`,
`std::optional`, `std::variant`, and most of the retained STL surface predate
C++20 and are governed by their own decisions.

## Audited absent or removed features

The closure inventories contain no first-party ranges/views, concepts or
requires-expressions, coroutines, structured bindings, defaulted comparison
machinery, `char8_t` program types, formatting library, source-location
machinery, `jthread`/stop-token machinery, barriers/latches/semaphores,
`atomic_ref`, C++20 synchronization streams, or C++20 bit-representation
operations. The one former requires-expression is now an explicit specialized
control-initialization trait.

Lambdas, `auto`, trailing-return syntax, ordinary `decltype(expression)`, and
pointer-member arrow spelling are house-policy questions rather than C++20-only
questions. They are nevertheless banned by the semantic closure gate.

## Toolchain framing

GUI.Forms remains `CMAKE_CXX_STANDARD 20` and may use current/newer compilers.
Moving to a newer toolchain does not implicitly admit C++23 or later features.
A new feature needs a concrete call site and the same epistemic-cost review:
types, control flow, lifetime, allocation, and actual execution must remain no
harder to know than the problem warrants.
