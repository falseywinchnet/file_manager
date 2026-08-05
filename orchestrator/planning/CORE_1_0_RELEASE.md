# Orchestrator Core 1.0 release profile

Status: **DECIDED target under ADR-006; current executable reports
`development`, not ready**.

Core 1.0 is the headless authority required before Frontend 001 bootstraps. It
does not mean every provider is complete and it does not collapse independent
contract versions into one API number.

## Required contract horizon

The release cannot report ready until these families are stable for the
bootstrap profile:

- `ORC-COM-001` — envelopes, terminal results, bounds, deadlines,
  cancellation, provenance, and version mismatch;
- `ORC-LIF-001` — release identity, discovery, lifecycle generation, status,
  restart, draining, and shutdown;
- `ORC-FE-001` — authenticated frontend session, immutable availability
  snapshot, stale-cache rules, and registered fallback state;
- `ORC-CLI-001` — human and structured projections over the same operations.

## Implementation sequence

### O1.1 — executable readiness manifest

Expose `orchestrator.release` through the kernel and CLI. It reports the target,
build version, readiness Boolean, and every unsatisfied requirement. A running
bootstrap must not imply a ready release.

### O1.2 — envelope enforcement

Validate contract family/range, deadlines, cancellation identities, and
response budgets. Add every terminal error and unknown critical-field fixture.

### O1.3 — production local transport

Select and implement framed local IPC with Unix-domain-socket and Windows
named-pipe adapters behind one semantic transport. Preserve stdio JSONL only as
the inspectable conformance laboratory unless measurement supports promotion.

**OBSERVED first-platform slice:** ADR-009's `ORC1` length framing, one-MiB
ceiling, Unix-domain socket server/client, malformed/truncated frame rejection,
and bounded connection timeout are executable. Concurrent backpressure,
hostile/cross-version promotion, and Windows named pipes remain open.

### O1.4 — discovery and authentication

Bind a lazy user daemon to the local login/session, protect its endpoint and
peer credentials, handle already-running and stale-endpoint cases, and publish
restart-safe instance/generation identity.

**OBSERVED first-platform slice:** an explicit private runtime leaf publishes a
same-owner `0600` socket, credential, and discovery record; a separate CLI
authenticates with an OS-random 256-bit credential, verifies instance identity,
reads status, and shuts the daemon down. Launchd-selected session location,
rotation/restart fixtures, and other platforms remain open.

### O1.5 — independent client

Build a C++-suitable client outside the daemon crate's private types. It must
negotiate, inspect release/lifecycle/availability, survive disconnect, and pass
the same golden and hostile fixtures.

### O1.6 — compatibility and release provenance

Run cross-version suites, freeze the support horizon, produce a digest-addressed
release manifest, and only then allow `ready: true` for target `1.0.0`.

## Provider law

Engine, Kolmogrov, settings, handlers, commands, plugins, hives, semantic facts,
audit, platform integrations, and federation keep their own contract gates.
Core 1.0 may ship while one is unavailable, deferred, negotiating, or stubbed,
provided the immutable availability snapshot states that condition exactly.

## Current blockers

The executable `orchestrator release --json` is the authoritative projection of
current blockers. At creation of this record all four required contracts lack a
stable horizon, and discovery, authentication, production transport,
independent-client conformance, hostile/cross-version coverage, and final
release provenance remain pending.
