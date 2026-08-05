# Durability, power-loss, and write-economy program

Status: **GIVEN correctness under failure; CANDIDATE acknowledgement policy and
platform flush adapters; MEASURED logical crash evidence only**.

## Position

The filesystem is the authority for file identity and content. The engine
index is a rebuildable exact projection. That does not make corruption
acceptable: after interruption the engine must select one complete checked
generation, explicitly serve an older/stale generation, or report that no
checked generation exists. It must never assemble a plausible mixture.

The recommended contract is:

1. **DECIDED:** publication is an immutable run followed by a small manifest
   commit; readers pin complete generations.
2. **GIVEN:** native events are coalesced. There is no file sync, manifest
   rewrite, or status-journal append per event.
3. **CANDIDATE:** ordinary background acknowledgement means “logically
   committed through the platform's documented file/directory flush path.” A
   hostile or lying device may still roll the projection back; restart detects
   the observation-watermark gap and reconciles from the filesystem.
4. **REJECTED:** spending additional writes to make derived index data more
   durable than its source or the OS event journal.
5. **REJECTED:** weakening checksum, generation, or fallback rules to save
   writes. Write economy comes from batching and deltas, not ambiguous state.

The Orchestrator owns approved policy. The engine persists only the projection
and the source-observation watermark incorporated into it; it must not create a
second per-event authorization journal.

## Failure contract

For every publish, exactly these post-restart outcomes are admissible:

| Interruption point | Admissible outcome |
|---|---|
| before new manifest commit | previous checked generation |
| after new manifest commit | new checked generation, or previous generation with an explicit stale/reconciliation state if the platform/device lost acknowledged writes |
| newest run or manifest torn/corrupt | previous checked generation plus preserved diagnostic evidence |
| no generation validates | unavailable projection; retain safe root evidence where possible and require authorized rebuild |
| native event watermark missing or discontinuous | checked generation may remain queryable as stale; reconciliation is mandatory before claiming current state |

Generation number, source-observation watermark, root policy revision, schema
version, and every component digest belong to the same manifest commit. None may
advance independently.

## Publication write path

The current full-generation control performs:

1. sequential temporary segment write;
2. segment header finalization and file sync;
3. rename to immutable name and directory sync;
4. alternate-slot manifest write and file sync;
5. rename over that slot and directory sync;
6. reader publication and later reclamation of unpinned debris.

The delta candidate keeps this ordering but replaces the full segment with a
bounded change run. Compaction writes one sequential replacement base and
publishes it through the same manifest rule. Foreground query never triggers
compaction and waits only for the short manifest exclusion.

**CANDIDATE batch policy:** a bounded volatile coalescer triggers on whichever
comes first: maximum observation age, maximum operation count, or memory
budget. Concrete ages/counts are experiment points. A quiet engine emits no
commit. A sustained burst writes sequential runs rather than repeatedly
rewriting the same objects.

## Platform durability adapters

These mechanisms need native-filesystem measurement before a decision:

| Platform | Current observation | Candidate experiment |
|---|---|---|
| macOS/APFS | Go `File.Sync` plus file rename and directory sync; no native power-cut evidence | compare `fsync`, `F_BARRIERFSYNC`, and `F_FULLFSYNC` at run/manifest boundaries; do not put full sync on every observation |
| Linux/ext4 | file `fsync`, rename, and parent-directory `fsync` | process-kill and power-cut campaign across mount modes with device write cache recorded |
| Windows/NTFS | `File.Sync`, then `MoveFileExW(..., MOVEFILE_WRITE_THROUGH)`; portable directory sync is absent | native service campaign using NTFS, `FlushFileBuffers`, write-through rename, and real Windows fault injection |

Apple documents `F_FULLFSYNC` as a request to flush drive buffers and also
warns that forced synchronization can be expensive and remains best effort on
fallible hardware. Microsoft likewise warns that `FlushFileBuffers` after every
small write is inefficient. Linux documentation requires syncing the parent
directory when durability of a newly named file matters. Those are source
claims, not measurements of this engine.

No adapter may silently treat Wine-on-APFS or a Lima virtual disk as native
NTFS/ext4 power-loss evidence. They remain API/compatibility oracles.

## SSD and filesystem write accounting

Every update/compaction result reports, separately:

- canonical logical change bytes (the fixed denominator);
- application data bytes written, including temporary/dead output;
- manifest/header bytes and rewrites;
- file create, rename, unlink, and directory-sync counts;
- file-sync/barrier/full-sync counts and latency distribution;
- live bytes, peak temporary bytes, and reclaimed bytes;
- host-reported physical bytes written when a trustworthy counter exists;
- amortized compaction bytes and write amplification;
- CPU time, peak/retained memory, query p95/p99 under overlap, and backlog age.

The admission gate remains less than 3x application durable-byte amplification
for representative 4,096- and 10,000-operation batches, including amortized
compaction. Physical NAND amplification is reported when observable but is not
invented from filesystem write counts. TRIM, controller caching, snapshots,
encryption, compression, and virtual-disk backing must be named because they
can separate application bytes from device wear.

## Write-minimizing rules

- Keep frequently changing delta runs separate from the mostly static base.
- Encode each run sequentially in one pass with a bounded final header rewrite.
- Open a candidate run lazily on its first change; a no-change reconciliation
  performs no create, write, rename, sync, or removal.
- Alternate small manifest slots; do not rewrite data pages in place.
- Coalesce duplicate observations for the same exact object/path before the
  durable batch without losing rename/delete semantics.
- Bound run count through measured background compaction; never compact merely
  because a query arrived.
- Retain at most the current/previous committed generations plus pinned readers
  and explicit quarantine evidence. Reclaim only after publication and pins.
- Keep status subscriptions memory-resident and bounded; reconnect uses a
  snapshot, not a durable notification log.
- Do not persist caches, query telemetry, access-time-like data, or periodically
  refreshed “still alive” timestamps.
- Do not use direct/unbuffered I/O, memory mapping, compression, filters, or a
  larger hash solely by intuition. Admit each against the same correctness,
  write, CPU, memory, and latency workload.

The future Kolmogrov “ultimate hashing” component is a versioned optional
descriptor in a run/manifest. Its absence must not change exact identity or
recovery. Its update cost and write amplification must pass this same program.

## Power-loss and disk-full campaign

Promotion requires all layers, with negative results retained:

1. deterministic short-write and fault-hook interruption at every write, sync,
   rename, directory-sync, manifest, quarantine, and compaction boundary;
2. SIGKILL/process termination at those boundaries on the native filesystem;
3. fixed-capacity native filesystem exhaustion before/during run, manifest,
   quarantine, and compaction output;
4. VM hard-stop tests as a reproducible intermediate oracle, explicitly
   labeled by guest filesystem and host backing store;
5. physical/native APFS, NTFS, and ext4 power-loss tests on expendable hardware
   or an equivalent independently validated fault appliance;
6. recovery comparison against the exact reference digest and the source
   observation watermark after every restart.

The invariant is stronger than “the process reopened”: old or new exact state
must match the reference digest; partial state, generation reuse, silent event
gaps, and destruction of the last diagnostic copy fail the campaign.

## Open decision for architect reconciliation

The remaining product-level choice is whether a successful background commit
promises survival of ordinary OS crash only, or also requests the strongest
available device-cache flush before acknowledgement. The recommendation is to
measure both and default to the strongest platform-documented ordering that
does not violate background latency/power budgets. Regardless of that choice,
batching—not weaker integrity—is the primary SSD-wear control.

## Source locators

- Current implementation: `internal/generation/store.go`,
  `internal/generation/segment.go`, `internal/generation/syncdir_unix.go`, and
  `internal/generation/syncdir_windows.go`.
- Existing logical evidence: `results/M2_DURABILITY_002.md`.
- Delta workload and gates: `docs/M2_DELTA_COMPACTION_SLICE.md`.
- Apple: <https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/fsync.2.html>
  and <https://developer.apple.com/documentation/xcode/reducing-disk-writes>.
- Microsoft: <https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-flushfilebuffers>
  and the `MoveFileEx` `MOVEFILE_WRITE_THROUGH` documentation.
- Linux: `fsync(2)` in the kernel.org Linux man-pages; it explicitly requires
  parent-directory `fsync` for durable directory entries.
