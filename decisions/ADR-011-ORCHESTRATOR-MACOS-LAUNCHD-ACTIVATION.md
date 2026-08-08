# ADR-011: Orchestrator macOS launchd activation

Status: **accepted and implemented; installed-artifact evidence passed**.

Date: 2026-08-05.

Owner approval: the grand architect delegated Orchestrator integration
decisions, selected a lazy user-scoped daemon, and directed continued Core 1.0
work after the Engine became available.

## Question

How does the first macOS artifact provide stable, lazy, restart-safe daemon
discovery without changing the `orchestrator.local` 1.0 client contract?

## GIVEN constraints

- Orchestrator is one user-scoped headless daemon; it is not systemwide.
- Local clients have equal CLI authority and must not configure an endpoint.
- The daemon is lazy and must survive process generations without losing the
  stable rendezvous location.
- Socket, credential, and discovery objects remain private to the user.
- Explicit runtime paths remain fixtures, not installed-product discovery.

## Workload and failure modes

The first client may arrive while no daemon process or discovery record exists.
After a prior daemon exits, launchd's socket and a stale credential/discovery
pair may remain. A client must trigger activation, reject the stale generation,
rediscover the rotated credential, and connect within a fixed bound. A wrong
listener path, owner, mode, descriptor count, or socket kind fails closed.

## Candidates

### A. Client-spawn and PID/lock files

Every client learns process construction and races to become the daemon owner.
This duplicates lifecycle policy and makes crash recovery a client concern.

### B. A per-login temporary path with launchd registration regenerated each login

This keeps ephemeral data in the temporary tree, but the LaunchAgent definition
cannot name one durable socket path across login generations.

### C. A stable private Application Support leaf and launchd socket activation

Use `~/Library/Application Support/fo-orchestrator` with mode `0700`, a launchd
`Sockets` entry named `orchestrator-control`, and a `0600` Unix socket. The
daemon adopts the supplied descriptor, validates its exact path and owner, then
atomically rotates instance identity and the 256-bit credential.

## Evidence and measurements

- **OBSERVED:** the adopted-listener fixture verifies exact path, private mode,
  same owner, fresh publication, and preservation of the supervisor socket.
- **OBSERVED:** the activation fixture begins with stale discovery, triggers the
  persistent listener, rejects the old credential, rediscovers the new instance,
  and authenticates within the five-second client bound.
- **OBSERVED:** Rust and independent C++ clients implement the same stable macOS
  default and bounded activation retry.
- **OBSERVED:** `orchestrator launchd-plist` emits a plist accepted by macOS
  `plutil`; it performs no installation or `launchctl` mutation.
- **UNMEASURED:** a packaged binary has not yet been bootstrapped and removed as
  a real per-user LaunchAgent.

## Decision

Choose C.

The macOS default runtime leaf is
`~/Library/Application Support/fo-orchestrator`. Packaging creates it as the
effective user with mode `0700` before loading the LaunchAgent. The installed
plist names `com.filemanager.orchestrator`, passes `serve-launchd`, and owns one
`orchestrator-control` Unix socket at the fixed path with mode `0600`.

The daemon obtains the named descriptor through `launch_activate_socket`,
adopts rather than rebinds it, and never unlinks a supervisor-owned socket.
Each daemon process still rotates and atomically publishes its own credential
and instance identity. Discovery and the prior credential may remain after
exit only as activation hints; they never authorize the next generation.

A client first tries normal discovery/authentication. If that fails but the
fixed socket accepts a connection, the client treats that connection as the
activation signal and retries fresh discovery for at most five seconds. There
is no unbounded retry or fallback to an insecure endpoint.

The implementation admits the narrowly scoped `service-binding` crate on
macOS to wrap the platform descriptor handoff. Other platforms do not acquire
that dependency or inherit launchd semantics.

## Why the other candidates lost

- A spreads daemon ownership and process-launch policy across every consumer.
- B makes a persistent LaunchAgent depend on an endpoint path selected by a
  previous login session.
- Polling without a stable supervisor socket cannot distinguish “not started”
  from “not installed” and provides no activation mechanism.

## Consequences

- Frontend and CLI clients use one discovery algorithm whether the daemon is
  resident, stopped, or restarting.
- Graceful daemon shutdown leaves supervisor-owned publication objects; the next
  generation replaces credential/discovery atomically.
- The explicit `serve-local --runtime-dir` fixture keeps ownership and removes
  its socket/runtime leaf on exit.
- Plist generation is safe and non-installing. Installation/loading remains a
  packaging operation with explicit user authority.

## Reversal and migration path

The stable runtime path can move only through a dual-discovery interval or a new
installed major because existing 1.x clients embed the default. The launchd
adapter can be replaced behind `UnixEndpoint::adopt` without changing framing or
semantic contracts.

## Installed release evidence — 2026-08-07

- **MEASURED:** on macOS 14.8.7 arm64, a release-built artifact installed at a
  disposable user-private path bootstrapped the generated per-user LaunchAgent,
  activated from the stable socket, authenticated status, shut down cleanly,
  reactivated within the five-second client bound, published a distinct
  instance identity, shut
  down again, booted out, and removed only its test artifacts.
- **OBSERVED negative result retained:** the first real trial exposed trailing
  NUL padding in launchd's inherited `sockaddr_un` pathname. The adapter now
  accepts only the exact expected bytes followed solely by NUL padding, then
  still validates filesystem socket type, owner, and `0600` mode.
- **OBSERVED negative result retained:** launchd's default ten-second throttle
  violated the accepted five-second reconnect bound after shutdown. The
  generated plist now declares a one-second `ThrottleInterval`; measured
  reactivation is within the bound.
- **OBSERVED:** bootout removed the supervisor job/socket and exact cleanup
  removed the disposable plist, binary, credentials, discovery record, and
  empty private directories. No persistent service was left installed.

This closes ADR-011's Core 1.0 release gate. Windows and Linux retain their own
supervisor decisions and evidence.
