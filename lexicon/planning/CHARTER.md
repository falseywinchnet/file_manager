# Lexicon charter

Status: **GIVEN role; runtime placement and source set open**.

## Mission

Given a local term, return concise, inspectable dictionary evidence quickly;
given an admitted corpus profile, enumerate exactly which word forms belong to
it. Do this without turning file search into a semantic platform or adding a
standalone dictionary program.

## GIVEN direction

- Lexicon is a plugin-like or search-extension capability, not another ordinary
  app.
- A user searching an exact term can receive its definition when the provider
  is enabled.
- This is semantic-engine dogfood in the narrow sense of typed local knowledge,
  but it does not require a huge semantic API, AI, or personal memory.
- Crossword begins with words from a versioned Shakespeare corpus; later users
  choose admitted dictionary/corpus subsets.
- Orchestrator, not Engine, is the likely merge/control boundary because Lexicon
  results are not file records. Exact trusted-versus-supervised placement
  remains open.

## Record classes

1. **Source artifact:** pinned edition/database and license record.
2. **Attestation:** exact span or source entry supporting a lexical claim.
3. **Headword:** normalized lookup identity and display form.
4. **Sense:** part of speech, definition, usage labels and provenance.
5. **Form relation:** inflection, spelling variant or derivation with evidence.
6. **Corpus membership:** word form admitted by a named profile and reason.
7. **Clue record:** separate crossword-suitable prompt and answer relation.

No record class silently promotes into another.

## Ownership

- Lexicon owns source ingestion, normalization, lexical records, lookup indexes,
  corpus profiles, provenance and local package migrations.
- Orchestrator owns provider identity, enablement, availability, query budget,
  result-envelope meaning, settings, audit and host merge.
- File Manager owns search presentation and clearly distinguishes lexical cards
  from file results.
- Games/Crossword owns grid generation, clue selection, difficulty and puzzles.
- Engine continues to own exact file/path catalogue and retrieval only.

## Explicit non-goals

- standalone Dictionary/Thesaurus application;
- web search, online definition fallback or automatic corpus download;
- general ontology, knowledge graph or personal-memory store;
- embedding/vector dependency or AI-generated definitions;
- grammar checker, spell checker, translation service or writing assistant in
  the first provider;
- treating word prevalence or dictionary membership as file relevance;
- provider-supplied controls, style, popup window or side panel.
