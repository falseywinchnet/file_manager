# Algorithm inventory

Status labels describe Zeta evidence, not File Manager selections.

## Layer map

| Layer | Observed mechanism | Authority |
|---|---|---|
| exact identity | anchored regex parse and dictionary membership | authoritative for Zeta records |
| durable source | validated JSON collections plus source/work files | authoritative evidence |
| stale detection | SHA-256 over canonical collections and source bytes | rebuild guard, not record identity |
| lexical candidates | inverted term-to-document postings | candidate generator |
| typo candidates | term prefix scan plus trigram/bigram-to-term maps | candidate generator |
| lexical rank | BM25-like field weighting, phrase and exact-ID bonuses | rank evidence |
| contextual rank | bounded BFS over undirected support/taxonomy graph | rank evidence |
| structural rank | ConeDAG cosine plus directional bottom-k containment | lossy rank evidence |
| result boundary | hybrid `1/e` score floor/adjacent-drop rule | presentation policy |

## 1. Exact identity routing

**OBSERVED — mechanics.** `identities.py` recognizes complete `R<num>`,
`CITE<num>`, and `CERT<num>` forms, reversibly converts them to fixed-width
internal IDs, and tests membership in the authoritative stores. `cmd_search`
performs this lookup before constructing ranked results.

**Complexity.** Regex and conversion are linear in the short query string;
dictionary membership is expected constant time in CPython. Worst-case behavior
belongs to the language runtime, not a Zeta-specific hashmap theorem.

**Invariant.** A complete identity never competes with fuzzy candidates. Prefixes
and substrings do not count as identities.

**File Manager relevance — CANDIDATE principle.** Resolve exact object handles and
exact path predicates before ranked retrieval. A filesystem path alone is not a
stable identity across rename, replacement, hard links, mounts, or case-policy
changes; the identity tuple remains a separate platform-reality question.

## 2. Source digest and full rebuild

**OBSERVED — mechanics.** `source_digest()` SHA-256 hashes an index-version tag,
canonical JSON for five collections, every admitted work file's relative path and
bytes, and every source-turn file. `load_index()` rebuilds when the JSON index is
missing, malformed by version, or has a mismatched digest. The completed index is
written by `atomic_json()` to a temporary sibling, flushed, file-synced, and
atomically replaced.

**Complexity.** Digest validation reads all authoritative input bytes: `O(B)`
time and I/O for `B` source bytes on every currentness check. Rebuild tokenizes
all indexed text and recreates every posting, typo map, graph, and sketch.

**Invariants.** A successfully loaded index corresponds to the exact digested
snapshot. A failed process cannot expose a partially written JSON file through
the final pathname.

**Failure modes.** There is no observed directory `fsync`, write-ahead log,
incremental segment, reader generation protocol, or multi-writer lock. A digest
does not capture live filesystem events. Full JSON replacement creates high write
amplification at scale.

## 3. Inverted postings and fielded lexical score

**OBSERVED — build.** Unicode/math translation, `casefold`, NFKD decomposition,
and combining-mark removal precede tokenization. For every document and each of
`title`, `body`, `topics`, and `links`, a `Counter` stores term frequency. A
posting maps each distinct term to sorted unique document ordinals. Average field
lengths are retained.

**OBSERVED — query.** For each expanded term, the engine visits its posting and
computes Robertson-style IDF plus a BM25-like saturation over a sum of
field-weighted, length-normalized term frequencies (`k1=1.2`; field-specific
weights and `b`). It is BM25F-like, not a cited exact BM25F implementation.

**Complexity.** Build is `O(T + G)` before sketch construction, where `T` is total
tokens and `G` is vocabulary n-grams. A normal exact-term query scores the union
of visited postings. Stored Python/JSON objects impose substantial per-entry
overhead. The later structural scan makes total production query cost linear in
all records regardless of lexical selectivity.

**Invariant.** Posting entries reference exact document ordinals; final results
return exact document records.

**Failure modes.** No positional postings, block compression, skip data, deletion
tombstones, incremental merge, or concurrent ingestion. Repeated terms are not
positions. Phrase evaluation scans full token streams for docs already scored.

**MEASURED current storage warning.** The inspected version-4 generated JSON is
534,022,217 bytes for 1,341 documents and 72,844 posting terms on the current Zeta
snapshot. This does not isolate which duplicated/object-heavy field dominates,
but it is enough to reject an assumption that the format remains compact merely
because document count is small.

## 4. Character n-gram typo expansion

**OBSERVED — mechanics.** Build maps padded character trigrams and bigrams to
vocabulary terms. If a query term is absent, expansion includes aliases, a full
vocabulary prefix scan, n-gram union candidates, trigram Jaccard filtering, and
bounded Damerau-Levenshtein for candidates within length delta two. Weights encode
the evidence source.

**Complexity.** Gram lookup is proportional to query grams and candidate terms,
but the prefix step scans the entire vocabulary `O(V)` for every absent term.
Damerau-Levenshtein uses a full `O(mn)` matrix per retained candidate. Build space
duplicates term references across gram maps.

**MEASURED in Zeta.** On the documented 399-document/8,193-term stage, adding the
bigram candidate map grew the JSON index from about 8.1 MB to 9.5 MB and reduced
the loaded-query mix from about 23.4 ms to 12.7 ms. The environment is
underspecified and the workload is not File Manager's.

**File Manager relevance — CANDIDATE.** Character grams are a plausible filename,
OCR, and extracted-text candidate channel. Prefix structures, edit algorithms,
and normalization policy require independent scale and language testing.

## 5. Exact phrase detection

**OBSERVED — mechanics.** After lexical scoring, each scored document's flattened
token stream is scanned for a slice equal to all query tokens; a match adds 1.6.

**Complexity.** The Python expression constructs slices at each offset, so cost is
at least linear in candidate stream length and includes temporary list work. It
does not provide arbitrary proximity despite documentation using that word.

**Failure modes.** Field boundaries are flattened, so a phrase can theoretically
span adjacent fields. No stored offsets exist for highlighting or evidence
anchors.

## 6. Graph-distance anchoring

**OBSERVED — mechanics.** Zeta constructs an undirected adjacency map from support,
topic, citation, certificate-dependency, and taxonomy-parent relations.
Comma-separated queries treat earlier segments as anchors and the last as target.
At most eight top lexical seeds per anchor start a breadth-first search to depth
seven. Target documents receive `3.2/(1+distance)` plus a bounded direct-overlap
bonus.

**Complexity.** Bounded BFS is `O(V+E)` in the reachable depth-seven subgraph per
anchor, followed by score adjustment over target candidates.

**Invariant.** Every graph bonus can name its anchor segment and distance.

**Failure modes.** Edge types and direction are erased. The numerical bonuses are
corpus policy, not calibrated truth. A File Manager folder tree is not by itself a
semantic graph.

## 7. ConeDAG feature-hashed structural sketch

**OBSERVED — mechanics.** Normalized text becomes independently normalized
channels:

- content: logarithmic word counts and character 1–5-grams;
- path: token n-grams through degree four, skip edges, bounded relative-order
  pairs, and character paths;
- position/combination: overlapping dyadic relative-position bins and
  edge-position crosses;
- shape: six bounded length/diversity values.

Each sparse feature is signed into a seeded BLAKE2b bucket. Default channels use
three widths of 128 plus six shape values, then normalize to a 390-value unit
vector. A separate log-character magnitude is retained. Candidate ranking is
cosine similarity; `ConeIndex` and production MIND scan every stored vector.

**Complexity.** The documented bounded-feature build is approximately
`O(cK + n(N+L+G))` for characters `c`, character orders `K`, words `n`, maximum
token n-gram `N`, position levels `L`, and order-pair gap `G`. Production bounds
structural input to 48 tokens. Exact scan query cost is `O(Rd)` for `R` records
and dimension `d=390`, before containment work.

**Supported invariant.** Deterministic configuration yields a fixed-width lossy
vector. Results return exact record IDs; vector equality never proves identity.

**MEASURED in Zeta.** On 1,080 generated transformations over 1,258 candidates,
ConeDAG reported MRR 0.955, Recall@1 0.919, Recall@5 1.0, beating equal-width bag,
character, and predecessor sketches. Five seeds ranged 0.900–0.931 Recall@1.

**REJECTED claim.** On twelve manually judged semantic queries it lost to MIND
search at Recall@1 (0.583 vs 0.750). It does not establish semantic equivalence,
collision-free identity, lossless compression, or an edit-distance embedding.

## 8. Directional bottom-k containment

**OBSERVED — mechanics.** For up to 48 tokens, enumerate distinct global ordered
subsequences of degrees two and three and contiguous paths of degrees two through
five. Hash each to a deterministic 128-bit rank. Store the smallest hashes,
stratified 60/40 across global/local families, plus source cardinalities. At
comparison, use a coordinated threshold from the sampled union and estimate
`|query ∩ record|/|query|`. Apply only when the query has fewer words. Production
blends 80% ConeDAG and 20% containment.

**Complexity.** Degree-three enumeration is cubic in token count; the 48-token
bound controls it. Sorting all unique hashes before retaining bottom-k costs
`O(F log F)` and transient `O(F)` space. The experiment itself calls streaming
bottom-k an unimplemented next step.

**MEASURED in Zeta.** A 256-hash sketch reported mean absolute estimator error
0.0021–0.0039 across tested seeds/cases. In the generated drift benchmark, 0.20
weight achieved edit-equivalent Recall@1 1.0 while strict identity fell because
many deleted queries had multiple minimum-edit parents.

**REJECTED variant.** Adding the same order evidence as a symmetric hyperspherical
channel reduced deletion Recall@1 from 0.567 to 0.183–0.417 at equal dimension;
even a 518-dimensional variant reached only 0.517. Extra candidate evidence was
incorrectly penalized and useful position evidence was diluted.

## 9. Combined rank and all-record scan

**OBSERVED — mechanics.** Search computes structural comparison for every allowed
document. Lexical/graph score is normalized by the maximum. Documents with lexical
evidence blend 78% normalized lexical/graph and 22% structural similarity; those
without lexical evidence receive 22% similarity. A kind prior then scales the
score. Sort order is descending score then stable document ID.

**Complexity.** The all-record structural pass and full sort make query cost at
least `O(Rd + R log R)`; bottom-k set union sorting adds work for shorter queries.
The inverted index does not bound the structural scan.

**Invariant.** Identical index/query/configuration produces deterministic ordering.

**Failure mode.** Maximum normalization makes a candidate's absolute contribution
depend on the best lexical hit. Weights and kind priors are policy constants from a
different corpus.

## 10. Dynamic result cutoff

**OBSERVED — mechanics.** On already sorted results: if at least eight scores are
at least `1/e`, keep that complete prefix up to the hard limit. Otherwise stop
before the first adjacent ratio below `1/e`. Equality does not stop. If neither
condition fires, return the hard ceiling.

**Complexity.** Linear in ranked result count. It saves no candidate-generation or
ranking work.

**Invariant.** The exclusion boundary is recorded for explanation.

**File Manager relevance — CANDIDATE only.** This may be tested as a result-list
policy, but it is not a theorem about user satisfaction, ambiguity, or file search.

## 11. Semantic identity and sense bridge

**HYPOTHESIS in Zeta.** Separate record identity, one-to-many sense identity, and
interpretation identity (exact anchors + candidate senses + typed roles + context
+ provenance + uncertainty). A frozen local encoder may propose versioned sense
IDs; typed graph alignment distinguishes equivalence, opposition, association,
entailment, and event roles.

**NOT IMPLEMENTED.** There is no selected model, trained bridge, vector index,
typed interpretation graph, calibration, or semantic benchmark large enough for
promotion. These documents are useful requirements, not code evidence.

## 12. Remembered structure audit

| Remembered idea | What actually exists | Status for File Manager |
|---|---|---|
| rainbow tables | one research paragraph says they invert hashes over bounded domains and are unrelated to ranking; no code | **REJECTED** as relevance/index architecture; unrelated tool |
| hashmaps | ordinary Python `dict`/`set` for stores, postings, maps, caches, and adjacency; no custom design/benchmark | **CANDIDATE** implementation primitive only after workload measurement |
| binary trees | no search-tree implementation | **OBSERVED:** not found in inspected scope |
| B/B+ trees | only vendored SQLite FTS5 documentation describes immutable segment B-trees and merges | **CANDIDATE** via a separately tested store, not Zeta evidence |
| tries/radix trees | no implementation | **OBSERVED:** not found in inspected scope |
| inverted index | term-to-document postings and per-doc frequencies | **OBSERVED** |
| fingerprints/hashes | SHA-256 staleness/stable IDs; BLAKE2b feature buckets/ranks | **OBSERVED**, with distinct purposes |
| ANN | named as a future substitution for exact vector scan | **OBSERVED:** not implemented; **CANDIDATE** only after scale gate |
| caches/locality | benchmark-local Python caches only; production index loads into object-heavy dictionaries | **NO DESIGNED PRODUCTION CACHE OR LOCALITY STUDY** |
| concurrent ingestion | no incremental ingester, event queue, snapshot protocol, or search-index locking | **OBSERVED:** not found in inspected scope |
