# Switchbox plan

Status: **GIVEN animation-dogfood sequence puzzle; character, clues and exact
rule profile open**.

## Purpose

Create a tactile lock-deduction fidget game: a girl inside a box reaches out and
flips switches back after the player changes them. The player must discover the
correct sequence before the move budget expires. Animation carries bounded
clues but never becomes the sole clue channel.

## Candidate rules

- a row or spatial arrangement of switches starts in a known reset state;
- a hidden ordered sequence or partial order is generated from a seed;
- each player switch action consumes a move;
- the character resets incorrect/provisional switches according to the exact
  rules and exposes a clue about sequence membership, relative order or prefix
  correctness;
- completing the sequence unlocks the box before the move budget reaches zero.

Sequence length, repeated switches, prefix retention, reset timing, clue family,
move count and difficulty require architect decisions. The model must be
solvable from the information supplied; animation flourish cannot add accidental
timing clues.

## Truth and clue model

- pure hidden sequence and attempt evaluator;
- typed clue record generated before animation;
- exhaustive solver verifies that the clue history leaves at least one intended
  solution and that accepted difficulty remains solvable within budget;
- small domains are enumerated completely; larger candidates use a retained
  candidate set with a simple reference verifier;
- save/replay never exposes the secret before terminal reveal.

## Primary surface and dialogs

The box, character and switches occupy one main window. There are no panels.
New Puzzle/difficulty, Rules, Clue History/Explanation and result dialogs are
owned popups. A clue-history dialog may be modeless, but it is not docked and
must follow the current game generation safely.

## Animation contract

- switch input commits to the puzzle model immediately;
- character animation consumes the typed response and ends in the committed
  switch state;
- glance, hand, hesitation, reset order or sound may reinforce a clue only when
  an equivalent visible/textual marker is present;
- input while the character acts is explicitly queued, ignored with feedback,
  or disabled; never race-dependent;
- reduced motion replaces reaching with direct switch transition and the same
  clue marker;
- test clocks and fixed seeds reproduce every animation command.

## Dogfood and evidence

- exhaustive solvability/budget checks on first profile;
- animation interruption, window hide/close and new-game cancellation;
- no clue leakage through duration beyond declared clue semantics;
- keyboard-only switches and textual clue equivalence;
- dirty-region, allocation, frame and idle-wakeup measurements;
- character/resource fallback that leaves the game fully playable.

## Exclusions

No violence, real lock-picking instruction, opaque random punishment, purchase
of moves, daily energy, network competition, or animation that blocks reduced-
motion/accessibility use.
