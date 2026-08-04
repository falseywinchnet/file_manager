# Zeta search research operating instructions

This directory is an isolated research subproject. It is not an architecture
decision and it does not contain production File Manager code.

## Scope

- Read Zeta only for indexing, exact lookup, lexical retrieval, structural
  similarity, ranking, persistence, and search evaluation.
- Do not import zeta mathematics, proof-search code, or unrelated algorithms.
- Do not mutate `/Users/quentinkuttenkuler/zeta`.
- Keep every write for this research round below `research/zeta_search/`.
- Treat the filesystem and exact stored records as authoritative. Approximate
  structures may only propose candidates.
- Keep exact identity, candidate generation, ranking, and result presentation as
  distinct layers.
- Go is a **CANDIDATE** for a future indexing service, not **DECIDED**.

## Evidence grammar

Use the repository-wide labels **GIVEN**, **OBSERVED**, **MEASURED**,
**HYPOTHESIS**, **CANDIDATE**, **REJECTED**, and **DECIDED** exactly as defined in
the root `AGENTS.md`. Every measurement needs a revision, corpus, environment,
command, and raw result. Preserve negative results.

## Copying and provenance

Prefer source locators and checksums over copied code. If a snippet is ever
copied, record its exact origin, revision, checksum, license, and local purpose in
`PROVENANCE.json`. Do not make a derivative implementation by accident.

## Change boundary

Do not edit parent planning records or production directories from this subtree.
An explicit architect-approved decision record is required before a candidate in
these documents may control implementation.
