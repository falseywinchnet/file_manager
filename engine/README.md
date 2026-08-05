# File Manager search engine

Status: **M1 exact reference, M2 durable-generation candidates, and an
experimental M4 macOS background-currentness slice; not yet promoted**.

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
  cache, last-valid fallback, evidence-preserving quarantine, bounded streaming
  generation diff, a standalone checksummed delta-run experiment, and
  cross-platform publication primitives. A separate caller-bounded
  base-plus-one-run exact overlay is measured but not connected to the live
  manifest or service. Digest-chained multi-run, checked disk-index sidecars,
  tiered exact query, and checkpoint-free cohort-compaction candidates are
  likewise measured but unadmitted.
- `internal/service/` and `internal/transport/` — process-instance lifecycle,
  effective configuration with stale-safe root apply, capability inventory,
  reconciliation, status, query, inspect, integrity, JSONL projection, and an
  opt-in one-root persistent mode.
- `internal/ownership/` — longest-approved-root ownership router.
- `internal/observation/` — bounded chained-cursor coalescing plus opt-in native
  macOS FSEvents and Windows `ReadDirectoryChangesW` adapters. Events remain
  hints; gaps and overflow invoke authoritative scans. The live watermark is
  volatile, and both native adapters currently declare incomplete exact-current
  coverage rather than hiding gaps with periodic scans.
- `internal/sandbox/` — path containment guard for development and testing.
- `internal/similarity/` — the versioned candidate-channel seam plus a disabled
  native dogfood adapter for Kolmogrov's sealed literal radius-one coupled
  history tuple. It has cross-language vectors, bounded exact
  edit/transposition verification, a checked off-heap component index, and a
  private pinned exact-generation service lease; no fuzzy method is advertised
  or admitted to the public query path yet.
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

On macOS, an explicit disposable-root dogfood instance can enable the
experimental native adapter without a polling loop:

```sh
go run ./cmd/fileman-engine \
  --sandbox-root /absolute/disposable/sandbox \
  --store-root /absolute/disposable/engine-store \
  --root-id dogfood \
  --root-path /absolute/disposable/sandbox/source \
  --background-observation=fsevents
```

The engine performs a baseline scan before assessing currentness. The measured
macOS adapter reports `observation_coverage_incomplete` even after it catches up
because final-hard-link removal coverage is unproved. Windows builds can select
`--background-observation=rdcw`; that bounded adapter also reports incomplete
coverage pending native NTFS journal/root-replacement work. Other platforms
reject either constructor as unavailable. No platform silently falls back to
polling.

The process speaks newline-delimited JSON on standard input/output. The M1
reference and persistent candidate expose the development aliases plus
canonical `engine.version`, `engine.status`, `engine.configuration_get`,
stale-safe `engine.root_plan`/`engine.root_apply`, reconciliation/rebuild,
bounded exact query/inspect, integrity, and draining shutdown. Persistent
reconciliation publishes a checked off-heap generation and restart selects the
last valid manifest/segment pair. An unchanged authoritative scan returns the
current generation without a segment/manifest write; explicit rebuild still
forces a checked replacement. It does not yet
implement live delta-run publication/compaction, a committed observation watermark,
lexical text, live fuzzy search, framed production IPC, or live-plus-committed
merging. A standalone delta writer/reader is measured but is not referenced by
the live manifest or service query path; the measured one-run overlay has the
same separation. Multi-run/consolidation experiments retain that separation;
eight runs are the strongest query/heap point, but repeated consolidation and
same-trigger full-base replacement both fail cumulative write gates. No
production bound, compaction, or pacing policy has been selected. The tiered
cohort schedule now has a checked disk-indexed exact view and checkpoint-free
streaming compactor: indexed writes, retained memory, exact state, and the warm
relative-p99 query gate pass. An isolated `TIERED.*` manifest also passes
logical/subprocess interruption and repair tests, but the live manifest,
long-horizon, and overlap trials remain open. Recovery quarantine is internal and
status-visible; it is not yet a public administrative API. Native launchd,
SCM, and systemd installation plus authenticated query/admin endpoints remain
explicit gates; see
[`docs/SERVICE_LIFECYCLE_AND_CONFIGURATION.md`](docs/SERVICE_LIFECYCLE_AND_CONFIGURATION.md).
The current integration-readiness rubric is
[`docs/INTEGRATED_DOGFOOD_READINESS.md`](docs/INTEGRATED_DOGFOOD_READINESS.md).

ADR-008 now requires `ORC-ENG-004`: bounded filename/path search directly over
an authorized filesystem scope when no catalogue is available. The current
`scan.reconcile`-then-query path does not satisfy that requirement. Engine
negotiation round 006 specifies progressive pages, containment, cancellation,
work ceilings, zero durable writes, and native measurement gates. The provider
continues to report `engine.live.query` unavailable until that operation and
its conformance evidence exist.

The first Kolmogrov history-tuple transfer is internally dogfooded against a
pinned persistent-service generation but remains unavailable to service
clients. Its projection is disposable, random-parameterized per generation,
and exact-ordinal anchored. It is one bounded descriptor/candidate tool within
the full engine; exact identity and stored records remain authoritative. Judged
quality/control, atomic similarity-manifest publication, update/compaction
write economy, and Orchestrator contract gates must close before public
admission.

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
11. bounded catalogue-independent live name/path search passes the
    `ORC-ENG-004` native conformance and resource gates.

Start a worker with [HANDOFF_PROMPT.md](docs/HANDOFF_PROMPT.md), then apply the
mandatory accepted direction in
[ARCHITECT_HANDOFF_001.md](docs/ARCHITECT_HANDOFF_001.md) and
[ARCHITECT_HANDOFF_002.md](docs/ARCHITECT_HANDOFF_002.md).
