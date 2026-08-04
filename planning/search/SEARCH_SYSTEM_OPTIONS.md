# Search system option map

Status: **CANDIDATE catalogue; no selections**. Date: 2026-08-03.

## 1. The system is several systems

```text
platform observations -> reconciliation log -> exact catalogue generation
                                                |       |        |
                                      equality/range  lexical  fingerprints
                                                \       |        /
                                                 query planner
                                                      |
                                    exact + lexical + structural candidates
                                                      |
                                calibrated ranking + explanations + cutoff
                                                      |
                                          exact file-object results

optional extractors -> disposable derived records -> optional semantic hive
                                                    -> local AI query compiler
```

The exact catalogue answers "which object was observed, where, and in which
generation?" An index answers "which exact records should be considered?" A
ranker answers "in what order?" A semantic hive answers "which derived
interpretations, user concepts, and provenance links are available?" None may
impersonate another layer.

## 2. Exact identity and catalogue algorithms

| Mechanism | Correct use | Gain | Loss / failure mode | Required gate |
|---|---|---|---|---|
| Runtime hash table | Exact object-ID/path-key lookup | Expected constant-time lookup; simple | Memory overhead, nondeterministic iteration, adversarial collision policy, poor range queries | Equality workload at 1m/10m records; memory and worst-tail behavior |
| Sorted vector + binary search | Immutable or batch-built exact maps | Compact, cache-friendly, deterministic | Insert/delete is expensive; snapshots or delta layer required | Read-heavy baseline; batch rebuild and delta cost |
| B/B+ tree | Durable ordered keys, ranges, pagination | Stable ordering, prefix/range lookup, page locality | Page splits, write amplification, collation complexity | Rename/event storm plus range and pagination workloads |
| LSM tree | High update throughput and immutable read generations | Sequential writes, batching, background merge | Compaction stalls, tombstones, read amplification, space amplification | Event storm, delete/rename, crash, compaction p99 and writes/day |
| Radix/ART/trie | Path/name prefix lookup | Prefix operations and shared prefixes | Unicode/case normalization, node overhead, persistence complexity | Prefix-heavy filename corpus versus B+ tree and FST |
| Minimal/FST dictionary | Compact immutable term/prefix dictionary | Excellent locality and compression | Rebuild-oriented; dynamic updates require segmenting | Vocabulary size, prefix/typo fanout, segment merge cost |
| Bloom/XOR filter | Negative membership check before disk access | Tiny fast rejection | False positives; never authoritative; rebuild complexity | Miss-heavy queries and segment count |
| Roaring/word-aligned bitmap | Filters and posting intersections | Fast AND/OR, good dense/sorted integer compression | Sparse small sets may lose; ordinal-generation management | Type/root/time filter intersections and memory |

**REJECTED as category errors:** a hash is not durable identity across object
replacement; a path is not universal identity across rename/hard links; a tree
does not provide relevance; a Bloom filter cannot prove presence.

## 3. Lexical candidate algorithms

| Mechanism | Best role | Gain | Loss / edge | Gate |
|---|---|---|---|---|
| Inverted postings | Name, path, metadata, extracted-text terms | Query touches matching documents rather than all documents | Update/merge design, compression, field/position schema | Scoped judged lexical set at 1m/10m |
| Positional/offset postings | Phrase, proximity, highlighting, exact evidence | Exact source fragments and fast phrases | Larger index and more extraction bookkeeping | Phrase/highlight correctness and bytes per token |
| BM25/BM25F | Explainable lexical ranking across fields | Strong baseline; field length and rarity accounted for | Weights are corpus policy; weak for exact structured predicates | Judged queries and per-field ablations |
| Block-Max WAND / MaxScore | Skip postings that cannot enter top-k | Large-query speed without changing scores | Block metadata and implementation complexity | Score equivalence plus p95/p99 on common/rare terms |
| Character n-gram index | Filename typos, OCR, partial tokens | Language-light local edit tolerance | Storage and candidate explosion, normalization collisions | Fanout, Recall@20, exact-name rank preservation |
| Deletion dictionary | One/few-edit typo candidates | Fast bounded edit lookup | Vocabulary expansion and bounded edit assumptions | Compare with n-grams at 100k/1m/10m terms |
| BK-tree / metric tree | Edit-distance vocabulary lookup | Natural bounded-distance query | Degenerates with high radius/dimensionality; dynamic persistence | Tail candidate count and Unicode costs |
| Trie/radix prefix index | Prefix/autocomplete | Direct prefix traversal | Not a general typo or relevance mechanism | Prefix latency/memory versus FST/B+ tree |
| Winnowing/fingerprints | Stable source fragments and near-copy anchors | Local edits disturb bounded neighborhoods | Collisions and parameter sensitivity; verification required | Fragment recall, collision audit, update locality |

## 4. Structural, duplicate, and semantic candidates

| Mechanism | Candidate role | Gain | Loss / prohibition |
|---|---|---|---|
| Whole-file cryptographic hash | Exact byte-duplicate verification | Strong equality after full read | Expensive reads; metadata/name search unaffected; sparse/logical-byte policy |
| Chunked Merkle tree | Incremental large-file identity and block reuse | Local changes rehash locally; proofable structure | Storage and chunk-boundary policy |
| Content-defined chunking (Rabin/Gear) | Similar large files and dedup candidates | Insertions preserve later chunks | CPU, adversarial chunking, not semantic similarity |
| SimHash | Near-duplicate text candidate | Tiny Hamming-space sketch | Frequency-weighted resemblance only; collision/threshold policy |
| MinHash/bottom-k | Set resemblance or directional containment | Mergeable/sampleable; explainable estimator | Discards order unless features encode it; uncertainty required |
| LSH | Sublinear approximate candidate buckets | Avoids all-record sketch scan | Recall/space tuning, bucket skew, rebuild/update semantics |
| ConeDAG-like signed feature sketch | Short deformed sequence candidate | Deterministic fixed width, order/content channels | Zeta lost semantic queries; exact scan is linear; not identity or meaning |
| HNSW | Approximate dense-vector search | Strong recall/latency for static-ish vectors | Memory-heavy graph, difficult deletion/update, nondeterministic construction risks |
| IVF-PQ | Large compressed vector collections | Lower bytes/vector and bounded probes | Training, quantization error, rebuild/migration burden |
| Flat vector scan | Correct approximate-score baseline | Simple, exact within vector model | Linear cost; useful until it actually misses a budget |
| Typed interpretation graph | Context, role, provenance, temporal/user concepts | Separates equivalence from association and preserves evidence | Schema and inference complexity; requires explicit uncertainty and erasure |

**GIVEN boundary:** semantic channels never overwrite exact metadata or file
contents. Every derived result carries source anchor, extractor/model/config
version, confidence, and deletion lineage.

## 5. Ranking and query planning

Candidate stages, in dependency order:

1. Parse scope, exact literals, negative filters, field/range predicates, and
   natural-language residual text.
2. Resolve exact IDs and exact path/name matches before fuzzy work.
3. Push root/type/time/size filters into the exact store or posting bitmaps.
4. Gather lexical candidates; optionally gather named structural/semantic
   channels under budgets.
5. Score channels independently. Preserve raw score, calibration version,
   evidence anchors, staleness, and rejection reasons.
6. Fuse with reciprocal-rank fusion, calibrated late fusion, or a tested linear
   model. Zeta's fixed weights are baselines, not defaults.
7. Apply a result-boundary policy only after ranking. Never let cutoff change
   candidate generation or scores.

Candidate rank algorithms include exact-match precedence, BM25F, typed temporal
decay, graph-distance/provenance boosts, reciprocal-rank fusion, calibrated
logistic fusion, LambdaMART-style learning-to-rank trained only on consented
local feedback, and deterministic hand-authored rules. Adaptive ranking loses if
it cannot explain, reset, export, or reproduce a result.

## 6. Database and index-store candidates

| Candidate | Gain | Loss / risk | Appropriate comparison role |
|---|---|---|---|
| SQLite + WAL + ordinary tables + FTS5 | One embedded durable system; transactions; B-trees; FTS; inspection/recovery tooling | Single-writer constraints, FTS merge/write behavior, extension/build policy, cross-process discipline | Strong baseline and possibly sufficient architecture |
| SQLite exact store + external segmented postings | Mature catalogue plus specialized lexical engine | Two generations/transactions to coordinate | Hybrid when FTS fails measured update/query gates |
| LMDB/MDBX | Memory-mapped ordered KV, snapshot readers, simple durability | Single writer; map sizing; no search/ranking; reader/process semantics | Exact catalogue and immutable-generation baseline |
| RocksDB/Pebble-style LSM | Write throughput, snapshots, column families | Compaction/tuning/dependency weight, write/space amplification | Event-heavy durable projection candidate |
| bbolt-style B+ tree | Small understandable embedded KV | Single writer, page growth, no lexical index | Go feasibility baseline, not full search solution |
| Tantivy | Rust Lucene-like segmented postings and BM25 | Separate store, Rust integration, merge/delete policies, no authoritative file catalogue | Lexical engine candidate behind exact store |
| Xapian | Mature C++ probabilistic retrieval | API/dependency/format governance and update semantics | Independent lexical baseline |
| Custom append log + immutable segments | Exact control, sequential writes, tailored postings | We own crashes, compaction, migration, corruption, tooling forever | Only if mature candidates fail declared gates |
| Flat files/monolithic JSON | Inspectable and simple for tiny fixtures | Full read/rewrite, huge object overhead, weak concurrency | Control baseline only; Zeta measured 534 MB/1,341 docs |

Lucene/Java may be used as an offline relevance oracle in experiments, but it is
not an admitted File Manager runtime dependency. A database choice does not
select the service language; a Go service may host SQLite, Pebble, a custom
engine, or no durable store in a bounded experiment.

## 7. Update and consistency models

### Candidate A — synchronous transactional update

Filesystem event enters one transaction; committed queries immediately see it.
Gain: simple read-your-writes. Loss: event storms create many writes and stalls;
native event loss still requires reconciliation.

### Candidate B — append observation log, micro-batched projection

Durably record normalized observations, then apply bounded batches to a new
catalogue/index generation. Gain: coalescing, replay, audit, stable query
snapshots. Loss: visible lag, log retention/compaction, two watermarks.

### Candidate C — mutable memtable plus immutable segments

Queries merge a recent in-memory/durable delta with immutable catalogue/posting
segments. Gain: fast foreground updates and snapshot publication. Loss:
tombstones, merge complexity, compaction policy, more query paths.

### Candidate D — rebuild projection per root

Keep filesystem observations authoritative and periodically replace each root's
projection. Gain: simple corruption recovery and privacy erasure. Loss: poor
freshness/write economy for large active roots unless paired with a delta.

Every model needs: event sequence/idempotency key, source-generation watermark,
gap detection, bounded queue/backpressure, rename/replace semantics, crash
replay, immutable reader generation, quarantine, integrity check, and full scan
reconciliation. Native watchers are hints, never the sole truth.

## 8. Sharding choices

| Shard key | Gain | Loss / question |
|---|---|---|
| Per volume | Matches availability and platform identity domain; detachable/offline catalogue | Queries fan out; cross-volume move becomes delete+create; huge volume imbalance |
| Per approved root | Privacy/erase/rebuild unit; configuration maps directly | Overlapping/nested roots and moved subtrees; many shards |
| Per user/profile | Strong access boundary and simple local query | Large shard; removable/offline policy mixed in |
| Per record-ID hash | Even load and parallelism | Destroys path/root locality; overkill on one machine; hard root erasure |
| Per time/segment generation | Immutable updates and rollback | Queries touch generations; tombstones/compaction required |
| By data class | Exact catalogue, lexical, thumbnails, semantic hive erase independently | Cross-store generation coordination |
| Hybrid volume/root + immutable generations | Availability/privacy boundary plus update locality | Most machinery; needs manifest and atomic generation publication |

Sharding is not automatically distribution. On one machine it should earn its
existence through availability isolation, privacy erasure, rebuild locality,
write contention, or bounded corruption—not fashion.

## 9. Hive model

"Hive" should name an authority and lifecycle boundary, not a generic database.
Candidate hive classes:

- **catalogue hive:** disposable exact projection of filesystem observations;
- **lexical hive:** disposable postings/term dictionaries and statistics;
- **media hive:** thumbnails, fingerprints, OCR/captions with extractor lineage;
- **semantic hive:** typed concepts/interpretations/user corrections and exact
  anchors, separately erasable;
- **view hive:** folder view/sort/geometry history, not filesystem truth;
- **federation manifest:** machine/shard identities, availability, schemas,
  watermarks, grants, and query endpoints; no hidden cloud discovery.

Questions for each hive: Who authors it? Is it rebuildable? Is any state
irreplaceable? What encrypts it? What is the erase proof? Can it sync? How are
conflicts represented? What schema migrates? What happens when absent, stale,
corrupt, locked, offline, or created by a newer version?

For future multi-machine search, prefer federated query over silent database
merging at first: each machine returns exact local records plus rank evidence;
the caller merges named channels and exposes availability. Copying a file is a
separate authorized operation. Cross-machine semantic/user facts may later need
an operation log, immutable fact IDs, explicit conflict/uncertainty records, and
possibly CRDT-like merge rules; none is selected.

## 10. Experiment and promotion order

1. Exact identity/reconciliation oracle.
2. Durable store/update candidates.
3. Equality and ordered metadata structures.
4. Positional lexical index and scoped query baseline.
5. Filename typo candidates.
6. Ranking explanations and result-boundary policies.
7. Event storms, backpressure, snapshots, compaction, and locality.
8. Go service versus native/in-process control.
9. Optional structural fingerprints/containment only if lexical quality leaves a
   named gap.
10. Semantic hive last, with deletion/provenance/calibration gates.

No ANN experiment begins merely because vectors exist. Flat scan remains the
correctness baseline until it fails an approved latency budget.
