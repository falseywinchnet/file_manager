# Purpose-built storage-engine program

Status: **GIVEN custom engine; immutable-generation spine DECIDED; codecs and
accelerators remain CANDIDATE**.

The controlling direction is `ARCHITECT_HANDOFF_001.md`: approved-root shards
use bounded immutable committed generations under volume manifests. The
candidate families retained below are controls and sources of submechanisms;
they are no longer coequal production directions.

## Required logical stores

The engine presents one service but keeps distinct logical authorities:

1. **Root manifest:** root identity, policy, exclusions, child-shard boundaries,
   platform volume identity, availability, and committed generation.
2. **Exact catalogue:** object identity, incarnation, observed paths, stat-like
   metadata, link relations, timestamps, and deletion tombstones needed for
   reconciliation.
3. **Ordered metadata:** normalized name/path components, sizes, dates, types,
   and stable sort/page keys.
4. **Lexical index:** term dictionary, document/field frequencies, positional
   postings, phrase offsets, and corpus statistics.
5. **Fuzzy structures:** character grams, deletion candidates, or another
   measured bounded vocabulary structure.
6. **Optional similarity channels:** fixed-width sketches and their candidate
   indexes, each versioned and disposable.
7. **Offline catalogue:** explicitly retained coarse records and availability;
   never silently created.

Logical stores may share pages and commits. They retain separate erasure,
integrity, and version identities.

## Reference-first construction

The first engine is intentionally exhaustive and inspectable:

- canonical records with stable serialization;
- sorted vectors and binary search controls;
- simple open-addressed or standard Go maps for exact transient lookup;
- uncompressed offset-bearing postings;
- full invariant checking after every mutation in tests;
- whole-store digest and per-block checksums;
- slow reference query path used as the correctness oracle.

Only after equivalence is established should the worker introduce custom pages,
prefix compression, FST dictionaries, posting blocks, skip data, bitmaps,
Bloom/XOR filters, memory mapping, direct I/O, or vectorized decoding.

## Durable-generation implementation problem

The custom store must implement and prove the selected immutable-generation
publication mechanism. Comparison/control families include:

- copy-on-write page roots plus atomic manifest replacement;
- bounded redo/intent record plus in-place or shadow-page commit;
- **DECIDED target:** immutable data runs with a small atomic manifest and
  bounded compaction;
- an append-only commit region plus periodic canonical checkpoint.

The architect rejected constant per-event journaling as the ordinary ingestion
policy. Persist the incorporated native observation watermark with the committed
generation; a gap invokes reconciliation. A committed engine generation must
survive power loss without presenting a mixture of old and new state.

Every candidate must specify:

- commit point and fsync order;
- torn-write assumptions;
- checksums and version fields;
- reader pinning and generation reclamation;
- idempotent replay or rollback;
- manifest recovery when newest and older copies are damaged;
- compaction interruption;
- disk-full behavior;
- migration and downgrade behavior;
- scan reconciliation after any event gap.

## M2 immutable-generation candidate 001

Status: **OBSERVED implementation and MEASURED component evidence; not a final
codec decision**.

`internal/generation` currently writes one immutable root segment with six
separate components: intrinsic objects, bindings, exact-name bytes, ordered
paths, name order, and object-identity order. The fixed header and every
component carry SHA-256 checksums; the small manifest binds the whole segment
digest, reference-catalogue digest, counts, root, generation, and schema major.

Publication is:

1. write, checksum, and sync the temporary segment;
2. publish its final name;
3. durably publish that directory entry (`fsync` on Unix; write-through rename
   on Windows);
4. write and sync the alternate manifest slot;
5. atomically publish the manifest slot and its directory entry.

Readers pin immutable files. Two manifest slots preserve the prior generation;
unreferenced segments are reclaimed only after all in-process pins close.
Recovery validates candidate headers, component checksums, and the manifest's
whole-file digest, reports rejected newer candidates, and falls back to the
last valid generation. A bounded lazy direct-mapped cache retains at most 8 MiB
per pinned reader and is bypassed by streaming integrity checks.

Known gaps remain release-blocking: there is no quarantine policy, no per-block
lazy checksum for queries before whole-store validation,
no disk-full/process-kill compaction campaign, no schema migration, no bounded
delta/compaction implementation, and no hardware power-loss result. The
checked-recovery scan misses the one-million <100 ms target while the bounded
manifest/header probe meets it; service readiness semantics must retain that
distinction.

## Root ownership

The ownership router uses most-specific approved-root matching. The durable
manifest must additionally solve:

- child root admitted beneath an already indexed parent;
- child root removed or made unavailable;
- subtree moved across roots or volumes;
- root path renamed while platform identity survives;
- overlapping policy edits published atomically;
- parent-scope query fan-out without duplicate objects;
- offline child catalogue routing.

No query may return both the parent projection and child projection of the same
owned object.

## Mature controls

SQLite/WAL/FTS5, Xapian, sorted vectors, and flat scans remain comparison
controls. They do not enter the shipped runtime unless an explicit later
decision reverses the custom-backend requirement. Controls must use the same
records, collation, predicates, update batches, and query judgments.

The custom engine earns its existence by jointly improving File Manager
correctness, tail latency, footprint, write economy, or inspectability—not by
winning a microbenchmark that omits durability.
