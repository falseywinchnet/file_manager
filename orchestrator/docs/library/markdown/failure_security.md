# Failure and security model

Status: **OBSERVED Core boundary; broader plugin and platform threat models remain gated**.

Core 1.0 protects its same-user local control plane with private publication, rotating credentials, bounded work, typed terminal results, and fail-closed compatibility. It does not claim containment from root compromise or same-user token theft.

## Named protections

- Private 0700 runtime leaf and same-owner 0600 publication objects.
- No-follow descriptor reads, exact type/mode/owner validation, atomic publication, and directory synchronization.
- Fresh OS-random 256-bit credential and daemon instance identity per process generation.
- Constant-time credential comparison and secure erasure of Rust-held secret buffers.
- One-MiB framing, bounded envelope identifiers, response budgets, deadlines, queue ceilings, and session timeouts.
- Unknown critical semantics and incompatible major versions fail closed.

## Explicit non-claims

- Same-user processes share the accepted CLI authority class.
- Root compromise and same-user credential theft are outside the Core 1.0 boundary.
- Runtime health counters are relaxed observability and never authorize routing or mutation.
- Windows named-pipe ACL/token behavior and Linux supervisor packaging need their own promotion evidence.
- Hostile third-party plugin execution remains outside this trusted process and is not implemented by Core 1.0.

## Authority and evidence locators

- [spec/contracts/COMMON.md](../../../spec/contracts/COMMON.md)
- [planning/AUTHORITY_AND_PROCESS_MODEL.md](../../../planning/AUTHORITY_AND_PROCESS_MODEL.md)
- [src/local_endpoint.rs](../../../src/local_endpoint.rs)
- [src/runtime_health.rs](../../../src/runtime_health.rs)
- [tests/local_hostile.rs](../../../tests/local_hostile.rs)
