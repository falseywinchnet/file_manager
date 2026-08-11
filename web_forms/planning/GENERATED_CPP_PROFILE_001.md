# Generated C++ profile 001

Date: 2026-08-10

Status: **GIVEN restrictions plus DECIDED compiler boundary under ADR-003**.

## Product baseline

Generated output is standard C++17-compatible source and must continue to
compile when the consuming GUI.Forms build selects a later standard. It uses no
compiler extension and brings no Python runtime into the product.

## Required generated idiom

- spell named types explicitly; do not emit `auto` or `decltype(auto)`;
- use explicit constructors/conversions and reject narrowing;
- emit named free/member functions or explicit listener objects, not lambdas,
  captures, closures, or capture-backed `std::function` values;
- do not emit `std::vector`;
- use `constexpr std::array<T, N>` for fixed generated tables;
- use a small explicit pointer-plus-count/array-view type for borrowed ranges;
- use a named owned-buffer/container type only where runtime-sized ownership is
  genuinely required;
- expose generated IDs as typed members, never mandatory runtime string lookup;
- keep initialization and handler connection order explicit and deterministic.

The restrictions apply to generated product source and generation-facing
support seams. They do not authorize a wholesale rewrite of existing GUI.Forms
or claim that BFFT already follows every rule.

## Still open

Exception policy, RTTI policy, template limits, allocation arenas, generated
namespace spelling, file partitioning, and the exact listener/token types need
separate decisions before output is frozen.
