# File Manager search engine

Status: **M1 exact reference plus M2 durable-generation candidate; not yet
promoted**.

This directory is the future purpose-built catalogue, indexing, and retrieval
backend for File Manager. It is designed to become independently testable,
benchmarkable, fuzzable, and callable through a narrow versioned API before the
parent application depends on it.

The production objective is not “a smaller SQLite.” It is a valid storage and
retrieval machine specialized around approved roots, platform file identity,
exact metadata, lexical postings, fuzzy candidates, evidence-preserving rank
fusion, offline catalogues, and deterministic recovery.

The grand architect has accepted the reference model, root/volume topology,
immutable-generation storage spine, update policy, ranking baseline, API
projection, external-volume policy, light-mode threshold, and initial resource
constitution. Every worker must read
[`docs/ARCHITECT_HANDOFF_001.md`](docs/ARCHITECT_HANDOFF_001.md) and
[`docs/ARCHITECT_HANDOFF_002.md`](docs/ARCHITECT_HANDOFF_002.md) before extending
the implementation.

## Current reference slice

- `api/` — transport-neutral public types and engine interface.
- `cmd/fileman-engine/` — sandbox-required standalone service entry point.
- `internal/catalog/` — immutable object-plus-binding reference generations,
  ordered exact-name/path indexes, root/volume manifest algebra, and canonical
  serialization.
- `internal/identity/` — typed macOS/Linux/Windows platform-observation adapters;
  Windows reparse-point identity remains an explicit gate.
- `internal/scan/` — metadata-only, capability-rooted reconciliation that does
  not follow directory symlinks or open file contents.
- `internal/exact/` and `internal/ranking/` — bounded exact candidates,
  metadata filters, deterministic ordering, generation-bound cursors, and
  evidence-preserving results.
- `internal/generation/` — versioned immutable object/binding/name/path/identity
  components, dual manifest slots, checksums, pinned readers, bounded lazy read
  cache, last-valid fallback, and cross-platform publication primitives.
- `internal/service/` and `internal/transport/` — root planning/application,
  reconciliation, status, query, inspect, integrity, JSONL projection, and an
  opt-in one-root persistent mode.
- `internal/ownership/` — longest-approved-root ownership router.
- `internal/sandbox/` — path containment guard for development and testing.
- `internal/similarity/` — an unimplemented, versioned candidate-channel seam
  for the gated production Kolmogrov transfer target; no hash/index/ranking
  method is admitted before its proof/vector/benchmark bundle arrives.
- `internal/workload/` — canonical generated logical corpora and digests shared
  by the reference engine and future immutable-segment/database controls.
- `docs/` — charter, API, storage, ranking, validation, and delivery guidance.
- `benchmarks/` — benchmark corpus and reporting contract.
- `testdata/` — generated/non-sensitive fixture boundary.
- `results/` — checked-in summaries and retained negative results; raw large
  outputs remain referenced by manifests rather than silently committed.

## Worker bootstrap

Go 1.24 or newer is required because metadata scanning uses `os.Root` to keep
filesystem traversal beneath the explicit sandbox even during symlink races.

```sh
go test ./...
go test -race ./...
go vet ./...
make cross-build
go test -run '^$' -bench . -benchmem ./...
```

Run the development service only against a disposable root:

```sh
go run ./cmd/fileman-engine --sandbox-root /absolute/disposable/root
```

The M2 persistent candidate additionally requires an existing, engine-owned
store directory outside the indexed source root:

```sh
go run ./cmd/fileman-engine \
  --sandbox-root /absolute/disposable/sandbox \
  --store-root /absolute/disposable/engine-store
```

The process speaks newline-delimited JSON on standard input/output. The M1
reference and persistent candidate expose `version`, `status`, `root.plan`, `root.apply`,
`scan.reconcile`, bounded exact `query`, `inspect`, `integrity.check`, and
`shutdown`. Persistent reconciliation publishes a checked off-heap generation
and restart selects the last valid manifest/segment pair. It does not yet
implement quarantine, incremental event ingestion, lexical text, fuzzy
search, framed production IPC, or live-plus-committed merging.

## Definition of standalone completion

The engine is ready for parent integration only when:

1. its API conformance suite passes from a separately built client;
2. the exact identity/mutation oracle passes on APFS, NTFS, and ext4;
3. root ownership and child-shard pruning are deterministic under moves;
4. crash injection cannot expose a partly committed generation;
5. corruption is detected, bounded, quarantined, and rebuildable;
6. exact, lexical, fuzzy, and optional ranking channels expose evidence;
7. benchmark claims beat or deliberately lose to named mature controls;
8. the service remains absent/disabled without harming navigation;
9. packaging requires no network access and no runtime database service;
10. a versioned release artifact, schema manifest, and migration/rebuild policy
    exist.

Start a worker with [HANDOFF_PROMPT.md](docs/HANDOFF_PROMPT.md), then apply the
mandatory accepted direction in
[ARCHITECT_HANDOFF_001.md](docs/ARCHITECT_HANDOFF_001.md) and
[ARCHITECT_HANDOFF_002.md](docs/ARCHITECT_HANDOFF_002.md).
