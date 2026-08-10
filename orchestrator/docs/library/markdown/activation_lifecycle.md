# Activation, lifecycle, and restart

Status: **DECIDED and MEASURED for the first macOS launchd artifact**.

The same local wire supports a self-bound development daemon and a launchd-supervised installed daemon, but only the supervised projection reports the accepted restart capability.

## Self-bound service

- serve-local binds and publishes its own private Unix socket.
- It may attach the explicitly configured development Engine child adapter.
- On shutdown it removes the socket, credential, discovery record, and empty fixture runtime leaf.
- It reports restart_strategy unavailable because no admitted supervisor owns reactivation.

## launchd-supervised service

- serve-launchd adopts the named orchestrator-control descriptor and validates its exact path and private mode.
- It rotates daemon instance identity and credential publication without unlinking the supervisor socket.
- Clients may use one bounded activation and rediscovery interval after stale publication or stopped service.
- The plist generator is pure and non-installing; installation remains an explicit packaging action.
- The accepted restart strategy is shutdown_then_supervisor_reactivate and produces a new instance identity.

## Authority and evidence locators

- [../decisions/ADR-011-ORCHESTRATOR-MACOS-LAUNCHD-ACTIVATION.md](../../../../decisions/ADR-011-ORCHESTRATOR-MACOS-LAUNCHD-ACTIVATION.md)
- [planning/CORE_1_0_RELEASE.md](../../../planning/CORE_1_0_RELEASE.md)
- [src/service/launchd.rs](../../../src/service/launchd.rs)
- [src/lifecycle.rs](../../../src/lifecycle.rs)
- [conformance/evidence/MACOS_LAUNCHD_CORE_1_0_2026-08-07.md](../../../conformance/evidence/MACOS_LAUNCHD_CORE_1_0_2026-08-07.md)
