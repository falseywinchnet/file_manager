# Architect handoff 001: accepted engine direction

Status: **mandatory DECIDED instruction**.

Source of authority:
[`../../decisions/ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md`](../../decisions/ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md).

This document is the engine-local shim for the grand architect's acceptance of
ENG-01 through ENG-12 on 2026-08-05. It supersedes older `CANDIDATE` wording
where that wording conflicts with the decisions below. It does not erase the
required control implementations or measurement gates.

## Direction to the active worker

1. Continue M1. Do not wait for a final page codec.
2. Make the exhaustive reference catalogue use objects plus path bindings.
3. Treat approved roots as exclusive shards organized under lightweight volume
   manifests; never create a redundant volume-wide copy.
4. Shape the durable implementation toward bounded immutable root generations
   and an atomic manifest. B+ trees and SQLite remain falsification controls,
   not coequal production directions.
5. Persist committed native observation watermarks, not a second ordinary
   per-event journal. Gaps are explicit and repaired by reconciliation.
6. Use adaptive bounded publication. Preserve committed-generation query
   determinism; do not add an ephemeral query overlay without evidence.
7. Keep catalogue, bindings, ordered metadata, lexical postings, and optional
   channels logically separate even when one generation commits them together.
8. Keep the whole catalogue off the Go heap. Start with sorted reference blocks
   and introduce FSTs, filters, bitmaps, compression, mapping, and custom caches
   only through differential measurements.
9. Implement deterministic evidence tiers before any voting/fusion system.
   Fuzzy filename candidates use transpositions plus character grams and are
   verified with weighted bounded Damerau–Levenshtein.
10. Treat the exact catalogue as current-plus-one-major migratable. Treat
    derived indexes as disposable. Use stable export rather than promising a
    public internal file format.
11. Preserve the canonical API model. JSONL remains the replay/AI/CLI transport;
    production local IPC begins with framed JSON and separated query/admin
    authority. Binary encoding must earn admission.
12. Enforce external placement policy exactly:
    - directly attached removable/local external volumes offer only on-volume
      full indexing or no persistent catalogue;
    - they do not retain a local full or coarse offline catalogue;
    - explicitly admitted network drives/other machines use remote full index
      plus local coarse catalogue semantics;
    - the remote coarse catalogue contains only name, path, type, size, and
      timestamps, and unavailable results are marked unavailable.
13. At 65,536 immediate children, default that directory to light indexing:
    identity, name/binding, type, size, and basic timestamps only.
14. Make every benchmark report against the accepted performance constitution
    in the ADR. Do not quietly weaken a target after a miss; record the miss and
    its cause before asking for a revision.

## Immediate M1 completion order

1. Freeze the reference `Object`, `Binding`, `Volume`, `Root`, and `Incarnation`
   algebra with exact serialization fixtures.
2. Prove hard-link, symlink, rename, move, and replace-at-path behavior in the
   sandbox identity oracle.
3. Complete approved-root manifests and nested-root pruning.
4. Produce exact path/name/basic-metadata queries over a committed immutable
   in-memory reader generation.
5. Expose concise evidence, staleness, generation, and unavailable state through
   the transport-neutral API and JSONL conformance fixtures.
6. Generate the canonical workload and correctness digest that M2 controls must
   consume unchanged.

## Do not reinterpret

- “Immutable segments” does not authorize an unbounded general-purpose LSM.
- “Platform identity” does not authorize treating inode/file ID alone as an
  eternal incarnation.
- “Coarse remote catalogue” does not authorize thumbnails, extracted content,
  semantic facts, or silent network discovery.
- “Light mode” does not authorize dropping names or refusing to catalogue a
  giant directory.
- “Evidence tiers” does not authorize collapsing provenance into one score.
- A performance target is not a measured claim until the protocol is executed.
