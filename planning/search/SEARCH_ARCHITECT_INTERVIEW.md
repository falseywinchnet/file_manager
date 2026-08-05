# Search, indexing, storage, shard, and hive interview

Status: **grand-architect questions; answers become GIVEN constraints, not
automatic ADRs**. Date: 2026-08-03.

## 1. Already established

- **GIVEN:** ordinary navigation works without an index.
- **GIVEN:** installation explicitly asks which roots to index, initially
  offering home; indexing is opt-in.
- **GIVEN:** hidden/dot paths and Git repositories are excluded by default.
- **GIVEN:** a selected tree automatically admits new descendants except below
  an excluded subleaf. Partially selected roots admit files but ask before
  recursively admitting recognized/new directory roots.
- **GIVEN:** a visible agent asks before admitting new drives or newly recognized
  sync/repository roots; default is no.
- **GIVEN:** ordinary indexing begins with names and metadata. Content-derived
  work is bounded by file size, extractor competence, and local availability.
- **GIVEN:** content is metadata, but derived descriptions/fingerprints never
  replace exact contents.
- **GIVEN:** search begins in the folder the user navigated to and applies below
  that folder; one search field is sufficient initially.
- **GIVEN:** thumbnails exist only for indexed locations.
- **GIVEN, clarified by ADR-003:** search does not smuggle file mutation.
  Humans, local AI tools, and developer agents use the same admitted local CLI
  operations; future plugin AI is a separately sandboxed capability surface.
- **GIVEN:** the core has no web search, store, or remote discovery.
- **GIVEN:** semantic/personal-assistant components are optional. File Manager
  exposes file objects and a future secure, synchronizable knowledge hive.
- **GIVEN:** future multi-machine search and explicit file copy are desired.

These answers still leave the authority, update, storage, ranking, sharding,
privacy, and synchronization semantics unresolved.

## 2. How to answer

Answer by ID, for example `S001 B, S002 D`. `D` means "discuss/defer," not a
fourth choice unless one is shown. A free-form answer may combine choices, but
state the boundary: default, optional advanced mode, or permanent prohibition.

## 3. Round 1 — decisions that constrain the whole system

### S001 — Is any search/semantic state user-authored authority?

**A. Everything is disposable projection.** Filesystem objects and file
contents are the only truth. Corrections merely bias a rebuildable cache.

- Gain: recovery is always delete-and-rebuild; syncing and corruption are
  simpler; the index cannot become a second filesystem.
- Loss: a user's corrections, learned phrases, cross-file relationships, and
  "winter of 28" knowledge can disappear or change after model replacement.

**B. Split authority.** Catalogue, postings, vectors, captions, and thumbnails
are disposable; explicit user corrections, named concepts, and approved
relations are durable knowledge with provenance and export.

- Gain: preserves genuinely authored meaning while keeping bulk projections
  rebuildable. Supports a useful synchronizable hive.
- Loss: creates a second authoritative class needing schema migration,
  encryption, conflict resolution, backup, deletion, and audit.

**C. Durable knowledge lives beside or inside files.** Use sidecars, portable
packages, or embedded metadata where supported; the central hive is a cache.

- Gain: knowledge travels with files and remains inspectable outside File
  Manager.
- Loss: modifies folders, interacts badly with read-only/cloud/package roots,
  leaks sensitive classifications, and creates conflict/clutter.

Other choice: permit B only for explicit corrections, never passive learning.

### S002 — What is the primary catalogue/shard boundary?

**A. One per user profile.**

- Gain: simplest local query and transaction model.
- Loss: one corrupt/huge/slow volume affects everything; root erasure and
  offline availability are harder to isolate.

**B. One per volume.**

- Gain: matches platform identity and availability; removable/offline catalogues
  detach naturally; corruption/rebuild is bounded.
- Loss: queries fan out; cross-volume moves change identity domain; one enormous
  volume can dominate.

**C. One per approved root.**

- Gain: privacy grant, deletion, rebuild, and resource accounting match user
  choices.
- Loss: nested/overlapping roots, moved directories, and many small shards need
  explicit resolution.

**D. Hybrid manifest: volume identity, root policy, immutable generations.**

- Gain: isolates availability/privacy while retaining append/merge locality.
- Loss: most machinery; requires atomic manifest publication and cross-shard
  ranking.

### S003 — What should an offline catalogue reveal?

**A. Nothing when the volume is absent.** Strong reality model and privacy;
loses "find which drive contains it."

**B. Names, paths, sizes, types, and timestamps only.** Useful drive discovery;
still exposes a sensitive inventory.

**C. Full approved lexical/thumbnail/semantic projections.** Powerful offline
search; highest privacy, disk, staleness, and deletion burden.

Other choice: B by default, C per volume with an obvious locked/offline badge.

### S004 — What exact object does a result identify?

**A. Current path.** Human-friendly and portable; rename breaks identity and
delete/recreate can silently target a different object.

**B. Platform object identity plus volume/incarnation, with paths as observed
addresses.** Correct across ordinary rename/hard links; platform differences,
inode/file-ID reuse, clones, and remote filesystems remain explicit.

**C. Content identity.** Useful for duplicates and synchronization; reading is
expensive, mutable files change identity, and equal content does not mean the
same file object.

Candidate boundary: B for object authority, A for display/entry, C as a separate
duplicate/version relation.

### S005 — What consistency promise should search make?

**A. Snapshot consistency only.** Every result belongs to one published
generation; recent changes may lag visibly. Predictable and testable, but not
instantaneous.

**B. Read-your-actions plus snapshot background state.** File Manager's own
operations are overlaid immediately; outside changes appear after ingestion.
Feels current, but requires an overlay and deduplication against later events.

**C. Merge live enumeration with index results for the active scope.** Most
current local folder; costs foreground I/O and creates complex ordering and
duplicate/stale reconciliation.

Other choice: B generally and C only when the UI explicitly says "verify now."

### S006 — Must filesystem observations be durably journaled before projection?

**A. No; watchers are hints and scans repair the catalogue.** Least writing and
simplest implementation; crashes or event gaps increase staleness and rescan
cost.

**B. Journal normalized observations, then micro-batch catalogue/index updates.**
Replayable, auditable, and coalescible; adds write traffic, retention,
backpressure, and two watermarks.

**C. Journal only File Manager's own operations; external events remain hints.**
Gives read-your-actions cheaply; weaker crash explanation for outside changes.

### S007 — What happens when the service/index is stale, corrupt, or absent?

**A. Search is unavailable; navigation continues.** Honest and simple; poor
utility during rebuild.

**B. Serve the last valid generation with a staleness badge and suppress unsafe
exact claims.** Useful and deterministic; users may act on old locations.

**C. Serve stale global results plus live verification of visible/top results.**
Best continuity; more I/O and result states such as moved, missing, or unverified.

Corruption subchoice: automatically quarantine/rebuild, or require explicit
approval before potentially long reads?

### S008 — Which storage tournament should be mandatory before selection?

**A. SQLite/WAL/FTS5 versus monolithic baseline only.** Fastest route to a
credible system; may miss a segmented engine that better meets write/latency
budgets.

**B. SQLite, LMDB/MDBX, and LSM candidate plus equal lexical engine.** Broad
durability comparison; significant harness work and apples-to-oranges risk.

**C. SQLite/FTS5, SQLite+Tantivy/Xapian, and custom immutable-segment control.**
Directly tests integrated versus hybrid search; less focus on KV alternatives.

**D. Begin with requirements/budgets, admit candidates only after the exact
schema and workload exist.** Most disciplined; delays prototype gratification.

Candidate experiment order is D, then at least SQLite and one segmented
alternative on identical records.

### S009 — Where should the indexing engine run?

**A. In the GUI process.** Lowest IPC and packaging cost; crash, memory, and
extractor stalls threaten the interface.

**B. Native/Go service owns ingestion and search.** Restartable, budgetable, and
AI/CLI accessible; IPC, runtime footprint, install lifecycle, and schema
compatibility become permanent work.

**C. Small native supervisor plus optional Go search worker.** Strong failure
separation and replaceability; two processes/protocols and more packaging.

**D. Compare Go service with an in-process native control using the same store.**
Does not pre-decide; costs a deliberate duplicate spike.

### S010 — Is the index format disposable, migratable, or stable?

**A. Always disposable.** Rebuild on incompatible version. Excellent freedom;
terrible for 10m records, offline catalogues, expensive extraction, and users
who need search during rebuild.

**B. Migratable within a bounded version window; projections may still rebuild.**
Pragmatic upgrades and rollback; migration/testing burden.

**C. Stable public format.** Enables third-party inspection/tools; freezes
representation and makes repair/security compatibility expensive.

Other choice: exact catalogue B, bulk lexical/semantic projections A, exported
inspection format C but not used internally.

### S011 — How many independently erasable hives exist?

**A. One database with namespaced tables.** Atomic joins and simplest backup;
privacy erasure, corruption, and rebuild couple unrelated data.

**B. Exact catalogue, lexical, media, semantic, and view hives with a generation
manifest.** Strong lifecycle and privacy boundaries; cross-hive consistency and
packaging are harder.

**C. Exact catalogue plus one disposable derived hive and one durable user
knowledge hive.** Fewer boundaries while retaining authority separation;
derived media/lexical workloads may still interfere.

### S012 — How should multi-machine search begin?

**A. Federated queries only.** Each machine searches its local exact catalogue
and returns evidence-bearing results; no silent merge. Honest availability and
simple erasure; requires machines online and cross-machine rank normalization.

**B. Sync offline catalogues, fetch bytes only on explicit copy/open.** Search
works while machines sleep; inventory privacy, conflict, freshness, and storage
increase.

**C. Merge a shared hive in cloud/local sync storage.** Seamless global view;
largest authority, encryption, conflict, provider, and network-policy expansion.

Other choice: A first, with explicitly exported encrypted B as a later plugin.

## 4. Round 2 — ingestion, mutation, and recovery

| ID | Question and choices | Gain/loss boundary |
|---|---|---|
| U101 | Watcher event, scan record, or operation log: which wins on conflict? | Watchers are fresh but lossy; scans are authoritative snapshots but expensive; own operations know intent but not all external reality. |
| U102 | Coalesce rename/write storms by path, object ID, or time window? | Path is cheap but wrong across rename; ID is accurate where available; time batching reduces writes but adds lag. |
| U103 | Publish every batch or fixed generations? | Frequent publication lowers lag but raises contention; coarse generations improve locality but make staleness visible. |
| U104 | What must be `fsync`ed: observation log, catalogue commit, manifest, all, or none on battery? | More durability means more latency/writes; weaker durability means longer reconciliation and less explainable crash state. |
| U105 | Queue policy under overload: backpressure watcher, spill log, drop+mark gap, or suspend extraction? | Never allow unbounded memory. Dropping is acceptable only with a declared rescan gap. |
| U106 | Reconciliation cadence: continuous low-rate, idle-only, startup, user-triggered, or adaptive? | Continuous catches silent loss but costs reads; idle-only may never run; adaptive needs transparent resource rules. |
| U107 | Replace detection: platform birth/version data, metadata heuristics, content sample, or full hash? | Stronger evidence costs reads; heuristics must expose uncertainty. |
| U108 | Open-file consistency: index current bytes, last closed bytes, metadata only, or defer? | Reading active files races writers; deferral is honest but less fresh. |
| U109 | Extraction failure policy: retry, quarantine type/plugin, metadata-only fallback, or hide content channel? | Retries can loop; quarantine limits blast radius; fallback must be visible in explanations. |
| U110 | Compaction priority: latency, writes/day, disk reclamation, or battery? | No single schedule optimizes all; expose counters and permit foreground yield. |
| U111 | Crash recovery may block service startup, or open last generation and repair behind it? | Blocking maximizes consistency; background repair maximizes availability but needs generation isolation. |
| U112 | Corruption granularity: rebuild table/index, shard/root, volume, or everything? | Smaller quarantine needs checksummed boundaries and manifests; global rebuild is simpler but operationally brutal. |

## 5. Round 3 — exact, lexical, and fuzzy search behavior

| ID | Question and choices | Gain/loss boundary |
|---|---|---|
| Q201 | Exact filename versus exact metadata predicate: which has rank precedence? | Filename feels direct; a structured predicate is stronger stated intent. Keep exact channels named rather than one opaque score. |
| Q202 | Does unquoted typing tokenize, prefix-match the final term, or require complete terms? | Live prefix feels instant but fans out and reshuffles; completed terms are stable but less fluid. |
| Q203 | Are quoted literals byte-exact, Unicode-normalized, case-sensitive, or selectable? | One hidden normalization cannot respect all filesystems and user intent. |
| Q204 | Should path terms score as one field, components, ancestors, or all three? | Components aid discovery; ancestor repetition can dominate; full path preserves phrase evidence. |
| Q205 | Are content matches allowed above name matches? | Content may answer the real query; users can lose obvious filename results. Consider relevance bands or hard exact-name precedence. |
| Q206 | Typo search automatic, fallback-only, or explicit `fuzzy:`? | Automatic rescues mistakes but creates surprising false positives and storage; fallback-only preserves exact behavior. |
| Q207 | Maximum edit tolerance by token length/script? | Fixed distance overmatches short names and misses long OCR; adaptive rules need tests and explanation. |
| Q208 | Phrase/proximity support at first usable search? | Positional postings cost space but enable exact evidence/highlights; deferring them risks a schema rewrite. |
| Q209 | Negative filters and exact literals mandatory in v1? | They make a real query language and deterministic AI compilation; increase parser/API contract surface. |
| Q210 | Should relevance be deterministic across machines with equal data? | Determinism aids trust/debugging; platform tokenization and adaptive local rank can differ. |
| Q211 | Tie-break by exactness, path, recency, stable ID, or configured sort? | Stable ID is reproducible but visually arbitrary; recency is convenient but adaptive; path is legible. |
| Q212 | No-results behavior: strict empty, diagnosed failed predicates, or automatic relaxation? | Relaxation helps novices but violates explicit intent unless each relaxation is shown and reversible. |
| Q213 | Result cutoff: fixed page, score bands, largest drop, Zeta `1/e`, or always show continuation? | Cutoffs reduce noise but can hide essential results; they must never alter scores and need judged completeness tests. |
| Q214 | Can search use operation/view history as a rank feature? | Useful personal context; creates surveillance, nondeterminism, sync/privacy issues. |
| Q215 | Should folder-local search include unindexed live names automatically? | Preserves current-folder usefulness; merging live and indexed result generations is more complex. |

## 6. Round 4 — fingerprints, content, and media

| ID | Question and choices | Gain/loss boundary |
|---|---|---|
| C301 | When is a full byte hash computed: ingest, idle, duplicate query, or never by default? | Early hashing improves duplicate/version search but causes large reads and SSD/HDD traffic. |
| C302 | Hash logical bytes, allocated bytes, resource forks/streams, or a typed tuple? | "Same file" differs by platform; one flat hash hides meaningful forks/metadata. |
| C303 | Content-defined chunking for large files? | Enables local-change and near-duplicate candidates; adds chunk index, privacy, CPU, and no benefit to most name searches. |
| C304 | Extract text eagerly by type/size, lazily on first query, or plugin schedule? | Eager improves recall; lazy avoids unused reads but causes query stalls; plugin schedule isolates risk. |
| C305 | Store extracted text, positional postings only, short distillation, or all with retention limits? | Stored text aids snippets/rebuild but duplicates sensitive content; postings alone are smaller but harder to inspect/migrate. |
| C306 | OCR/captions default off per root, per type, or globally? | Root policy matches consent; type policy is understandable; global is simpler but coarse. |
| C307 | Fingerprints searchable directly by user/API or internal candidate channels only? | Direct APIs aid power users and AI inspection; freeze representations and invite misuse as identity. |
| C308 | Near-duplicate relation symmetric, directional containment, or typed variants? | Crop/partial evidence is directional; one symmetric score loses meaning. |
| C309 | Retain old fingerprints across file versions? | Enables history/version discovery; increases stale sensitive state and needs version authority. |
| C310 | Index archives/packages internally? | Powerful discovery; decompression bombs, write/read budget, nested identity, encrypted archives, and plugin isolation. |

## 7. Round 5 — semantic hive and personal context

| ID | Question and choices | Gain/loss boundary |
|---|---|---|
| H401 | Are passive model outputs always disposable, while explicit corrections are durable? | Cleanest authority split; requires a distinct correction/fact schema. |
| H402 | Store embeddings, versioned concept IDs, typed interpretation graphs, or only derived text? | Embeddings are efficient candidates; concepts/graphs are inspectable; derived text is simple but conflates assertion and evidence. |
| H403 | May one file have several surviving interpretations? | Preserves ambiguity and later context; UI/query API must carry classes rather than one answer. |
| H404 | May later context revise old interpretations automatically? | Improves coherence; destroys reproducibility unless revisions and model versions are retained. |
| H405 | What does a user correction mean: ranking preference, fact assertion, concept link, or model feedback? | These have different authority and sync/erasure rules; never store one opaque "thumbs up." |
| H406 | May semantic facts link unrelated roots? | Enables "all my recipes" and cross-machine knowledge; crosses privacy grants and complicates root erasure. |
| H407 | Must every semantic result show exact fragments/thumbnails and model lineage? | Strong transparency; costs storage/query work and some models cannot provide faithful rationales. |
| H408 | How is temporal language grounded: explicit calendar map, file/EXIF events, operation history, or learned user epochs? | Rich grounding answers "winter of 28"; each source has different confidence/privacy. |
| H409 | Do user phrases sync separately from file-derived semantics? | Small durable vocabulary can sync safely; still reveals personal concepts. |
| H410 | Can the semantic hive survive deletion of its source? | Retention helps memory; conflicts with privacy and makes unsupported claims. Candidate default: retain only explicit user-authored facts with tombstoned provenance. |
| H411 | Is semantic query allowed to run when exact/lexical channels return strong disagreement? | It may rescue intent; exact evidence should remain first or separately banded. |
| H412 | Are local models core packs, plugins, or external assistants? | Core packs give consistency; plugins isolate size/risk; external assistants preserve File Manager simplicity but need a stable read API. |

## 8. Round 6 — shards, federation, synchronization, and conflicts

| ID | Question and choices | Gain/loss boundary |
|---|---|---|
| F501 | Query shards serially, parallel fan-out, or through a manifest-selected subset? | Parallel lowers latency but spikes I/O; selection needs good scope metadata. |
| F502 | Normalize ranks globally, merge per-shard ranks, or show shard bands? | Global scores need shared statistics; reciprocal-rank merge is robust but loses magnitude; bands are honest but less seamless. |
| F503 | Can one unavailable shard delay the whole query? | Waiting improves completeness; bounded deadlines keep UI snappy and require partial-result status. |
| F504 | Are offline catalogues encrypted per volume, user, or machine? | Per-volume deletion/portability versus simpler key management. |
| F505 | Sync exact catalogue rows, exported summaries, or only user-authored hive facts? | More sync gives offline search; also exports sensitive inventory and staleness. |
| F506 | Conflict policy for user concepts: last-write, operation log, CRDT set/map, or explicit conflict objects? | Automatic convergence is convenient; explicit conflicts preserve epistemic honesty. |
| F507 | Do machine-local paths remain opaque machine-qualified addresses? | Prevents false universal paths; queries and UI need machine context. |
| F508 | Can duplicate content across machines collapse into one result? | Reduces clutter; equal bytes are still distinct objects with availability and metadata. Prefer grouping over identity collapse. |
| F509 | Is remote copy part of search protocol or a separate operation protocol? | Separation preserves read-only AI search and explicit authority; combined protocol is convenient but dangerous. |
| F510 | May plugins contribute remote shards to the same merger? | Extensible; must be visually/protocol-segregated with deadlines, provenance, and no silent web search. |

## 9. Round 7 — budgets, inspectability, and failure posture

| ID | Question and choices | Gain/loss boundary |
|---|---|---|
| B601 | Set index limits as bytes/record, percentage of source bytes, absolute cap, or per-hive quota? | Bytes/record suits metadata; percentage suits content; quotas give predictable policy. Likely need all three. |
| B602 | Idle memory/CPU/wake budget on the oldest guarded machine? | Without a number, "lightweight" cannot reject Go, SQLite caches, ANN graphs, or resident services. |
| B603 | Writes/day and write-amplification ceiling on SSD and HDD? | Low writes favor batching/immutable segments; freshness and crash durability may lose. |
| B604 | Search latency gates at 1m/10m/100m: p50/p95/p99/worst and cold/warm? | Average alone hides compaction/page-fault stalls. |
| B605 | Maximum index lag during normal work and event storms? | Tight lag raises writes/CPU; loose lag demands prominent staleness semantics. |
| B606 | Must every expensive channel expose bytes, CPU, backlog, and last-run provenance in settings? | Enables informed consent/debugging; adds instrumentation/UI surface. |
| B607 | Human-inspectable internal database, export format, or both? | Internal inspectability constrains engine; stable export provides transparency without freezing storage. |
| B608 | Integrity check automatic, scheduled, manual, or on suspicious failure? | Automatic costs reads; manual may never run. Checksummed segments enable targeted checks. |
| B609 | Rebuild may consume full resources, polite background resources, or ask for a temporary boost? | Polite rebuild can take days; boost affects foreground work and battery. |
| B610 | What data must remain searchable during rebuild? | Last valid generation gives continuity; keeping it consumes disk and complicates migration. |
| B611 | Should the service keep zero resident memory when disabled? | Strong economy/privacy; cold query/startup cost and process churn. |
| B612 | Should all query plans, score components, staleness, and shard failures be available to CLI/AI? | Excellent transparency and testing; risks exposing private derived labels unless redacted/capability-gated. |

## 10. Decision sequence after the interview

1. Close S001–S007 and S010–S012: authority, identity, consistency, lifecycle,
   hive, and federation semantics.
2. Set B601–B605 enough to reject candidates.
3. Run exact identity and durable-store experiments before choosing language or
   database.
4. Establish lexical/positional baseline before structural or semantic work.
5. Decide process topology only after equal-store Go/native measurements.
6. Admit shards because they isolate privacy/availability/rebuild, not merely to
   parallelize.
7. Build semantic projection last and require exact anchors, ambiguity,
   calibration, and provable erasure.
