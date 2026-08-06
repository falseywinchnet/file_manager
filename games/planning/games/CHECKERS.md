# Checkers plan

Status: **GIVEN module and non-neural opponent; rules profile open**.

## Purpose

Offer the collection's one conventional adversarial game and dogfood a custom
board, forced-move cues, piece animation, cancellable background search and
human-readable move evaluation.

## Rules choices

American/English draughts is the recommended first profile: 8×8 board, twelve
pieces, mandatory capture, multi-jump continuation and king promotion. The
timing of promotion during a capture sequence, maximum-capture variants, draw
rule and alternate draughts profiles require explicit selection.

## Exact model

Use a compact board representation plus a deliberately simple array reference.
Differential tests compare move generation, forced captures, multi-jumps,
promotion, repetition/draw state and terminal result over generated positions.

## Opponent candidate

- iterative-deepening negamax/minimax with alpha-beta pruning;
- deterministic move ordering and transposition table;
- bounded time/node budget with cancellation;
- hand-authored evaluation whose features and weights are inspectable;
- optional proven endgame table only if its generator, size and license are
  recorded.

This is a search opponent, not a neural network and not a semantic/AI service.
Difficulty changes search budget and perhaps admitted evaluation terms; it does
not secretly change rules or pieces.

## Surface and dialogs

One board/status surface, no panels. New Game/side/difficulty, Rules, Move
Explanation and result dialogs are owned popups. Search progress must not add a
dashboard or block window input indefinitely.

## Dogfood and evidence

- forced capture, branching multi-jump, promotion and draw fixtures;
- exhaustive small-position agreement with reference move generator;
- deterministic best move for fixed budget/seed/build;
- cancellation on new game, close and undo;
- search latency/nodes/table memory across difficulty levels;
- keyboard-only play, color-independent piece semantics and reduced-motion
  movement.

## Exclusions

No online opponent, ELO, chat, matchmaking, opening service, neural model,
cheating difficulty, or chess/Go expansion through this module.
