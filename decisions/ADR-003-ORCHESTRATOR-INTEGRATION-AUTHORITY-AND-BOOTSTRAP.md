# ADR-003: Orchestrator integration authority and negotiated bootstrap

Status: **accepted**.

Date: 2026-08-05.

Owner approval: the grand architect renamed The Oracle to **Orchestrator**,
confirmed that Orchestrator is the program integration authority, authorized a
core Rust kernel before every producer interface is finished, and directed
project-to-project ABI negotiation by written proposal and reply.

## Question

How can Orchestrator begin as the authoritative integration core when the
engine, GUI.Forms, Kolmogrov, and frontend each need Orchestrator requirements
in order to finish their own interfaces?

## GIVEN constraints

- The component and executable are named **Orchestrator**. “Oracle” is no longer
  the product/component name; the lowercase term remains valid for correctness
  and measurement oracles.
- Orchestrator is the integration authority. It decides which capabilities the
  product requires, which provider supplies them, and which capabilities are
  currently available.
- File Manager consumes Orchestrator as its normal integration and policy
  surface. GUI.Forms remains the frontend's in-process rendering/control
  dependency rather than an Orchestrator GUI.
- The engine is a systemwide service. Orchestrator is user-scoped and lazy.
- A registered direct-engine path may provide degraded fallback when
  Orchestrator is unavailable.
- Producer and consumer interfaces are negotiated through project-local notes:
  Orchestrator proposes schemas, calls, versions, failure behavior, and ABI/wire
  needs; the affected project records a reply; Orchestrator reconciles the next
  round into the canonical registry.
- Orchestrator may begin a competent contract/lifecycle/availability/CLI kernel
  before every provider snapshot is complete. It may not invent a provider's
  private implementation or claim an unimplemented capability is available.
- The initial public API is the local CLI. A human and a local AI or developer
  agent invoking that CLI have the same authority of the local user.
- A future plugin-AI API is part of the plugin capability program and is not the
  API used by project agents or ordinary local developer agents.
- Plugin and semantic-fact APIs are stubs in the bootstrap. Semantic-fact
  authority remains deferred until the architect's design exists.
- Orchestrator has no GUI. File Manager renders settings, service state, grants,
  and Orchestrator-specific controls using GUI.Forms.

## Workloads and failure modes

The bootstrap must survive absent providers, incompatible versions, partial
implementation, repeated proposal rounds, stale availability, daemon restart,
and structured CLI use. It must not freeze an accidental Go/Rust/C++ layout,
misreport a stub as functional, let an unfinished provider block unrelated core
work, or make File Manager infer integration policy from private APIs.

## Candidates

### A. Retain the paper-only dependency gate

Wait for named GUI.Forms and engine snapshots before writing Rust. This avoids
early adapter churn but preserves the circular dependency: producers cannot
finish what Orchestrator has not yet asked them to provide.

### B. Let each producer publish an interface and integrate afterward

This allows local speed but returns integration authority to producer accidents
and makes the frontend reconcile incompatible assumptions.

### C. Bootstrap the integration kernel and negotiate every edge in writing

Build only provider-independent Orchestrator competence first. Record proposed
cross-project calls in the affected project's notes, accept explicit replies,
and reconcile the result into transport-neutral contracts and fixtures before
freezing adapters.

## Decision

Choose C.

### Authority and routing

Orchestrator owns the program capability catalogue and the observed
availability state. Normal File Manager integration traverses Orchestrator for
policy, settings, handlers, commands, provider routing, and combined search.
GUI.Forms remains direct and in-process. A direct registered engine query path
is retained as an explicit degraded fallback, not a second integration
authority.

### Negotiation loop

Each active subproject receives an `ORCHESTRATOR_INTERFACE_NEGOTIATION.md` note
in its own documentation tree. Every round contains:

1. the Orchestrator requirement and contract IDs;
2. proposed operations, types, transport/ABI, bounds, cancellation, errors,
   lifecycle, and fixtures;
3. a provider/consumer reply that accepts, rejects, or counterproposes each
   point with evidence;
4. an Orchestrator reconciliation and next-round disposition;
5. a link to canonical semantics once agreed.

Project-local notes are the dialogue record. Accepted semantics move into
`orchestrator/spec/`; the notes do not become a second source of truth.

### Bootstrap implementation

Implementation opens immediately for a provider-independent Rust kernel:

- common contract IDs, versions, terminal status, and envelopes;
- deterministic lifecycle and shutdown;
- capability/availability catalogue;
- canonical registry projection;
- human and structured CLI surfaces;
- stdio contract laboratory and deterministic fixtures;
- explicit plugin and semantic-fact stubs.

Real engine adapters, durable hives, settings mutation, handlers, plugin
workers, platform integration, and frontend bindings remain gated by their own
negotiated contract and fixture readiness rather than one global O0 barrier.

### Storage direction

Orchestrator may evaluate SQLite for distinct low-volume transactional stores
such as settings, operational registries, and audit. Bulk provider generations
remain a separate workload and corruption boundary. No one universal database
is selected by this ADR.

Microsoft Registry hives are admitted only as a structural comparison:
independent logical hives have backing files, transaction logs, and backups.
Their private binary format is not an Orchestrator candidate or compatibility
target.

## Why the other candidates lost

- A global producer-first gate creates the circular wait now observed.
- Producer-owned interfaces turn implementation accidents into program ABI.
- A speculative monolithic Orchestrator would claim unavailable capabilities
  and duplicate provider implementations.
- A GUI inside Orchestrator would violate frontend ownership and create a
  second visual application.

## Consequences

- The old Gate O0 becomes a per-interface readiness check, not a prohibition on
  the core kernel.
- Orchestrator can enumerate required, available, degraded, stubbed, and
  unavailable capabilities before their providers are finished.
- Subprojects receive concrete interface questions early and retain their
  counterproposals beside their own implementation evidence.
- File Manager gets one normal integration authority plus declared degraded
  fallbacks.
- Plugin AI and semantic facts cannot leak into the initial CLI by implication.

## Reversal and migration path

Provider adapters and process routing can change behind registered contracts.
If the normal query broker violates a measured latency or availability budget,
the direct engine projection may become the normal hot path through a later ADR
while Orchestrator retains contract and availability authority. Project-local
negotiation notes and fixtures remain useful under either routing.

## Unresolved edges

- Final local daemon discovery/authentication on each platform.
- Engine systemwide installation, user authorization, and multi-user root
  policy boundaries.
- Stable IDL and production local framing beyond the JSON fixture laboratory.
- Settings/registry/audit physical-store measurements and recovery policy.
- Numeric Orchestrator budgets and idle-exit policy.
- Plugin-AI, general plugin, and semantic-fact semantics.
