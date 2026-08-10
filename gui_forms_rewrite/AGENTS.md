# GUI.Forms rewrite planning guardrails

This directory is an interview-first planning and handoff package for a future
in-place rewrite of `../gui_forms/`. It does not authorize that rewrite.

## Absolute boundary

- Do not edit `../gui_forms/` while working from this directory.
- Do not run formatting, code generation, dependency updates, or mechanical
  rewrites against `../gui_forms/`.
- Read the parent `AGENTS.md`, `../gui_forms/AGENTS.md`, and the parent planning
  and decision protocol before proposing implementation.
- Existing code, a research source, or this draft does not silently create an
  architecture decision.
- Only entries marked `DECIDED` in `DECISION_LEDGER.md`, with owner approval,
  may become rewrite requirements.
- Preserve the current dirty working tree. The rewrite must be based on a named
  source snapshot after the architect opens implementation.

## Purpose

The grand architect wants a selective BFFT-house-style rewrite of GUI.Forms:
explicit types, owned heap arrays in place of `std::vector`, deliberate lambda
policy, reduced and classified `std::function`, and a measured reduction of
hosted C++ and libc assumptions. The long horizon is that a meaningful portable
GUI.Forms nucleus could someday run in a UEFI or bare-metal environment. That
horizon is a design pressure, not a claim that AppKit, Win32, Skia, HarfBuzz, or
the complete control framework can become freestanding unchanged.

## Required working method

1. Keep facts, hypotheses, candidates, and decisions separately labelled.
2. Inventory each dependency before replacing it.
3. Preserve behavior with characterization tests before changing representation.
4. Introduce replacement primitives behind narrow seams and prove them before
   mass migration.
5. Make allocation, failure, ownership, callback lifetime, and thread affinity
   explicit in every new primitive.
6. Retain negative results and measured regressions.
7. Never equate fewer standard-library names with better portability without a
   smaller measured runtime/compiler/ABI dependency surface.

## Implementation gate

Implementation remains **BLOCKED** until all phase-zero decisions in
`DECISION_LEDGER.md` are accepted and the grand architect explicitly directs a
sibling to begin. When that happens, the sibling must first refresh the audit,
record the exact Git state, and produce the phase plan required by
`SIBLING_HANDOFF.md`.

