# Orchestrator ↔ Engine interface negotiation

Status: **ORC-ENG round 001 reconciled under ADR-007; semantic v0 frozen for
experimental implementation; required catalogue-independent fallback round 006
awaits Engine reply under ADR-008**.

Participants: Orchestrator integration authority and the systemwide Go engine.
Canonical families: `ORC-COM-001`, `ORC-LIF-001`, `ORC-ENG-001`,
`ORC-ENG-002`, `ORC-ENG-003`, `ORC-ENG-004`.

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
| `engine.status` | `status` | **OBSERVED implemented**, including experimental currentness/backlog cursors; committed watermark and complete integrity projection remain open |
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

Status: **reconciled 2026-08-05 under accepted ADR-007**.

Orchestrator accepts the engine's reply and counterproposals with the following
disposition:

- `ORC-ENG-001` freezes `engine.version`, `engine.status`, bounded exact
  `engine.query`, and `engine.inspect` semantics for experimental implementation.
- `ORC-ENG-002` freezes configuration inspection, root plan/apply, manual
  reconcile, integrity check, projection rebuild, and shutdown meanings.
  Administrative semantics do not grant authority; authenticated query/admin
  separation remains a deployment-adapter gate.
- `ORC-ENG-003` freezes the status snapshot vocabulary, including explicit
  manual, volatile-current, coverage-incomplete, and unavailable states. The
  sequenced subscription/replay operation remains an optional negotiating
  extension and is not an I1 prerequisite.
- Caller identity and capability are session-derived. `source_generation` is an
  optional method-specific precondition, not a universal envelope field.
- JSONL remains the canonical fixture laboratory. Production framing,
  discovery, peer verification, and binary/C ABI are independently versioned
  and not frozen by this reconciliation.
- A checked stale generation remains queryable with explicit stale provenance.
  Provider absence, strict-current failure, and unsupported channels do not map
  to empty success.
- Background observation, committed watermarks, lexical/fuzzy/content lanes,
  native supervisors, and platform promotion advance through capability state;
  none changes the major-zero exact query meaning.
- `manual_reconcile` is not renamed or advertised as live unindexed search. A
  true zero-catalogue filesystem traversal method is currently unavailable and
  requires its own optional proposal if frontend workflow evidence demands it.

Canonical semantics:
`../../orchestrator/spec/contracts/ENGINE_AND_KOLMOGROV.md`.
Canonical fixtures:
`../../orchestrator/conformance/fixtures/engine/semantic-v0/`.

This reconciliation opens Orchestrator I1 fake-provider, adapter-trait,
availability, cached-exact routing, manual-reconcile routing, and degraded
fallback work. Installed runtime routing still waits for its platform-local
authenticated transport; native indexing completion does not block that work.

## Engine ORC-KOL reply 002

Status: **project reply recorded 2026-08-05 from disabled native dogfood;
awaiting Orchestrator reconciliation and not a production contract**.

The engine accepts Kolmogrov's sealed
`filename-literal-r1-q2-d20-n64-p65536` usage guidance as sufficient for an
experimental adapter. The adapter is implemented under `internal/similarity`
and measured in `results/M5_KOLMOGROV_HISTORY_TUPLE_001.md`. It does not change
`engine.v0` features or make fuzzy query available.

### Semantics the engine can now exercise

- complete configuration descriptor plus SHA-256 parameter identity;
- explicit per-generation coordinate pairs, never the published E13 parameters
  as the random dogfood default;
- one coupled 40-bit key per complete literal deletion history and one full key;
- typed stored-shorter, same-length, and stored-longer plans;
- exact engine ordinal and committed-generation anchor resolution;
- a private pinned exact-generation reader exercised across mutation and
  restart, without exporting the reader or storage layout through `engine.v0`;
- candidate-only evidence with matched plan/key provenance and no fabricated
  relevance score;
- exact one-edit/transposition adjudication as parent-engine evidence;
- explicit `unsupported_input`, `over_capacity`, `configuration_mismatch`,
  `rebuild_required`, `budget_exceeded`, `cancelled`, and
  `corrupt_projection` failures;
- erase/rebuild migration and immutable segment liveness.

The service dogfood and same-corpus resource result are recorded in
`results/M5_KOLMOGROV_SERVICE_DOGFOOD_002.md`. This is evidence for eventual
planner integration, not a counterproposal to expose Go reader handles or
component bytes across the Orchestrator boundary.

### Counterproposal: sparse address family, not one sketch blob

The coupled-history object has fixed-width addresses but a bounded variable
number of addresses (`n` history keys plus one full key). `ORC-KOL-001` should
therefore describe:

```text
projection descriptor
address {kind, source_length, semantic_view, coupled_key_bytes}
candidate {exact anchor/ordinal, matched plans/keys, candidate_only}
query stats {probes, posting visits, live/dead visits, capacity state}
```

A single `width_bytes` remains the width of one coupled key, not the entire
record projection. **REJECTED:** serializing all history keys into one nominal
fixed sketch and then treating that variable blob as one hash location.

Tuple byte order remains private in this round. If bytes cross the reconciled
seam, the descriptor must own coordinate order, cell bits, packing, and byte
order. A parameter digest is correlation evidence; the complete canonical
descriptor remains semantic authority.

### Still-red fixtures

Production availability, deterministic continuation, live manifest
publication, configuration rollover while readers are pinned, over-capacity
fallback through the service planner, update/compaction accounting, folded or
structural view negotiation, and independent-client wire conformance remain
red. The engine requests that Orchestrator preserve `available_experimental`
separately from production `available` and accept `over_capacity` as a terminal
fallback state rather than an empty result.

## Engine lifecycle and configuration reply 003

Status: **project reply recorded 2026-08-05; development projection implemented;
ORC-LIF/ENG snapshot semantics later reconciled by ADR-007; native service host
remains open**.

This reply declares the whole engine service, not a Kolmogrov service. Exact
catalogue identity and stored records are authoritative. Lexical, metadata,
content-fragment descriptor, and similarity mechanisms contribute bounded,
versioned evidence. Kolmogrov-derived descriptors are one such mechanism where
their measured workload warrants admission.

### Implemented semantic projection

| Operation/field | Engine declaration |
|---|---|
| `engine.version` | component/build/protocol, random process `instance_id`, feature list, and enumerated capability states |
| `engine.status` | lifecycle, active work, enacted configuration, catalogue generation/root state, warnings, and the same capability ledger |
| `engine.configuration_get` | schema/version, stable content digest, deployment/persistence/store placement, ingestion mode, canonical root policy, and separate durable-policy state |
| `engine.root_plan` | validates/canonicalizes roots and returns current/proposed configuration digests plus `changed` |
| `engine.root_apply` | requires `expected_configuration_digest`; returns `STALE_CONFIGURATION` without mutation on mismatch |
| `engine.shutdown` | rejects new operations, cancels service-derived contexts, drains active work, closes the exact reader, and returns terminal lifecycle status |

The unqualified v0 aliases remain test/development projections. In particular,
unconditional `root.apply` is not the canonical cross-project operation.

Lifecycle values are `starting`, `ready`, `draining`, `stopped`, and `faulted`.
The current implementation constructs synchronously into `ready` or returns an
error; it does not fabricate observable startup progress. `start` is supervisor
construction and checked recovery. `restart` is supervisor process replacement
and produces a new `instance_id`; it is not an in-process method. Clean shutdown
does not create a durable clean-stop record or rewrite the generation.

### Configuration authority and durability

The expected digest is concurrency evidence, not authorization. Orchestrator
must supply already-authorized policy through an authenticated administrative
session. The engine validates containment and projects it. Payload JSON cannot
assert caller identity or authority.

`root_policy_persistent` is health/durability state and is excluded from the
configuration digest so the digest does not change merely when the same policy
reaches a checked generation. **Open:** current root policy exists only in
process memory until persistent reconciliation. Production acknowledgement
needs the manifest's authoritative root-policy revision and observation
watermark in the same commit as all component digests. The engine will not
invent a process-local revision that appears durable.

### Capability declarations for dependency planning

Runtime states use `available`, `available_experimental`, `negotiating`,
`unavailable`, and `deferred`; every non-available state includes a reason.
Current declarations include:

- exact catalogue/query, ordered metadata, root policy, and manual scan:
  `available`;
- immutable checked generation: `available` only for persistent instances;
- delta publication: `available_experimental`, component-only and not live;
- lexical index: `unavailable`;
- anchored content-feature intake and fragment descriptors: `deferred`;
- fixed-width similarity: `available_experimental`, disabled from the public
  planner;
- native observation/watermark/coalescer: `available_experimental`; portable
  coalescing, macOS FSEvents, and Windows `ReadDirectoryChangesW` exist, but
  exact-current coverage is incomplete, the watermark is volatile, Windows has
  compatibility-only validation, and Linux is absent;
- bounded status subscription: `negotiating`;
- authenticated framed local transport: `deferred`;
- `ORC-LIF-001`, `ORC-ENG-001`, and `ORC-ENG-002`: this reply originally
  declared `negotiating`; ADR-007 now freezes their semantic-v0 subset while
  leaving the installed transport adapter gated;
- `ORC-ENG-003`: this reply originally declared `unavailable`; ADR-007 now
  freezes the status snapshot vocabulary while status subscription remains
  `negotiating`.

This list is intentionally honest enough for Frontend/Orchestrator to disable
or annotate controls without treating absent work as an empty queue or healthy
production service.

### Native control seam requested from Orchestrator

The engine proposes system service adapters as platform-specific projections:
launchd plus separated Unix sockets on macOS, SCM plus ACL-separated named
pipes on Windows, and systemd plus separated `AF_UNIX` sockets on Linux. These
remain **CANDIDATE** until install/update/uninstall, peer authentication,
crash-loop, deadline, endpoint-replacement, and native filesystem fixtures
pass. Wine and Lima remain compatibility oracles, not native durability or
service-supervisor evidence.

Orchestrator reconciliation is requested for:

1. envelope and terminal mapping for `STALE_CONFIGURATION`;
2. authoritative policy revision/capability evidence at apply and manifest
   commit;
3. instance identity/discovery and supervisor lifecycle mapping;
4. bounded status replay/resynchronization without a durable event journal;
5. query/admin authority separation and framed transport limits;
6. capability state/reason projection to Frontend controls.

Source locators: `api/service.go`, `api/types.go`,
`internal/service/lifecycle.go`, `internal/service/service.go`,
`internal/transport/jsonl.go`, and
`docs/SERVICE_LIFECYCLE_AND_CONFIGURATION.md`. Conformance locators:
`internal/service/lifecycle_test.go` and
`internal/transport/jsonl_test.go`.

## Engine tiered-publication reply 004

Status: **project reply recorded 2026-08-05; isolated candidate semantics
observed; live contract admission remains negotiating**.

The engine now has a checked, isolated `TIERED.0`/`TIERED.1` experiment for an
authenticated base plus ordered delta/index pairs. This is not a request for
Orchestrator to consume storage filenames or Go representations. The proposed
cross-project semantics are:

```text
committed exact generation {
  root policy identity,
  generation,
  catalogue digest,
  component health,
  source observation watermark (still open)
}

component health = checked | rebuildable_degraded | unavailable
recovery selection = newest_complete | previous_complete | none
```

**OBSERVED:** logical and subprocess interruption, partial manifest write,
corrupt manifest, corrupt/rebuilt sidecar, newer schema, cancellation, and
pin-aware reclamation tests select one complete exact generation or fail
closed. The isolated manifest binds root, base, ordered run chain, exact target
digest, live length, change count, and index bytes.

**GIVEN:** exact source records remain authoritative. A missing disposable
index is `rebuildable_degraded`, not an empty exact result and not corruption of
the filesystem. If no complete manifest validates, availability is
`unavailable`; the engine must not assemble a plausible mixed chain.

**UNRESOLVED:** live manifest schema/ADR, root-policy revision, observation
watermark, component-level degradation projection, manifest-byte accounting,
quarantine integration, startup status, and authority for repair/rebuild.
Until reconciled, `delta publication` remains `available_experimental` and the
public service continues to use full checked generations.

Source locators: `internal/generation/tiered_manifest_candidate.go`,
`internal/generation/tiered_manifest_candidate_test.go`, and
`results/M2_TIERED_MANIFEST_RECOVERY_005.md`.

## Engine background-currentness reply 005

Status: **project reply recorded 2026-08-05; macOS/portable experimental
semantics observed; committed-currentness contract remains negotiating**.

The engine now proposes these representation-free status meanings:

```text
observation state =
  manual_reconcile |
  baseline_required |
  reconciling |
  catching_up |
  current_volatile |
  observation_coverage_incomplete |
  observation_unavailable

observation cursor = (adapter_source, adapter_epoch, opaque_ordered_position)
current_volatile = exact scan reconciled through observed cursor in this process
committed_current = exact generation and cursor authenticated by one manifest
```

**OBSERVED:** status projects background activity, whether backlog is known,
pending count/oldest age, gap state, adapter source/epoch, observed position,
reconciled position, `coverage_incomplete`, and `watermark_durable`. The
portable coalescer plus macOS FSEvents and Windows `ReadDirectoryChangesW`
adapters converge through duplicates, sparse positions, reported or detected
gaps, bounded overflow, root invalidation, event storms, and arrivals during a
scan. Adapter stop makes backlog unknown. Queries name stale roots until
reconciliation catches up or whenever adapter coverage remains incomplete.

**GIVEN:** `current_volatile` may not be projected as committed currentness.
Every adapter start currently forces a full baseline because `MANIFEST.*` does
not bind the cursor. The engine added no cursor sidecar, per-event journal, or
durable status log.

**MEASURED:** on native macOS/APFS, two retained 50 ms FSEvents / 100 ms
coalescer-age repetitions measured 161.767-163.695 ms p50 and
200.987-228.183 ms p95 across 32 sequential event-to-reconciled-generation
cycles. A 4,096-write/256-path storm converged in 338.253-489.390 ms with three
full-generation publications; a two-second quiet interval made zero
application durable writes. Three file-level hard-link fixtures did not deliver
the final remaining-link removal within two seconds, so macOS reports
`observation_coverage_incomplete`. The Windows adapter passes a Wine
create/cancel compatibility fixture but has no native NTFS credit. See
`results/M4_BACKGROUND_CURRENTNESS_001.md` for limitations and the retained
rejected 20-publication tuning.

**MEASURED idle:** a separate 600.008-second native run consumed 39.927 ms of
process CPU (0.006654% of one core), made zero application durable writes, and
reported 10,076,160 bytes maximum RSS. Battery/device power and physical-write
attribution remain open.

Orchestrator reconciliation is requested for:

1. whether the status names above, including coverage-incomplete separately
   from catching-up/unavailable, enter `ORC-ENG-003` snapshots/events or map
   to a smaller shared enum;
2. the atomic manifest fields and acknowledgement boundary for
   `committed_current` (cursor, root-policy revision, generation, schema, and
   component digests);
3. supervisor policy when observation is unavailable but a checked stale
   generation remains queryable;
4. ownership of battery/power modes and the bounded policy values supplied to
   the engine.

Source locators: `api/service.go`, `internal/observation`,
`internal/observation/fsevents`, `internal/service/background.go`, and
`results/M4_BACKGROUND_CURRENTNESS_001.md`.

## Orchestrator requirement 006 — catalogue-independent live search

Status: **required proposal issued 2026-08-05 under accepted ADR-008; Engine
reply and implementation evidence required**.

The grand architect has clarified that search must work without a catalogue.
Manual reconciliation followed by catalogue query does not satisfy that
requirement. Orchestrator therefore opens `ORC-ENG-004` as a separate required
Engine provision rather than changing the frozen `ORC-ENG-001` catalogue-query
meaning. This provision gates File Manager search readiness, not Orchestrator
Core 1.0, so the headless core and Engine implementation continue in parallel.

### Required capability

The Engine must provide `engine.query_live`: a read-only, zero-catalogue
filesystem traversal over an already-authorized root/scope. The first slice
must support bounded filename and relative-path matching and return exact
filesystem observations progressively. It must work when:

- persistent indexing is disabled by policy;
- no catalogue has ever been built;
- the catalogue is rebuilding, quarantined, corrupt, or version-incompatible;
- the caller rejects a stale catalogue generation; or
- background observation/currentness is unavailable.

The operation must not build, persist, or require a catalogue as a side effect.
It must not write inside the source tree, open file contents by default, follow
directory symlinks/junctions/reparse points outside policy, or treat an
unreadable scope as an empty successful result.

### Semantic proposal

```text
engine.query_live request {
  authorized scope {root_id, optional relative_path, descendants},
  bounded name/path predicate,
  optional ephemeral continuation cursor,
  result/work/output budgets,
  deadline and cancellation identity
}

engine.query_live page {
  source = live_filesystem,
  scan_id,
  complete,
  optional next_cursor,
  exact observed file identities and bindings,
  discovery-order rank within this scan,
  visited_entries / stat_calls / elapsed_ms,
  partial or unavailable subtrees,
  warnings and terminal status
}
```

The page has no catalogue generation. It is an observation during a mutable
filesystem traversal and may not claim global snapshot completeness. A cursor
is bounded, process-local, query-bound, expiring state. Expiry returns `stale`;
it never causes a durable cursor journal or hidden index.

Every continuation cursor names its source lane. Orchestrator will not switch a
catalogue continuation to live traversal, or vice versa, in the middle of a
result stream; fallback is reconsidered only for a new cursorless query.

The reference traversal should stream depth-first or use another bounded-state
enumeration. It may retain `O(depth + page + fixed work buffers)` state and a
fixed descriptor cap; it may not materialize or globally sort the whole tree
before returning. Results use discovery order for the live lane. Rich indexed
ranking remains a separate catalogue advantage.

### Initial resource constitution

The server clamps caller budgets. These values are **CANDIDATE defaults and
ceilings pending native measurement**, not performance claims:

| Resource | Proposed default slice | Proposed hard ceiling per slice |
|---|---:|---:|
| returned results | 128 | 1,000 |
| visited directory entries | 100,000 | 1,000,000 |
| metadata/stat calls beyond enumeration facts | 4,096 | 65,536 |
| wall time | 250 ms | 5 s |
| simultaneously open directories | 8 | 32 |
| encoded response | 256 KiB | 1 MiB |

Reaching a work ceiling with resumable state returns a partial page plus a
cursor. Reaching it without safe resumability returns `budget_exceeded`, never
empty success. Cancellation and deadline checks occur during enumeration and
before metadata expansion; blocked output cannot retain an unbounded producer
queue.

### Required route and authority behavior

- `engine.query_live` uses query authority only and cannot admit roots.
- The Engine resolves authorized root identity before traversal and fails
  closed if the root is replaced or escapes containment.
- Symlinks are returned as objects/bindings when matched but are not traversed
  as directories in the first slice.
- Permission and transient subtree failures produce named partial state.
- A catalogue `denied`, `invalid`, `budget_exceeded`, `timeout`, or `cancelled`
  result must not be used to bypass policy into live traversal.
- Orchestrator may fall back from catalogue `unsupported`, `unavailable`,
  `stale`, or `quarantined` to live traversal when caller policy permits.
- An authoritative catalogue no-match does not trigger fallback.
- Frontend does not walk the filesystem. It consumes the common result identity
  and terminal vocabulary with explicit `live_filesystem` source and
  completeness. Any direct Engine hot path must use a registered conformant
  adapter and the Orchestrator-issued capability snapshot.

### Required conformance and measurement reply

The Engine reply must accept or counterpropose each item and identify code plus
evidence for:

1. no-catalogue and catalogue-damaged success fixtures;
2. APFS, NTFS, and ext4 containment/identity fixtures;
3. million-entry wide and deep-tree memory/descriptor bounds;
4. first-result and page p50/p95/p99/max latency, CPU, bytes read, and RSS;
5. cancellation during enumeration, metadata expansion, and blocked output;
6. permission failure, root replacement, concurrent mutation, symlink loop,
   mount-boundary, and cursor-expiry behavior;
7. zero engine durable writes during live queries; and
8. semantic equivalence of matched exact observations against a subsequent
   authoritative reconcile on a quiescent fixture.

Compatibility environments may exercise protocol and broad behavior, but
native filesystem promotion evidence remains required. The first implementation
may be basic-name/path-only; unsupported richer predicates must be explicit.

## Engine reply 006 — reserved

Status: **awaiting Engine reply**.

The Engine should record accepted items, counterproposals, implementation
locators, measured budgets, and red fixtures here. It must not answer this
requirement by renaming `scan.reconcile` or by constructing a disposable full
catalogue before returning the first result.
