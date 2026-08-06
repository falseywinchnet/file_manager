# Lexicon implementation sequence

Status: **post-gate candidate plan; no implementation authorized**.

## L0 — source manifest and deterministic importer

Pin one tiny test dictionary and one tiny Shakespeare fixture. Build a replayable
importer producing source/attestation/headword/sense/corpus records and a stable
digest. Preserve rejected/ambiguous inputs.

## L1 — exhaustive reference lookup

Implement a simple sorted/in-memory reference supporting exact and admitted
normalization only. Establish golden fixtures for homographs, inflections,
punctuation, missing terms, bounds and provenance.

## L2 — measured store/index candidates

Compare pragmatic immutable or embedded store/index layouts against the
reference using cold/warm lookup, startup, RSS, page reads, package size, build
time and corruption behavior. Select only through an ADR; do not invent another
general database without evidence.

## L3 — provider boundary

Implement the selected trusted/supervised projection behind the reconciled
Orchestrator lexical contract. Prove cancellation, limits, unavailable/corrupt
packages, version mismatch, no UI injection and no Engine mutation.

## L4 — File Manager search dogfood

Exact term produces a clearly typed definition result alongside unchanged file
results. Provider timeout, disablement and absence remain quiet and bounded.

## L5 — Shakespeare corpus and Crossword

Build the approved full corpus profile reproducibly; publish immutable
enumeration/prefix/shape access; generate fixed Crossword fixtures with corpus
and clue provenance.

## L6 — native packaging and release

Package code/data separately as decided, complete attribution, updates/
uninstall, migrations, local docs, three-platform evidence and Malkuth manifest
entries.

## Measurements

- cold/warm exact lookup p50/p95/p99/worst and deadline misses;
- provider startup, RSS, idle wakeups, package bytes and pages read;
- import/index build time, peak memory and deterministic digest;
- normalization/source coverage and rejection counts;
- corpus enumeration/prefix latency during Crossword generation;
- corrupt/truncated/adversarial package containment;
- File Manager search latency with provider fast, slow, absent and disabled.
