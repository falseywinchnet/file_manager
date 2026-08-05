# M2 bounded delta and compaction slice

Status: **GIVEN bounded immutable runs; naive repeated consolidation and
same-trigger base replacement REJECTED; disk-indexed tiered cohort query and
streaming compaction implemented and measured; warm relative query gate and
isolated atomic-manifest recovery pass; long-horizon levels and live service
publication remain open**.

## Fixed direction and measured reason

- **GIVEN:** ordinary ingestion may not fsync one engine transaction per native
  event. Events are coalesced, a committed observation watermark identifies
  gaps, and a gap invokes authoritative reconciliation.
- **DECIDED:** the durable spine is immutable committed runs under a small
  atomic manifest with bounded background compaction.
- **MEASURED:** on `scale-v1` at one million files plus 100,000 directories, a
  one-object metadata change made the full-snapshot control write at least
  164,201,438 bytes. Against the 64-byte canonical changed object record this
  is 2,565,647.47x amplification. The replacement shard took 1.433 s to build
  and 212.959 ms to publish on the named M2 macOS environment.
- **MEASURED:** the path-ordered streaming diff found that one change among
  1,100,000 bindings in 260.420 ms. It allocated 55,924,224 bytes over the run,
  retained 6,324,224 bytes after GC as bounded reader-cache growth, and did not
  materialize a change array.

The full snapshot remains the compaction/rebuild control. It is **REJECTED** as
the ordinary small-update publication path by measured write amplification, not
as the authoritative recovery mechanism.

## Workload contract

Every delta candidate must run these deterministic cases against the same base:

1. one metadata update;
2. 4,096 and 10,000 mixed metadata updates;
3. creates and deletes at both ends and the middle of path/name order;
4. rename represented as old-path deletion plus new-path addition;
5. hard-link add/remove and intrinsic metadata change shared by two bindings;
6. directory subtree churn crossing the light-mode threshold;
7. interrupted delta publication and interrupted compaction at every durable
   boundary;
8. corrupt newest delta, corrupt base, missing run, disk full, and stale native
   observation watermark.

The canonical denominator for write amplification is the uncompressed logical
operation stream: operation tag, fixed object and parent identity/metadata,
length fields, and exact path/name bytes. Candidate durable bytes include all
temporary run bytes written, header rewrites, manifest bytes, and compaction
output; filesystem metadata writes are reported separately when the platform
can measure them. This prevents a codec from claiming a win by choosing its own
denominator.

## Retained candidates

### Path-keyed immutable overlay run

**CANDIDATE:** store path-sorted add/update rows and deletion tombstones, plus
name and identity orders for live rows. Newer runs shadow older paths. Exact
path lookup checks newest to base; ordered queries merge run iterators while a
bounded shadow set suppresses replaced paths. The run must retain parent
identity so compaction can reconstruct object/binding authority.

Open questions are the maximum run count, shadow-set representation, whether
name/identity suppression is cheaper as path probes or a measured filter, and
how light-directory counters are updated. Values such as four, eight, or
sixteen runs are experiment points, not architecture decisions.

**MEASURED component evidence:** a heap overlay collapsed 2/4/8/16 checked runs
into their net changed-path union while leaving the million-record base
off-heap. Eight runs at up to 80,000 disjoint changes are the strongest tested
point. Sixteen is **REJECTED for this representation**: the 10,000-change case
retained 52,674,472 bytes for the overlay alone and the 4,096-change case missed
the paced-consolidation overlap bound. Run count, cumulative changes, net paths,
and retained bytes must all be bounded; see
`results/M2_MULTI_RUN_CONSOLIDATION_001.md`.

### Net-run consolidation

**CANDIDATE:** consolidate a checked run chain into one path-ordered run against
the same immutable base before replacement-base compaction is economical. A
catalogue-digest chain prevents combining runs from unrelated exact states.
Canceled cross-run changes emit no net record. The first measured cycle was
about 2.31x application bytes before manifest and replacement-base costs.

This is not permission for repeated rewriting. **MEASURED:** enforcing the
eight-run bound by repeatedly rewriting the growing net run reached 3.52x by
cycle three for both required batch sizes. Replacing the full 250,000-record
base at those same trigger points measured 3.60x–9.60x at the failing points.
Both policies are **REJECTED for this representation/workload**. Alternate
amortization, smaller independently compacted components/shards, a lower-memory
leveled representation, crash campaign, and explicit background scheduler
remain open.

### Size-tiered cohort compaction

**CANDIDATE:** replace each completed cohort of eight fresh runs with one net
run relative to the state before that cohort. Do not rewrite earlier cohorts.
At 22 disjoint epochs the chain is two cohort runs plus six fresh runs: eight
visible runs, about 2.00x cumulative application bytes for both required batch
sizes, and exact digest equivalence after every generation.

The existing heap overlay is not the retained query representation. It kept the
complete net changed-path union and retained 79.2 MB for the 10,000-change
case. That representation is **REJECTED**. The schedule now has checked on-disk
path/name/identity indexes, bounded read caches, and a streaming cohort
compactor. At 22 epochs the indexed form measured 2.42x final and at most 2.80x
cohort-boundary application bytes, with 12.1/16.7 MB retained heap growth. It
no longer uses checkpoint oracles or a full changed-path union. Bounded
immutable proof caches bring measured warm path p99 to 0.12-0.32x base-only
and repeated-name p99 to 0.16x; cold path and 25,000-match name costs remain
separately reported. See `results/M2_TIERED_INDEXED_COMPACTION_004.md`.

After enough cohorts, visible run count again needs another level or base
replacement. The 22-epoch result does not establish that long-horizon policy.
See `results/M2_TIERED_COHORT_COMPACTION_003.md`.

### Copy-on-write ordered tree

**CANDIDATE control:** copy only changed search paths/pages and publish a new
root. It must run the identical record, crash, cache, and query workload. It is
not rejected merely because immutable overlay runs are the selected spine; it
is the required reversal control for write and read amplification.

### SQLite control

**CANDIDATE control only:** use the existing full-record schema and add the
identical update batches. System SQLite remains outside the shipped Go module.

## Admission gates

A delta format is not added to the production manifest until it demonstrates:

- exact digest equivalence after every batch and after compaction;
- no partial generation at any publication/compaction crash boundary;
- less than 3x durable write amplification for the 4,096- and 10,000-operation
  batches, with the one-record result reported separately;
- at least 10x fewer durable bytes than full-snapshot publication for one
  changed record;
- exact-query p99 no more than 2x the base-only p99 at the admitted maximum run
  count;
- no query-triggered compaction and no foreground wait on compaction other than
  the short manifest commit exclusion;
- bounded retained memory for diff, merge, and compaction, with cancellation;
- current and previous schema-major recovery or an explicit rebuild migration
  accepted before the schema major changes.

## Implementation order

1. **OBSERVED:** cancellable, path-ordered `generation.Diff` with O(1) change
   retention and exact add/update/delete tests.
2. **OBSERVED and component-MEASURED:** standalone candidate delta-run writer,
   checksummed reader, exact add/update/delete replay, cancellation, corruption,
   truncation, and no-op suppression without changing the live v1 manifest.
   At one million files plus 100,000 directories, the one-record candidate was
   536 live bytes and had a 792-byte write lower bound including its header
   rewrite, versus 164,201,437 bytes for the full snapshot. It allocated
   49,601,264 bytes over the path merge and retained 0 additional bytes after
   GC. Standalone 4,096- and 10,000-update runs measured 1.1610x and 1.1604x
   run-level amplification against the declared logical-operation denominator,
   also with 0 additional retained heap after GC. These exclude manifest
   publication, query merging, crash recovery, and amortized compaction, so
   they do not pass the complete admission gate yet.
3. **OBSERVED and component-MEASURED:** a generation-pinned exact overlay for
   one checked base plus one checked run, bounded by a caller-supplied change
   budget. Mixed 4,096- and 10,000-change workloads at one million files
   matched the reference state and query transcript. The named warm run
   retained about 1.0/2.27 MB, measured 1.342x/1.289x exact-path p99 and
   0.385x/0.394x repeated-name p99 versus base-only, and kept run-only write
   amplification at 1.1578x/1.1572x. It neither selects a multi-run bound nor
   includes publication and compaction. See
   `results/M2_OVERLAY_CANDIDATE_001.md`.
4. **OBSERVED and component-MEASURED:** digest-chained multi-run exact overlay
   and one net-run consolidation cycle at 2/4/8/16 runs. Paired quiescent query
   ratios stayed below 2x. Eight runs passed the paced overlap point for both
   batch sizes; sixteen failed the representation's memory/overlap boundary.
   First-cycle write amplification was about 2.31x, excluding manifest,
   repeated consolidation, and replacement-base compaction.
5. **MEASURED negative policy evidence:** three actual eight-run enforcement
   cycles at 250,000 records. Cumulative run-plus-consolidation amplification
   was 2.31x, 2.93x, then 3.52x. Actual replacement bases written instead of
   the current consolidation also failed the complete byte gate at the named
   triggers. See `results/M2_REPEATED_CONSOLIDATION_002.md`.
6. **MEASURED retained schedule / rejected query representation:** cohort-only
   tiering measured about 2.00x after 22 epochs with eight visible runs. The
   current heap overlay retained 79.2 MB at 10,000-change scale and is
   rejected.
7. **OBSERVED and MEASURED bounded replacement:** checked 36-byte/change exact
   sidecars, aggregate 8 MiB read-cache budget, a 400,000-byte maximum immutable
   name-result cache, an O(run-count) tiered view, and checkpoint-free streaming
   cohort compaction. Write, retained-memory, exact-state, and warm relative
   query gates pass at both required batches. See
   `results/M2_TIERED_INDEXED_COMPACTION_004.md`.
8. **OBSERVED isolated durability continuation:** dual checked `TIERED.*`
   candidate slots pass logical/subprocess interruption, partial-write,
   fallback, sidecar repair, newer-schema rejection, and pin-aware reclamation.
   They do not alter the live service manifest. See
   `results/M2_TIERED_MANIFEST_RECOVERY_005.md`.
9. Measure long-horizon levels, diverse/cold/background-overlap queries, live
   manifest integration, and the copy-on-write/SQLite controls;
   preserve failures.
10. Select run/change/memory bounds, merge strategy, pacing, and compaction
   trigger through a numbered
   decision record owned by the grand architect.
11. Only then introduce a new live manifest/schema major and migration/rebuild
    path.

Delta mechanics are shared by exact, lexical, and independently versioned
content/fragment descriptor components. A Kolmogrov/ConeDAG fixed-width
channel may occupy one bounded component where its workload evidence admits
it. The storage mechanics do not guess any channel's vectors, hashing, or
candidate semantics, and no channel replaces exact catalogue authority.
