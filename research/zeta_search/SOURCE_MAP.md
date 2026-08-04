# Source map

## Revision and integrity

- **OBSERVED:** Zeta repository root:
  `/Users/quentinkuttenkuler/zeta`.
- **OBSERVED:** inspected Git revision:
  `a27cf26cdf911de6274c253873a21d876274e903`.
- **OBSERVED:** the Zeta worktree was clean when inspected on 2026-08-03.
- **OBSERVED:** no Zeta file was modified or copied into this subproject.
- **OBSERVED:** detailed SHA-256 values are recorded in `PROVENANCE.json`.

## Authoritative implementation paths

| Source | Role | What was traced |
|---|---|---|
| `mindlib/identities.py` | exact routing | reversible public/internal IDs and full-match identity lookup |
| `mindlib/search_backend.py` | exact retrieval boundary | exact factoid/topic resolution independent of ranked search |
| `mindlib/search_index.py` | production ranked search | normalization, postings, typo maps, BM25-like score, phrase scan, graph anchors, all-record similarity scan, dynamic cutoff, source-digest rebuild |
| `mindlib/similarity.py` | promoted sketch wrapper | 48-token structural bound; 80/20 ConeDAG/containment fusion |
| `mindlib/store.py:32-45` | persistence primitive | temporary file, flush, file `fsync`, atomic replace |
| `mindlib/cli.py:276-304` | query routing | exact identities bypass `SearchEngine`; other queries rank |
| `search_engine_experimental/experiments/cone_dag.py` | structural candidate sketch | signed feature hashing of content/path/position/shape and exact cosine scan |
| `search_engine_experimental/experiments/drift_retrieval.py` | containment reranker | bottom-k global ordered subsequences and local path strata |
| `search_engine_experimental/experiments/sequence_fingerprint.py` | rejected predecessor | earlier content/edge/path sketch |

## Current generated-index observation

**MEASURED, read-only inspection on 2026-08-03:** at the pinned source revision,
the existing `mind-data/search-index.json` is index version 4 and contains 1,341
documents, 72,844 posting terms, 12,816 trigram keys, 1,638 bigram keys, and 1,341
graph nodes. It occupies 534,022,217 bytes. The first inspected document stores a
390-value ConeDAG vector, 154 global-order hashes, and 89 local hashes; the sketch
size is a maximum, so a record with fewer distinct features need not fill all 256
slots.

Environment: Mac15,5, arm64, 8 GiB RAM, macOS 14.8.7 (23J520). The inspection
parsed the existing JSON with Python 3 and counted top-level collections; it did
not rebuild, time queries, or modify Zeta. This is a storage observation, not a
controlled performance benchmark.

**OBSERVED:** `results/production_search_promotion.md` reports historical index
version 3 at 403 documents and about 18.2 MB. Current source declares version 4.
The historical timing figures and current artifact size describe different
snapshots and must not be combined into one scaling curve.

## Research and evidence paths

| Source | Evidence role |
|---|---|
| `search_engine_experimental/README.md` | scope, baseline, anchor-and-verify law |
| `research/architecture.md` | production structure and explicit rejection of rainbow-table relevance |
| `research/cone_dag.md` | representation, asymptotic estimate, limits |
| `research/cone_dag_mathematics.md` | feature-map, drift, containment, and ambiguity support boundaries |
| `research/semantic_identity_architecture.md` | proposed record/sense/interpretation separation |
| `research/sense_bridge.md` | proposed local frozen encoder and versioned sense registry |
| `research/next_experiments.md` | Zeta's own outstanding evaluation gates |
| `results/initial_results.md` | small-corpus index size/build/query observations |
| `results/cone_dag_report.md` | adversarial retrieval, ablations, semantic negative |
| `results/cone_dag_drift_report.md` | failed symmetric channel, ambiguity audit, containment results |
| `results/production_search_promotion.md` | promoted fusion, timings, 48-token bound |
| `results/hybrid_secretary_cutoff.md` | exact result-boundary rule and tests |
| `experiments/benchmark_mind_search.py` | seven-case smoke-test mechanics |
| `experiments/benchmark_cone_dag.py` | equal-width baselines and synthetic transformations |
| `experiments/benchmark_drift_rerank.py` | strict versus edit-equivalent evaluation |
| `experiments/analyze_containment_sketch.py` | estimator error and subset-preservation audit |
| `tests/test_search_experimental.py` | deterministic/fixed-width/local-edit and containment unit tests |

## Read exclusions

**GIVEN:** Zeta theorem/proof work, proof-search scripts, numerical experiments,
and unrelated algorithms were excluded even where their filenames contained the
word “search.” The copied SQLite FTS5 documentation was consulted only to locate
why B-trees appear in repository text; it is third-party reference material, not
an observed Zeta implementation.

**OBSERVED:** BFFT was not needed to interpret these mechanics. Its algorithm code
was not consulted or imported.

## Provenance limitations

**OBSERVED:** Zeta result documents record corpus size and timings but not a full
machine/storage/power-state manifest. Those numbers remain Zeta-local evidence,
not File Manager performance forecasts. Reproduction on this host would still not
establish filesystem-scale behavior because the corpus and storage format are
different.
