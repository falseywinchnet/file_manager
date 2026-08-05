# M4 background currentness 001

Status: **OBSERVED portable/native experimental slice and MEASURED macOS
sandbox dogfood; not committed-watermark or cross-platform admission**.

Date: 2026-08-05. Host: Apple M3, 8 GiB, macOS 14.8.7 (23J520), APFS,
Go 1.26.5 darwin/arm64. The native measurement used disposable temporary
source/store directories. Command:

```sh
FILEMAN_ENGINE_MEASURE=1 go test ./benchmarks \
  -run '^TestM4FSEventsBackgroundCurrentnessDogfood$' -count=1 -v
```

## Implemented boundary

- A native batch carries an adapter source, random subscription epoch, opaque
  ordered position, the exact preceding cursor, and an explicit discontinuity
  bit. Positions need not be contiguous. Duplicate/replayed batches do not
  create work; a broken chain, epoch change, adapter loss, native overflow,
  root invalidation, malformed batch, or coalescer overflow forces an
  authoritative scan.
- Events are bounded hints keyed by approved root and canonical relative path.
  Rename input retains old and new dirty addresses; create/remove/recreate
  histories remain ambiguous until the filesystem scan. Events never establish
  platform identity or final existence.
- Default retained-state bounds are 4,096 events per adapter batch, 4,096
  operations per flush, 8,192 coalesced addresses, and 2 MiB of conservatively
  charged address state per active buffer. Ingestion remains active during a
  scan, so at most two coalescer buffers exist. Overflow drops hint detail and
  schedules a full scan rather than growing memory.
- The macOS adapter uses one recursive FSEvents stream, a 50 ms native latency,
  four bounded raw callback slots, two output slots, and at most 4,096 events or
  1 MiB of path bytes per callback. The callback never blocks; handoff overflow
  becomes a discontinuity. Roots are canonicalized once, and native
  must-scan/user-drop/kernel-drop/ID-wrap/root-change/mount flags invalidate the
  incremental hint path. There is no polling fallback.
- The service separates callback ingestion from authoritative scanning.
  Observations received during a scan remain pending and require a subsequent
  pass before currentness advances. Failed scans retry with bounded exponential
  backoff. With no pending state, no timer is armed.
- Status distinguishes manual, baseline-required, reconciling, catching-up,
  process-local current, and observation-unavailable states. It reports
  observed/reconciled cursors and backlog age/count. Adapter stop changes
  backlog knowledge to false rather than reporting an invented zero.

## Correctness evidence

Deterministic tests cover sparse but correctly chained positions, replay,
out-of-order/gap and epoch changes, native-reported discontinuity, malformed
and oversized batches, memory overflow, duplicate path coalescing,
rename/delete/recreate histories, operation/age triggers, scan failure, root
policy change, adapter stop, and root invalidation without cursor advance.

A blocked-scan service fixture delivers another mutation while the first scan
is in flight. Status remains non-ready with both observations accounted for;
the engine performs two scans and advances only through the second cursor.
Race instrumentation passes. Separate native tests create a file beneath a
temporary macOS root, observe a chained FSEvents batch, drive the service to a
new exact generation, and query the file without a stale marker.

## Measurement

The retained candidate used 50 ms FSEvents latency, 100 ms maximum observation
age, and the 4,096-operation safety trigger. Two same-revision repetitions of
32 sequential file writes, each waiting for exact-current status, measured:

| event-to-current | Result |
|---|---:|
| p50 | 163.281-163.404 ms |
| p95 | 173.712-175.957 ms |
| max | 183.578-226.394 ms |

A 4,096-write storm across 256 paths converged in 441.467-467.930 ms with three
to four exact full-generation publications, equal observed/reconciled FSEvents
positions, and post-GC-to-settled `HeapAlloc` deltas of 1,828,352-1,889,128
bytes. This heap delta is a process observation, not a peak-RSS or exact
ownership attribution.

The engine then remained quiet for 2.001-2.002 seconds: generation, file count,
aggregate store bytes, and maximum durable-file modification time were
unchanged, so application durable writes were zero. The checked store held four
files and 72,792 bytes. This short interval proves the no-timer/no-write path;
it does not satisfy the required ten-minute idle CPU below 0.1%, battery, or
device-power gate.

### Retained negative tuning result

An earlier 25 ms native latency, 25 ms maximum age, and 256-operation trigger
completed the same storm in 539.553 ms but published 20 full generations. It is
**REJECTED as the default full-generation batching envelope** because it spends
far more application and metadata writes for no useful convergence gain. The
result does not reject a future cheap delta-run trigger at a separately
measured boundary.

## Durability and remaining gates

`current_volatile` is intentionally weaker than committed currentness. The
live generation manifest has no source cursor or root-policy revision, so a
clean or unclean restart always performs a full baseline scan before making a
currentness claim. No cursor sidecar or per-event journal was added. This
preserves the accepted requirement that cursor, policy, generation, schema,
and component digests advance in one manifest commit.

Still open:

- architect/Orchestrator reconciliation of the live manifest watermark and
  root-policy revision, followed by interruption/recovery tests;
- live tiered delta publication so a sustained storm does not rewrite full
  generations;
- native Windows and Linux adapters and their journal/overflow semantics;
- moves across roots/volumes, hard-link identity mutation, detach/reattach,
  permission churn, and large-tree scan/event overlap campaigns;
- ten-minute idle CPU/write/RSS, sustained-backlog, battery/power, and source
  operation versus application/physical write accounting;
- install/start/restart/stop behavior under launchd, SCM, and systemd.

## Verification

- **PASS:** `go test ./...` and `go test -race ./...` on macOS/arm64, including
  real FSEvents temporary-root tests;
- **PASS:** `go vet ./...`;
- **PASS build:** Linux/amd64 and Windows/amd64 with `CGO_ENABLED=0`; the
  FSEvents constructor returns explicit unavailable on those targets.
- **PASS compatibility oracle:** portable observation and service package tests
  as Windows/amd64 binaries under Wine 11.10, and Linux/arm64 binaries in the
  Lima guest with temporary files on guest `/var/tmp`.

Wine and Lima remain compatibility environments only and are not native
Windows/Linux observation, durability, or power evidence.

Implementation: `internal/observation`,
`internal/observation/fsevents`, `internal/service/background.go`, and
`cmd/fileman-engine`. Measurement:
`benchmarks/m4_background_currentness_darwin_test.go`.
