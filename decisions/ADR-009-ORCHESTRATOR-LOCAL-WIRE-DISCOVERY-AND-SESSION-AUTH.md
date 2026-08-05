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
- **UNMEASURED:** concurrent-client throughput, slow-client containment, native
  Windows named-pipe behavior, launchd integration, and long-run resource use
  remain promotion gates.

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

- launchd-selected runtime location and socket activation on macOS;
- systemd and Windows named-pipe discovery/ACL projections;
- bounded concurrent sessions, cancellation, and slow-client backpressure;
- credential rotation across daemon restart and cached frontend discovery;
- independent C++ client and hostile/cross-version corpus;
- wire latency/CPU/allocation measurements against alternatives.
