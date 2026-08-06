# Games implementation sequence

Status: **post-gate candidate plan; no implementation authorized**.

## G0 — deterministic kernel and test clock

Build the small state/command/replay/save envelope, seed/version handling,
animation command stream and reference fixtures without GUI.Forms. Avoid a
general scene graph or game-object framework.

## G1 — bounded controls proof

Implement Four Pegs as the first recommended slice: repeated controls, color
plus symbol accessibility, keyboard focus, result dialog, deterministic seed,
headless replay and local help. Inspect live and close it.

## G2 — card-table proof

Implement Solitaire and FreeCell on shared card primitives while keeping rules
separate. Exercise z-order, hit testing, drag/drop, keyboard moves, undo if
admitted, legal-move explanations and local damage.

## G3 — grid/text proof

Implement Sudoku; then Crossword after the Lexicon gate. Exercise cell editors,
validation, clue dialogs, printable/exportable state only if admitted, and
human-readable hints.

## G4 — board/search proof

Implement Checkers with its reference oracle and bounded non-neural opponent.
Measure response budgets and prove deterministic cancellation/fallback.

## G5 — animation puzzle proof

Implement Atom Probe, Switchbox and Rendezvous Riders. Exercise custom drawing,
semantic clues, bounded animation, interrupted transitions, reduced motion,
test clocks and exact solution certificates.

## G6 — native collection dogfood

Close focus, dialog, input, font, scaling, sound, accessibility, resource,
save/settings and performance gaps on physical macOS, Windows and Linux. Each
module reports availability independently.

## G7 — Malkuth release candidates

Package only modules that pass their individual gates. Publish rules/help,
licenses, seeds/corpus versions, save compatibility, performance/accessibility
records and known issues in the Malkuth manifest.

## Measurements

- startup/first board, RSS and idle wakeups;
- input-to-state and input-to-visible p50/p95/p99/worst;
- dirty pixels, allocations and missed frames per animation/move;
- solver/opponent latency, nodes, cancellation and oracle agreement;
- generator uniqueness/difficulty agreement and rejection rates;
- deterministic replay/save round trips;
- keyboard-only, reduced-motion, high-contrast and scale workflows.
