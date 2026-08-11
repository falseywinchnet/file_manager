# ADR-002: compiler language and generated C++ profile

Date: 2026-08-10

Status: **SUPERSEDED by ADR-003 on 2026-08-10**.

## Question

Which permitted language implements the build-time compiler, and what language
profile may appear in product output?

## Constraints

- Compiler choices are Rust, Go, C++, or Python.
- The product must be C++17 or later only.
- Generated product code follows an orthodox profile: explicit types and
  ownership, with no implicit typing, `std::vector`, lambdas, captures, or
  closure-like handler machinery.
- No compiler implementation language may become an application runtime.
- Output and diagnostics must be deterministic and fixture-testable.

## Candidates

- Rust: typed parser/IR, explicit error handling, memory-safe untrusted-input
  processing, and a self-contained build tool.
- C++: ecosystem alignment, but greater parser memory-safety risk and tighter
  coupling between compiler internals and generated ABI.
- Go: viable typed tooling, but offers less benefit here than Rust's enums and
  ownership model and would conflate this GUI tool with the Go Engine domain.
- Python: excellent for experiments, but weaker as the reproducible packaged
  compiler and easier to let dynamic schema errors escape until runtime.

## Evidence

**OBSERVED:** BFFT demonstrates useful explicit low-level C++ buffers,
workspaces, and stable C boundaries, but its public convenience APIs also use
`std::vector`. It is evidence for selected idioms, not proof of one repository-
wide normative grammar.

**GIVEN:** the architect's stated restrictions are authoritative for generated
Web.Forms product code whether or not existing GUI.Forms or BFFT code uses a
broader C++ dialect.

## Decision

Use Rust for the build-time validator/compiler. It emits deterministic,
C++17-compatible source conforming to `GENERATED_CPP_PROFILE_001.md`. No Rust
library, ABI, allocator, panic, or runtime crosses into the application.

Existing GUI.Forms may remain C++20. Generated code is a C++17 subset that can
be compiled by the GUI.Forms C++20 build. GUI.Forms must provide generator-
friendly pointer/count, array-view, named listener, or equivalent public seams
where its convenience APIs would otherwise force forbidden syntax or types.

## Failure modes

- A generated file uses a forbidden construct or depends on compiler-private
  Rust representation.
- Reordering source without a semantic change creates nondeterministic output.
- A GUI.Forms public seam forces `std::vector`, lambda capture, or string lookup
  for stable generated identity.
- The compiler becomes necessary after the native binary has been built.

Each is a conformance failure.

## Reversal path

The language front end targets a versioned, serialized construction IR and
golden fixture corpus. Another permitted implementation language may replace
Rust without changing source or application ABI. Changing the generated C++
profile requires a new ADR and regenerated conformance corpus.

## Supersession note

The grand architect subsequently selected Python explicitly and opened the
two-stage implementation/dogfood experiment. ADR-003 retains the orthodox C++
output constraints while replacing only the compiler language and experiment
sequence.
