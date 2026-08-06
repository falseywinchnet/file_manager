# Sudoku plan

Status: **GIVEN module; grid sizes and hint vocabulary open**.

## Purpose

Provide a calm logic puzzle and exercise grid navigation, compact text entry,
candidate marks, constraint highlighting, generated-puzzle validation and
stepwise explanations.

## Initial rules candidate

- standard 9×9 grid with 3×3 boxes and digits 1–9;
- one unique solution required for generated puzzles;
- pencil marks, conflict indication, check-cell/check-puzzle and hints are
  individually configurable;
- timed/competitive scoring is not a default objective.

## Solver/generator split

- A simple exact-cover/Algorithm X solver verifies solution count and catches
  generator defects.
- A separate human-technique solver supplies hints and difficulty evidence:
  singles first, then only architect-admitted techniques with named reasoning.
- Puzzle generation begins from a complete valid grid, removes clues under
  uniqueness checks, and records seed/generator/solver versions.
- “Easy/medium/hard” is never based only on clue count; it names the techniques
  and branching the human solver required.

## Surface and dialogs

One grid and compact numeric controls/status, no panels. New Puzzle/difficulty,
Rules, Hint Explanation and result dialogs are owned popups. Candidate marks
live inside cells, not a side palette.

## Interaction

Keyboard arrows/numbers/delete, pointer cell selection and optional small number
buttons. Color never carries the only distinction between clue, entry,
candidate, conflict and hint. Reduced motion avoids celebratory sweeps while
retaining terminal feedback.

## Dogfood and evidence

- known zero/one/multiple-solution corpus;
- generator uniqueness and deterministic seed replay;
- every human hint verified as legal and progress-making by the exact oracle;
- full keyboard and screen-reader grid navigation;
- large hint corpus with no stale highlight after edit/undo;
- generation latency/rejection rate by admitted difficulty.

## Exclusions

No daily streak, account sync, competitive leaderboard, opaque “AI hint,”
advertising, or arbitrary puzzle packs fetched from the web.
