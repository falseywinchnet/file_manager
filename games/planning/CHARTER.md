# Games charter

Status: **GIVEN roster and product direction; exact packaging, rulesets and art
remain open**.

## Mission

Provide a small cabinet of games that are quick to understand, pleasant to
touch, honest about their rules, and useful for exercising the retained UI
framework under interactions that ordinary utility applications rarely reach.

## GIVEN direction

- The project is named **Games** and contains the nine accepted modules listed
  in the planning index.
- Competition is minimized. Checkers may offer a local opponent; the collection
  has no multiplayer or competitive service.
- Switchbox and Rendezvous Riders are deliberately animation-heavy fidget games
  whose animation also communicates bounded game information.
- Four Pegs uses a simple four-peg hidden-code ruleset.
- Crossword begins with words admitted by a versioned Shakespeare corpus and
  later permits user-selected dictionary/corpus subsets.
- Every module uses GUI.Forms directly and helps expose defects in controls,
  custom drawing, damage tracking, animation, input, dialogs, accessibility,
  resource packs, and native hosting.
- No game uses persistent side panels. Rules, new-game options, hints, result
  details, and preferences are owned popup dialogs.

## Collection shape

- one board/table/playfield per ordinary window;
- no mandatory launcher topology has yet been selected;
- menus and a compact factual status line are allowed where useful;
- direct manipulation and keyboard operation are first-class;
- sound may enrich feedback but never carries the only clue;
- reduced motion replaces animation with legible state transitions;
- deterministic seeds make every generated board or deal reproducible;
- saved games are compact versioned state, not accounts or cloud profiles.

## Ownership

- Each game module owns rules, legal moves, state, scoring, generation, solver
  or opponent behavior, serialization and replay.
- The shared Games kernel owns deterministic seeds, clocks, animation commands,
  undo records, common dialogs, card/board primitives, local statistics shape,
  and conformance fixtures—not game-specific truth.
- GUI.Forms owns controls, retained drawing, timers, input, accessibility,
  dialogs, resources and native hosting.
- Orchestrator owns application identity, settings namespace, local help topic
  registration, availability and package manifest semantics.
- Lexicon owns word/sense/corpus evidence used by Crossword. Crossword owns
  grids, clue selection and puzzle validity.

## Explicit non-goals

- network play, matchmaking, friends, chat, spectators or LAN opponents;
- leaderboards, achievements platform, daily rewards/streaks or monetization;
- chess, Go, Towers of Hanoi, generic game emulation or an extensible mod SDK;
- a general-purpose game engine, physics engine, scene graph or GPU requirement;
- procedurally generated content described as intelligent without an oracle;
- persistent sidebars, dashboards, feeds, card-store ornament or casino sound.
