# Games architect interview

Status: **prepared; no unanswered choice inferred**.

The future Games task spends at least one turn refining these questions before
implementation. Each answer receives gains, losses and viable alternatives.

## Collection questions

1. One Games launcher with child windows, nine executables in one repository,
   or a hybrid package with direct launch aliases?
2. Which game is the first GUI.Forms dogfood target? Recommended comparison:
   Four Pegs for bounded controls, Solitaire for drag/z-order, or Switchbox for
   animation.
3. Do saved games resume automatically, only through Save/Load dialogs, or not
   at all initially? Which games need undo?
4. Are hints free explanations, limited moves, score-affecting actions, or
   selectable per game?
5. What local statistics are worth keeping? Wins/losses, time, move count and
   best known solution are distinct; none requires a leaderboard.
6. What sound family is desired, and is silent the default? Confirm reduced
   motion and no sound-only clues.
7. Choose card/board/character art direction and whether every resource is
   built-in, theme-replaceable, or both.
8. Mouse and keyboard are required; should controller/touch be planned now or
   deferred to the Malkuth 3.0 touch horizon?
9. Confirm public names for Atom Probe, Four Pegs, Switchbox and Rendezvous
   Riders after legal/name review.
10. Should generation expose seeds visibly, through an advanced New Game
    dialog, or only in diagnostics/replay exports?

## Per-game closure

For each admitted game, review its file under `games/` and decide:

- exact rules profile and accepted house rules;
- legal move and scoring/time objective;
- difficulty source and whether optimality can be certified;
- allowed dialogs and primary-window chrome;
- animation cancel/input behavior;
- hint/explanation depth;
- saved-state/statistics requirements;
- first real dogfood workflow and definition of done.

## Interview output

Produce a labelled decision ledger, selected first module, shared-kernel delta,
per-game unresolved list, required ADRs, dependency status and exact smallest
post-gate vertical slice. Do not implement merely because the interview ended.
