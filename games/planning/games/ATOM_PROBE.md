# Atom Probe hidden-clues game plan

Status: **GIVEN hidden-clues module; final name and exact rules open**.

## Purpose

Build a compact spatial deduction game in the family of hidden-atoms/beam-probe
puzzles. It exercises custom grid drawing, directional tokens, bounded clue
animation, hypothesis marking and explanation without becoming another direct
competition game.

## Rules candidate

A board hides a known count of atoms. The player launches probes from boundary
ports. A probe may pass through, be absorbed, deflect, or reflect according to a
small local rule. The player marks hypotheses and submits a final configuration,
seeking correctness with few probes.

The exact neighbor geometry, edge reflection, ambiguous outcome rules, atom
count, board size and scoring must be fixed before implementation. The working
name avoids claiming compatibility with any particular historical game.

## Model and oracle

- immutable hidden board from deterministic seed;
- pure probe tracer returning an exact typed outcome and optional path for
  diagnostics, not normally exposed during play;
- simple exhaustive board enumeration for small grids;
- generator rejects duplicate-equivalent or insufficiently discriminable boards
  under an architect-approved criterion;
- hint explanation cites probe outcomes and remaining candidate count.

## Surface and dialogs

One board with boundary probe ports and compact status, no panels. New Game,
Rules, Hint Explanation and result/reveal are owned dialogs. Hypothesis markers
are board objects, not a sidebar inventory.

## Animation

The probe cue may travel to its outcome but must not reveal a hidden internal
path unless rules allow it. Reduced motion highlights origin and result. Sound
cannot distinguish outcomes by itself.

## Dogfood and evidence

- exhaustive tracer comparison on bounded boards;
- edge/corner/reflection fixture corpus;
- deterministic remaining-candidate counts after probes;
- keyboard-only boundary-port selection and hypothesis marking;
- no hidden-information leak through animation duration, damage, accessibility
  labels, logs or save files.

## Exclusions

No opponent, time-pressure default, online puzzle feed, invisible random rule
changes, or use of the Kolmogrov/file-search project merely because the game is
about hidden structure.
