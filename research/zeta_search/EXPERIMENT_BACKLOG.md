# Experiment backlog

Every experiment produces a checked-in manifest, raw measurements, summary, and
negative-result entry. No result promotes an architecture without an approved ADR.

## Common harness

### Environments

Record CPU, RAM, OS/build, filesystem and format, storage model, free space,
power/thermal state, dependency revisions, compiler/runtime, cold/warm state, and
whether antivirus/indexers are active. Run on at least one macOS/APFS host first;
repeat storage-sensitive gates on Windows/NTFS and Linux/ext4 before portability
claims.

### Datasets

1. `tiny-real`: 10k consented metadata-only records from a non-sensitive tree.
2. `wide-1m`: one directory with 1,000,000 generated records.
3. `mixed-1m`: deep/wide tree; Unicode NFC/NFD, mixed case, long names, hard links,
   symlinks, packages, sparse and zero-byte files.
4. `mixed-10m`: metadata generator with the same distribution and deterministic
   seed; virtual fixture is allowed only for storage/query tests.
5. `storm`: create/rename/move/delete/replace bursts, including overflow and
   deliberately dropped event ranges.
6. `lexical-judged`: at least 300 queries split across exact name/path, prefix,
   phrase, typo, metadata predicates, scoped folder queries, and no-result cases;
   relevance grades and ambiguity sets are independently reviewed.
7. `structural-judged`: at least 200 typo/OCR/crop/partial/near-duplicate queries
   plus order-altered and semantically unrelated hard negatives.
8. `semantic-judged`: later, at least 300 locally stored cases covering paraphrase,
   polysemy, antonyms, association, object/time/provenance conjunctions, unknowns,
   and exact source anchors.

### Metrics

Correctness precedes speed: exact identity precision/recall, stale/ghost/missing
records after quiescence, snapshot consistency, crash recovery, and explanation
traceability. Then record p50/p95/p99/max latency, throughput, CPU time, resident
and peak bytes, bytes read/written, index bytes/record, write amplification,
rebuild time, idle CPU/wakeups, and backlog age. Retrieval adds Recall@k, MRR,
nDCG, ambiguity retention, calibration, and false positives by failure class.

## E01 — exact identity and mutation oracle

**GIVEN / mandatory core gate:** this is no longer an optional search-research
experiment. It belongs to core filesystem correctness testing and must pass
before an indexed result can authorize or target a file operation. It remains in
this backlog because the index projection and reconciliation path are subjects
of the oracle.

**HYPOTHESIS:** an exact projection can preserve platform identity through normal
rename/move/link operations and expose replacement as a new incarnation.

**Baselines:** live filesystem enumeration by path; candidate exact store(s).

**Workload:** replay deterministic operations over 100k objects, crash after each
durability boundary, inject duplicate/out-of-order events, overflow the native
event stream, then reconcile by scan.

**Exact rejection gates:** any mutation authorized against the wrong object; any
undetected identity collision; any ghost or missing live record after reconciliation;
non-idempotent replay; snapshot returning mutually impossible generations.

## E02 — durable store and update strategy

**HYPOTHESIS:** an incremental transactional store reduces writes and rebuild
latency versus Zeta-style monolithic JSON without weakening auditability.

**Baselines:** canonical JSON full rebuild/atomic replace; SQLite ordinary tables;
at least one purpose-built append/segment candidate. Keep payload/schema equal.

**Workload:** initial loads at 10k/1m/10m; one-file updates; 10k-operation bursts;
rename-heavy builds; deletes; crash/restart; corruption of newest and old segments.

**Exact rejection gates:** correctness failure; unrecoverable committed generation;
write amplification not at least 10x lower than full JSON for a one-record update;
p99 read stall worse than the full-rebuild baseline during 10k-operation bursts;
format cannot be integrity-checked or rebuilt from its declared authority.

## E03 — equality and ordered metadata structures

**Question:** where do hash maps, ordered trees, and persistent indexes actually
win?

**Baselines:** sorted-vector binary search, runtime hashmap, SQLite indexed tables,
and one explicit B/B+tree or LSM-backed candidate if the store does not already
provide it.

**Queries:** exact object ID/path, path prefix, size/date ranges, sort-by-name/time,
and 1k-result pagination at 1m/10m records.

**Exact rejection gates:** incorrect collation/case behavior; unstable page order
under an unchanged snapshot; candidate uses more than 2x baseline resident bytes
without a statistically significant p95 latency win; any claim based only on
average latency.

## E04 — lexical index substrate

**HYPOTHESIS:** offset-bearing postings provide scalable scoped filename/content
retrieval and explanations better than Zeta's object-heavy JSON postings.

**Baselines:** linear normalized scan; Zeta-equivalent term/document postings;
SQLite FTS5; one purpose-built segmented postings candidate.

**Queries:** `lexical-judged`, current-folder and descendant scopes, exact literals,
negative filters, phrase/highlight, and no-result queries.

**Exact rejection gates:** exact literal or negative-filter error; Recall@10 more
than 0.5 percentage points below the best correct baseline; phrase evidence cannot
return exact offsets; p95 at 1m fails to beat normalized linear scan; updates block
readers for more than the unresolved UI/search budget.

## E05 — typo candidate structures

**Candidates:** Zeta trigram+bigram maps, trigram-only, deletion dictionary,
bounded BK-tree, and radix/trie prefix candidate if a complete implementation is
admitted. Damerau-Levenshtein reranking stays common.

**Workload:** filenames across lengths/scripts, one/two edits, transpositions,
prefixes, OCR noise, code identifiers, and exact hard negatives at 100k/1m/10m
vocabulary.

**Exact rejection gates:** typo Recall@20 below 0.98 on generated single-edit cases;
exact names displaced from rank one; false candidate fanout above 1% of vocabulary
at p99; normalization merges two distinct exact names without retaining both exact
records.

## E06 — ConeDAG transfer test

**HYPOTHESIS:** a ConeDAG-like short-text channel improves near-name/OCR/partial
retrieval over character grams after lexical fusion.

**Baselines:** word bag, equal-width character 3–5 grams, Zeta predecessor,
lexical baseline alone, and lexical+ConeDAG. Use at least ten seeds and widths
102/198/390/774.

**Workload:** `structural-judged` plus 10k/100k/1m candidate scale. Preserve exact
ambiguity classes.

**Exact rejection gates:** fused nDCG@10 does not improve by at least 0.02 on a
held-out set; exact-name MRR declines by more than 0.005; seed 95% interval includes
a material regression; query p95 or bytes/record exceed the best character-gram
baseline without judged-quality gain. ANN is tested only after exact scan misses a
declared latency budget.

## E07 — containment without cubic transient work

**HYPOTHESIS:** streaming bottom-k over bounded ordered features preserves Zeta's
sample distribution and partial-text recall while controlling ingestion memory.

**Baselines:** exact feature sets; Zeta enumerate/sort implementation; proposed
streaming implementation; character-shingle containment.

**Exact rejection gates:** mismatch from reference bottom-k sample for identical
hash ranks; estimator MAE above 0.01 at 256 samples; any unbounded memory growth
with token length; edit-equivalent Recall@1 below character-shingle baseline;
strict-ID results reported without ambiguity sets.

## E08 — ranking and explanation ablations

**Candidates:** lexical only; lexical+metadata; lexical+typed structural channels;
graph/provenance/temporal fusion; Zeta-style max normalization; calibrated late
fusion. Result cutoff is evaluated separately from ranking.

**Exact rejection gates:** repeated identical query/index produces different
ordering; result cannot enumerate each score/evidence anchor; nDCG@10 gain below
0.01 with more than 10% latency or storage cost; contradiction/antonym false
positive rate worsens in semantic phases.

## E09 — result-boundary policy

**Candidates:** fixed top-k, score threshold, largest adjacent drop, Zeta hybrid
`1/e`, relevance bands, and user-requested pagination.

**Workload:** independently judged “complete enough” result sets, including exact,
ambiguous, many-good-result, gradual-tail, and no-result queries.

**Exact rejection gates:** hides a judged essential result above the next page in
more than 1% of cases; returns zero for a query with a judged relevant candidate;
changes candidates or scores (the boundary may only select an already ranked
prefix); lacks a visible continuation path.

## E10 — event ingestion, backpressure, and cache locality

**HYPOTHESIS:** bounded batches and immutable read generations keep queries stable
during event storms without an unbounded queue.

**Baselines:** synchronous per-event update; fixed-time batch; fixed-count batch;
append log plus background segment merge.

**Exact rejection gates:** lost event without declared reconciliation; unbounded
queue or memory; reader sees partial generation; idle polling consumes measurable
CPU above timer noise; p99 query latency exceeds 2x quiescent p99 during a 10k/s
synthetic burst after excluding storage saturation.

## E11 — Go service feasibility spike

**CANDIDATE only.** Implement the same narrow API in Go and one native/in-process
control: ingest fixture events, exact lookup, scoped lexical query, status,
integrity check, and clean shutdown. Use identical on-disk substrate where
possible so language/runtime is the variable.

**Exact rejection gates:** idle resident memory or wakeups exceed the native
control by more than 2x without a measured operational benefit; p99 IPC+query adds
more than 25% over in-process at 1m records; clean install cannot avoid network
access; crash can corrupt the committed store; GUI navigation depends on service
availability.

## E12 — local semantic projection (later gate)

Do not begin until exact and lexical layers pass. Compare exact/lexical,
character/ConeDAG, frozen local sense prototypes, typed-graph reranking, and fused
retrieval on `semantic-judged`.

**Exact rejection gates:** missing exact source anchor; derived fact lacks model,
extractor, confidence, and version; antonym/association false-positive rate above
the lexical baseline; unknown-sense rejection below 0.95; deleting an opted-in root
cannot prove removal of its derived projection; core search fails when every
semantic component is absent.

## Promotion order

E01 → E02 → E03/E04 → E05 → E08/E09 → E10/E11. E06/E07 are optional branches
after a lexical baseline. E12 remains last. This order makes identity and
consistency authoritative before relevance experiments can decorate them.
