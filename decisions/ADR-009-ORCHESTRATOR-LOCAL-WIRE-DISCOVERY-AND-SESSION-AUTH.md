# ADR-009: Orchestrator local wire, discovery, and session authentication

Status: **accepted for Core 1.0 implementation; platform promotion gates remain
open**.

Date: 2026-08-05.

Owner approval: the grand architect delegated Orchestrator's key integration
decisions, directed Core 1.0 to precede frontend bootstrap, and explicitly
opened the next implementation stage after Engine integration.

## Question

What production-shaped local bootstrap transport can serve the C++ frontend,
CLI, and independent clients without embedding Rust layout or a web runtime?

## Constraints

- Orchestrator is one lazy user-scoped daemon; the Engine remains systemwide.
- The wire must be bounded, versioned independently, locally authenticated, and
  usable from disciplined C++ without Rust ownership or allocator coupling.
- JSONL remains an inspectable fixture/development projection, not the installed
  daemon framing.
- Unix-domain sockets and Windows named pipes must project the same messages and
  authentication states without pretending their OS security mechanisms are
  identical.
- No malformed, unauthenticated, slow, or abruptly disconnected client may
  create unbounded allocation or ambient authority.

## Candidates

### A. Keep newline-delimited JSON on inherited stdio

This is excellent for fixtures and supervised development processes, but it has
no installed discovery or independent multi-client endpoint and weak framing
under arbitrary local peers.

### B. Bundle HTTP/gRPC or a web engine

This supplies mature framing but adds an unnecessary protocol/runtime surface,
does not by itself solve local user authentication, and conflicts with the
product's no-bundled-web-engine boundary.

### C. Small framed semantic wire over native local transports

Use an eight-byte header (`ORC1` plus unsigned 32-bit big-endian length), a
nonempty UTF-8 JSON payload capped at 1 MiB, an authenticated first-frame hello,
and native Unix-socket/named-pipe adapters. Payload semantics remain the named
ORC contracts and can later gain another codec under a new wire version.

## Evidence and measurements

- **OBSERVED:** the Rust implementation rejects bad magic, empty and oversized
  frames, truncated headers/payloads, malformed JSON, incompatible hello
  versions, wrong credentials, insecure runtime-directory modes, and discovery
  mismatches.
- **OBSERVED:** a separate CLI process discovers an actual daemon, authenticates,
  reads status, requests shutdown, and observes endpoint cleanup on macOS.
- **OBSERVED:** macOS rejects long Unix-socket paths; the adapter now enforces a
  platform byte ceiling before bind.
- **UNMEASURED:** throughput tuning, native Windows named-pipe behavior, the
  installed launchd lifecycle, and long-run resource use remain separate gates.

## Decision

Choose C.

Wire `orchestrator.local` major `0`, minor `1` uses `ORC1 || u32be(length) ||
JSON`, with a one-MiB hard payload ceiling. Each connection begins with a
bounded client hello naming the wire range, client class, and a 256-bit bearer
credential generated from the operating system's preferred secure random
source. The server returns its random instance identity, lifecycle generation,
selected wire version, and frame ceiling. Credential values are redacted from
debug output and authentication failure receives no server hello.

The first Unix projection uses an explicit absolute runtime leaf with mode
`0700`; socket, credential, and discovery files use `0600` and must share the
directory owner. Discovery records instance and fixed endpoint identity.
Already-running endpoints are refused; only a same-owner private socket that
does not accept a connection may be removed as stale. The current CLI requires
an explicit runtime directory until the launchd/login-session adapter supplies
the platform location.

The current server is an experimental serial control-plane slice with bounded
handshake and connection timeouts. It is not promoted as production-ready until
bounded concurrent-session/backpressure tests and hostile fixtures pass.

The narrow `getrandom` dependency is admitted solely for cross-platform
operating-system randomness. Its failure is terminal for endpoint publication;
Orchestrator never substitutes a predictable credential.

## Why the other candidates lost

- A cannot satisfy live frontend discovery or an installed user daemon.
- B expands the trusted and packaging surface without removing the need for OS
  endpoint policy.
- Raw process-local C/Rust APIs are excluded because they freeze ownership and
  layout across the frontend boundary.

## Consequences

- macOS/Linux can exercise real framed daemon/client bootstrap now.
- The frontend can implement an independent client against fixed-width framing
  and semantic JSON without linking Rust.
- Windows still needs the named-pipe projection and its ACL/token fixtures.
- JSON payload cost remains measurable and reversible before stable wire v1.

## Reversal path

A measured binary codec may be added under a distinct wire family or major
while preserving the semantic contract fixtures. Unix sockets may move to
launchd-owned descriptors and Windows to named pipes without changing payload
meaning. Credential authentication may be strengthened with peer credentials
or OS tokens while keeping the hello/version sequence.

## Unresolved edges

- installed launchd bootstrap/reactivation/removal on macOS;
- systemd and Windows named-pipe discovery/ACL projections;
- wire latency/CPU/allocation measurements against alternatives.

## Implementation evidence — 2026-08-05

ADR-010 promotes the implemented framing to the stable 1.0 compatibility line;
the 0.1 version selected here is retained only as pre-release rejection input.

- **OBSERVED:** a separately configured C++17 library and executable now
  authenticate without linking Rust, inspect typed version/release/lifecycle/
  availability state, shut down the daemon, and reconnect at the same runtime
  location after a new daemon instance is published.
- **OBSERVED:** the C++ consumer independently validates endpoint ownership and
  modes, instance identity, frame ceilings, JSON depth/value bounds, duplicate
  keys, and UTF-8.
- **OBSERVED:** the Unix daemon now uses four fixed session workers and an
  eight-session pending queue. Authentication and socket waits occur outside
  the serialized kernel-state lock; excess sessions are closed. Shutdown is
  published before active socket clones are interrupted and workers are joined.
- **OBSERVED:** all four workers may be occupied by stalled unauthenticated
  peers without starving a queued authenticated status request beyond the
  one-second handshake bound; shutdown remains bounded.
- **OBSERVED:** a saturation fixture fills the four workers and eight pending
  slots, observes excess connections close, and then verifies recovery and
  authenticated shutdown without spawning more threads.
- **OBSERVED:** macOS Rust and C++ callers derive the same per-login default
  endpoint without configuration; the explicit path remains a development
  override. Server publication proves effective directory ownership.
- **OBSERVED:** daemon restart rotates both instance identity and credential;
  the new daemon rejects the prior instance's token.
- This closes the Core macOS independent-consumer, bounded-overload, and hostile
  implementation slices. It does not freeze a cross-compiler client ABI or
  close installed launchd, long-run tuning, or Windows promotion gates. The
  current worker/queue counts are safety ceilings, not a throughput optimum.

## Runtime hardening evidence — 2026-08-05

- **OBSERVED:** macOS checks `getpeereid` through a safe Rust wrapper before
  parsing the credential hello. This is defense in depth within the accepted
  same-user authority boundary.
- **OBSERVED:** endpoint reads open fixed publication files with no-follow and
  validate owner, exact mode, type, and length on the opened descriptor.
  Publication synchronizes content and the containing directory around atomic
  rename. Symlink and noncanonical-mode fixtures fail closed.
- **OBSERVED:** Rust token bytes and deserialized hello credential strings are
  securely erased on drop; authentication comparison remains constant-time.
- **OBSERVED:** request envelope identities have fixed count/byte ceilings and
  oversized identifiers are not reflected into error envelopes.
- **OBSERVED:** the stdio fixture transport bounds while reading, drains
  oversized and non-UTF-8 records, and preserves alignment for the next valid
  request. A worker panic requests complete daemon shutdown instead of leaving
  an unnoticed reduced-capacity service.
- **OBSERVED:** optional saturating runtime counters report local session
  pressure as relaxed, non-authoritative telemetry.

## Launchd implementation evidence — 2026-08-05

ADR-011 resolves the macOS location and activation mechanism without changing
the wire selected here. The daemon can adopt the exact named launchd listener,
rotates publication without unlinking that supervisor socket, and Rust/C++
clients perform bounded activation rediscovery across stale credentials. The
generated plist passes `plutil`. Real installed LaunchAgent bootstrap/removal
remains the final macOS discovery promotion measurement.
