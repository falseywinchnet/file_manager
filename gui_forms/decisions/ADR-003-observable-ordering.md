# ADR-003: Use one deterministic event order and preserve DML assignment order

Status: accepted

Date: 2026-08-04

Owner approval: grand architect selected pragmatic blocker analysis, one
GUI.Forms order, and preserved DML order.

## GIVEN constraints

- GUI.Forms behavior is deterministic and retained.
- Compatibility does not require preserving every WinForms bug or alternate
  historical order.
- DML is authoritative input to retained construction.

## Workloads and failure modes

Observable ordering includes property changes, synchronous events, focus,
capture, activation, layout invalidation, disposal, and designer-style staged
initialization. Multiple context-dependent orders make traces unstable and
application reasoning difficult. Reordering DML assignments can change event,
layout, resource, or initialization behavior even when final values match.

## Candidates

1. Reproduce multiple consumer/platform orders.
2. Specify one deterministic GUI.Forms order and compare captured retired compatibility specimen paths for
   concrete blockers.
3. Select correctness-first order without inspecting the compatibility corpus.

For DML, either preserve authored property-assignment order or permit compiler
reordering after a claimed independence proof.

## Evidence and measurements

**OBSERVED:** existing lifecycle and host traces already use one synchronous,
deterministic order. **OBSERVED:** Forms designer initialization is an ordered
imperative sequence with batching scopes and observable hooks.

## Decision

**DECIDED:** select event candidate 2. GUI.Forms publishes and tests one order.
Captured retired compatibility specimen paths are examined pragmatically; a concrete blocker may change
the specified order through a reviewed compatibility change, but no second
runtime order is introduced.

**DECIDED:** DML preserves authored initialization assignment order. A compiler
may coalesce downstream rendering/layout work at transaction boundaries, but it
may not reorder authored assignments.

## Why the other candidates lost

Candidate 1 undermines determinism and multiplies the conformance surface.
Candidate 3 can miss a known application blocker. DML reordering confuses final-
state equivalence with observable construction equivalence.

## Consequences

- Every control family receives a normative trace, not a set of selectable
  ordering modes.
- Compatibility deviations name the blocker and the chosen invariant.
- DML generators and round-trip tools retain property order.

## Reversal and migration path

Changing a published order requires a versioned trace update and compatibility
analysis. If future declarative optimization is needed, it may optimize work
scheduled by assignments, not the assignment sequence itself.

## Unresolved edges

- The normative order for control families not yet implemented.
- The threshold for classifying an retired compatibility specimen difference as a blocking incompatibility.
