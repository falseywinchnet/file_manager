# Zeta indexing and search research

Status: **research extraction; no architecture selected**.

This subproject traces the actual indexing and retrieval mechanisms in Zeta at
revision `a27cf26cdf911de6274c253873a21d876274e903`. It translates those mechanisms
into falsifiable File Manager experiments without importing Zeta's mathematical
research or treating a clever sketch as filesystem truth.

## Principal finding

**OBSERVED:** Zeta is not a catalogue of interchangeable exotic structures. Its
working search path is a conventional in-memory term-to-document inverted index,
fielded BM25-like scoring, bounded typo expansion, exact phrase bonus, graph
anchors, and an all-record scan of deterministic structural sketches. Its static
JSON index is rebuilt in full when a source digest changes.

**OBSERVED:** exact public identities bypass all ranking. This is the strongest
transferable design boundary:

1. exact filesystem identity and exact records remain authoritative;
2. indexes generate candidates;
3. independent channels rank candidates;
4. a result returns an exact record plus an explanation;
5. UI result-count policy runs only after ranking.

**OBSERVED / correction:** `search_engine_experimental/README.md` calls the
baseline a “positional inverted index,” but `mindlib/search_index.py` stores only
term-to-document postings and per-document field frequencies. Phrase detection
scans each candidate's stored token stream. No token positions are stored in the
postings. This discrepancy matters when estimating File Manager scale.

## Documents

- `SOURCE_MAP.md` — exact read boundary, revisions, checksums, and source role.
- `ALGORITHM_INVENTORY.md` — mechanics, complexity, invariants, evidence, and
  failures for every observed or remembered structure.
- `FILE_MANAGER_APPLICABILITY.md` — strict separation of identity, storage,
  candidate generation, ranking, and semantic projections.
- `EXPERIMENT_BACKLOG.md` — replayable next experiments, datasets, baselines,
  metrics, and rejection gates.
- `PROVENANCE.json` — machine-readable source manifest. No Zeta code is copied.

## Explicit non-findings

- **OBSERVED:** rainbow tables are mentioned only to reject them as a relevance
  mechanism. No rainbow-table implementation exists in the inspected scope.
- **OBSERVED:** Python dictionaries implement maps throughout the prototype, but
  Zeta contains no novel hashmap design or benchmark in the inspected scope.
- **OBSERVED:** no binary-search-tree or trie implementation appears in the
  inspected search code. The vendored SQLite FTS5 documentation describes
  immutable segment B-trees, but Zeta does not implement or exercise them.
- **OBSERVED:** approximate-nearest-neighbor indexing is proposed, not
  implemented. Current sketch retrieval scans every record.
- **OBSERVED:** typed semantic graphs and the learned sense bridge are designs,
  not working semantic search.

## Boundary for future work

The next step is not to port `search_index.py`. It is to run the experiments in
`EXPERIMENT_BACKLOG.md` against File Manager workloads and unresolved budgets in
`planning/QUESTION_ATLAS.md`, especially I001–I020, J001–J018, K001–K022,
T006–T012, V008, and W001/W005/W007/W008.
