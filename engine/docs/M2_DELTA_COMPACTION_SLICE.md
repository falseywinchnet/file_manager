# M2 bounded delta and compaction slice

Status: **GIVEN bounded immutable runs; CANDIDATE mechanics pending control
comparison**.

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
3. Measure all retained candidates and preserve failures.
4. Select run bound, merge strategy, and compaction trigger through a numbered
   decision record owned by the grand architect.
5. Only then introduce a new manifest/schema major and migration/rebuild path.

The future Kolmogrov/ConeDAG fixed-width channel remains a separately versioned
optional component. Delta mechanics reserve versioned component descriptors;
they do not guess its vectors, hashing, or candidate semantics.
