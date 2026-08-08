# macOS LaunchAgent Core 1.0 evidence — 2026-08-07

Status: **MEASURED pass for the first-platform Orchestrator Core 1.0 gate**.

## Environment

- macOS 14.8.7 (23J520), Darwin 23.6.0 arm64
- `rustc 1.96.1`, `cargo 1.96.1`
- Apple Clang 16.0.0, CMake 4.2.3
- release artifact SHA-256:
  `74ad4a9d00335c557b5bf45ac3b970483b53ffc18b4257e2fd32edf3143ebc84`
- release artifact size: 1,041,152 bytes
- embedded Core manifest digest:
  `e70db3d9c4bbe2d4e8d071a7534bd8ac7a03d8cb9a8f91da845f824bc0d59ea7`

## Executable gates

- **MEASURED:** `cargo test --locked --offline` passed all 61 tests: Rust unit
  and CLI tests, independent C++ client, Engine contract/port tests, golden
  fixtures, hostile local-client containment, and local-wire fixtures.
- **MEASURED:** `cargo clippy --locked --offline --all-targets --all-features
  -- -D warnings`, `cargo fmt --all -- --check`, and `git diff --check` passed.
- **MEASURED:** a clean release build and an independently configured C++17
  client build passed before the installed trial.

## Installed lifecycle trial

The release artifact was copied to a disposable private path. The generated
plist was validated with `plutil`, bootstrapped into the per-user launchd
domain, and exercised through the canonical Application Support endpoint.

- **MEASURED:** first-use socket activation and authenticated `ORC-FE-001`
  bootstrap succeeded.
- **MEASURED:** the independent C++ client reported `release=ready`,
  `ready=true`, `orchestrator-gate=ready`, `opening-blockers=0`,
  `gui-forms-gate=negotiating`, and `architect-gate=recorded`.
- **MEASURED:** graceful shutdown followed by supervisor reactivation completed
  in 1.201 seconds, below the five-second client bound.
- **MEASURED:** the first instance identity
  `11c529eb1cad137ba3f18d7f112c8b85` changed to
  `fe7c46d2471eff6a33f50ed1e6b4dfaf` after reactivation.
- **MEASURED:** both generations reported
  `restart_strategy=shutdown_then_supervisor_reactivate` and the same ready
  Core manifest digest.
- **MEASURED:** second shutdown and launchd bootout succeeded. The job, plist,
  runtime endpoint records, socket, and disposable binary tree were absent
  after exact-file cleanup.

## Retained negative evidence

- **OBSERVED:** an early attempt was not evidence because a host security
  authorization dialogue had not yet been accepted. The successful evidence
  run occurred after authorization.
- **OBSERVED:** the first real listener-adoption attempt exposed trailing NUL
  padding in launchd's inherited Unix-socket pathname. The implementation now
  permits only the exact expected pathname followed by NUL padding and retains
  socket type, owner, and `0600` validation.
- **OBSERVED:** launchd's default ten-second throttle exceeded the five-second
  activation bound after shutdown. The generated plist now sets a one-second
  throttle; the final measured reactivation is above.
- **OBSERVED:** a trial harness that did not precreate the runtime directory
  produced a root-owned `0755` parent, which Orchestrator correctly rejected as
  insecure. The installer precondition remains a user-owned `0700` runtime
  directory.

## Scope exclusions

This evidence promotes the macOS Orchestrator Core 1.0 edge only. It does not
claim package signing, Windows or Linux transport conformance, GUI.Forms FM0
availability, installed Engine transport, catalogue-free Engine search, plugin
runtime availability, or semantic-fact availability.
