# GUI.Forms exact-behavior rewrite program

Status: **architect decisions reconciled; implementation forbidden until an
explicit sibling start direction**.

This directory holds the governing instructions for a future sibling to rewrite
the existing `../gui_forms/` first-party C++ in place. It is planning material,
not permission to begin.

## Mission

Rewrite production `include/` and `src/` first, then repair supporting
first-party source where required, while preserving:

- files and directory organization;
- behavior and dependent APIs, with only germane minor native C++ API changes;
- C ABI ownership and semantics;
- retained identities and lifecycle order;
- control behavior, rendering behavior, host behavior, and dependent-library
  boundaries;
- tests, fixtures, and measured evidence.

This is not an STL purge, portability project, private-standard-library project,
allocator rewrite, API redesign, or UEFI deliverable.

## Governing criterion: epistemic cost

Remove constructs that make type, control flow, lifetime, allocation, or actual
execution harder to know than the problem warrants.

Compiler machinery is welcome when architecture has already made the decision
and the compiler merely instantiates explicit machinery. It is undesirable when
inference itself makes the architectural choice.

Consequently:

- ordinary useful STL facilities remain;
- templates and `consteval` remain;
- `auto`, trailing-return syntax, lambdas, coroutines, structured bindings,
  generic lambdas, general `std::any`, and convenience-defaulted comparison
  machinery are banned in scope; the existing WinForms-compatible Tag surface
  is the only `std::any` exception;
- `std::vector`, `std::unique_ptr`, `std::span`, `std::optional`,
  `std::variant`, necessary RTTI, and the approved standard algorithms remain;
- `std::function` remains generally allowed, with obvious waste audited;
- events receive a named, zero-bind-allocation Delegate/Event design;
- sorting and stable sorting receive a call-site workload audit and measured
  small house alternatives where justified;
- binary search is small enough to house-own;
- special heap arrays, stable pools, allocators, and concurrency machinery enter
  only for concrete workloads.

## Documents

- `DECISION_LEDGER.md` — the architect's authoritative response, including
  decided, open, and audit items.
- `GIVENS.md` — compact operational policy extracted from the response.
- `STYLE_REFERENCES.md` — pinned BFFT/Cleanup source evidence and its bounded
  interpretation.
- `SORT_AND_SEARCH_AUDIT.md` — current sorting and binary-search call-site map.
- `OPEN_ITEMS.md` — resolved option catalogue retained as decision/reversal
  evidence.
- `AUDIT_AND_MEASUREMENTS.md` — repeatable rewrite audit method.
- `MIGRATION_SKELETON.md` — candidate, behavior-preserving implementation order.
- `SIBLING_HANDOFF.md` — instructions for the future implementing sibling.
- `ARCHITECT_SELECTIONS.md` — authoritative closure of every option in
  `OPEN_ITEMS.md`.
- `FEATURE_EDGE_AUDIT.md` — observed C++20 feature edges and their selected
  treatment.
- `OWNERSHIP_AUDIT.md` — generated ownership/lifetime evidence banked for a
  future lifecycle round.
- `tools/trace_ownership.py` — reproducible ownership inventory generator.
- `ENFORCEMENT_SPEC.md` — selected LibTooling, textual-control, and warning
  ratchet design.
- `IMPLEMENTATION_START_CHECKLIST.md` — exact preflight and first-batch handoff
  for the implementing sibling.

## Current gate

The option interview is closed. Implementation remains blocked until the exact
starting snapshot and overlap are recorded, semantic inventories/oracles are
refreshed, the first batch is reviewed, and the architect explicitly directs
the implementing sibling to begin.
