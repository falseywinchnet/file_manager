# File Manager applicability

This is a candidate boundary map, not an ADR.

## GIVEN constraints carried into the map

- **GIVEN:** ordinary navigation works on unindexed locations.
- **GIVEN:** indexing is explicit and privacy-controlled; hidden/dot paths and
  repository roots have conservative defaults.
- **GIVEN:** content-derived and semantic work is optional and subordinate to
  exact file objects.
- **GIVEN:** the core does not search the web. Remote discovery is plugin policy.
- **GIVEN:** AI interrogation is read-only enhanced transparency; mutation uses
  native command facilities elsewhere.
- **GIVEN:** Go is admitted as a backend-service candidate, not selected.

## Required layer separation

### A. Filesystem authority

**CANDIDATE model:** the filesystem adapter emits exact observations carrying a
platform-qualified volume identity, object identity where available, observed
path/link, object kind, metadata snapshot, and observation generation. Paths are
addresses and evidence, not universal object identities.

**Invariant:** approximate matches, captions, embeddings, fingerprints, and stale
index rows never authorize mutation and never overwrite exact observed metadata.

**Unresolved:** identity across delete/recreate, mount cloning, network volumes,
hard links, APFS snapshots, Windows file IDs, Linux inode reuse, case policy, and
offline volumes belongs to the platform-reality workstream.

### B. Exact projection store

**CANDIDATE responsibilities:** current records, path/link mappings, scan
generations, event sequence, roots/policy, extractor provenance, and a consistency
watermark. It must support transactional replacement and rebuild from filesystem
observations.

**Zeta lesson:** content digests and atomic replacement protect a small static
snapshot. They do not scale to a live filesystem catalogue without an incremental
log/snapshot design.

### C. Exact and lexical indexes

**CANDIDATE channels:** exact identity map; normalized name/path lookup; ordered
metadata indexes for type/size/time predicates; positional or offset-bearing
inverted postings for names, metadata, and approved extracted text; character
n-grams for typo candidates.

**Boundary:** ordered metadata lookup and lexical relevance are separate. A
hashmap accelerates equality; a B-tree accelerates range/order; postings retrieve
terms. None supplies relevance by itself.

### D. Structural candidates

**CANDIDATE channels:** character grams for misspelling/OCR; content fingerprints
for duplicate/near-duplicate candidates; ConeDAG-like sequence sketches for
locally deformed short text; asymmetric containment for partial/cropped text.

**Boundary:** these are optional derived projections. Their records contain
extractor/configuration versions and exact source anchors. They may be discarded
and rebuilt independently.

**Initial exclusion:** do not run cubic subsequence enumeration over arbitrary
file contents. Zeta's 48-token bound is part of why the implementation terminates.

### E. Ranking and explanations

**CANDIDATE query plan:** parse explicit scope and predicates; resolve exact
identities; gather exact/lexical/structural candidates; compute named channel
scores; fuse only calibrated channels; return stable exact records with evidence
anchors, staleness, and uncertainty.

**Zeta lesson:** graph anchors demonstrate explainable context. File Manager must
preserve relation types and direction rather than collapsing folder ancestry,
provenance, temporal relations, and semantic claims into an undirected graph.

### F. Semantic projection

**CANDIDATE future model:** a disposable, separately erasable hive of locally
derived descriptions, versioned concepts, temporal/user-context facts, exact
fragment/thumbnail anchors, confidence, and provenance. A personal assistant may
query it, but its output remains a proposed interpretation of exact file records.

**HYPOTHESIS:** “pecan pie recipe from the winter of 28” is better represented as
an auditable query plan over object/type, lexical/content, temporal, provenance,
and user-context channels than one nearest-neighbor embedding.

## What transfers directly as a principle

1. **OBSERVED → CANDIDATE principle:** exact IDs bypass ranking.
2. **OBSERVED → CANDIDATE principle:** lossy candidates always return to exact
   records.
3. **OBSERVED → CANDIDATE principle:** structural computation can be bounded while
   lexical indexing stays untruncated.
4. **OBSERVED → CANDIDATE principle:** ambiguous erased evidence should return an
   ambiguity class instead of manufactured certainty.
5. **OBSERVED → CANDIDATE principle:** ranking components and cutoff reasons are
   inspectable.
6. **OBSERVED → CANDIDATE principle:** stale indexes are rejected explicitly.

## What must not be ported as-is

- **REJECTED for scale:** one monolithic JSON index and full rewrite.
- **MEASURED support for that rejection:** the current Zeta version-4 generated
  artifact is about 534 MB for 1,341 documents; it is not a File Manager-scale
  storage baseline.
- **REJECTED for live consistency:** whole-source digest on every load as the only
  currentness protocol.
- **REJECTED for scale:** all-record structural scan without a measured corpus
  threshold.
- **REJECTED for long content:** cubic subsequence enumeration without a strict
  bound or streaming replacement.
- **REJECTED as semantic claim:** ConeDAG similarity.
- **REJECTED as filesystem identity:** hashes, paths, vectors, or captions alone.
- **REJECTED as established policy:** Zeta field weights, kind priors, fusion
  weights, and `1/e` cutoff.
- **REJECTED as sufficient phrase index:** scanning flattened token streams.

## Future Go service boundary

**CANDIDATE, not DECIDED.** A Go service is worth testing because it can host
filesystem adapters, bounded worker pools, a query API, and explicit lifecycle in
one separately killable process. Language choice does not solve the data model.

If admitted to a spike, the service boundary should expose:

- root-policy configuration and scan/event watermarks;
- exact record upsert/delete/move observations with idempotency keys;
- snapshot-consistent exact lookup and scoped query;
- named candidate/ranking components and evidence anchors;
- rebuild, integrity-check, quarantine, and derived-layer erase operations;
- resource counters: resident bytes, idle CPU, bytes read/written, backlog,
  compaction, write amplification, and extractor time;
- no GUI types, no mutation authority, no network discovery, and no requirement
  that navigation wait for it.

**Alternatives that must remain fair:** an in-process native service boundary;
SQLite/FTS-backed storage; a purpose-built segmented index; and hybrid exact-store
+ replaceable lexical/semantic projections. Go loses if its runtime footprint,
packaging, platform-event integration, tail latency, or write behavior fails the
same gates.

## Open architect decisions this research cannot close

- indexed roots and offline catalogues (I001–I003);
- resource and write budgets (I004–I005, T006–T012);
- content scope and prohibited types (I006–I010);
- inspectability, rebuild, corruption, and staleness behavior (I012–I017);
- query language, scope, explanations, and ambiguity UI (J001–J018);
- semantic privacy, models, evidence, and deletion (K001–K022);
- format migration and filesystem-as-sole-truth posture (V008, W001).
