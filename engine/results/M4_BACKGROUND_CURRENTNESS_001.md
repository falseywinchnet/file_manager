# M4 background currentness 001

Status: **OBSERVED portable/macOS/Windows experimental slice, MEASURED macOS
sandbox dogfood, and OBSERVED Wine compatibility; not exact-current,
committed-watermark, or cross-platform admission**.

Date: 2026-08-05. Host: Apple M3, 8 GiB, macOS 14.8.7 (23J520), APFS,
Go 1.26.5 darwin/arm64. The native measurement used disposable temporary
source/store directories. Command:

```sh
FILEMAN_ENGINE_MEASURE=1 go test ./benchmarks \
  -run '^TestM4FSEventsBackgroundCurrentnessDogfood$' -count=2 -v
FILEMAN_ENGINE_MEASURE_HARDLINK=1 go test ./internal/observation/fsevents \
  -run '^TestMeasureFSEventsFinalHardLinkRemoval$' -count=3 -v
FILEMAN_ENGINE_LONG_MEASURE=1 go test ./benchmarks \
  -run '^TestM4FSEventsTenMinuteIdle$' -count=1 -v -timeout=12m
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
- The macOS adapter uses one recursive file-level FSEvents stream, a 50 ms native latency,
  four bounded raw callback slots, two output slots, and at most 4,096 events or
  1 MiB of path bytes per callback. The callback never blocks; handoff overflow
  becomes a discontinuity. Roots are canonicalized once, and native
  must-scan/user-drop/kernel-drop/ID-wrap/root-change/mount flags invalidate the
  incremental hint path. There is no polling fallback. Native measurement did
  not deliver final-hard-link removal, so the adapter declares incomplete
  exact-current coverage.
- The Windows adapter uses one overlapped `ReadDirectoryChangesW` handle and
  one fixed 64 KiB buffer per approved root, one completion port, four bounded
  raw batches, and two output batches. Zero-byte completion, malformed native
  records, queue overflow, handle failure, or root invalidation becomes a
  discontinuity. It has no polling fallback or per-directory watch tree. It
  declares incomplete coverage because it has no durable journal cursor and
  does not yet guard each approved root against parent-level replacement.
- The service separates callback ingestion from authoritative scanning.
  Observations received during a scan remain pending and require a subsequent
  pass before currentness advances. Failed scans retry with bounded exponential
  backoff. With no pending state, no timer is armed.
- Status distinguishes manual, baseline-required, reconciling, catching-up,
  process-local current, coverage-incomplete, and observation-unavailable
  states. It reports
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
temporary macOS root, observe a chained FSEvents batch, and drive the service
to a new exact generation. The mutation oracle covers rename, same-volume
move, replace-at-path, hard-link creation and unlink, delete/recreate, and
repair after a later observed event. Because adapter coverage is incomplete,
queries retain a stale-root marker and service readiness remains false.

**MEASURED negative coverage result:** three APFS repetitions with file-level
FSEvents observed hard-link creation and non-final unlink but did not deliver
the final remaining-link path within two seconds. A restart baseline cannot
reconstruct that lifetime history. The service therefore fails closed for the
entire adapter, including roots with no currently visible multiple binding;
it does not use periodic full-tree scans to conceal the gap. This observation
does not claim that Apple's API promises the omission. The relevant native API
surface documents file-level delivery and hard-link flags:
<https://developer.apple.com/documentation/coreservices/kfseventstreamcreateflagfileevents>
and
<https://developer.apple.com/documentation/coreservices/file_system_events/1455361-fseventstreameventflags>.

## Measurement

The retained candidate used 50 ms FSEvents latency, 100 ms maximum observation
age, and the 4,096-operation safety trigger. Two same-revision repetitions of
32 sequential file writes, each waiting for the exact catalogue to reconcile
through the latest observed position while status remained coverage-incomplete,
measured:

| event-to-current | Result |
|---|---:|
| p50 | 161.767-163.695 ms |
| p95 | 200.987-228.183 ms |
| max | 225.147-332.021 ms |

A 4,096-write storm across 256 paths converged in 338.253-489.390 ms with three
exact full-generation publications, equal observed/reconciled FSEvents
positions, and post-GC-to-settled `HeapAlloc` deltas of 1,562,160-1,914,768
bytes. This heap delta is a process observation, not a peak-RSS or exact
ownership attribution.

The short post-storm interval remained quiet for 2.002 seconds: generation,
file count, aggregate store bytes, and maximum durable-file modification time
were unchanged, so application durable writes were zero. The checked store held
four files and 72,792-72,794 bytes.

A separate ten-minute idle run measured 600.008 seconds wall time, 39.927 ms of
total process user-plus-system CPU, or 0.006654% of one core. Generation stayed
at one and application durable writes were zero. `HeapAlloc` changed by -6,328
bytes, `HeapInuse` by -73,728 bytes, and process maximum RSS was 10,076,160
bytes. Maximum RSS is a high-water mark, not an adapter ownership attribution.
This passes the declared below-0.1% one-core idle CPU and zero-application-write
gate; battery/device power and physical-device write accounting remain open.

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
- native NTFS validation and a Windows journal/root-replacement coverage
  strategy; a Linux adapter and its journal/overflow semantics;
- moves across roots/volumes, detach/reattach, permission churn, and large-tree
  scan/event overlap campaigns;
- sustained-backlog, battery/power, and source operation versus
  application/physical write accounting;
- install/start/restart/stop behavior under launchd, SCM, and systemd.

## Verification

- **PASS:** `go test ./...` and `go test -race ./...` on macOS/arm64, including
  real FSEvents temporary-root tests;
- **PASS:** `go vet ./...`;
- **PASS build:** Linux/amd64 and Windows/amd64 with `CGO_ENABLED=0`; the
  FSEvents constructor returns explicit unavailable on those targets.
- **PASS compatibility oracle:** all 14 test-bearing Windows/amd64 packages
  under Wine 11.10, including direct `ReadDirectoryChangesW` create/cancel and
  service-level notification/reconcile/query fixtures; all 14 test-bearing
  Linux/arm64 packages in the Lima guest with temporary files on guest
  `/var/tmp`.

Wine and Lima remain compatibility environments only and are not native
Windows/Linux observation, durability, or power evidence.

Implementation: `internal/observation`,
`internal/observation/fsevents`, `internal/observation/rdcw`,
`internal/service/background.go`, and
`cmd/fileman-engine`. Measurement:
`benchmarks/m4_background_currentness_darwin_test.go` and
`benchmarks/m4_idle_darwin_test.go`.
