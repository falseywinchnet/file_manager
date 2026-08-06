# Lexicon — Orchestrator interface negotiation

Status: **proposal intake; no operation admitted**.

## Proposed family

`ORC-LEX-001` would name bounded local lexical lookup, immutable corpus
generation enumeration, typed result provenance and truthful availability.

Orchestrator owns request/result meaning, provider identity, enablement,
availability, budgets, cancellation, settings/audit and File Manager merge
policy. Lexicon owns the data model, store, source packages, normalization and
lookup implementation. Games/Crossword owns puzzle generation.

## Forbidden edges

- Lexicon record presented as an Engine file/path record;
- Lexicon or plugin worker writing Engine indexes;
- provider-supplied GUI controls, native windows or result-card layout;
- ambient web lookup/download;
- query history deposited into personal semantic hives without a separate
  explicit capability;
- corpus-generation change during one pinned enumeration job.

## Required fixtures

- exact term with multiple senses and source attribution;
- normalized variant and explicitly recorded inflection;
- missing term, disabled provider, missing corpus and corrupt package;
- timeout/cancellation and response-budget truncation;
- File Manager file results unaffected by provider failure;
- pinned Shakespeare corpus enumeration and generation replacement;
- forged provider/package identity and forbidden UI/Engine mutation attempt.

Lexicon replies after LX0–LX2. The registry may list this family as proposed,
but no runtime ABI freezes from this note.
