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

## Named GUI.Forms gate redeployment

Status: **MEASURED rollback-safe update and live bootstrap pass**.

After GUI.Forms published its named FM0 manifest, the complete prescribed M4
gate passed again: the service atlas was regenerated and checked, the C atomic
worker primitive passed, all Rust/C++/Go/hostile tests passed, Clippy completed
with warnings denied, and Cargo produced the locked release build.

- replacement artifact SHA-256:
  `10415336f77978bc5b088233cf7fa07157923e92f3322b980ce52fb3d37ae359`
- embedded Core manifest digest:
  `fb1b2a5eea27fd0dfcc662e2958817e88d5b77260e40f1d63c5c737f3064fa3c`
- previous installed artifact retained for rollback as:
  `orchestrator.previous-dea0b4d5`
- the installed artifact hash matched the replacement before activation;
- graceful shutdown returned `state=stopped`;
- the next authenticated bootstrap activated launchd run 3 at PID 21759;
- the live `frontend_opening` projection reported the Orchestrator gate
  `available`, GUI.Forms gate `available`, and architect direction `recorded`,
  all with `satisfied=true`;
- the live GUI.Forms capability named
  `gui-forms-fm0-macos-arm64-2026-08-10` and `ORC-GUI-001` projected
  `frozen_v0`.

Installed Engine transport remained explicitly unavailable. This deployment
closes the live Frontend 001 opening predicate; it does not promote later
provider contracts.
