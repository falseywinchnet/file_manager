# Shared deterministic game kernel

Status: **CANDIDATE architecture for interview and measurement**.

## Separation law

Every module is split into four layers:

```text
rules/state -> command/replay -> presentation model -> GUI.Forms view/dialogs
```

The rules/state layer has no GUI.Forms, filesystem, wall-clock, sound, locale,
or platform dependency. Rendering never decides legality or scoring. Animation
acknowledges a committed state transition; it does not create one.

## Shared objects

- `GameId`, semantic version and rules-profile identity;
- reproducible seed with named generator/version;
- immutable state snapshot plus bounded command/result records;
- legal-action enumeration and action rejection reason;
- deterministic replay digest and optional undo boundary;
- monotonic presentation clock separate from game logic;
- animation command: target, start/end state, duration, easing, cancellation and
  reduced-motion replacement;
- local result/statistics record without a global ranking service;
- versioned save envelope with game payload, seed, rules profile and checksum;
- local help/rules topic IDs.

## GUI.Forms dogfood matrix

| Capability | Proving modules |
|---|---|
| card drag, overlap, z-order, keyboard alternatives | Solitaire, FreeCell |
| custom board drawing and animated pieces | Checkers |
| grids, text entry, selection and validation cues | Sudoku, Crossword |
| bounded clue visualization and spatial selection | Atom Probe |
| repeated peg controls and accessible color/symbol pairing | Four Pegs |
| character/switch animation and event interruption | Switchbox |
| animated graph edges, paths and two moving tokens | Rendezvous Riders |
| owned modal/modeless dialogs and focus restoration | all modules |
| reduced motion, high contrast and scale | all modules |

## Animation discipline

- Invalidate only dirty board regions; idle boards stop requesting frames.
- Cap concurrent animations and queue or coalesce nonessential transitions.
- Logic commits atomically before presentation begins. Input during animation is
  either explicitly buffered, rejected, or allowed by a game-specific rule.
- Test clocks step deterministically without sleeps.
- Reduced motion uses immediate placement, dissolve, highlight, or textual clue
  updates rather than simply deleting feedback.
- Sound is independently suppressible and never the sole state indicator.
- Record animation count, dirty pixels, allocations, input-to-visible response,
  missed frames, cancellation, and idle wakeups.

## Dialog law

There are no persistent side panels. Common owned popup dialogs may cover:

- New Game / deal / difficulty / seed;
- Rules and local Help;
- Hint or explanation;
- result and local statistics;
- accessibility, sound and motion preferences.

Modal dialogs block a necessary decision. Repeated tools such as a hint
explanation may be modeless only when their state follows the owner safely.
Closing the game window closes its owned dialogs and restores no stale focus.

## Persistence

- Initial slices may keep no automatic save. If resume is admitted, saves are
  bounded, local, checksummed, versioned, and atomically replaced.
- Never store executable code, renderer objects, native pointers, or external
  resource paths in a game save.
- Unknown versions fail read-only with an explanation; migrations preserve the
  original until verified.
- Statistics are local factual counters. No behavioral profiling or telemetry.

## Algorithms and controls

Optimized solvers/opponents are checked against simple reference or exhaustive
oracles on bounded positions. Generated puzzles record uniqueness, difficulty
evidence and generator version. “Optimal” is displayed only when the exact
reference objective and proof/certificate are available.
