# Solitaire plan

Status: **GIVEN module; CANDIDATE Klondike rules profile**.

## Purpose

Deliver a quiet one-player card table and dogfood overlapping retained objects,
z-order, hit testing, drag capture, double-click actions, keyboard equivalents,
deal animation, local damage and undo.

## Initial rules candidate

- Klondike with a standard 52-card deck, seven tableau columns, four
  foundations, stock and waste;
- draw-one as the first profile; draw-three remains an optional later profile;
- exact redeal count, scoring, timed mode, auto-finish and solvable-deal policy
  require architect selection;
- deals identify generator and seed for reproducible failures.

## Model and oracle

Represent card identity, face state, ordered piles, stock pass and legal moves
without UI coordinates. A simple reference validator enumerates legal moves and
checks invariants: one of each card, no duplicate ownership, foundation order,
tableau alternation/descending rule and win predicate. Saved/replayed games name
the rules profile.

## Primary surface and dialogs

The primary window is the table plus compact menu/status. There are no panels.
New Deal/options, Rules, Hint/explanation and result/statistics are owned popup
dialogs. A hint highlights a legal move on the table; it does not open a sidebar.

## Interaction

- click/drag a legal run; click or keyboard-select source and destination as an
  accessibility equivalent;
- reject illegal drops by returning the cards and explaining on request;
- double-click-to-foundation and auto-finish are separately configurable;
- deal animation is bounded and skippable; reduced motion places immediately;
- input during motion is either queued once or rejected consistently.

## Dogfood and evidence

- scripted deal, stock pass, run move, flip, foundation move, undo and win;
- randomized legal-play invariant corpus and corrupted-save rejection;
- hit-test/z-order correctness at multiple scales;
- dirty-region and allocation evidence for dragging one run;
- keyboard-only completion of a prepared near-win fixture.

## Explicit exclusions

No gambling currency, daily challenge, online leaderboard, advertisements,
cloud deal history, card store, or multiplayer race.
