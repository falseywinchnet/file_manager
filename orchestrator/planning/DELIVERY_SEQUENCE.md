# Orchestrator and File Manager delivery sequence

Status: **DECIDED Core 1.0 and frontend opening order under ADR-003, ADR-004,
ADR-006, ADR-007, and ADR-008**.

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

GUI.Forms, engine, and Kolmogrov work in parallel with Core 1.0 implementation.
Orchestrator proposes an edge in the affected project's note; that project
accepts or counterproposes; Orchestrator reconciles the canonical contract and
fixtures. File Manager's Core 1.0 bootstrap direction is accepted in negotiation
round 002; production transport and authentication fixtures remain open.

An individual adapter may begin when its edge has:

- named provider and consumers;
- identities, versions, lifecycle, errors, bounds, cancellation, and authority;
- minimal and hostile fixture drafts;
- a recorded provider/consumer reply;
- no unresolved call edge required for that adapter's first slice.

There is no longer one global O0 prohibition on all Rust implementation.

## I1 — engine integration (active)

Implement user-scoped discovery/authentication of the systemwide engine,
availability monitoring, exact query/status routing, normal Orchestrator query
broker behavior, and registered degraded direct-engine fallback.

**OPEN under ADR-007:** `ORC-ENG-001` and `ORC-ENG-002` semantic v0 are frozen
for experimental implementation. I1 adapter, routing, and fake-provider work
does not wait for native background-currentness completion, durable observation
watermarks, status subscriptions, lexical/fuzzy channels, or platform service
packaging. Those features advance through capability state. Production
deployment still requires its platform discovery/authentication adapter.

The first slice may route checked cached exact results and may explicitly invoke
manual reconciliation through an authorized admin port. Neither mode is a true
zero-catalogue live filesystem query. ADR-008 requires that distinct Engine
provision under `ORC-ENG-004`; Orchestrator may prepare its fake-provider and
routing policy while the Engine implementation and evidence advance.

**OBSERVED development increment:** the bounded Rust JSONL peer and typed exact
query adapter pass against a separately built Go Engine process for version,
status, canonical root plan/apply, reconciliation, exact query, and shutdown.
This admits development integration and does not satisfy installed discovery,
peer authentication, or production framing.

Exit: normal broker and catalogue-independent fallback preserve the common
result identity and terminal vocabulary on the same fixture while naming their
different source/completeness semantics, and provider absence is explicit.

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

## O1 — Orchestrator Core 1.0

Advance the headless service independently of GUI.Forms. Freeze and conform the
Core 1.0 profile: common envelope, lifecycle/release, frontend bootstrap,
structured CLI, production local transport, discovery/authentication, immutable
availability snapshots, and an independent client.

Exit:

- the release manifest reports the Core 1.0 bootstrap profile ready;
- `ORC-COM-001`, `ORC-LIF-001`, `ORC-FE-001`, and `ORC-CLI-001` meet their
  accepted compatibility horizon;
- cold start, already-running discovery, mismatch, cancellation, restart,
  shutdown, malformed frame, and unavailable-provider fixtures pass;
- Engine, plugin, and semantic capability state remains truthful even when
  those providers are not ready.

## Frontend 001 opening

The old global Gate F0 is superseded. Frontend 001 begins after:

1. Orchestrator Core 1.0 is available through the live frontend bootstrap edge;
2. GUI.Forms gives a named go-ahead for the Frontend 001 consumption surface;
3. the grand architect explicitly directs frontend implementation to begin.

Frontend tests may replay canonical Core 1.0 fixtures, but product bootstrap
uses the live Orchestrator. It does not wait for Engine, plugin AI, semantic
facts, federation, or Kolmogrov transfer; those states are reported through the
Core 1.0 availability surface and advance behind individual gates.
