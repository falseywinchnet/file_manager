# ORC-COM-001: common contract vocabulary

Status: **stable 1.0 Core bootstrap projection under ADR-010; additive shared
vocabulary expansion remains open**.

The executable bootstrap defines terminal status, typed errors, a bounded JSONL
request/reply shape, optional contract/version reference, deadline,
cancellation identity, and response budget. It now rejects incompatible
declared contract ranges, expired deadlines, and oversized requested responses.
Core 1.0 additionally caps request IDs at 128 bytes, methods at 256 bytes,
contract IDs at 64 bytes, cancellation IDs at 128 bytes, and critical
extensions at 16 entries of 128 bytes each. Invalid oversized request IDs are
not reflected into error responses.
Canonical cross-project identifiers,
generation/snapshot semantics, provenance chains, capability contexts, stream
sequence numbers, and partial-result detail remain negotiated work.

Current source projection: `../../src/common.rs`.
Current fixtures: `../../conformance/fixtures/bootstrap/`.

## Core 1.0 release projection

`orchestrator.release` is the executable `ORC-LIF-001` readiness projection for
the Core 1.0 profile. It returns target and build versions, a readiness Boolean,
and every named unsatisfied requirement. It reports `development` until the
contract, discovery, authentication, transport, independent-client,
cross-version, hostile-fixture, and provenance gates in
[`../../planning/CORE_1_0_RELEASE.md`](../../planning/CORE_1_0_RELEASE.md) pass.
A running lifecycle is not evidence that Core 1.0 is ready.

The release result includes `provenance.algorithm`, `scope`, `digest`,
`signed`, and `embedded_inputs`. Digest scope v1 covers the manifest identity,
ordered requirement states/evidence, and the named embedded Core registry,
contract, bootstrap-request, and local-wire inputs. It deliberately excludes
response fixtures to avoid self-reference. `signed: false` is authoritative;
package signing must not be inferred from a content digest.

`orchestrator.frontend.bootstrap` is an additive `ORC-FE-001` 1.0 method. It
returns the release, lifecycle/status, contract catalogue, availability
catalogue, routing declaration, and service-control eligibility in one bounded
response. It does not replace the independently callable inspection methods.
`orchestrator.version` names the structured CLI/JSONL protocol and the installed
`orchestrator.local` wire separately; a frontend never infers one namespace's
version from the other.

Requirements:

- paths are addresses, not file identity;
- exact, derived, inferred, user-authored, unavailable, and stale facts remain
  distinguishable;
- process-local ordinals never leave their generation without a durable mapping;
- every derived value identifies producer, version, source anchor, and source
  generation;
- unknown critical semantics fail closed;
- all collections are bounded, paged, streamed with backpressure, or explicitly
  rejected.

The bootstrap terminal vocabulary is `success`, `partial`, `invalid`, `denied`,
`unsupported`, `unavailable`, `stale`, `version_mismatch`, `budget_exceeded`,
`timeout`, `cancelled`, `quarantined`, and `internal_fault`. The JSONL laboratory
caps one frame at 1 MiB. This is a fixture projection, not the production local
daemon codec. The stdio reader applies that ceiling while reading and drains an
oversized or non-UTF-8 record before processing the next record; it never
allocates an unbounded line first. `max_response_bytes` bounds the successful
response; the bounded terminal error envelope remains returnable when that
requested success budget is too small to contain an error.

Core bootstrap operations are bounded and atomic; they do not advertise
cooperative cancellation. A supplied nonempty `cancellation_id` therefore
returns typed `unsupported`/`CANCELLATION_UNSUPPORTED` instead of being ignored.
An empty identity is invalid. Longer-running future families must define and
test their own before/during/after-terminal cancellation state machine.

Unknown ordinary JSON fields are optional extensions and are ignored. A caller
places every must-understand extension name in `critical_extensions`; because
Core 1.0 currently defines none, any nonempty list fails closed as `invalid`.

ADR-009's framing, promoted by ADR-010's compatibility horizon, is separately
versioned as `orchestrator.local` 1.0. Each message uses `ORC1`, a four-byte unsigned
big-endian payload length, and a nonempty UTF-8 JSON payload capped at 1 MiB.
The first frame is a credential/version hello; only an authenticated connection
may submit ORC-COM requests. The 0.1 development hello is retained only as an
incompatible-major fixture. Unix discovery and framing are executable; platform
promotion remains artifact-specific.

On macOS, Rust and C++ projections derive the ADR-011 stable private leaf at
`~/Library/Application Support/fo-orchestrator`; `--runtime-dir` remains an
explicit test/development override. A self-bound daemon proves effective
ownership before publication. A launchd-started daemon adopts and validates the
named supervisor socket without unlinking it, then publishes a new instance and
credential. Clients use one bounded activation/rediscovery interval so stale
credentials fail closed across process generations. Installed LaunchAgent
lifecycle evidence remains a separate gate. The macOS server now also verifies
the kernel-vouched effective peer UID before reading the bearer hello. Endpoint
publication uses exact private modes, no-follow descriptor reads, file and
directory synchronization around atomic renames, and secure drop-time erasure
for Rust-held credential values.

`orchestrator.status` and the embedded frontend status carry optional
`runtime_health`. These saturating counters expose worker/queue ceilings,
accepted, rejected, active, authenticated, failed-authentication, malformed,
and completed-request observations. They declare `authoritative: false` and
`consistency: relaxed_observability`; they diagnose pressure but never grant
authority or participate in routing decisions.

Open work: identifier equality/lifetime, binary/text normalization, timestamp
vocabulary, capability context encoding, field-presence rules, provenance,
stream backpressure, and the complete cross-provider error detail taxonomy.
