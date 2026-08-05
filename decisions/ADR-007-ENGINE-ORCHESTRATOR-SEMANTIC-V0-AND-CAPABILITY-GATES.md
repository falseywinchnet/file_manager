# ADR-007: Engine–Orchestrator semantic v0 and capability-gated integration

Status: **accepted**.

Supersession note: ADR-008 supersedes only this record's classification of
zero-catalogue live traversal as optional. Catalogue query/status semantics,
packaging boundaries, and platform capability gates remain accepted.

Date: 2026-08-05.

Owner approval: the grand architect determined that native Windows/Linux
campaigns, background-currentness completion, and packaging are dogfood and
platform-capability work rather than blockers for Orchestrator's core. The
engine must yield a stable semantic surface now, with unfinished behavior
visible through capability and currentness state.

## Question

Can Orchestrator begin its real engine-integration core before native indexing,
durable observation watermarks, status subscriptions, and production local IPC
are complete?

## GIVEN constraints

- Orchestrator owns cross-project contract meaning and capability truth.
- The Go engine owns exact catalogue observations and retrieval; no Go or Rust
  native layout crosses the process boundary.
- The engine executable may be packaged with File Manager while retaining the
  independently restartable service boundary established by ADR-003.
- Navigation and reduced search survive engine absence. An absent, stale, or
  capability-limited engine is never represented as an empty successful search.
- Platform completion must not be hidden behind polling, periodic whole-tree
  rescans, or optimistic currentness claims.

## Workloads and failure modes

The first Orchestrator adapter must discover or launch a fixture engine, read
version/status, route bounded exact query and inspect calls, preserve exact
evidence and stale state, and report provider absence explicitly. It must not
wait for native background indexing, confuse cached exact results with live
filesystem truth, grant administrative authority through a query session, or
freeze the JSONL laboratory codec as the production transport.

## Candidates

### A. Wait for full native indexing and packaging on all platforms

This maximizes platform evidence before integration but recreates the global
gate rejected by ADR-003. Orchestrator cannot exercise discovery, routing,
availability, or fallback until unrelated platform campaigns finish.

### B. Integrate directly against the current Go structs and JSONL methods

This starts quickly but turns implementation layout and development aliases
into accidental ABI. Missing operations and currentness limitations become
implicit consumer knowledge.

### C. Freeze a transport-neutral semantic v0 and gate optional capabilities

Stabilize the implemented query, inspection, status-snapshot, lifecycle, and
administrative meanings. Keep production framing, authentication, status
streaming, native currentness, lexical search, and true live traversal as
independently reported capabilities.

## Evidence and measurements

- **OBSERVED:** the engine implements canonical version, status,
  configuration, exact query, inspect, root plan/apply, manual reconcile,
  integrity, rebuild, and shutdown projections with bounded queries and typed
  failures.
- **OBSERVED:** the engine reports enumerated capability state and explicit
  manual, catching-up, volatile-current, coverage-incomplete, and unavailable
  currentness states.
- **MEASURED:** native macOS background reconciliation, event storms, mutation
  sequences, and a ten-minute idle run pass within the recorded M4 bounds.
- **OBSERVED compatibility only:** the bounded Windows adapter and full Windows
  package suite pass under Wine; Linux package suites pass under Lima. These do
  not substitute for native NTFS/ext4 promotion evidence.

Evidence locators:
`engine/results/M4_BACKGROUND_CURRENTNESS_001.md`,
`engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`, and
`engine/docs/INTEGRATED_DOGFOOD_READINESS.md`.

## Decision

Choose C.

### Frozen semantic v0

The following meanings are frozen for experimental cross-project
implementation:

- `ORC-LIF-001` engine projection: `engine.version`, `engine.status`, and
  `engine.shutdown`; restart remains supervisor process replacement.
- `ORC-ENG-001`: bounded exact `engine.query` and `engine.inspect`, including
  generation-bound continuation, exact evidence, partial/unavailable state,
  and stale provenance.
- `ORC-ENG-002`: `engine.configuration_get`, `engine.root_plan`,
  `engine.root_apply`, `engine.scan_reconcile`, `engine.integrity_check`, and
  `engine.projection_rebuild`. These are semantic operations, not grants of
  administrative authority.
- `ORC-ENG-003` snapshot vocabulary: generation, root state, integrity,
  currentness, observed/reconciled watermark, backlog, coverage, and warnings.
  The subscription operation and replay protocol remain a later minor-version
  capability.

Fields may be added only as optional compatible additions within major zero.
Changing identity, exact/stale meaning, terminal status, authority class, or
pagination generation rules requires a new major.

### Capability gates

Orchestrator I1 may begin when the semantic fixtures pass. Each runtime feature
is negotiated independently:

| Capability | I1 requirement | Current disposition |
|---|---|---|
| semantic v0 query/status contract | required | frozen for experimental implementation |
| engine runtime discovery/adapter | required for real routing, not kernel work | unavailable until implemented |
| checked cached exact query | optional provider lane | implemented by engine |
| manual reconcile then exact query | optional administrative lane | implemented by engine |
| true zero-catalogue live traversal query | required under ADR-008, separate from catalogue v0 | unavailable pending `ORC-ENG-004` implementation |
| native background observation | optional optimization | experimental and coverage-incomplete |
| committed-current watermark | optional promotion capability | unavailable |
| lexical/fuzzy/content channels | optional evidence lanes | unavailable or experimental as individually reported |
| status event subscription | optional diagnostics optimization | negotiating |
| production framed/authenticated IPC | required for installed deployment, not semantic adapter work | deferred |
| launchd/SCM/systemd packaging and native campaigns | required for platform promotion | dogfood/platform gates |

`manual_reconcile` means the engine can operate without continuous background
indexing. It does not mean a query streams the live filesystem without building
or consulting a catalogue. Orchestrator and frontend copy must preserve this
distinction.

### Packaging boundary

“Bundle the engine” means ship a separately versioned engine executable and its
service/install assets in the File Manager distribution. It does not mean link
the Go runtime into the trusted Rust process or collapse systemwide/user-scoped
authority. Development conformance may spawn the engine JSONL laboratory;
installed routing waits for the platform-authenticated local endpoint.

## Why the other candidates lost

- A repeats the circular global gate already rejected by ADR-003.
- B gives accidental Go names and JSON layout permanent authority and cannot
  express capability absence cleanly.
- Treating background currentness as mandatory would suppress useful checked
  cached search and manual reconciliation.
- Calling manual reconciliation “live unindexed search” would promise a method
  the engine does not implement.

## Consequences

- Orchestrator can implement discovery-independent adapter traits, fixtures,
  routing, availability, and stale-result policy now.
- The frontend can consume one stable result/status meaning whether results
  arrive through Orchestrator or the registered direct-engine fallback.
- Native platform work advances capability state and promotion evidence without
  forcing an `ORC-ENG-001` major change.
- A production wire format is still not frozen; API semantics and wire ABI have
  independent version namespaces.

## Reversal and migration path

The engine may replace storage, observation, and transport implementations
behind semantic v0. ADR-008 enters live traversal as the required separate
`ORC-ENG-004` contract family. If performance rejects Orchestrator
proxying, the direct engine route may become the hot path while retaining the
same semantic fixtures and Orchestrator capability authority.

## Unresolved edges

- Production local framing, discovery, peer authentication, and query/admin
  endpoint separation.
- Exact source-root authorization and multi-user policy for the systemwide
  engine.
- The exact `ORC-ENG-004` live-traversal schema, limits, and native promotion
  evidence required by ADR-008.
- Status subscription replay bounds.
- Native currentness, power-loss, packaging, and update campaigns per platform.
