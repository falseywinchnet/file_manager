# Orchestrator and File Manager delivery sequence

Status: **DECIDED negotiated bootstrap and frontend opening order under ADR-003
and ADR-004**.

## B0 — integration kernel (open)

Orchestrator may implement provider-independent common types, lifecycle,
availability, contract enumeration, CLI/structured output, stdio fixtures, fake
peers, cancellation, quotas, and explicit stubs.

Exit:

- deterministic Rust tests and JSON fixtures pass;
- required and available capabilities are distinct;
- absent engine/plugin/fact services cannot be mistaken for empty success;
- every proposed provider edge has a project-local negotiation note.

## N1 — per-interface negotiation

GUI.Forms, engine, and Kolmogrov work in parallel with the bootstrap.
Orchestrator proposes an edge in the affected project's note; that project
accepts or counterproposes; Orchestrator reconciles the canonical contract and
fixtures. File Manager's real Orchestrator client reply is deferred until that
adapter follows Frontend 001.

An individual adapter may begin when its edge has:

- named provider and consumers;
- identities, versions, lifecycle, errors, bounds, cancellation, and authority;
- minimal and hostile fixture drafts;
- a recorded provider/consumer reply;
- no unresolved call edge required for that adapter's first slice.

There is no longer one global O0 prohibition on all Rust implementation.

## I1 — engine integration

Implement user-scoped discovery/authentication of the systemwide engine,
availability monitoring, exact query/status routing, normal Orchestrator query
broker behavior, and registered degraded direct-engine fallback.

Exit: normal broker and fallback return semantically equivalent core results on
the same fixture, and provider absence is explicit.

## C1 — core user services

Implement immutable registry snapshots, settings transactions, audit, handler
and declarative-command registries, and structured CLI operations over the same
semantic API. Orchestrator remains headless; File Manager owns all UI.

Exit: CLI and fake File Manager clients perform the same settings/handler/
command operations and receive the same failures.

## P1 — plugin and semantic placeholders remain closed

Plugin lifecycle, plugin AI, provider deposits, and semantic facts remain stubs
until their own contracts are opened. Storage experiments may establish
controls, but no guessed fact model becomes an API.

## P2 — reef and platform realization

After explicit plugin decisions, integrate OS-sandboxed workers, package/grant
lifecycle, provider generations, first-party platform adapters, quotas,
quarantine, and recovery. Semantic facts advance on their separate architect
design.

Exit: hostile suites pass on named macOS, Windows, and Linux versions; no
third-party code or ambient authority enters a trusted process.

## Frontend 001 opening

The old global Gate F0 is superseded by ADR-004. Frontend 001 begins after:

1. GUI.Forms gives a named go-ahead for the Frontend 001 consumption surface;
2. the grand architect explicitly directs frontend implementation to begin.

Frontend 001 uses deterministic frontend-owned ports for Engine and
Orchestrator states. It does not wait for Engine, Orchestrator adapters, plugin
AI, semantic facts, federation, or Kolmogrov transfer. Replacing a fake port
with a real adapter remains gated by that individual negotiation and fixture
snapshot.
