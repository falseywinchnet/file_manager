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
and bounded connection timeout are executable. A fixed four-worker service with
an eight-session pending queue isolates socket/authentication waits from the
serialized kernel dispatcher, closes excess sessions, and actively interrupts
stalled peers after shutdown response publication. Those constants are safety
ceilings, not throughput-tuned values. A cross-process saturation fixture holds
all workers and pending slots, then observes excess peers being closed without
thread growth. A full-worker stalled-handshake fixture now proves the control
plane recovers within the one-second authentication bound. Worker panics request
whole-daemon shutdown rather than silently reducing capacity. Stdio applies its
one-MiB bound before allocation and drains hostile records without losing the
next request. Saturating non-authoritative health counters make worker pressure
observable. Broader hostile promotion and Windows named pipes remain open.

### O1.4 — discovery and authentication

Bind a lazy user daemon to the local login/session, protect its endpoint and
peer credentials, handle already-running and stale-endpoint cases, and publish
restart-safe instance/generation identity.

**OBSERVED first-platform slice:** macOS Rust and C++ clients derive ADR-011's
stable private Application Support leaf without caller configuration. The
self-bound fixture publishes a same-owner `0600` socket, credential, and
discovery record and proves effective ownership before publication. Accepted
sessions must also match the runtime owner's kernel-vouched effective UID.
Credential/discovery reads reject symlinks and noncanonical modes, atomic
publication synchronizes the file and containing directory, and Rust-held
credential buffers erase on drop. The
launchd adapter adopts an exact supervisor-owned listener, rotates instance and
credential publication, and leaves that socket intact. A stale-generation
fixture triggers the listener and rediscovers/authenticates the replacement
within a five-second bound. The plist fixture also proves that an explicit
runtime-directory override is carried into both the socket declaration and the
daemon arguments, preventing listener adoption from validating a different
path. The generated plist passes `plutil`; real installed LaunchAgent
bootstrap/removal evidence and other platforms remain open.

### O1.5 — independent client

Build a C++-suitable client outside the daemon crate's private types. It must
negotiate, inspect release/lifecycle/availability, survive disconnect, and pass
the same golden and hostile fixtures.

**OBSERVED first-platform slice:** the standalone C++17 library validates
private discovery, authenticates `orchestrator.local` 1.0, enforces bounded
framing, and materializes one atomic `ORC-FE-001` snapshot containing release,
lifecycle/cache generations, contracts, availability, routing/fallback state,
and service-control eligibility. It validates the Core readiness predicate and
reconnects after a real daemon restart. Cargo's integration suite configures and
builds it independently through CMake before exercising two daemon generations.
Windows transport remains open.

### O1.6 — compatibility and release provenance

Run cross-version suites, freeze the support horizon, produce a digest-addressed
release manifest, and only then allow `ready: true` for target `1.0.0`.

**OBSERVED hostile increment:** the live Unix daemon contains incompatible-major
hello, bad magic, oversized declaration, partial-payload EOF, byte-fragmented
valid frame, slow-handshake, saturation, and post-failure status/shutdown cases.
Bootstrap fixtures additionally prove that atomic methods reject rather than
ignore cancellation identities, unknown must-understand extensions fail closed,
and unknown ordinary fields remain forward-compatible. The ADR-010 macOS 1.x
compatibility horizon passes; other-platform differential suites remain their
own promotion gates.

**OBSERVED provenance increment:** `orchestrator.release` publishes a
deterministic SHA-256 digest over its identity, requirement states/evidence, and
nineteen embedded Core registry/contract/request inputs. The golden release
fixture pins that digest. This satisfies Core's digest-addressed manifest requirement;
packaged binary signing remains a later platform-release concern and is not
misreported as present.

## Provider law

Engine, Kolmogrov, settings, handlers, commands, plugins, hives, semantic facts,
audit, platform integrations, and federation keep their own contract gates.
Core 1.0 may ship while one is unavailable, deferred, negotiating, or stubbed,
provided the immutable availability snapshot states that condition exactly.

## Current blocker

The executable `orchestrator release --json` is the authoritative projection.
All Core-profile requirements except `daemon.discovery` are currently
satisfied. That final requirement needs the real installed LaunchAgent
bootstrap, first-use activation, shutdown, reactivation, bootout, and removal
trial named by ADR-011; simulated listener adoption does not close it.
