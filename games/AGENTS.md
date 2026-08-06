# Games subproject operating instructions

This is the planning-only project for Malkuth's small first-party games
collection. It exists to make pleasant deterministic games and to dogfood
GUI.Forms controls, dialogs, animation, input, drawing, accessibility, and
resource packaging. It is not a game platform, social service, or excuse to add
a game engine runtime.

## Current permission

Research, architect interview, paper design, reference algorithms, fixtures,
and benchmark planning are open. Application source, build files, art, generated
puzzles, and executable prototypes remain closed until the gates in
`planning/DEPENDENCY_GATES.md` pass and the grand architect explicitly opens
implementation.

Before work, read:

1. `README.md`
2. `planning/README.md`
3. `planning/CHARTER.md`
4. `planning/SHARED_GAME_KERNEL.md`
5. every admitted plan under `planning/games/`
6. `planning/DEPENDENCY_GATES.md`
7. `planning/ARCHITECT_INTERVIEW.md`
8. `planning/IMPLEMENTATION_SEQUENCE.md`
9. `planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`
10. root planning records and accepted ADRs, especially ADR-013

## Hard boundaries

- No .NET, Java, Godot, bundled browser engine, network account, multiplayer
  service, store, ads, telemetry, achievements platform, daily streaks, or
  casino presentation.
- GUI.Forms is consumed through a named public snapshot. Games do not patch it
  or introduce another renderer/game engine.
- No game has a persistent left or right panel. Secondary tools are ordinary
  owned popup dialogs. Only File Manager and its embedded picker may use panels.
- Animations are bounded, event-driven, deterministic where tested, and
  replaceable by reduced-motion state transitions. No perpetual redraw while
  idle.
- Game truth is held by a deterministic model independent of animation, sound,
  wall-clock scheduling, or renderer state.
- A non-neural opponent or solver must expose its rules, seed, limits, and
  reference oracle. Do not call it AI merely because it searches.
- Save/state/settings/help use admitted Orchestrator application contracts; no
  private settings authority is invented here.
- Competition is minimized. No online opponent or global ranking is inferred.
