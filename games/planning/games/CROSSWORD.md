# Crossword plan

Status: **GIVEN module and Shakespeare-first vocabulary; clue/grid policy open**.

## Purpose

Create and solve compact crosswords from a deliberately bounded corpus, proving
Lexicon integration, grid editing, clue/result dialogs, constraint generation,
local provenance and graceful provider unavailability.

## Initial corpus law

- Candidate entries come only from a pinned, normalized Shakespeare vocabulary
  manifest supplied by Lexicon.
- Edition, source digest, tokenization, case, apostrophe/hyphen handling,
  inflections, names, obscenities, archaic forms and minimum length are explicit.
- Vocabulary admission does not create a clue. Dictionary definitions and
  manually authored/mechanical clue records have separate provenance.
- Later, users may select admitted corpus subsets through a New Puzzle dialog.

## Generator candidate

Start from curated symmetric grid templates rather than generating arbitrary
black-square topology. Fill slots as a constraint-satisfaction problem using
backtracking, most-constrained-slot ordering, prefix indexes and arc-consistency
pruning. Record seed, template, corpus and clue-set versions. A completed puzzle
must have a verified fill and the selected uniqueness policy.

## Surface and dialogs

One crossword grid with current clue/status; no side panels. The full Across/
Down clue list may be an owned modeless popup dialog, opened and closed by a
command, because persistent side panels are forbidden. New Puzzle/corpus,
Rules, Hint and result details are dialogs.

## Interaction

Pointer and keyboard select cells/direction; typing advances; space/backspace,
word navigation and rebus behavior require decisions. Error checking must be
user-invoked or clearly configured, not silently punitive. Clues remain usable
if Lexicon becomes unavailable after the puzzle is generated.

## Dogfood and evidence

- pinned tiny Shakespeare corpus fixture and production-scale corpus manifest;
- deterministic generated fill and clue provenance;
- no-entry, ambiguous-fill and generation-budget exhaustion states;
- keyboard/screen-reader cell and clue navigation;
- provider missing/version mismatch behavior;
- generation latency, backtracks, candidate counts and cancellation.

## Exclusions

No web-fetched daily crossword, account, leaderboard, opaque AI clue writing,
copyrighted clue scraping, or claim that every dictionary definition is a good
crossword clue.
