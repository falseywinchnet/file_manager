# Malkuth future-scope ledger

Date: 2026-08-06.

Status: **GIVEN scope under ADR-013; implementation permissions remain local to
each project gate**.

This ledger routes the deliberately small applications, provider extensions,
contextual File Manager commands, and first-party plugin proofs that follow the
File Manager/GUI.Forms/Orchestrator backbone. It is not permission to implement
all of them at once or to make them Malkuth 1.0 blockers.

## Surface topology

| Consumer | Primary surface | Secondary surface | Persistent side panels |
|---|---|---|---|
| File Manager | location-oriented browser window | dialogs plus preview/properties composition | yes |
| Embedded Document Picker | bounded File Manager navigation projection | validation/overwrite dialogs | yes, only as part of the embedded browser projection |
| Paint | one canvas/document window | owned Color, Attributes, Resize, Help and other dialogs | no |
| Text Editor | one text/document window | owned Find/Replace, Characters, Help and file-information dialogs | no |
| Games | one game/board window at a time | New Game, Rules, Hint, Statistics/Result and settings dialogs | no |
| Lexicon | no ordinary application window | result cards or host-owned lookup dialogs | no provider-owned UI |
| Plugins | no trusted application window | host-rendered prompts and progress only | forbidden |

**DECIDED:** only File Manager and its embedded picker/browser projection own
persistent left/right panels. A dialog may be modal or modeless but remains an
owned popup with explicit open/close/focus behavior.

## Project routing

| Piece | Path | Shape | Current permission |
|---|---|---|---|
| Games collection | `../games/` | planning-only first-party GUI.Forms application project | interview/design open; code closed |
| Lexicon provider | `../lexicon/` | planning-only exact lexical query/corpus provider | research/design open; code closed |
| Paint color utility | `../paint/planning/COLOR_DIALOG.md` | Paint-owned modeless/modal popup dialog | planning open; code follows Paint gates |
| Text Editor characters | `../text_editor/planning/CHARACTERS_DIALOG.md` | Text Editor-owned modeless popup dialog | planning open; code follows Text Editor gates |
| Checksums and terminal | `../frontend/planning/CONTEXTUAL_BUILTIN_COMMANDS.md` | trusted context commands | planning open; implementation follows frontend/command gates |
| Desktop boundary | `../frontend/planning/DESKTOP_INTEGRATION_BOUNDARY.md` | platform integration, not a DE | planning open; implementation separately gated |
| Archive Viewer | `../orchestrator/proposals/first_party_extensions/ARCHIVE_VIEWER.md` | supervised first-party plugin using external engine | proposal only; plugin execution closed |
| Image Converter | `../orchestrator/proposals/first_party_extensions/IMAGE_CONVERTER.md` | supervised create-new transform plugin | proposal only; plugin execution closed |

## Games roster

The normative per-game plans live under `../games/planning/games/`:

1. Solitaire (initially Klondike).
2. FreeCell.
3. Checkers with a small non-neural search opponent.
4. Sudoku with human-readable hints.
5. Crossword, initially limited to a Shakespeare vocabulary corpus.
6. Atom Probe, or a later-named hidden-clues deduction equivalent.
7. Four Pegs, a four-peg Mastermind-like deduction game.
8. Switchbox, an animated sequence/lock-deduction puzzle.
9. Rendezvous Riders, an animated bidirectional shortest-route puzzle.

The shared project favors solitary reasoning and tactile play. No network
competition, engagement economy, achievements platform, account, store,
leaderboard, or daily-streak coercion is inferred.

## Lexicon boundary

Lexicon accepts a bounded normalized term and returns typed dictionary senses,
source/provenance, and optional related forms. It may enumerate an admitted
word corpus for Crossword. It does not:

- masquerade as files in Engine;
- require AI, embeddings, semantic hives, or broad ontology operations;
- inject a search box, controls, or native window;
- acquire ambient filesystem or network authority;
- allow its result to overwrite exact filesystem evidence.

Orchestrator owns provider discovery, availability, query budgeting, typed
result merging, settings, and audit. The eventual worker/trust placement remains
open until its threat and update model are resolved.

## Trusted contextual commands

- Checksum calculation is on-demand. Executable, archive, and disk-image file
  types receive a prominent default command; other types remain available under
  user configuration or an expanded command set.
- `Open Command Line Here` launches the configured native terminal in the
  selected/current directory. Repository, hidden, and system/configuration
  contexts make the command prominent; the user may hide it globally.
- Neither command becomes a plugin merely for uniformity. Both use registered
  Orchestrator command meaning and first-party platform implementations.

## Plugin write reef

For transforms and extraction:

```text
approved input handle
        -> supervised worker/external engine
        -> bounded output or virtual hierarchy
        -> trusted validation and destination policy
        -> atomic host publication
```

The plugin does not enumerate arbitrary directories, choose an unapproved
destination, overwrite/delete a source, inject controls, or call Engine's
writable/index APIs. The host renders selection, collision, password, progress,
and error dialogs.

## Deferred but retained

- OCR and receipt workflows are future consumers of the same host-mediated
  transform contract, not part of the first plugin proof.
- Wider dictionary corpora, user corpus selection, authored crossword packs,
  pronunciations, and etymology wait on provenance/licensing and product review.
- Additional games require a separate accept/reject round; Towers of Hanoi and
  generic cliché accumulation remain rejected direction.
- Desktop replacement means a platform-supported file-object/background role,
  not virtual desktops, application hosting, a compositor, Wayland replacement,
  or a desktop-environment project.
