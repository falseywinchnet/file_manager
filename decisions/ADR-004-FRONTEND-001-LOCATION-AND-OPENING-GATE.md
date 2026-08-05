# ADR-004: Frontend 001 location and opening gate

Status: **accepted for frontend location and Design DNA; opening predicate and
fake-Orchestrator product bootstrap superseded by ADR-006**.

Date: 2026-08-05.

Owner approval: the grand architect directed that frontend work, gates, and
visual design records move into `frontend/`; selected Design DNA 006 as the
design constitution for the first slice; and authorized Frontend 001 to begin
as soon as GUI.Forms gives its consumption go-ahead and the architect directs
the frontend to proceed.

Supersession note: ADR-006 later requires a live Orchestrator Core 1.0 before
frontend bootstrap. The `frontend/` location, Design DNA 006 authority, and
GUI.Forms consumption requirement remain accepted.

## Question

Where does the File Manager application work live, and which dependencies must
be ready before its first implementation slice can start?

## Constraints

- **GIVEN:** the application repository is `frontend/`; **File Manager** remains
  the product name.
- **GIVEN:** GUI.Forms is the in-process UI foundation and must expose the
  declared first-slice consumption surface before application code uses it.
- **GIVEN:** the architect may open Frontend 001 immediately after GUI.Forms
  gives that go-ahead.
- **GIVEN:** Engine, Orchestrator, plugin, and semantic-fact implementations are
  not prerequisites for Frontend 001. Deterministic frontend-owned fixtures
  stand in for those service edges.
- **GIVEN:** real Engine and Orchestrator adapters remain subject to their own
  Orchestrator negotiation and conformance gates.
- **GIVEN:** Kolmogrov continues independently and yields its admitted results
  to Engine when ready. That transfer is not a Frontend 001 start condition.
- **GIVEN:** Design DNA 006 is normative for Frontend 001. Its governing
  principles and entries already marked GIVEN or DECIDED bind the slice;
  entries still marked CANDIDATE or HYPOTHESIS remain open and must not be
  silently promoted.
- **DECIDED:** Orchestrator remains headless. File Manager eventually renders
  its settings and service controls.

## Candidates

### A. Keep `file_manager/` and Gate F0

Wait for GUI.Forms, Engine, and Orchestrator first-slice contracts before any
application implementation. This protects every integration edge but preserves
the circular wait and prevents fixture-backed composition work.

### B. Move to `frontend/` but remove all gates

Start against whatever GUI and service APIs currently exist. This creates
private reach-through and turns temporary implementation shapes into ABI.

### C. Move to `frontend/` and split the gates by slice

Require GUI.Forms readiness for the visual/application slice, use deterministic
ports and fixtures for services, and open each real adapter only after its own
contract negotiation.

## Measurements and evidence

This is an ownership and sequencing decision, not a performance selection; no
runtime benchmark is claimed. The accepted evidence is the architect's explicit
direction, the existing independently buildable GUI.Forms boundary, the
fixture-capable Frontend F1 plan, and ADR-003's already accepted per-interface
negotiation model. Frontend 001 must establish its own measurements at exit.

## Decision

Choose C.

`frontend/` owns application planning, design evidence, gates, source, tests,
and later packaging. Frontend 001 is a deterministic application shell over the
named GUI.Forms consumption snapshot and frontend-owned fake ports. The old
global Gate F0 is superseded.

The opening transition has two events:

1. GUI.Forms records that its Frontend 001 consumption surface is ready.
2. The grand architect explicitly directs frontend implementation to begin.

Neither event alone is inferred from passing tests or document edits. Engine,
Orchestrator, plugin, semantic-fact, and final Kolmogrov readiness do not enter
this opening predicate.

Real integration advances independently:

- Kolmogrov transfers admitted results to Engine when its evidence gate passes;
- Engine and Orchestrator negotiate and implement their adapter;
- Frontend replaces a fake port only when that exact client edge has a recorded
  compatible snapshot.

## Rejected options

- Reject A because it makes unrelated Engine and Orchestrator readiness veto a
  GUI/application composition slice and recreates the circular global gate.
- Reject B because private reach-through would turn temporary producer details
  into application dependencies without conformance evidence.

## Frontend 001 boundary

Frontend 001 proves application composition, house rendering, interaction
state, deterministic traces, and explicit service-unavailable presentation. It
does not prove real filesystem mutation, indexed search quality, Orchestrator
settings mutation, plugin execution, semantic facts, packaging, or daily
replacement readiness. Those remain later slices.

## Failure modes

- Treating a fake service response as a stable Engine or Orchestrator contract.
- Reaching into GUI.Forms private headers because a required primitive is
  absent from its go-ahead snapshot.
- Calling Frontend 001 “integrated” when its ports are fixture-backed.
- Promoting unresolved DNA candidates into permanent tokens or behaviors.
- Allowing Engine or Orchestrator delay to re-close an already opened Frontend
  001 application-composition slice.

## Consequences

- Frontend work has one obvious repository and instruction boundary.
- GUI.Forms can be exercised by a real consumer earlier.
- Engine and Orchestrator keep control of their real contracts without blocking
  visual/application composition.
- The frontend must maintain explicit port/fixture boundaries and label
  simulated capability state.
- Historical ADR-002 and Gate F0 language remains evidence of the earlier plan,
  but cannot govern current implementation permission.

## Reversal path

The directory may later become an independent repository without changing its
product role. Fixture ports may be replaced behind frontend-owned interfaces.
If the GUI.Forms snapshot cannot support a coherent 001 slice, the architect
may narrow the slice or request a GUI.Forms follow-up; this does not authorize a
private implementation reach-through.
