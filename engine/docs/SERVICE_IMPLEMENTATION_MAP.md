# Engine service implementation map

Status: **OBSERVED development implementation map and M4-only installed
projection; DECIDED semantic boundaries; general production adapters remain
gated**.

This document maps the running Go backend after the service-coordination
refactor. It describes implementation ownership, not a new cross-project ABI or
storage decision. Canonical cross-project meaning remains in the Orchestrator
contract registry and `API_CONTRACT.md`.

The navigable source atlas is
[`library/index.html`](library/index.html). It is generated from Go source plus
reviewed status/boundary language in `LIBRARY_MANUAL.json`, works directly from
`file://`, and has an AI-readable Markdown mirror.

## Service source map

| File | Responsibility | Boundary |
|---|---|---|
| `internal/service/service.go` | façade state, construction, durable recovery, close | Owns coordination only; storage representations stay in their packages |
| `internal/service/lifecycle.go` | admitted-operation accounting, drain, shutdown | Restart is supervisor process replacement, not an in-process method |
| `internal/service/configuration.go` | version and effective-configuration projection | Digest is compare-and-apply identity, not authorization |
| `internal/service/capabilities.go` | truthful runtime capability ledger | Experimental/deferred work is not advertised as production-available |
| `internal/service/roots.go` | root plan/apply and stale-plan check | Applies already-authorized policy; it does not grant authority |
| `internal/service/reconciliation.go` | authoritative scan and conditional/forced publication | One complete checked generation is published or the previous reader remains |
| `internal/service/query.go` | exact query, live query, inspect, integrity | Catalogue and live lanes remain source-distinct |
| `internal/service/status.go` | root/generation/readiness/currentness snapshot | Incomplete observation reduces readiness honestly |
| `internal/service/background.go` | experimental native observation controller | Events are hints; gaps invoke authoritative reconciliation |
| `internal/service/similarity_generation.go` | private exact-generation lease for candidates | Candidate projection never owns identity or public query semantics |

## Locking and lifetime rules

The service has four deliberately separate synchronization domains:

1. `lifecycle.mu` admits operations only while the instance is `ready`, counts
   query/admin work, supplies lifetime cancellation, and signals drain.
2. `admin` serializes root-policy mutation, reconciliation/publication,
   rebuild, similarity-generation acquisition, and final reader close.
3. `readerMu` protects the current durable reader and generation floor. Query
   and inspect hold a read lock for the complete checked-reader operation;
   publication swaps readers under the write lock.
4. background and live-query managers own their internal controller/session
   locks. Live sessions are reset on root-policy change and closed on shutdown.

When locks nest in the service path, the order is `admin` before `readerMu`.
Lifecycle accounting surrounds the operation but does not remain locked while
the operation executes. Background controller locks are not held while a full
reconciliation is running.

## Construction and recovery

`New` requires a non-nil sandbox guard and creates the in-memory exact reference
store. `NewPersistent` additionally opens an existing engine-owned store outside
the indexed source root, reads the monotonic generation floor, probes the newest
checked header/manifest, restores its approved root only after containment
validation, recovers the last complete generation, and preserves rejected
artifacts through quarantine when a valid reader exists.

Installed startup rechecks authority at two distinct layers. The guard binds
the actual opened root object to the manifest identity for scan and live-query
traversal. The deployment `ADMISSION` record binds the recovered checked
generation to the current host/root/exclusion digest; mismatch forces a scan
and checked publication before local endpoints open. Installed reconcile and
rebuild acknowledgements advance that record to their resulting generation.

**OBSERVED limitation:** persistent mode currently admits one root. Root policy
is not acknowledged as durable until reconciliation commits a checked
generation. The live tiered manifest, committed observation watermark, and
root-policy revision remain open work.

## Request paths

### Exact catalogue query

1. Transport decodes one bounded JSONL or authenticated `ENG1` frame and maps a
   canonical method.
2. Lifecycle admits a query operation and binds it to service-lifetime
   cancellation.
3. Service snapshots approved-root routing and pins the current reader (or the
   immutable in-memory reference snapshot).
4. `internal/exact` validates the query/cursor, enforces candidate/page bounds,
   applies filters and deterministic ordering, and returns exact matches.
5. `internal/ranking` projects named exact evidence without manufacturing a
   later relevance-fusion claim.
6. Response names the committed generation, next cursor, plan, stale roots,
   and currentness warnings.

### Catalogue-independent live query

1. Service verifies the root is already in policy, the requested starting scope
   is not excluded, and the already-opened root retains its admitted identity.
2. `internal/live` creates or resumes a process-local session with a 30-second
   expiry and a 32-session process ceiling.
3. Metadata-only traversal stays within result, visit, stat, wall-time,
   open-directory/depth, response-byte, and unavailable-path bounds.
4. Directory symlinks are objects but are not traversed. Matching paths receive
   an exact platform observation at read time.
5. Response uses `source=live_filesystem`, discovery order, work counters, and
   explicit partial/unavailable state. It has no catalogue generation and makes
   no catalogue write.

### Reconcile and rebuild

Both operations serialize as administrative work and use the same authoritative
scanner and checked publication path. Ordinary reconcile suppresses an
unchanged durable publication. Rebuild forces a checked replacement. Publication
swaps the current reader only after the new generation validates; the previous
reader stays usable until that point.

## Error and authority boundary

The API exposes machine-readable faults while retaining internal causes only in
the process. Empty success never substitutes for invalid input, unavailable
state, budget exhaustion, stale configuration, expired generation, or integrity
failure. The development JSONL loop is sequential. The M4 projection has
same-uid authenticated query/admin Unix sockets with distinct credentials, 32
query and 4 admin connection slots, and bounded handshake, idle, request, and
write time. Multiplexed cancellation and the general system-service transports
remain gates.

## Build and developer installation

From `engine/`:

```sh
make docs
make check
make build
make install PREFIX=/explicit/prefix
```

`make install` copies the development binary and static atlas. It does not
register launchd, systemd, or SCM state and therefore does not claim that the
gated production service adapter exists. The binary still refuses to run
without `--sandbox-root`. Build, distribution, temporary, coverage, raw-result,
profile, and test artifacts are ignored by `engine/.gitignore`; checked-in
documentation and evidence summaries remain visible to Git.

## Documentation maintenance contract

- `docs/LIBRARY_MANUAL.json` owns reviewed package summaries, epistemic status,
  invariants, architecture layers, runtime flows, protocol status, and operator
  commands.
- `tools/generate_library_docs.py` owns deterministic source inventory, HTML,
  sidebar search data, declaration/source locations, test entry points, and the
  Markdown mirror.
- `tools/test_generate_library_docs.py` requires every Go package to have manual
  metadata and verifies the service façade remains split at the intended seams.
- `make docs-check` rejects stale output. Generated HTML is checked in beside
  source; it is documentation, not a hidden build artifact.
