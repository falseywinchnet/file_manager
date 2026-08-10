# M4 Orchestrator redeployment — 2026-08-10

Status: **MEASURED persistent LaunchAgent deployment and reactivation pass**.

## Installed objects

- binary:
  `/Users/joshuahkuttenkuler/Library/Application Support/com.filemanager.orchestrator/bin/orchestrator`
- LaunchAgent:
  `/Users/joshuahkuttenkuler/Library/LaunchAgents/com.filemanager.orchestrator.plist`
- private runtime:
  `/Users/joshuahkuttenkuler/Library/Application Support/fo-orchestrator`
- launchd label: `com.filemanager.orchestrator`
- release artifact SHA-256:
  `dea0b4d5da57aeb647168b535ecfc9d7abc97aaac37e8e2a897adbf6eb4c0506`
- release artifact size: 1,303,920 bytes
- embedded Core manifest digest:
  `5dfe40a73cd9d3c3eb90903beaeccf032f71d3d89b6f62708be48ade80d60d63`

The installed binary hash matched the M4 release product. The binary and
runtime parent directories were created user-private (`0700`), the generated
plist passed `plutil`, and launchd created the named `0600` user-owned socket.

## Live verification

- **MEASURED:** first-use activation returned successful authenticated status
  and the immutable frontend bootstrap.
- **MEASURED:** Core reported `ready=true`, lifecycle `ready`, runtime kind
  `local_daemon`, and a satisfied Orchestrator opening gate.
- **MEASURED:** GUI.Forms remained correctly attributed as `negotiating`; the
  deployment did not manufacture that external gate.
- **MEASURED:** graceful shutdown followed by client-triggered supervisor
  reactivation and authenticated status completed in 1.474 seconds.
- **MEASURED:** instance identity rotated from
  `e3038ff9d28e09a9495eac4aac8d47a8` to
  `61f0ce191854798f80cd3aa877b90e04`.
- **MEASURED:** after the trial, launchd reported the second generation
  `state = running` and `active count = 1`.

## Scope

This is the persistent user-scoped Core LaunchAgent requested by the architect,
not the disposable 2026-08-07 trial. The accepted launchd projection still
does not admit the development JSONL Engine child; installed Engine discovery
and authentication remain a separate contract gate. The internal atomic batch
primitive is compiled into the Rust library but is not selected for blocking
session or Engine-child I/O.
