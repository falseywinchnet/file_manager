# ADR-001: Engine reference model and storage spine

Status: **accepted**.

Date: 2026-08-05.

Owner approval: the grand architect accepted ENG-01 through ENG-12 as
recommended, with the ENG-10 external-volume and network-machine correction
recorded below.

## Question

What exact model and architectural target should direct the standalone Go
engine beyond its in-memory reference catalogue, while leaving codecs and
low-level optimizations answerable to measurements?

## GIVEN constraints

- The engine is a standalone Go service and purpose-built backend.
- Filesystem objects are authoritative. Paths are observed addresses and
  content hashes express duplicate/version/similarity relations.
- The most-specific approved root exclusively owns an object.
- Core indexing covers names and intrinsic metadata. Content-derived evidence
  enters through independently erasable optional providers.
- Public interrogation/plugin APIs do not mutate source files.
- Indexing, external roots, and offline catalogues require explicit consent.
- Navigation survives engine absence. Search may degrade honestly.
- The shipped engine does not delegate its store to a general database; mature
  stores remain correctness and performance controls.

## Workloads and failure modes

The decision must survive rename, hard links, replace-at-path, event gaps,
service downtime, nested approved roots, removable and network volumes,
millions of records, giant generated directories, low-power operation, torn
commits, corruption, migration, and concurrent readers. It must not obtain a
fast median by loading all records into Go heap objects, writing every watcher
event, destabilizing result order, or weakening exact identity.

## Decision

### ENG-01 — Object plus path bindings

Represent an observed file object separately from its path bindings.

- The object carries the volume identity, native file identifier, incarnation
  evidence, object type, and intrinsic metadata.
- A binding connects an object to a parent object and exact observed name.
- One object may have several hard-link bindings.
- A rename changes bindings; replace-at-path creates a new incarnation.
- A symbolic link is an object distinct from its target.
- Platform evidence remains typed and inspectable rather than compressed into a
  false lowest-common-denominator identifier.
- Compact ordinals are generation-local posting keys, never durable API
  identity.

### ENG-02 — Approved-root shards under volume manifests

The approved root is the ownership, privacy, erasure, rebuild, accounting, and
corruption-containment shard. A lightweight volume manifest supplies the volume
identity, availability, mount state, observation watermark, and routing needed
by its root shards. It is not a redundant volume-wide copy of their records.

An independently approved child root prunes its subtree from the parent shard.

### ENG-03 — Bounded immutable generational segments

The durable target is a purpose-built immutable-segment store:

- catalogue, path-binding, ordered-metadata, lexical, and optional-channel
  segments are logically distinct;
- a root generation is published by a small atomic manifest only after its new
  files validate and reach the required durability boundary;
- readers pin committed generations;
- deletions and replacements use explicit tombstones until bounded compaction;
- the number of readable delta generations is bounded;
- root components may be quarantined and rebuilt independently where their
  manifest dependencies permit;
- compaction is never initiated by a foreground query.

This selects the storage spine, not its final block size, compression, posting
codec, fanout, filter, checksum, cache, or mapping policy. Copy-on-write B+ tree
and SQLite implementations remain controls capable of falsifying the target on
declared workloads.

### ENG-04 — Commit observation watermarks, not every event

The engine does not normally maintain a second durable copy of every filesystem
event. A committed root generation records the native journal cursor or adapter
watermark it incorporates. Events since that point may be accumulated and
coalesced in bounded memory.

- A queue overflow or non-replayable discontinuity marks a gap.
- A gap schedules authoritative scan reconciliation.
- Adapters without persistent replay reconcile after downtime rather than
  claiming completeness.
- Losing an unpublished batch may cause bounded staleness; it may not expose a
  partly committed generation.

### ENG-05 — Adaptive bounded publication

Publication is driven by a quiet window, bounded event/byte thresholds, an
explicit synchronization request, clean shutdown opportunity, or maximum age.
Initial candidate policy:

- ordinary quiet-window publication after 2–5 seconds;
- maximum ordinary lag of 15 seconds;
- sustained event storms publish no more often than once per 30 seconds;
- battery or thermal pressure lengthens the interval and prohibits compaction;
- File Manager's own operations need not force an engine commit merely to make
  the GUI reflect an action it already knows occurred.

Queries bind to committed generations. An ephemeral query overlay is deferred
unless measurements show the accepted lag is inadequate.

### ENG-06 — Specialized structures by role

The initial target structure family is:

- sorted immutable object-key blocks with an optional compact hash directory
  for exact platform-object lookup;
- sorted `(parent, normalized name, exact name)` binding blocks;
- ordered or compressed posting structures for size, time, type, and other
  sortable metadata;
- front-compressed sorted term blocks as the reference dictionary;
- immutable inverted postings for lexical retrieval;
- sparse integer lists for sparse filters and bitmaps only when density wins;
- Bloom/XOR filters only as measured negative-lookup accelerators.

An FST, memory mapping, direct I/O, custom page cache, block skipping, or
vectorized decoder must win an equivalence-preserving measurement gate. The
service may not require a resident Go heap object for every indexed record.

### ENG-07 — Evidence tiers before voting fusion

Initial ranking uses deterministic evidence tiers:

1. exact filename or exact path;
2. exact normalized filename;
3. contiguous prefix, final-token, or path-component match;
4. verified fuzzy filename similarity;
5. structured metadata evidence;
6. weaker lexical association;
7. segregated plugin, content, or semantic evidence.

Within the fuzzy lane, enumerate adjacent transpositions, use character grams
for wider candidate generation, and verify with a weighted bounded
Damerau–Levenshtein calculation. Extension, token-boundary, prefix, and order
preservation are named evidence. Recency breaks genuine evidence ties. Content
does not outrank a strong name result.

**Amendment by ADR-002 / architect handoff 002:** these structures are reference
controls and the interim baseline. A rigorously transferred
Kolmogrov/ConeDAG-derived fixed-width mechanism is the intended core fuzzy and
structural candidate channel, subject to its formal, conformance, adversarial,
and performance gates. It is not the deferred AI-semantic provider.

The architect's future voting method may fuse mature channel rankings. It does
not replace their raw evidence or the initial deterministic baseline.

### ENG-08 — Bounded migration, stable export

- The exact catalogue reads/migrates the current and immediately previous major
  format unless a later ADR changes the window.
- Lexical, fuzzy, thumbnail, and provider projections remain disposable.
- An offline coarse catalogue is migratable because its source may be absent.
- The stable inspection surface is an exported interchange representation, not
  the internal page format.
- Upgrade and repair preserve the last valid reader generation when possible.

### ENG-09 — One API model, inspectable and local projections

The transport-neutral API has two initial projections:

- JSONL over inherited standard streams for tests, replay, CLI, diagnostics,
  and AI tooling;
- locally permissioned socket/named-pipe transport using explicitly framed JSON
  for the GUI/service relationship.

A binary payload is admitted only after JSON costs violate a measured budget.
Query and administrative capabilities remain separate. Ordinary clients receive
concise evidence, availability, and staleness; complete query plans are a
diagnostic/development surface. Plugins normally reach search through their
capability broker rather than receiving the administrative endpoint.

### ENG-10 — External, network, and other-machine placement

For a directly attached removable/local external volume, offer only:

- **B:** full index on the volume, with no locally retained offline catalogue;
  or
- **D:** no persistent catalogue, using reduced live search while mounted.

Do not offer **A**, a locally retained full index of a removable external
volume. Do not offer a local coarse inventory for these volumes. The accepted
cost is that File Manager will not tell the user which absent removable drive
contains a remembered collection.

For a network drive or another machine, use only **C semantics** when that source
is explicitly admitted:

- the full index remains with the network source/other machine;
- File Manager retains the explicitly approved coarse local catalogue required
  to locate unavailable remote material;
- local coarse records are names, paths, types, sizes, and timestamps only;
- unavailable results appear visibly unavailable;
- no full content, thumbnail, or semantic projection is silently mirrored.

Internal approved roots store their shards in per-user application data on the
same machine. All choices still obey setup-time consent and default-no root
admission.

### ENG-11 — Large-directory light mode

There is no hard total-root indexing limit. At 65,536 immediate children, a
directory enters light mode by default:

- retain identity, binding/name, type, size, and basic timestamps;
- suppress thumbnails, plugin extraction, expensive fingerprints, and
  speculative fuzzy expansion;
- expose the active policy and allow the user to change the threshold.

### ENG-12 — Initial performance constitution

These are accepted rejection targets to be measured on named representative
corpora and hardware:

- cold service ready with manifests opened: under 100 ms;
- idle private memory: at most 48 MiB at one million records and 96 MiB at ten
  million, with no complete per-record heap mirror;
- idle CPU: under 0.1% averaged over ten minutes;
- no index disk writes while the filesystem is unchanged;
- exact/prefix query at one million records: p95 under 8 ms, p99 under 20 ms;
- exact/prefix query at ten million records: p95 under 25 ms, p99 under 50 ms;
- fuzzy filename query at one million records: p95 under 35 ms;
- warm GUI-to-first-correct-result: under 50 ms;
- aggregate name/basic-metadata index target: under 384 bytes per object on the
  declared representative corpus;
- update write amplification: under 3× serialized changed projection bytes over
  a 24-hour mixed workload, including amortized compaction;
- HDD remains a first-class guard;
- foreground search never initiates compaction.

These thresholds are architecture gates, not measurements already achieved.
Experiment records must define corpus, machine, filesystem, cold/warm state,
and the precise accounting denominator.

## Why the other candidates lost

- One-record-per-path models cannot honestly represent hard links, rename, and
  replacement simultaneously.
- Volume-only shards do not match consent and erasure boundaries.
- In-place tree mutation and per-event journaling impose random-write and
  recovery machinery inconsistent with the desired batched projection model.
- A monolithic score hides exactness and makes asynchronous evidence motion
  difficult to explain.
- A permanently public binary format freezes implementation before the engine
  earns its representation.
- Local full copies of removable-volume indexes retain more information than
  the chosen utility justifies.
- A hard large-directory refusal removes search where search is most valuable.

## Consequences

M1 proceeds as the exhaustive object-plus-bindings reference implementation and
identity oracle. M2 implements the immutable-generation target and compares it
against the declared controls. Later stages may optimize individual structures
only against the reference digest and accepted resource constitution.

## Reversal and migration path

If the immutable-generation target fails crash integrity, tail latency, write
amplification, or footprint after its strongest reasonable implementation, a
new ADR may replace its physical store while preserving the object/binding
model, root-shard contract, generation semantics, API, and exported fixtures.
Internal formats are intentionally not the public compatibility boundary.

## Unresolved edges

- Exact platform incarnation evidence and continuity limits on APFS, NTFS, and
  ext4 require oracle results.
- Block sizes, checksums, compression, segment fanout, compaction triggers, and
  cache policy remain measured choices.
- The exact local transport authentication mechanism is platform work.
- The representative performance corpora and HDD reference machine must be
  frozen before performance claims can be promoted to **MEASURED**.
- The remote/network catalogue protocol and trust model remain a later bounded
  design; ENG-10 decides placement and disclosure, not network authority.
