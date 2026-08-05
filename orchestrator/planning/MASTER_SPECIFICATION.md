# Oracle master API/ABI specification

Status: **GIVEN contract authority; individual entries range from outline to
DECIDED**.

## Purpose

The repository must make every boundary legible before the frontend depends on
it. “The Oracle has all the APIs” means:

1. every cross-project operation has one canonical semantic definition here;
2. every provider and consumer is named;
3. call direction, authority, transport, version, cancellation, bounds, and
   failure are explicit;
4. C, C++, Go, Rust, CLI, and plugin bindings project the same meanings;
5. conformance fixtures can prove reciprocal interoperability without running
   the entire File Manager.

It does not mean every call traverses one runtime process. The C++ frontend may
use GUI.Forms in-process and may query the Go engine over its private service
protocol. The Oracle registers those contracts and coordinates cross-system
work, while avoiding an unnecessary bottleneck on pure rendering or exact
engine operations.

## Boundary taxonomy

### Semantic contract

Language- and transport-neutral definitions of operations, objects, states,
errors, authority, and lifecycle. This is always the source of truth.

### Wire API

A framed cross-process projection. It never exposes native pointers, allocator
ownership, Go interfaces, Rust enums, C++ exceptions, or process-local ordinals.

### C ABI

A deliberately small in-process or client-library seam using opaque handles,
fixed-width values, caller-visible ownership, paired allocation/free rules,
explicit status, and no unwinding. A C ABI is not used merely to avoid defining
a protocol.

### Source API

Ergonomic language wrappers generated or handwritten over a semantic/wire/C
contract. They are replaceable conveniences, not contract authority.

### Command grammar

Human CLI and structured automation projections. A command must map to the same
semantic operation and error vocabulary as other clients.

## Canonical contribution flow

1. A subproject discovers a missing or inadequate boundary.
2. It adds a proposal under `proposals/<project>/` stating the user operation,
   provider, consumers, authority, payload, performance need, cancellation,
   failures, version effect, and alternatives.
3. The Oracle specification task reconciles the proposal with existing types and
   assigns or revises contract IDs.
4. The grand architect resolves product authority or capability expansions.
5. The accepted semantic contract moves under `spec/contracts/`.
6. Golden fixtures are added before generated bindings.
7. Provider and consumer projects implement independently against the fixtures.
8. Cross-language and cross-version conformance runs before integration.

No subproject edits another subproject's private implementation to make an
unspecified boundary “work.”

## Universal envelope

Every request, response, event, and streamed chunk must define:

```text
contract_id
contract_major / contract_minor
message_kind
request_or_subscription_id
caller_identity / capability_context
deadline / cancellation identity
source and configuration generations where applicable
payload length and resource budget
provenance
```

Every terminal result is exactly one of success, partial, invalid, denied,
unsupported, unavailable, stale, version mismatch, budget exceeded, timeout,
cancelled, quarantined, or internal fault. Empty success never stands in for an
error.

## Shared object vocabulary

The registry will stabilize common semantic identifiers before binding work:

- `MachineId`, `VolumeId`, `RootId`, `FileObjectId`, `Incarnation`, `BindingId`;
- `Generation`, `SnapshotId`, `ProviderId`, `PluginId`, `PackageDigest`;
- `HiveId`, `NamespaceId`, `SchemaId`, `FactId`, `ProvenanceId`;
- `HandlerId`, `CommandId`, `CapabilityId`, `GrantId`, `JobId`;
- `QueryId`, `Cursor`, `Evidence`, `Availability`, `Staleness`;
- `LocaleId`, `ThemeId`, `SettingId`, `IntegrationId`.

An identifier's equality and lifetime are defined once. Display paths, labels,
and process-local ordinals never masquerade as durable identity.

## Reciprocity rule

Interoperation is bidirectional but not callback-entangled:

- clients send commands or queries;
- services reply or expose bounded event subscriptions;
- subscribers acknowledge sequence/watermark progress;
- cancellation and shutdown are explicit messages;
- services may disappear and reconnect at a declared generation;
- no component calls arbitrary foreign-language code on an uncontrolled thread.

This allows independent restarting, recording, fuzzing, substitution, and
version testing.
