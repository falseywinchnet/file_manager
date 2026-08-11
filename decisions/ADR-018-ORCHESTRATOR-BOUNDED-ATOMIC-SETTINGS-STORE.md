# ADR-018: bounded atomic Orchestrator settings store

Status: **accepted for File Manager 1.0 implementation and contained M4 dogfood**.

Date: 2026-08-10.

Owner approval: the grand architect opened File Manager implementation, permitted
the required Orchestrator extensions, and explicitly cleared the remaining
implementation blockers for the 1.0 ascent. This record selects the reversible
low-volume physical store required by that direction; it does not select a
universal hive or provider database.

## Question

What physical store should back the first bounded `ORC-SET-001` core settings
transactions while preserving optimistic revision checks, crash-safe atomic
publication, recovery, offline reproduction, and a later migration path?

## GIVEN constraints

- Orchestrator owns schemas and committed values; File Manager and the CLI use
  the same transaction.
- The first profile has at most 256 scalar fields and a 128 KiB canonical
  snapshot. Bulk provider generations, semantic memory, plugin data, and Engine
  storage are excluded.
- Every transaction supplies an expected revision. Invalid or stale mutations
  publish nothing.
- A commit publishes all changed values or none, fsyncs its file and containing
  directory, retains one verified previous snapshot, and returns a durable audit
  identity.
- The store is private to the user, rejects symbolic-link store objects, and
  never contains plaintext secrets. Secret settings remain unavailable until a
  platform-secret-handle contract passes.
- The M4 contained profile must be reproducible without adding a database
  runtime or network fetch.

## Workload and measurement

`orchestrator/tools/measure_settings_store.py` ran on the M4 Mac mini on
2026-08-10. The fixed workload used 48 Boolean fields, 300 one-field commits,
100 eight-field commits, and full durability on every transaction. It compared
an atomically replaced JSON snapshot plus previous snapshot with SQLite WAL in
`synchronous=FULL` mode.

| Candidate | Single p50 / p95 / p99 / max | Batch-8 p50 / p95 / p99 / max | Bytes after checkpoint | Recovery probe |
|---|---|---|---:|---|
| atomic canonical JSON | 0.299 / 0.385 / 0.398 / 0.402 ms | 0.322 / 0.388 / 0.406 / 0.408 ms | 930 | corrupt primary recovered verified previous snapshot |
| SQLite WAL FULL | 0.038 / 0.061 / 0.106 / 3.295 ms | 0.040 / 0.050 / 0.059 / 0.070 ms | 49,152 | rollback and integrity check passed |

**MEASURED:** SQLite has lower ordinary commit latency. **MEASURED:** both are
well below a 10 ms administrative-settings budget at this workload. The probe
does not simulate sudden power loss, storage-media faults, multi-process writers,
or schema migration; those limits remain explicit.

## Candidates

### A. One canonical JSON snapshot with atomic replacement and one backup

The daemon validates a bounded typed document in memory, writes and fsyncs a
private create-new temporary file, atomically replaces the primary, then fsyncs
the directory. Before publication it atomically refreshes a verified previous
snapshot. Startup validates the primary and recovers only from the verified
previous snapshot when necessary.

### B. SQLite with WAL and full synchronous commits

Use separate metadata and value tables with an optimistic revision predicate.
This has stronger future query/migration machinery and the best measured commit
latency, but adds a native dependency and a much larger physical mechanism for
the bounded scalar workload.

### C. Platform preference APIs

Use `NSUserDefaults`, Windows Registry, or desktop settings services. This is
convenient but forks physical semantics by platform, weakens exact recovery
fixtures, and does not give Orchestrator one portable transaction contract.

## Decision

Choose A for the bounded File Manager 1.0 core-settings profile.

The selection is based on mechanism size, offline reproducibility, inspectable
recovery, and the fact that its measured worst commit is 0.408 ms—not on a claim
that JSON is faster. SQLite remains the preferred migration candidate if the
bounded ceiling is exceeded, multi-process writers are admitted, or settings
and operational registries require indexed queries.

The document has a format version, schema revision, monotonic value revision,
monotonic audit sequence, and exact stable-ID/value map. Unknown fields, invalid
types, oversized documents, duplicate mutations, and revisions outside the
accepted horizon fail closed. Defaults remain in the executable schema rather
than being silently copied forward as stored authority.

## Failure modes and recovery

- A stale expected revision returns `stale` with the current revision and no
  write.
- Invalid value/type/bounds returns `invalid` with field diagnostics and no
  write.
- A write, fsync, rename, or directory-fsync failure returns an internal fault;
  the last atomically published primary remains authoritative.
- A malformed primary is never treated as defaults. A valid previous snapshot
  is recovered and republished with explicit recovery provenance; if neither is
  valid, the service is unavailable and preserves both for diagnosis.
- Orphan `.next-*` files are non-authoritative and removed only after their
  private leaf/type checks pass.
- Reset is a normal optimistic transaction to the declared default, not file
  deletion.

## Rejected options

- Reject B for this bounded first profile because its measured speed advantage
  is unnecessary and does not justify another native runtime/store boundary yet.
- Reject C because it makes the same semantic transaction platform-dependent
  and complicates contained recovery evidence.
- Reject line-oriented append logs as the primary because replay, compaction,
  duplicate terminal records, and torn-tail policy are more mechanism than one
  bounded snapshot requires.

## Reversal path

The semantic contract exposes only typed schemas, revisions, transactions,
recovery provenance, and audit identities. A future ADR may migrate the exact
validated snapshot into SQLite or another measured store by reading one format
version, publishing the new store atomically, retaining the old artifact until
verification, and keeping the same `ORC-SET-001` meanings.

## Unresolved edges

- sudden-power-loss and physical disk-fault campaigns;
- Windows and Linux private-directory/no-follow implementations;
- platform secret handles;
- a durable general audit event store beyond the settings commit identity;
- representative plugin schemas and namespace quotas;
- migration beyond format 1.
