# Open C++ feature-edge audit

Status: **OBSERVED orientation with O-002 through O-010 decisions applied**.

Snapshot: dirty working tree, 2026-08-10. Refresh before implementation.

## C++20-added feature inventory

This is a source-level orientation count over production `include/` and `src/`.
The future LibTooling checker must refresh and semantically classify it from the
exact start snapshot.

| Feature/surface | Textual observations | Selected treatment |
|---|---:|---|
| `if constexpr` | 126 | keep in named templates/functors; generic lambdas still go |
| requires-expression | 1 | replace with explicit initialization trait |
| named concepts | 0 | no general admission; concrete proposal required |
| designated initialization | about 80 lines in five files | keep plain options; named factories for result states |
| `std::span` | 189 | keep |
| `std::erase_if` | 14 | keep as a clear named erase operation |
| associative/string `starts_with`/`ends_with` spellings | 20 / 1 | keep clear standard string predicates; checker must distinguish receiver types |
| `std::has_single_bit` | 4 | keep |
| spaceship operators | 11 | preserve public signatures explicitly; remove private convenience by use |
| `consteval` / `constinit` | 0 / 0 | `consteval` permitted; no need established for `constinit` |
| coroutines | 0 | banned |
| ranges/views | 0 | concrete proposal required |
| `bit_cast` | 0 | concrete representation case required |
| `format`, `source_location` | 0 | no admission needed for this rewrite |
| `jthread`/stop tokens, barriers/latches/semaphores, `atomic_ref` | 0 | no concurrency import; preserve current machinery |
| `char8_t`/`u8string`, `using enum`, likelihood/no-unique-address attributes | 0 | no admission needed for this rewrite |
| C++20 numeric/utility additions such as `midpoint`, `lerp`, `to_array`, `bind_front`, `remove_cvref`, `type_identity`, `is_constant_evaluated` | 0 | no admission needed for this rewrite |

The zeroes matter: C++20 mode is infrastructure for selected facilities, not a
standing invitation to grow the language surface. Current toolchains may still
be used.

## `decltype(expression)`

Observed: 21 occurrences. They are all `std::decay_t<decltype(value)>`-style
type recovery inside generic lambda visitors over variants/events.

Because all lambdas and generic lambdas are already banned, these concrete uses
will probably disappear when replaced with named overloaded visitors/functors.
They do not presently establish a need to permit `decltype(expression)`.

Decision: permit `decltype(expression)` only inside named private
type-trait/detection plumbing. It remains forbidden for ordinary declarations,
public signatures, return inference, and `decltype(auto)`.

## Concepts and requires-expressions

Observed: no named concepts and one actual requires-expression, in
`make_control`. It detects whether a `ControlType` supplies
`initialize_control_tree()` and conditionally invokes it after
`make_shared`.

The architectural decision is explicit—compound controls may opt into one
post-construction hook—but the detection is implicit structural inference.
The considered replacements were:

1. keep the requires-expression because the architecture already chose the
   optional hook and the compiler only instantiates it;
2. use an explicit trait specialized by compound controls;
3. use a named base/interface marker;
4. split ordinary and compound factories.

Decision: use an explicit specialized trait. This makes opt-in searchable and
reviewable without spending a runtime interface or multiplying factories.

## `if constexpr` and pack folding

Observed: about 130 `if constexpr` occurrences in the surveyed tree, many in
generic-lambda visitors and some in ordinary template implementations.

Decision: permit predictable compile-time branching, pack expansion, and
folding in named templates/functors. Lambda closure machinery remains banned.
This is compiler execution of a stated choice, not architectural inference.

## Ranges

Observed: no `<ranges>`, `std::ranges`, or views usage in current production
source. Decision: do not admit them without a concrete pipeline whose types,
lifetimes, and execution can be reviewed.

## `std::bit_cast` and `<bit>`

Observed: no `std::bit_cast`. `<bit>` is included for four
`std::has_single_bit` uses validating flag values. `has_single_bit` is a clear,
named numeric predicate and does not hide type/lifetime/allocation/control flow.

Decision: keep `std::has_single_bit`; require a real representation-conversion
case before admitting `std::bit_cast`.

## Designated initializers

Observed: 80 lines across five files:

- ErrorProvider and ToolTip `PopupOptions` construction;
- text-shaping validation limits;
- Skia raster error results;
- extensive image-registry error/success results.

They make selected fields explicit, but default all unmentioned fields and can
hide which result state is semantically complete. The image/resource cases also
encode the approved safe content-failure philosophy.

The considered policies were:

1. keep designated initializers because field names improve epistemic clarity;
2. replace with named result factories such as `Failure(error)` and
   `Success(image)` where a sum-state is intended, while retaining them for
   ordinary option records;
3. ban them and require explicit aggregate/object construction everywhere.

Decision: retain designated initializers for plain option/configuration records.
Use named success/failure factories for result-state alternatives.

## Comparison machinery

Observed: 11 defaulted spaceship operators and 20 defaulted equality operators.
The approved rule bans convenience-defaulted comparison machinery and requires
only comparisons actually used.

Decision: preserve existing public comparison signatures with explicit
implementations. For source-private types, inventory real use and implement only
the required comparisons. Do not replace every spaceship with six operators.

## C++20 mode and toolchains

Decision: remain in C++20 language mode while allowing current compiler
toolchains. Audit actual C++20-added feature use under the house framing; the
version mode does not admit every feature added by C++20, C++23, or later.
`consteval` is permitted.
