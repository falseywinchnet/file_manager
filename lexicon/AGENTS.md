# Lexicon subproject operating instructions

This is the planning-only first-party lexical provider and corpus project for
Malkuth. It proves that a local term can produce a typed definition result and
that an application such as Crossword can consume a controlled word corpus
without requiring AI, semantic hives, or absorption into Engine's file index.

## Current permission

Corpus/source research, licensing/provenance analysis, paper schemas, query and
benchmark fixtures, and Orchestrator negotiation are open. Provider code,
packaged dictionaries, generated corpora and runtime registration remain closed
until `planning/DEPENDENCY_GATES.md` passes and the grand architect explicitly
opens implementation.

Before work, read:

1. `README.md`
2. every file under `planning/` in the order given by `planning/README.md`
3. root planning records and accepted ADRs, especially ADR-013
4. Orchestrator registry and plugin/capability contracts
5. Games Crossword plan

## Hard boundaries

- Lexicon is not a desktop dictionary application and owns no native window,
  controls, styling, panels, search box, or web UI.
- It does not enter Engine's authoritative file catalogue and does not
  masquerade definitions as files.
- It uses no ambient web lookup at runtime. Corpora are pinned, local, licensed,
  versioned and auditable.
- It does not require embeddings, neural networks, personal semantic memory,
  semantic-fact APIs, or AI-generated definitions/clues.
- Exact source text, normalized entries, senses and crossword clues are distinct
  records with provenance. Do not manufacture one from another silently.
- Eventual first-party-provider versus supervised-plugin placement remains open
  until its update and trust model are decided.
- No result can overwrite exact filesystem evidence or mutate source files.
