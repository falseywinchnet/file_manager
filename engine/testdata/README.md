# Test-data boundary

Only generated, public, or explicitly consented fixtures belong here.

Required fixture families:

- `tiny-real`: 10,000 non-sensitive metadata-only records;
- `wide-1m`: one generated million-entry directory model;
- `mixed-1m`: deep/wide Unicode, case, link, package, sparse, and zero-byte mix;
- `mixed-10m`: deterministic virtual metadata fixture;
- `storm`: create/rename/move/delete/replace sequences with dropped events;
- `lexical-judged`: exact, phrase, prefix, typo, predicate, scoped, and no-result
  judgments;
- `rank-fusion-judged`: conflicting exact, lexical, fuzzy, structural, and plugin
  partial rankings with expected evidence and certainty ordering.

Generators must publish their seed and schema. Never copy a private home-tree
listing into this directory.

Current frozen fixtures:

- `protocol/v0/` — JSONL version/shutdown conformance messages;
- `catalog/v1/object-binding.hex` — exact reference object/binding canonical
  serialization;
- `workload/v1/correctness.sha256` — digest of the generated, non-sensitive
  `internal/workload.CorrectnessV1` corpus. M2 candidates and controls must
  consume this logical corpus without weakening its hard-link, symlink,
  Unicode, case, natural-number-name, or metadata distinctions.
- `generation/v1/correctness.segment.sha256` — byte length and SHA-256 of the
  immutable format-v1 segment produced from `correctness-v1` at generation 7.
  Linux, macOS, and Windows builds must reproduce it exactly.
