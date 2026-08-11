# Engine API contract

Status: **ORC-ENG semantic v0 frozen for experimental implementation;
development JSONL and contained M4 installed `ENG1` projections OBSERVED;
general cross-platform transport promotion gated**.

The Go package `api` is the provider projection. Canonical cross-project meaning
lives in `../../orchestrator/spec/contracts/ENGINE_AND_KOLMOGROV.md`; neither side
defines storage through the contract.

## Transport

The conformance/CLI/AI transport is newline-delimited JSON over standard
input/output. It is deliberately easy to sandbox, record, replay, fuzz, and call
from any language. Production GUI/service IPC will project the same semantic
messages over a bounded authenticated platform-local transport. Its framing is
not frozen:

- Unix domain socket on macOS/Linux;
- named pipe or local authenticated transport on Windows;
- inherited stdio for supervised workers and test sandboxes.

The contained M4 profile uses bounded length-framed JSON over distinct
same-user authenticated Unix query/admin sockets (`ENG1` v1). That installed
projection is measured for the named host-bound File Manager service but does
not promote Windows/Linux transports or make framing semantic authority. Query
and administrative capabilities use distinct authority even if a development
launcher exposes both in one process.

Every request contains `id`, `method`, and optional `params`. Every response
contains the same `id` and exactly one of `result` or `error`. Unknown fields are
ignored within a protocol major version; unknown methods return a named error.

## Method families

### Always available

- `engine.version` — protocol, engine build, process instance, feature flags,
  and enumerated capability state.
- `engine.status` — lifecycle/work state, enacted configuration, roots,
  generations, backlog knowledge, capability state, and warnings.
- `engine.configuration_get` — enacted service configuration and stable digest.
- `engine.shutdown` — cancel, drain, and close one process instance; no
  source-file operation. Start/restart belong to the native supervisor.

The unqualified names remain development/fixture aliases during v0. Canonical
cross-project names use the `engine.*` family.

### Query surface

- `query` — scoped exact/lexical/fuzzy query with filters, limit, and cursor.
- `engine.query_live` — **OBSERVED development implementation**; bounded
  filename/path traversal over an authorized filesystem scope without building,
  persisting, consulting, or requiring a catalogue. It returns process-local
  expiring continuation cursors, exact live observations, work counters, and
  named unavailable paths under `ORC-ENG-004`. Native NTFS/ext4, million-entry,
  promotion evidence remains open beyond the contained M4 installed route.
- `inspect` — exact record and all stored evidence/provenance.
- `explain` — concise ordinary evidence, exclusions, staleness, and unavailable
  shards; complete query plans and internal candidate diagnostics require the
  diagnostic/development capability.
- `subscribe` — bounded status/generation events; never filesystem mutation.

### Administrative projection surface

- `engine.root_plan` — validate an approved-root configuration without
  applying it and return current/proposed configuration digests.
- `engine.root_apply` — compare the required expected configuration digest and
  apply an already user-approved root policy to the projection.
- `scan.reconcile` — observe and reconcile an approved root.
- `integrity.check` — verify pages, manifests, postings, ownership, and anchors.
- `projection.rebuild` — rebuild an erasable projection while preserving the last
  valid reader generation where possible.

Administrative calls mutate only engine state. They do not create, rename,
replace, or delete source files.

The experimental M4 status projection distinguishes `manual_reconcile`,
`baseline_required`, `reconciling`, `catching_up`, `current_volatile`,
`observation_coverage_incomplete`, and `observation_unavailable`. It reports
adapter source/epoch, observed and
reconciled positions, pending observation count/age, gap state, and whether the
watermark is durable. `coverage_incomplete=true` means an exact scan has caught
up to every delivered hint but the adapter cannot support an exact-current
claim; queries retain stale-root provenance. `backlog_known=false` is not an
empty queue. Until the
cursor is committed in the same manifest as exact components,
`watermark_durable=false` and every adapter start performs an authoritative
baseline scan.

See `SERVICE_LIFECYCLE_AND_CONFIGURATION.md` for state transitions,
configuration authority, native supervisor candidates, and the complete
capability ledger.

## Result contract

Every result contains:

- platform object reference and observed path;
- owning root and committed generation;
- unavailable/stale state;
- stable rank and calibrated certainty;
- an ordered list of channel evidence;
- exact/inferred provenance for each component;
- source anchor or matching field/offset where applicable.

Raw scores from unrelated channels are not comparable without a named
calibration. A future PRV-derived fusion layer receives truncated channel ranks
and emits a complete audit trace; it never overwrites original ranks.

## Implemented M1 exact subset

Status: **OBSERVED in the reference implementation; not the complete v0 query
language**.

- `root.plan` canonicalizes and validates existing directories beneath the
  mandatory development sandbox without changing service state. It now also
  reports current/proposed effective-configuration digests and change state.
- canonical `engine.root_apply` rejects a missing expected digest and a stale
  plan with `STALE_CONFIGURATION`. The development `root.apply` alias remains
  unconditional and is not an authorization or cross-project contract.
- root apply atomically publishes approved-root policy in process memory. A policy change marks
  reused projections stale until reconciliation.
- `scan.reconcile` performs a metadata-only full scan, prunes more-specific
  child roots, and publishes one immutable in-memory reader generation.
- `query` currently requires `filters.name` or `filters.path`. Optional exact
  filters are `kind`, `size_min`, `size_max`, `modified_after`, and
  `modified_before`; sort keys are `name`, `path`, `size`, and `modified`.
- Query pages are limited to 1,000 results. The reference candidate budget is
  100,000; exceeding it returns `RESOURCE_BUDGET_EXCEEDED`, never a silently
  truncated success.
- Cursors bind to the committed generation and normalized query. Page ranks are
  global within that cursor stream.
- Residual `text`, quoted literals, lexical/fuzzy channels, `explain`,
  `subscribe`, and `projection.rebuild` return a named unavailable error or are
  not advertised. Exact raw observed names are currently byte/case-sensitive;
  platform normalization/collation remains a separate fixture gate.
- `inspect` requires a path when one object has multiple hard-link bindings.
  `integrity.check` verifies the canonical object/binding digest, root state,
  and ownership invariants of the reference generation.

The reference implementation joins objects and bindings only for selected
results. In storage, intrinsic objects, parent/name bindings, relative path
order, and exact indexes remain separate.

## Implemented M2 persistent candidate

Status: **OBSERVED candidate behavior; API v0 remains unfrozen**.

- `--store-root` selects a one-root persistent service. The existing directory
  is canonicalized and must remain outside the indexed source root.
- Reconciliation publishes a versioned immutable segment through two checksummed
  manifest slots. On restart, full checked recovery selects the newest valid
  pair or reports fallback while serving the last valid generation.
- A complete reconciliation whose exact catalogue digest is unchanged returns
  the current generation with `published=false` and performs no durable
  publication. Explicit projection rebuild remains a forced checked publish.
- Exact query, inspect, ranking evidence, and generation-bound cursors use the
  same semantic API as the M1 reference. Reopening does not reconstruct the
  complete catalogue on the Go heap.
- The common exact-name/root-scope/path-order plan reads only the requested
  generation-bound window. Filters, child-root exclusions, alternate scopes,
  and alternate sorts retain exhaustive bounded validation.
- `version` advertises `immutable-generation-v1` only in persistent mode.
- `engine.version`, `engine.status`, `engine.configuration_get`, and
  `engine.shutdown` expose process identity, drain state, capability inventory,
  and effective configuration. A supervisor-style restart uses a new instance
  identity while reopening the same checked generation.

`projection.rebuild` performs a full authoritative scan through the same atomic
publication path and remains reachable when startup has no valid segment but a
safe manifest/header retains the approved root. Recovery now quarantines
rejected engine-owned artifacts after a valid fallback or replacement exists;
`status` reports the preserved-artifact count or an incomplete-quarantine
warning. When both manifests are unreadable, no root is inferred: an approved
root must be reapplied before `projection.rebuild` can run. The candidate does
not yet expose a quarantine administration method, migration execution,
committed background-watermark state, or production framed IPC. Its storage layout is
never a public API or Orchestrator ABI.

## Pagination and motion

- Cursors bind to a committed generation and normalized query plan.
- Repeating a cursor against the same generation is deterministic.
- A newer generation may produce a new result stream; it does not silently
  mutate an old cursor.
- Asynchronous provider evidence may produce a revised result set only when it
  increases calibrated certainty. The response names moved objects and reasons.
- UI selection/focus preservation belongs to the GUI, but the engine supplies
  stable object IDs and revision events required to do it correctly.

## Error families

Errors are machine-readable and include at least:

- invalid query or unsupported predicate;
- unapproved/outside root;
- unavailable, locked, stale, or newer-version shard;
- generation expired;
- integrity failure/quarantined shard;
- resource budget exceeded;
- provider timeout/failure;
- protocol/schema mismatch;
- method unavailable while rebuilding.
- stale configuration precondition.

No error is encoded as an empty successful result.

## Compatibility promise

The v0 protocol may change while the standalone engine is under construction.
Before parent integration, freeze a v1 corpus of request/response fixtures and
require old clients to pass against every compatible build. Storage migrations
and API compatibility are separate promises.
