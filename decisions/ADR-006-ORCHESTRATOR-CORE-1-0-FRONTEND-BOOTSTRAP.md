# ADR-006: Orchestrator Core 1.0 precedes frontend bootstrap

Status: **accepted**.

Date: 2026-08-05.

Owner approval: the grand architect clarified that GUI.Forms gates only the
visual application, directed Orchestrator to advance independently toward 1.0
availability, and required that release before the frontend bootstraps on
Orchestrator.

## Question

May the visual File Manager frontend bootstrap against an Orchestrator fixture,
or must a live, versioned Orchestrator core be available first?

## GIVEN constraints

- Orchestrator is a headless core system and has no dependency on GUI.Forms.
- The visual C++ File Manager application lives in `frontend/` and consumes
  both GUI.Forms and Orchestrator.
- Orchestrator must advance to a named Core 1.0 release before the frontend
  bootstraps on it.
- GUI.Forms and Orchestrator Core 1.0 advance independently; both are frontend
  bootstrap prerequisites.
- Engine, Kolmogrov, plugins, and semantic facts retain their separate gates.
  Core 1.0 may truthfully report those capabilities unavailable, deferred, or
  stubbed.
- A frontend test double may replay Core 1.0 conformance fixtures, but the
  product bootstrap may not replace the live core with a frontend-owned schema.
- The independent contract-version namespaces in the conformance constitution
  remain authoritative. “Core 1.0” is a release/readiness label, not one global
  API version.

## Workloads and failure modes

The release boundary must cover cold start, already-running discovery,
incompatible client, restart, shutdown, unavailable providers, explicit stubs,
bounded requests, malformed frames, and deterministic structured inspection.
It must not depend on a window system, claim unavailable providers work, or
require the frontend itself in order to determine release readiness.

## Candidates

### A. Bootstrap the frontend against its own Orchestrator fixture

This lets visual composition start earlier but gives the consumer authority
over a core contract and permits the real daemon to diverge from the bootstrap
assumptions.

### B. Wait for every final provider before releasing Orchestrator

This makes Core 1.0 comprehensive but reintroduces the global musical-chairs
gate and blocks stable lifecycle/availability work on unrelated research.

### C. Release a provider-honest Orchestrator Core 1.0 first

Stabilize discovery, session/lifecycle, contract negotiation, availability,
structured CLI, and the frontend bootstrap projection. Report unfinished
provider families explicitly and evolve them behind their own contracts.

## Evidence and measurements

This sequencing decision is owner-directed rather than a performance result.
The current Rust bootstrap already demonstrates deterministic lifecycle,
availability enumeration, structured CLI, explicit stubs, and golden fixtures.
It does not yet demonstrate daemon discovery/authentication, a production local
transport, an independent frontend-protocol client, or a stable compatibility
horizon; those are measured Core 1.0 exit work, not assumed facts.

## Decision

Choose C.

Orchestrator development is not gated by GUI.Forms. The Core 1.0 release target
requires:

1. a user-scoped lazy daemon with documented discovery and authentication on
   the first supported platform;
2. stable `ORC-COM-001`, `ORC-LIF-001`, `ORC-FE-001`, and `ORC-CLI-001`
   compatibility horizons;
3. a production local client transport and an independently executable
   conformance client suitable for the C++ frontend;
4. immutable contract and availability snapshots with explicit provider state;
5. bounded framing, cancellation, deadline, version-mismatch, restart, and
   shutdown behavior;
6. human and structured CLI projections over the same operations;
7. a signed or digest-addressed release manifest reporting whether the Core 1.0
   bootstrap profile is ready;
8. passing unit, hostile-fixture, cross-version, and independent-client suites.

Settings, handlers, commands, Engine integration, plugin execution, and
semantic facts join Core 1.0 only if their contracts independently satisfy the
release gate. Their absence does not delay the core when the availability map
reports it honestly.

Frontend 001 opens only after all three events are recorded:

1. Orchestrator Core 1.0 is available through its real bootstrap client edge;
2. GUI.Forms gives its named FM0/Frontend 001 consumption go-ahead;
3. the grand architect explicitly directs frontend implementation to begin.

## Why the other candidates lost

- Reject A because frontend-owned fixtures cannot be the authority for the
  core system on which the application bootstraps.
- Reject B because stable core availability does not require final search,
  plugin, or semantic implementations.

## Consequences

- Orchestrator now has a release target rather than an indefinitely provisional
  kernel.
- The frontend consumes a live authority from its first product bootstrap while
  retaining fixture replay for deterministic tests.
- GUI.Forms work cannot block headless Orchestrator implementation.
- Core 1.0 can be available with visibly unavailable optional or separately
  gated providers.
- ADR-004 remains authoritative for the `frontend/` location and Design DNA 006,
  but its GUI.Forms-only opening predicate and fake-Orchestrator product
  bootstrap are superseded.

## Reversal and migration path

Individual Core 1.0 transports and bindings can be replaced behind stable
semantic contracts and compatibility fixtures. A later ADR may expand or split
the release profile, but it must preserve truthful provider availability and a
frontend bootstrap path that does not depend on frontend-owned core semantics.

## Unresolved edges

- First production local transport and platform discovery mechanism.
- Exact authentication material and user-session binding per OS.
- Compatibility support duration after Core 1.0.
- Release-manifest signing versus local digest-only provenance.
- Whether settings, handlers, and commands reach the first Core 1.0 profile or
  a later independently versioned capability release.
