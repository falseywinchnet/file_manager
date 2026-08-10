# Rewrite givens and observed facts

Date: 2026-08-10

## GIVEN — direct architect requirements

### G-001 — plan now; do not rewrite now

Prepare a massive in-place rewrite plan and leave executable instructions for a
future sibling. The current task must not change GUI.Forms implementation.

### G-002 — selective BFFT house style

GUI.Forms will use a house style derived from selected BFFT values. The project
will hand-select which constraints matter. It will not blindly copy every idiom
present in the BFFT repository.

The broader BFFT subsystems that use `auto`, capturing lambdas, and
`std::vector` do not represent a competing approved style. The architect has
identified them as undesirable cleanup debt that BFFT itself has not yet
removed. They may be studied as migration examples and failure evidence, but
must not be cited to admit the same constructs into rewritten GUI.Forms.

### G-003 — no `auto`

Rewritten GUI.Forms production source does not use `auto`. The decision ledger
must close edge cases such as explicit ordering return types, structured
bindings, iterator spellings, and third-party-facing types; these edge cases do
not silently weaken the requirement.

### G-004 — explicit typing

Important types, ownership, ranges, callback state, and conversions are written
explicitly. “Explicit” must become a reviewable grammar rather than a preference
applied inconsistently.

### G-005 — GUI.Forms-owned heap arrays

GUI.Forms will use its own heap-array/storage vocabulary instead of
`std::vector`. The exact family of fixed-size, growable, small-buffer, borrowed,
and arena-backed types remains to be designed and measured.

### G-006 — lambdas require classification

Lambda policy depends on the kind of lambda. Capture, escape, ownership,
allocation, ABI visibility, thread transfer, and use as a local algorithm
predicate must be discussed separately.

### G-007 — reduce, do not automatically abolish, `std::function`

The rewrite will constrain `std::function`. The project must decide which uses
remain permitted and which callback roles become in-house types.

### G-008 — future UEFI/bare-metal pressure

GUI.Forms should someday be capable of supporting a meaningful UEFI or
bare-metal configuration. The present rewrite is a pragmatic first step:
minimize hosted assumptions and improve portability without pretending the
complete desktop stack is immediately firmware-ready.

### G-009 — audit libc and standard-library heavy lifting

The program must inventory what libc and the C++ standard library do for
GUI.Forms, including search and sorting algorithms. Replacements must be chosen
from evidence, not from name-count reduction.

### G-010 — sibling implementation

Another sibling will perform the rewrite after the decisions and opening gate.
This directory must be sufficient to prevent that sibling from guessing the
architect's intent.

## OBSERVED — current GUI.Forms source snapshot

These are textual inventory results from the dirty working tree on 2026-08-10.
They are orientation, not a frozen baseline:

- production scope inspected: `gui_forms/include` and `gui_forms/src`;
- approximately 80,523 lines across 421 C/C++/Objective-C++ files;
- `auto`: 1,640 textual occurrences in 109 files;
- likely lambda definitions: 753 matching lines;
- `std::vector`: 577 occurrences in 126 files;
- `std::function`: 181 occurrences in 42 files;
- `throw`: present in 109 files;
- `std::shared_ptr`: present in 103 files;
- `std::weak_ptr`: present in 37 files;
- `std::unique_ptr`: present in 36 files;
- `std::string`: present in 195 files;
- `std::optional`: present in 86 files;
- standard algorithms are used substantially, led by `min`, `max`, `clamp`,
  `find`, `find_if`, sorting, erasure, copying, and accumulation.

The most frequent included hosted headers include `utility`, `algorithm`,
`stdexcept`, `vector`, `string`, `memory`, `string_view`, `optional`,
`unordered_set`, `span`, `functional`, `unordered_map`, `array`, `sstream`,
`chrono`, `atomic`, `thread`, `numeric`, and `mutex`.

Current GUI.Forms explicitly requires C++20 with extensions disabled. Its local
guardrails do not currently impose the proposed house-style restrictions.

## OBSERVED — BFFT source

The available source is `/Users/ultimussecundai/bfft`; the older evidence
register path is stale. Its useful central patterns include:

- a stable C ABI using opaque plans/workspaces, pointer-plus-size buffers,
  explicit status results, and caller-visible storage requirements;
- reusable workspaces to avoid hidden hot-loop allocation;
- a thin RAII C++ layer over the C authority;
- C++17 as the baseline, with C++20/C++23 checks;
- warnings-as-errors in its standards checks.

The BFFT repository is not yet a literal no-`auto`, classified-lambda,
no-`std::vector` codebase. Its broader sources and convenience wrapper use all
three. **GIVEN architect interpretation:** those uses are undesirable legacy
deviations that were never cleaned up, not positive evidence for admitting the
constructs. “BFFT house style” means the intended selected discipline visible
most clearly in its stable C seam, explicit caller storage, reusable workspaces,
and bounded core—not the union of every construct currently found in BFFT.

## Not yet decided

The givens do not decide:

- whether the portable nucleus is C++17, C++20, or a smaller language subset;
- whether all controls or only lower layers join the first minimal-runtime
  profile;
- exact lambda categories;
- callback storage layout and inline capacity;
- allocator API and out-of-memory policy;
- replacement policy for strings, associative containers, ownership pointers,
  exceptions, RTTI, threading, clocks, atomics, or algorithms;
- whether picolibc, klibc, EDK II facilities, or a purpose-built shim is the
  first minimal-runtime conformance environment;
- public C++ source compatibility during migration;
- schedule, batch size, or completion date.
