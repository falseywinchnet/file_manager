# Four Pegs deduction game plan

Status: **GIVEN four-peg module; public name and duplicate-color rule open**.

## Purpose

Provide the smallest complete GUI.Forms game proof: repeated controls,
selection, drag/click input, color plus symbol semantics, result evaluation,
deterministic state, dialogs and accessible feedback.

## Initial rules candidate

- secret code of four pegs selected from six symbols/colors;
- a bounded number of guesses;
- feedback reports correct symbol and position versus correct symbol in the
  wrong position without revealing which peg;
- duplicates may be allowed or forbidden; that choice materially changes
  evaluation and difficulty;
- code length, palette size and attempt count are fixed by the selected profile.

## Exact model

A pure evaluator handles duplicates with counted multisets after exact-position
matches. Exhaustively enumerate every code/guess pair for the initial bounded
domain and compare the optimized evaluator to the simple reference. Generator
seed and profile reproduce every game.

## Surface and dialogs

One guess board and peg palette, no panels. New Game/options, Rules, optional
Hint Explanation and result dialogs are owned popups. Pegs pair color with
shape/pattern/label so color vision is never required.

## Interaction

Click or drag a peg, keyboard select/place/remove, submit complete row, receive
ordered-independent feedback, and advance. Guess feedback animation is short,
skippable and replaced by immediate markers under reduced motion.

## Dogfood and evidence

- exhaustive evaluator truth table;
- duplicate-heavy fixtures;
- focus order and keyboard-only complete game;
- high-contrast/color-independent recognition;
- deterministic replay/save if admitted;
- zero idle wakeups and bounded dirty regions after feedback.

## Exclusions

No branded compatibility claim before name/legal review, online code sharing,
leaderboard, monetized palette or deceptive adaptive secret changes.
