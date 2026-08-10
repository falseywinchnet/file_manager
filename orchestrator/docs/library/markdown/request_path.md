# Authenticated request path

Status: **OBSERVED implementation of accepted ADR-009 framing and session law**.

A local request crosses narrowly separated discovery, authentication, queueing, framing, validation, dispatch, and response-budget stages. No stage grants more authority because a later stage succeeds.

## Sequence

- A client derives the stable private runtime leaf or uses an explicit fixture override.
- Discovery opens no-follow private files and validates owner, mode, type, size, instance, and socket identity.
- The client connects and sends the credential/version hello as the first bounded ORC1 frame.
- The daemon verifies the kernel-vouched peer UID on macOS and authenticates the rotating token.
- A fixed four-worker pool receives through an eight-session bounded queue; excess peers are closed.
- The authenticated worker reads one-MiB-capped frames and serializes access to the stateful kernel.
- The kernel validates request identity, shape, critical extensions, cancellation, deadline, and contract range before dispatch.
- The response is bounded, framed, written, and counted only as non-authoritative runtime health.

## Failure containment

- Authentication failure receives no server hello.
- Malformed, partial, oversized, stalled, and saturated sessions do not create unbounded threads or allocations.
- A worker panic requests complete daemon shutdown instead of leaving silent reduced capacity.
- Shutdown closes active socket clones, joins workers, and then performs endpoint cleanup according to endpoint ownership.

## Authority and evidence locators

- [../decisions/ADR-009-ORCHESTRATOR-LOCAL-WIRE-DISCOVERY-AND-SESSION-AUTH.md](../../../../decisions/ADR-009-ORCHESTRATOR-LOCAL-WIRE-DISCOVERY-AND-SESSION-AUTH.md)
- [src/service/local.rs](../../../src/service/local.rs)
- [src/local_endpoint.rs](../../../src/local_endpoint.rs)
- [src/local_session.rs](../../../src/local_session.rs)
- [src/local_wire.rs](../../../src/local_wire.rs)
- [tests/local_hostile.rs](../../../tests/local_hostile.rs)
