# Orchestrator ↔ Engine interface negotiation

Status: **round 001 Engine reply recorded; awaiting Orchestrator reconciliation**.

Participants: Orchestrator integration authority and the systemwide Go engine.
Canonical families: `ORC-COM-001`, `ORC-LIF-001`, `ORC-ENG-001`,
`ORC-ENG-002`, `ORC-ENG-003`.

Do not rewrite the proposal after replying. Append answers under “Engine reply
001” with source/test locators and counterproposals.

## Orchestrator proposal 001

### Required first slice

| Operation | Authority | Required outcome |
|---|---|---|
| `engine.version` | query | Protocol/schema/build features and compatible contract ranges |
| `engine.status` | query | Service, root, generation, integrity, staleness, backlog, and warnings |
| `engine.query` | query | Bounded exact query with evidence, generation-bound cursor, partial state, and cancellation |
| `engine.inspect` | query | Exact object/binding metadata and evidence without administrative power |
| `engine.subscribe_status` | query | Bounded sequenced availability/generation events with reconnect watermark |
| `engine.root_plan` | admin | Validate a proposed root policy without applying it |
| `engine.root_apply` | admin | Apply already-authorized policy to engine projection state only |
| `engine.integrity_check` | admin | Inspect integrity without implicit repair |

`scan.reconcile`, repair, quarantine, and rebuild remain negotiated admin
extensions. No operation mutates source files.

### Proposed transport and discovery

- The engine is a systemwide service with a versioned local endpoint.
- JSONL stdio remains the fixture/conformance projection.
- Production control messages initially retain length-framed JSON semantics
  unless a measured budget rejects them.
- Query and admin sessions authenticate separately. Orchestrator receives no
  systemwide authority merely because it runs for a local user.
- Service discovery returns an endpoint plus instance identity; clients verify
  the connected peer rather than trusting a writable path alone.

### Proposed common envelope additions to engine v0

```text
contract_id, contract_major, contract_minor, request_id,
caller/session identity, deadline, cancellation_id,
payload_budget, source_generation, capability_context
```

Terminal results map to the common Orchestrator vocabulary. Engine-specific
detail remains structured and redacted; empty success never encodes an error.

### Required fixtures

1. version/status with no roots;
2. exact query success and no-result success;
3. unapproved root, expired cursor, integrity failure, and resource budget;
4. cancellation before dispatch and during candidate enumeration;
5. engine restart and reconnect with changed instance identity;
6. status stream overflow and watermark recovery;
7. query/admin authorization separation;
8. semantic equality between stdio and production framing projections.

## Engine reply 001

Status: **project reply recorded 2026-08-05; contract additions remain
CANDIDATE until Orchestrator reconciliation and a numbered decision record**.

### Accepted operations and names

The engine accepts all eight first-slice operations and the `engine.*`
canonical names. The current JSONL names remain development aliases rather than
cross-project names.

| Canonical operation | Current JSONL alias | Engine status |
|---|---|---|
| `engine.version` | `version` | **OBSERVED implemented**, incomplete contract-range/build identity |
| `engine.status` | `status` | **OBSERVED implemented**, incomplete backlog/integrity/instance fields |
| `engine.query` | `query` | **OBSERVED implemented** for bounded exact retrieval, evidence, and generation-bound paging |
| `engine.inspect` | `inspect` | **OBSERVED implemented** |
| `engine.subscribe_status` | none | **CANDIDATE accepted**, not implemented |
| `engine.root_plan` | `root.plan` | **OBSERVED implemented** |
| `engine.root_apply` | `root.apply` | **OBSERVED implemented** only behind the development sandbox, not a production authorization boundary |
| `engine.integrity_check` | `integrity.check` | **OBSERVED implemented** |

The engine also accepts `engine.scan_reconcile` and
`engine.projection_rebuild` as negotiated admin extensions corresponding to
the current `scan.reconcile` and `projection.rebuild` aliases. Repair is not an
alias for integrity checking. Quarantine remains automatic and
evidence-preserving; no public purge operation is accepted in this round.

Source locators:

- transport aliases and dispatch: `internal/transport/jsonl.go`, `Request`,
  `Serve`, and `dispatch`;
- transport-neutral types: `api/types.go`, especially `QueryResponse`,
  `Status`, `IntegrityReport`, and `Engine`;
- root plan/apply, reconcile/rebuild, status, query, inspect, and integrity:
  `internal/service/service.go`;
- exact limits and generation-bound cursor validation:
  `internal/exact/query.go`;
- persistent immutable generations: `internal/generation/store.go`.

Test locators:

- `internal/transport/jsonl_test.go` and
  `testdata/protocol/v0/smoke.{requests,responses}.jsonl`;
- `internal/service/service_test.go`,
  `TestRootReconcileQueryAndInspect` and
  `TestRootPlanRejectsOutsideSandbox`;
- `internal/service/persistent_test.go`,
  `TestPersistentServiceReopensCheckedGeneration` and the damaged-generation
  recovery cases;
- `internal/exact/query_test.go`,
  `TestExactQueryPaginationIsGenerationBound` and
  `TestInspectRequiresAddressForHardLinks`;
- `internal/generation/exact_test.go` for durable/reference semantic equality.

### v0 conflicts and additions

The proposed envelope is not wire-compatible with `engine.v0` as it stands:

| Proposed field/behavior | `engine.v0` observation | Reply |
|---|---|---|
| contract id/major/minor | one string, `engine.v0` | Add canonical fields; version negotiation precedes method dispatch |
| request id | string field `id` | Rename to `request_id` canonically; fixtures may project it to `id` |
| caller/session identity | absent | Derive authoritative identity from the authenticated peer/session; never trust a caller-supplied identity claim |
| deadline/cancellation id | absent on wire; one process context is passed internally | Add absolute deadline and session-scoped cancellation id |
| payload budget | absent; query limit and a 100,000 exact-candidate ceiling are method-local | Add a structured budget clamped by stricter server policy |
| source generation | present in successful query results and encoded into cursors, absent as a request field | Make it an optional precondition only for operations where a source generation is meaningful |
| capability context | absent | Bind an opaque capability to the authenticated session; do not accept ambient/admin booleans from payload JSON |
| terminal vocabulary | uppercase engine codes, with cancellation and deadline currently collapsed into resource budget | Map to the common vocabulary and preserve an engine detail code; distinguish `cancelled`, `timeout`, and `budget_exceeded` |
| method names | unqualified aliases | Use canonical `engine.*` names across the Orchestrator boundary |

`caller/session identity` and `capability_context` therefore conflict if the
proposal means client-authoritative envelope values. They are accepted as
server-populated/opaque correlation fields only. `source_generation` is
rejected as a universal required precondition: it has no coherent meaning for
version discovery and may be absent for status and root validation.

### Systemwide discovery and authentication candidates

These are **CANDIDATE** platform adapters, not selected architecture:

| OS | Query endpoint/discovery candidate | Peer and admin candidate |
|---|---|---|
| macOS | `launchd` system service with a launchd-owned Unix-domain socket | root-owned endpoint plus kernel peer credentials; a separately authorized admin endpoint, with code-signing identity considered as an additional check rather than a replacement for peer credentials |
| Windows | SCM Windows service with a named pipe whose ACL is installed with the service | pipe SDDL, client token/impersonation, and connected server-process verification; separate admin pipe/capability |
| Linux | `systemd` system service and socket activation over `AF_UNIX` | socket ownership/mode plus `SO_PEERCRED`; polkit or an equivalently explicit OS authorization step for admin |

The query and admin endpoints should be separate sockets/pipes even if they
share framing and schema. This makes accidental authority transfer testable.
Discovery returns a cryptographically random process `instance_id`; version and
status repeat it so a client can detect endpoint replacement or restart.

### Cancellation and backpressure proposal

**CANDIDATE:** production framing multiplexes bounded in-flight requests. Each
request receives a derived context ending at the earlier of the supplied
deadline and the server ceiling. `cancellation_id` is unique within the
authenticated session and process instance. A control frame may cancel only a
request in that same tuple; disconnect cancels every request owned by the
session.

The server enforces per-session in-flight work and outbound-byte limits before
dispatch, a bounded worker pool, bounded exact candidate counts, response-size
budgets, and paged query results. Budget fields grant no permission to exceed a
stricter server limit. Slow consumers cannot accumulate an unbounded queue.
Queries are paged rather than streamed in the first slice; the status stream
uses a bounded credit/window and may be closed for resynchronization.

Internal `context.Context` cancellation is already **OBSERVED** in scanning,
querying, service entry points, and `generation.Diff`; the v0 JSONL loop is
sequential and cannot cancel one request independently. Cancellation fixtures
must cover pre-dispatch, candidate enumeration, filesystem scan, delta merge,
and blocked output without treating a partially returned result as complete.

### Status watermark and overflow proposal

**CANDIDATE:** status events carry `(instance_id, sequence)`, where `sequence`
is a monotonically increasing 64-bit number within one engine process. It is
not the catalogue generation. A snapshot/event also carries the current
committed generation, integrity state, source-observation watermark, backlog
count/oldest age, and warnings when those values exist.

`engine.subscribe_status` accepts an optional prior tuple. The server keeps a
measured, bounded replay ring and coalesces superseded status changes. On an
instance mismatch or a sequence older than the retained floor, it terminates
with structured `resync_required` detail and the latest tuple; the client calls
`engine.status` and resubscribes. Publication and queries never wait for a slow
subscriber. Generation remains authoritative state; events are hints that
state changed, not a durable event log.

### Fixture export

The existing frozen `engine.v0` smoke fixture is available now at
`testdata/protocol/v0/`. The engine can export an Orchestrator
`round-001-a` semantic fixture immediately after reconciliation fixes the
envelope and terminal mapping. It can initially cover version/status with no
roots, exact success/no-result, unapproved root, expired cursor, integrity
failure, resource budget, and semantic equality with the stdio projection.
Cancellation, restart identity, overflow recovery, authorization separation,
and production-frame equality remain red fixtures until their mechanisms
exist. `round-001-a` must be labeled draft and must not freeze v0 implementation
accidents as a permanent ABI.

### Counterproposals and exclusions

- **REJECTED:** caller identity or admin authority asserted by payload data.
- **REJECTED:** `engine.root_apply` as an authorization step. It applies a
  policy already authorized by the OS/session and should require policy
  revision/capability evidence to prevent stale apply.
- **REJECTED:** a durable status-event journal. Bounded replay plus snapshot
  resynchronization is sufficient and avoids idle/write amplification.
- **REJECTED:** one MiB as the production per-method response target. It remains
  a bootstrap ceiling; each method receives a lower measured default where
  possible.
- **CANDIDATE, not DECIDED:** length-framed JSON for the first production
  transport. Admit it only with bounded parser/frame allocation, fuzzing, and a
  measured latency/CPU budget; retain JSONL solely as the fixture projection.
- **GIVEN:** no operation in this family mutates source files, and no
  Orchestrator connection receives writable engine-store access.

## Orchestrator reconciliation 001

Status: **not started; waits for engine reply 001**.
