# ADR-013: Future utility scope and surface topology

Status: **accepted**.

Date: 2026-08-06.

Owner approval: the grand architect selected the future games and utility
scope, placed lexicon capability outside the core file index, classified
checksums and terminal launch as contextual File Manager built-ins, selected
first-party archive and image-conversion plugin proofs, and explicitly reserved
persistent side panels to File Manager and its embedded browser view.

## Question

Which deliberately small first-party applications and extension proofs belong
in Malkuth's future scope, and what window/dialog topology prevents their
secondary tools from turning into docked application furniture?

## GIVEN constraints

- Only File Manager and its bounded embedded Document Picker/browser view may
  use persistent left or right panels.
- Text Editor, Paint, Games, and other ordinary applications have one primary
  work surface. Secondary tools appear in owned popup dialogs, either modal or
  deliberately modeless; they are not docked, collapsible, or persistent side
  panels.
- Paint's detailed color selector and CMYK/OKLCH/RGB/hex converter are dialogs.
- Text Editor's character chooser is a dialog reachable through an ordinary
  command; it is not a side panel or separate Character Map application.
- The `games/` project contains Solitaire, FreeCell, Checkers, Sudoku,
  Crossword, an Atom Probe-like hidden-clues game, a four-peg Mastermind-like
  deduction game, a switchbox sequence puzzle, and a two-motorcycle
  bidirectional route puzzle.
- The games minimize player-versus-player competition. They are small,
  deterministic GUI.Forms dogfood consumers, not a network platform.
- Lexicon/dictionary capability is a first-party extension/provider rather than
  another desktop application. Exact term lookup returns definitions through a
  bounded search-provider path. It does not require AI or a general semantic
  fact system.
- Crossword begins with an explicitly versioned Shakespeare vocabulary corpus.
  Later corpus selection is user-controlled.
- Plugins never receive ambient filesystem write authority. Inputs, outputs,
  destination selection, publication, quotas, and external-process execution
  remain host-mediated.
- Checksums and `Open Command Line Here` are File Manager built-ins, not
  plugins. Checksums are contextually prominent for executables, archives, and
  disk images and computed only on request. Terminal launch is contextually
  prominent in repositories, hidden directories, and system/configuration
  directories and launches a configured host terminal.
- A first-party archive viewer plugin uses the shared Document Picker and a
  supervised external archive engine such as 7z. A first-party image converter
  remains a controlled transformation proof. OCR is deferred.
- File Manager may expose platform-supported desktop-file/background commands,
  but Malkuth does not become a compositor, virtual-desktop system, window
  manager, Wayland replacement, or desktop environment.

## Workloads and failure modes

- A generic `Panel` control must not silently make every Malkuth application a
  multi-pane shell.
- A modeless dialog must not become an unowned floating window, lose its owner,
  steal persistent focus, or survive after its document closes.
- Game animation must not determine puzzle truth, prevent reduced-motion use,
  or require perpetual redraw.
- A lexicon result must not impersonate a file, overwrite exact Engine facts,
  or become an excuse to absorb a dictionary database into the Go catalogue.
- A public-domain source text is not automatically a suitable definition or
  crossword-clue corpus. Edition, tokenization, normalization, exclusions, and
  clue provenance remain explicit.
- An archive worker must not extract outside a host-approved destination,
  follow malicious archive paths, exhaust memory/disk/processes, or inject UI.
- An image converter must not overwrite the source by default, enumerate the
  surrounding directory, or gain a general decoder inside the trusted UI
  process.
- A checksum affordance must not trigger eager background hashing of large
  files. A terminal command must not embed a shell, reinterpret commands, or
  imply privilege elevation.

## Candidates

### A — separate conventional utility application for every capability

Familiar packaging, but it produces a drawer of tiny programs and duplicates
document selection, settings, help, and lifecycle machinery.

### B — place every utility inside persistent side panels

Superficially compact, but it makes ordinary applications resemble IDEs,
reduces primary work area, creates focus/resize complexity, and contradicts the
desired classic single-purpose shape.

### C — primary surface plus owned dialogs, providers, contextual commands, and supervised plugins

Each capability receives the smallest honest shape: application where it owns a
durable interaction model, dialog where it assists one application, provider
where it contributes structured knowledge, built-in command where it is trusted
and filesystem-adjacent, and plugin where hostile formats or external engines
require containment.

## Evidence and measurements

The product scope and surface topology are **GIVEN**. Their responsiveness,
accessibility, containment, and usefulness remain unmeasured until the named
dogfood fixtures run. Algorithm choices in the individual plans are
**CANDIDATE** reference mechanisms, not performance claims.

## Decision

Choose C.

### Surface law

Persistent side panels are a File Manager interaction primitive. The bounded
Document Picker/browser view may reuse them because it is a projection of that
same navigation model. No other first-party application may acquire left/right
panels without a superseding architect-approved ADR.

Other applications use:

- ordinary owned modal dialogs for decisions that block the current operation;
- ordinary owned modeless dialogs for repeated secondary actions such as Find,
  Characters, or detailed Color, when continuing to edit is useful;
- menus, compact toolbars, status lines, and canvas/document controls on the
  primary surface;
- local help dialogs or greaseboard overlays, never a help side panel.

Popup is a presentation form, not an authority change. Dialog contents still
obey the owning application, GUI.Forms, and Orchestrator boundaries.

### Future project scope

- `games/` is one planning and implementation project with shared deterministic
  infrastructure and separately gated game modules.
- `lexicon/` is one independently testable first-party lexical provider project.
  Its eventual execution classification—trusted first-party provider or
  supervised first-party plugin—remains a negotiation edge, but it stays outside
  Engine's authoritative file catalogue.
- Archive Viewer and Image Converter remain proposal-stage first-party plugins
  under Orchestrator's future supervisor.
- Checksums, terminal launch, and bounded desktop integration are File Manager
  commands with explicit contextual-visibility policy.

## Why the other candidates lost

A creates application sprawl for capabilities that do not own a document or
durable interaction model. B imports IDE-style furniture and makes the visual
topology inconsistent with the architect's explicit direction. C preserves
small applications while still exercising shared contracts and GUI.Forms.

## Consequences

- `games/` and `lexicon/` begin as planning-only roots; their implementation
  gates remain closed.
- Paint and Text Editor plans must remove panel-shaped help/utility language and
  specify owned dialog lifecycle explicitly.
- GUI.Forms must support robust modal and modeless owned dialogs, focus return,
  deterministic headless dialog fixtures, and reduced-motion animation without
  presuming dock panels for every consumer.
- Orchestrator may register help topics, provider identities, capabilities, and
  commands, but never supplies arbitrary dialog controls or layouts.
- Malkuth release manifests list each application/provider/plugin independently;
  future scope does not make every piece a 1.0 blocker.

## Reversal and migration path

Game modules, providers, commands, and plugins retain independent manifests and
can move between release trains. Dialog widgets remain ordinary reusable
GUI.Forms controls, so a future application can change invocation without
changing file formats or provider protocols. Reversing the no-panel law
requires a new ADR naming the application, essential workflow, focus/geometry
cost, and why a dialog cannot serve it.

## Unresolved edges

- Whether `games/` ships as one launcher with game windows, several executables
  from one repository, or both.
- Final public names and art direction for Atom Probe, the switchbox puzzle,
  the motorcycle puzzle, and the four-peg deduction game.
- Lexicon runtime classification and corpus licenses/editions.
- Exact archive engine packaging/discovery and platform sandbox strength.
- Exact checksum algorithms shown by default and terminal application selection.
- Which future Malkuth release first admits each project.
