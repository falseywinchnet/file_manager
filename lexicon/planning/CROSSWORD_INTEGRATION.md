# Crossword integration

Status: **CANDIDATE consumer contract; Games/Crossword owns puzzle truth**.

## Boundary

Lexicon supplies immutable corpus generations and lexical/clue records.
Crossword supplies templates, slot constraints, fill search, puzzle seed,
difficulty, clue selection and final puzzle serialization.

Lexicon never receives a GUI.Forms object or mutates an in-progress puzzle.
Crossword never queries an unversioned “current dictionary” repeatedly during
one generation job.

## Generation session

1. Crossword requests one admitted `CorpusProfileId` and pins its generation.
2. Lexicon returns word-shape/prefix indexes or paged candidates under bounds.
3. Crossword fills a curated grid through its own CSP/backtracking solver.
4. For each selected word, Crossword requests or reads clue candidates with
   provenance.
5. Crossword rejects missing/unsuitable clues or marks a deliberately
   mechanical definition clue.
6. The saved puzzle records corpus, clue pack, template, generator and seed.

## Failure behavior

- provider unavailable before generation: New Puzzle reports unavailable and
  existing saved puzzles remain playable;
- generation changes mid-job: pinned generation continues or job cancels; no
  mixed generation;
- no fill under budget: report honestly and allow another seed/template;
- word has no suitable clue: choose another fill or use an explicitly labelled
  mechanical clue under accepted policy;
- later corpus uninstall: saved puzzle carries its clue text and remains
  solvable, while provenance may report package unavailable.

## Later corpus selection

The New Puzzle owned dialog may enumerate installed, admitted corpus profiles
with language, scope, version, provenance and content flags. It does not browse
an online store. User-created corpora require a later importer/security/license
plan and do not enter through an arbitrary plugin UI.
