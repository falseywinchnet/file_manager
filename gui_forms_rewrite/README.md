# GUI.Forms in-place rewrite program

Status: **architect interview and rewrite design; implementation forbidden**.

This directory holds instructions for a future sibling agent to rewrite the
existing `../gui_forms/` implementation in place. It is deliberately outside
the implementation subtree so planning work does not masquerade as a local
GUI.Forms directive before the decisions are accepted.

## Objective

Rewrite GUI.Forms toward a deliberately selected BFFT house style while
preserving its retained behavior, public C contract, platform boundaries, and
measured conformance evidence.

Existing broader BFFT sources are not normative merely because they exist.
Their uses of `auto`, capturing closures, and `std::vector` are recorded by the
architect as unfinished cleanup debt, not permission for GUI.Forms.

The intended direction is:

- explicit named types rather than `auto`;
- explicit allocation, ownership, capacity, and failure;
- GUI.Forms-owned heap-array and range primitives instead of `std::vector`;
- lambdas classified by capture, escape, allocation, and lifetime rather than
  accepted or rejected as one undifferentiated feature;
- `std::function` reduced to approved hosted seams, with important callback
  roles moved to purpose-built in-house representations;
- a complete audit of standard-library and libc heavy lifting, including
  algorithms, text conversion, allocation, exceptions, threading, clocks,
  filesystem access, and dynamic loading;
- a portable nucleus whose contracts do not preclude a future UEFI/bare-metal
  host and renderer.

This is not a promise to remove every standard-library facility, target a
freestanding implementation immediately, or port Skia/AppKit/Win32 to firmware.
Those would be separate decisions.

## Authority

The grand architect's statements recorded in `GIVENS.md` are authoritative.
All unresolved language, runtime, ABI, and migration choices live in
`DECISION_LEDGER.md`. No candidate becomes policy until discussed and marked
`DECIDED` with owner approval.

The active GUI.Forms lifecycle decision and public/cross-project contracts must
survive the rewrite unless separately amended through their existing decision
and negotiation processes. Representation changes do not acquire permission to
change event order, retained identity, host lifecycle, accessibility behavior,
or ABI semantics.

## Documents

- `GIVENS.md` — architect requirements and observed facts.
- `DECISION_LEDGER.md` — questions that must be resolved together.
- `AUDIT_AND_MEASUREMENTS.md` — repeatable dependency inventory and baselines.
- `MIGRATION_SKELETON.md` — phase structure without prematurely selected
  implementation details.
- `SIBLING_HANDOFF.md` — exact instructions for the future implementing agent.

## Current gate

The next activity is discussion, not code. Begin with decision cluster A in the
ledger: portability target, rewrite boundary, language level, and `auto` policy.
