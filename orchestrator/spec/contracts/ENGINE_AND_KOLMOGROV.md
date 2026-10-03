# ORC-ENG / ORC-KOL: Engine and Kolmogrov contracts

Status: **ORC-ENG-001/002 and the ORC-ENG-003 status snapshot semantic v0 are
frozen for experimental implementation; ORC-ENG-004 has a fixture-draft
development implementation; the status-subscription extension and ORC-KOL-001
are negotiating**.

Authority: Orchestrator owns the cross-project meaning and capability map. The
Go Engine owns exact catalogue observations, retrieval mechanics, and core
candidate evidence. This specification does not expose Go structs, storage
components, platform handles, or a production wire codec.

Decision authority:
[`ADR-007`](../../../decisions/ADR-007-ENGINE-ORCHESTRATOR-SEMANTIC-V0-AND-CAPABILITY-GATES.md).
Catalogue-independent search authority:
[`ADR-008`](../../../decisions/ADR-008-CATALOGUE-INDEPENDENT-LIVE-SEARCH.md).
Provider dialogue:
[`engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../../engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md).

## Version and compatibility

The frozen experimental family is semantic major `0`, minor `1`.

- Optional fields and optional operations may be added in a later minor.
- Unknown optional fields are ignored and preserved where a projection supports
  round-tripping.
- Unknown critical fields, identity meaning, authority changes, exact/stale
  meaning, terminal-status changes, or pagination-generation changes require a
  new major or fail with `version_mismatch`.
- JSON fixtures are the conformance projection. They do not freeze production
  framing, discovery, authentication, or binary ABI.

## Required I1 operation subset

The separately gated
[catalogue substring development profile](ENGINE_CATALOGUE_SUBSTRING_DEVELOPMENT.md)
specifies implementation/conformance work under ORC-ENG-001. Its capability is
not available and its bounds are not measured performance claims. Existing exact
requests and legacy continuations remain unchanged. Reserving a new cursor
prefix in the adapter does not enable a provider or advertise the new predicate.

| Operation | Authority | Frozen semantic outcome |
|---|---|---|
| `engine.version` | query | Component/build/protocol, process `instance_id`, contract families, feature names, and capability states/reasons. |
| `engine.status` | query | One bounded snapshot of lifecycle, configuration identity, roots, exact generation, integrity, currentness, backlog, and warnings. |
| `engine.query` | query | Bounded root-scoped exact query over one checked generation with evidence, deterministic order, continuation, and explicit stale/partial state. |
| `engine.inspect` | query | Exact object/binding metadata and evidence from one generation without administrative power. |

These four operations are sufficient for Orchestrator to implement discovery-
independent adapter traits, fake peers, exact routing, provider availability,
and stale-result policy. Provider absence returns `unavailable`; it is never an
empty successful result.

## Frozen administrative subset

| Operation | Frozen semantic outcome |
|---|---|
| `engine.configuration_get` | Return enacted configuration schema, digest, deployment/store facts, ingestion mode, canonical root policy, and separate persistence health. |
| `engine.root_plan` | Validate and canonicalize proposed already-authorized roots without mutation; return current/proposed configuration digests and change state. |
| `engine.root_apply` | Compare the required expected configuration digest and project already-authorized policy; stale comparison fails without mutation. |
| `engine.scan_reconcile` | Authoritatively scan one approved root and publish or reuse a checked exact generation. |
| `engine.integrity_check` | Inspect checked state without implicit repair. |
| `engine.projection_rebuild` | Rebuild erasable engine projection while preserving source-file authority and the last valid reader where possible. |
| `engine.shutdown` | Drain service-owned work and stop the process instance; it does not mutate source files or create a clean-stop heartbeat. |

Administrative semantics do not grant administrative authority. Installed
deployments require a separately authenticated admin endpoint/capability.
`engine.root_apply` consumes authorization; it does not decide it.

## Query request

The semantic request contains:

```text
scope {root_id, optional relative_path, descendants}
optional text
bounded limit
optional generation-bound cursor
zero or more exact literals
bounded exact metadata filters
zero or more requested evidence channels
bounded deterministic sort keys
optional required source generation
deadline and cancellation identity from ORC-COM-001
response/work budget clamped by server policy
```

Semantic v0 guarantees exact name, path, object/binding identity, and the
implemented intrinsic metadata predicates. A channel request never makes an
unavailable lexical, fuzzy, content, or semantic channel appear successful.

## Query and inspect result

Each result preserves:

```text
exact object identity {root_id, file_object_id, incarnation}
observed binding address
exact intrinsic metadata
checked source generation
stable rank within the response
certainty and ordered evidence records
unavailable/stale state
source anchor or matched field where applicable
```

The response also contains the generation, optional next cursor, query plan,
stale roots, partial/unavailable channels, warnings, and terminal status. A
cursor is valid only for the generation and query identity that created it.
Generation change returns `stale` detail rather than silently continuing over a
different snapshot.

Paths are addresses, not file identity. Inspecting an object with multiple
bindings requires an address when the requested result would otherwise be
ambiguous.

## Currentness and staleness

The accepted snapshot states are:

```text
manual_reconcile
baseline_required
reconciling
catching_up
current_volatile
observation_coverage_incomplete
observation_unavailable
```

The snapshot carries, when meaningful:

```text
background_ingestion
backlog_known
pending_observations
oldest_observation_ms
observation_gap
observation_source / observation_epoch
observed_watermark / reconciled_watermark
watermark_durable
coverage_incomplete
observation_error
```

Rules:

- `backlog_known=false` is not an empty backlog.
- `current_volatile` means the exact scan caught up to the adapter position in
  this process; it is not committed currentness across restart.
- `observation_coverage_incomplete` means delivered hints were reconciled but
  the adapter cannot support an exact-current claim. Results remain queryable
  with stale provenance.
- `manual_reconcile` permits checked cached exact query without continuous
  background indexing. It does not promise live filesystem truth.
- A checked stale generation may produce successful or partial results only
  when the caller's freshness policy permits it; stale roots and warnings stay
  attached.
- A strict-current request fails `stale` or `unavailable`; it never returns an
  unmarked cached response.

`engine.subscribe_status` is not required for I1. Its `(instance_id, sequence)`
replay/resynchronization protocol remains an optional minor-version extension.
Polling `engine.status` at a caller-bounded cadence is permitted; a durable
status journal is rejected.

## Capability gates

Capability state is one of `available`, `available_experimental`,
`negotiating`, `unavailable`, or `deferred`, with a reason required for every
non-available state.

The following capabilities are independent:

```text
engine.exact.catalogue
engine.exact.query
engine.live.query
engine.scan.reconcile
engine.background.observation
engine.durable.immutable_generation
engine.lexical.index
engine.similarity.fixed_width
engine.content_feature.intake
engine.status.subscribe
engine.transport.framed_local
```

Orchestrator must gate controls and routes on capability plus observed provider
availability. Contract availability does not imply a connected runtime
provider. Experimental capability does not satisfy a request that requires
production availability.

### Non-indexed/live-search boundary

The engine currently supports manual reconcile followed by exact catalogue
query. Continuous background indexing is optional. ADR-008 requires a true
zero-catalogue live filesystem traversal provision as the separate
`ORC-ENG-004` family; it is not inferred from `manual_reconcile`. Engine reply
006 and the development provider/adapter are now observed. Native NTFS/ext4,
million-entry resource evidence, and installed authenticated transport remain
promotion gates.

The required first slice is bounded name/path search over an already-authorized
scope. It returns progressive pages without creating or requiring a persistent
catalogue. Each response names its live source, scan identity, completeness,
visited-work counters, inaccessible or changing subtrees, and exact evidence
observed at traversal time. It never claims a catalogue generation or indexed,
similarity, content, or semantic evidence.

Orchestrator may route to this lane only when the caller permits fallback and
the catalogue outcome is `unsupported`, `unavailable`, `stale`, or
`quarantined`. It does not route around `denied`, `invalid`,
`budget_exceeded`, `timeout`, or `cancelled`, and it does not reinterpret an
authoritative catalogue no-match as provider failure. Numeric work, memory,
descriptor, result, output, and deadline ceilings remain negotiation and
measurement inputs rather than unverified performance claims.

Continuation cursors carry their source lane. Once a catalogue or live stream
has begun, its cursor keeps subsequent pages on that lane; Orchestrator does not
silently switch source semantics midstream. A caller may restart the query
without a cursor to select fallback again.

## Terminal mapping

Every reply maps to the ORC-COM-001 terminal vocabulary and may preserve a
structured engine detail code.

| Condition | Terminal status |
|---|---|
| valid result, including an authoritative no-match result | `success` |
| usable results with named stale roots or unavailable optional channels | `partial` or `success` with required stale detail, according to caller policy |
| malformed field, root, filter, sort, or cursor | `invalid` |
| query/admin authority absent | `denied` |
| operation/channel not implemented by negotiated version | `unsupported` |
| provider/root/generation unavailable | `unavailable` |
| generation precondition, configuration digest, or continuation expired | `stale` |
| contract major incompatible | `version_mismatch` |
| candidate, response, memory, or work ceiling exceeded | `budget_exceeded` |
| deadline elapsed | `timeout` |
| same-session cancellation accepted | `cancelled` |
| only checked copy isolated for evidence | `quarantined` |
| non-public implementation failure | `internal_fault` |

Empty success never substitutes for unavailable, stale, denied, unsupported, or
budget-exceeded state.

## Bounds, cancellation, and backpressure

- Requests are paged; first-slice query results are not an unbounded stream.
- The provider enforces stricter server ceilings over caller budgets.
- Each request is cancelled by context, deadline, disconnect, or a
  session-scoped cancellation identity.
- A slow consumer cannot retain unbounded provider output.
- Status snapshots are bounded. Future event delivery uses a bounded replay
  window and explicit resynchronization, never a durable per-event journal.
- No query waits for compaction or optional provider lanes.

Exact numeric defaults remain implementation policy and may change within the
same semantic version when they stay within declared server ceilings and return
the same terminal family on rejection.

## Security and process boundary

- Query and admin authority are separate authenticated sessions/endpoints.
- Caller identity and capability are derived from the authenticated peer; JSON
  payload claims do not grant authority.
- The Engine remains a separately restartable process. Packaging it with File
  Manager does not link Go into Orchestrator or grant store-file access.
- JSONL stdio is permitted for fixtures and development subprocess adapters.
  Production local framing, discovery, peer verification, and installation are
  independently gated platform projections.
- No operation in ORC-ENG mutates source-file bytes or exposes writable engine
  storage.

## Fixture set

Canonical semantic fixtures live under
`../../conformance/fixtures/engine/semantic-v0/` and cover:

- capability/currentness snapshot with exact query available and background
  coverage incomplete;
- cached exact query with explicit stale provenance;
- authoritative no-result success;
- engine provider unavailable;
- live-query capability unavailable.

Cancellation, production-frame equivalence, query/admin authentication,
installed discovery, and status-subscription replay remain red adapter fixtures
without blocking the semantic-v0 fake-provider core.

## ORC-KOL-001 boundary

Kolmogrov remains a planned first-class core fuzzy-candidate mechanism, not a
semantic plugin. It is independent of ORC-ENG-001 exact-query integration and
does not block I1.

The eventual contract must expose configuration/family identity, source-feature
policy, sparse fixed-width address identity, candidate limits and continuation,
raw distance/evidence, exact catalogue verification, fallback/control identity,
and rebuild/migration behavior. The active engine reply proposes a sparse
address family rather than one variable sketch blob. Production availability,
wire conformance, capacity fallback, and live manifest publication remain
negotiating.
