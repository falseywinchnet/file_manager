# Architecture inputs from grand architect

Status: rounds 1–3 transcription. GIVEN items are requirements; HYPOTHESIS items
require experiments or further definition.

## First usable system

- First dogfood platform: macOS.
- Early mutation testing occurs only inside a test chroot/sandbox root.
- Required early qualities: drag-and-drop, editable right-click behavior,
  search, list/icon views, and smooth interaction.
- The GUI must remain useful when the index service is absent.
- “If it breaks, return to Finder” is not yet an acceptance criterion; concrete
  break classes remain to be elicited.

## Main window

- Persistent but fully collapsible folder tree on the left; the entire pane also
  collapses through a small inner-edge control.
- Content surface in the center.
- Preview/properties surface on the right. Property groups open and close below
  the preview in the manner of Font Viewer properties and are editable in place.
- No tabs or dual panes.
- Default content view: small icons.
- Single-click selects; double-click opens.
- Filename extensions are optional as in classic Explorer. Inline rename always
  reveals the extension and selects only the basename by default.
- Navigation uses one vertically dense continuous chevron breadcrumb cascade.
  Its first-order inline path-editing operator turns that same retained trail
  into editable text without opening a separate path matrix.
- Search has its own field to the right. The combined command/path/search field
  idea is deferred in favor of an unmistakable classic split.
- A compact dynamic ribbon/control shelf is allowed when its structure is
  intelligible and its controls are well designed.
- Custom title bars.
- Interface sounds enabled by default; exact sonic grammar remains open.

## Navigation and organization

- Home is the actual user home directory. Desktop is ordinarily navigable.
- The earlier pulled-down recent full-trail matrix is no longer authoritative
  as the path editor. Whether full recent trails survive behind a separate
  history affordance remains open; it may not displace the inline editor.
- The inline editor preserves the breadcrumb row's identity and geometry and
  supports canonical resolution, autocomplete, explicit commit, and cancel.
- Search is scoped to the current folder subtree. It does not silently widen to
  the whole machine; this is intentionally classic Explorer behavior.
- Instead of grouping, support explicit virtual criteria views. The proposed
  mental model resembles `drive:/parent/child/Type/Year`: a single-layer virtual
  fanout available relative to a location, where selecting a criterion changes
  the displayed contents without corrupting the real hierarchy.
- Display recursive **allocated** folder size on the folder icon only after the
  index has computed it; otherwise show no number. At or below 1 MB show the
  ordinary icon. Candidate display quantization supplied by the architect is
  5 MB increments below 100 MB, 25 MB below 1000 MB, hundreds of MB from GB to
  TB scale, and hundreds of GB per TB. Exact boundary/rounding notation remains
  to be made unambiguous.
- Folder hover may disclose richer index-known information such as item counts
  and an exact allocated size, saving a properties-pane round trip.
- The tree has two honest starting structures: a Regedit-like drive-rooted view
  and a Home-rooted view. Mounted OneDrive and iCloud locations behave like
  drives. There are no synthetic `This PC`, `Quick Access`, `Libraries`, or
  OneDrive hierarchy aliases.

## File operations

- Ordinary operations begin immediately.
- Replacement is exceptional and requires a dialogue.
- The right-click menu exposes one-step undo only for deletion and rename.
- File Manager does not keep its own backups and does not promise undo for
  replacement.
- Initial file mutation support is constrained to a test root until correctness
  gates are met.
- Dragging means move on the same volume, copy across volumes, and export/copy
  when dropped into an application.
- Packages/bundles retain the platform's package-like treatment where it exists.
- `.DS_Store` is ignored. View, sort, and geometry state lives in File Manager's
  own hive. Hidden files are shown only when explicitly enabled; copying a
  folder may optionally exclude hidden files.

## Native quality ordering

The most important meanings of “native,” in order supplied:

1. startup;
2. memory;
3. input;
4. accessibility.

Native appearance is explicitly not a goal. Native controls, menus, shell
integration, and packaging matter only where evidence shows they improve the
required qualities.

Visual consistency across Windows, macOS, and Linux is required. Per-platform UI
implementations are a last resort, not the first architecture.

## Index service and consent

- The indexing/search backend is a standalone Go engine behind a narrow,
  versioned local API. It must be independently buildable, testable,
  benchmarkable, and usable without importing the File Manager GUI.
- Its production store is purpose-built for File Manager's exact identity,
  ownership, update, catalogue, and retrieval workloads. SQLite and other mature
  databases remain required comparison controls; this direction is not evidence
  that the custom implementation is already faster or correct.
- Development and destructive fault testing occur only inside an explicitly
  named disposable sandbox root. Exact filesystem identity, crash recovery,
  corruption containment, and rebuildability are release gates.
- A persistent indexing/search service may run while the GUI is closed.
- Indexing is opt-in, never opt-out.
- During installation, an inline folder picker is prefilled with Home plus
  explicit checkboxes for exclusions. Scanning begins only after affirmative
  root selection; after launch, the engine scans only configured directories.
- File creations are tracked only above an unresolved threshold and outside
  excluded high-churn trees.
- A second background component is the visible tray agent for index consent. It
  notices newly relevant roots such as new drives or newly created synchronized/
  repository folders, asks whether to index them, and defaults to no.
- A fully selected indexed Home subtree automatically admits new files and
  directories. An indeterminate/partially selected subtree automatically indexes
  new files but asks before admitting a directory, using file count and root
  recognition as prompting evidence. A `do not index` subleaf recursively
  excludes all descendants.
- Mounted cloud-provider folders are local for indexing and search once their
  root is admitted.
- Users may navigate any unindexed drive. Thumbnails are index-connected:
  unindexed locations do not receive thumbnails.
- Ordinary indexing begins with names and metadata. Content is a richer metadata
  channel: for sufficiently small files and when a competent local agent exists,
  store a distillation plus a small set of fingerprints/keyword hashes.
- Content-derived records remain exact-source-anchored and disposable.

## Kolmogrov and ConeDAG research boundary

- Kolmogrov is an independent, high-ambition formal and experimental program,
  and its successful gated transfer is the intended core fuzzy/structural
  candidate mechanism for the production engine. Initial exact/lexical engine
  work does not wait for it.
- It investigates the architect's practical superposition thesis: content,
  position, combination, hierarchy, prevalence, containment, scale, and context
  can provide complementary addressable locations whose joint fixed-width
  representation preserves useful perceptual distance better than a single
  undifferentiated location.
- Research order is exhaustive inspectable breakdown first, formal geometry and
  counterexamples second, fixed-width construction third, retrieval evaluation
  fourth, and speed optimization last.
- Proofs, formally delimited hypotheses, adversarial failures, and equal-budget
  comparisons are retained. No unmeasured result is promoted into exact file
  identity or production ranking.

## Orchestrator and program topology

- The Rust component is named **Orchestrator** and lives under `../orchestrator/`.
- Orchestrator is the integration authority and owns the canonical requirement,
  specification, version, capability, availability, and conformance record for
  every cross-project API and ABI. It proposes interfaces in each affected
  project's notes; that project replies and Orchestrator reconciles the result.
  Producers retain their private implementations.
- Orchestrator is the trusted control plane for hives, settings, handler association,
  command/CLI authority, plugin lifecycle and containment, and platform policy.
- Semantic/provider memory lives in Orchestrator-managed hives outside both the Go
  engine's authoritative catalogue and untrusted plugin processes.
- The C++ File Manager frontend lives under `../frontend/`, consumes
  Orchestrator as its normal integration/policy surface, and uses GUI.Forms
  in-process. A registered direct-engine route is degraded fallback. File
  Manager is also the GUI for Orchestrator settings and service controls.
- Orchestrator's headless Rust core advances alongside and independently of
  GUI.Forms, the systemwide engine, and Kolmogrov. Frontend 001 opens only after
  Orchestrator Core 1.0 is live, GUI.Forms gives its named FM0 consumption
  go-ahead, and the architect explicitly starts it. Engine and later providers
  retain their own negotiation/fixture gates; Orchestrator reports their actual
  state.
- The earlier `../plugin_runtime/` plan is preserved as research input but is
  not a second runtime implementation beside Orchestrator.

## First-party application backbone

- **GIVEN:** File Manager is the first substantial GUI application on the
  Orchestrator/GUI.Forms backbone, not its only future consumer.
- **GIVEN:** File Manager and a reusable open/save-as Document Picker are built
  from one navigation substrate when frontend implementation opens.
- **GIVEN:** a future minimal classic-Paint descendant uses the picker and
  accepts File Manager files/previews through typed drag/drop. It emphasizes
  alpha-capable reusable clipart/composites rather than layer-management and
  revision-history feature growth.
- **GIVEN:** a future **Text Editor** is a tabless plain-text/configuration
  editor, not an RTF/TextEdit/WordPad trajectory. Its picker may show hidden
  files under an application-scoped preference independent of File Manager.
- **GIVEN:** Orchestrator remains headless. Its settings/service GUI may be
  projected into File Manager and a possible standalone first-party shell from
  shared typed semantics and GUI.Forms composition; arbitrary server/plugin
  controls remain forbidden.
- **GIVEN:** application help content is local and application-owned. GUI.Forms
  owns help-key/overlay mechanics; Orchestrator may own provider/topic
  registration and availability.
- **CANDIDATE:** first-party apps link a bounded in-process picker surface for
  correct modality; native dialogs remain fallback and a dedicated Picker Host
  remains a later sandbox/incompatibility route.

Detailed proposal:
`APPLICATION_BACKBONE_AND_DOCUMENT_PICKER.md`.

## Future utilities, games, and extension proofs

- **DECIDED:** persistent left/right panels belong only to File Manager and its
  bounded embedded picker/browser projection. Paint, Text Editor, Games, and
  other applications use owned modal/modeless popup dialogs for secondary
  tools.
- **GIVEN:** Paint's detailed color selector and CMYK/OKLCH/RGB/hex converter
  are popup-dialog utilities. Text Editor's character chooser is an owned popup
  dialog, not a separate application or side panel.
- **GIVEN:** the future `games/` project contains Solitaire, FreeCell, Checkers,
  Sudoku, Crossword, an Atom Probe-like hidden-clues game, a four-peg deduction
  game, Switchbox, and a two-motorcycle bidirectional route puzzle. Competition
  and platform/social mechanics are minimized.
- **GIVEN:** the two latter games deliberately dogfood animation. Animation
  remains event-driven, bounded, reduced-motion capable, and subordinate to a
  deterministic rules model.
- **GIVEN:** Lexicon is a first-party provider/extension rather than another
  desktop application. It returns typed local definitions for exact terms and
  supplies Crossword corpora without requiring AI or the broad semantic hive.
- **GIVEN:** Crossword begins with a pinned Shakespeare vocabulary corpus and
  later admits user-selected installed corpus profiles. Vocabulary, definition,
  and clue provenance remain separate.
- **GIVEN:** checksums and `Open Command Line Here` are trusted File Manager
  built-ins. Checksum UI is contextually prominent for executables, archives,
  and disk images and reads only on request. Terminal launch is contextually
  prominent in repositories, hidden folders, and system/configuration folders;
  File Manager launches a configured native terminal and does not embed a shell.
- **GIVEN:** a first-party Archive Viewer plugin uses the embedded File Manager
  browser/Document Picker plus a supervised external engine such as 7z. A
  first-party Image Converter remains a create-new transform proof. Plugins
  receive scoped input/output capabilities, never ambient filesystem writes or
  UI injection. OCR is deferred.
- **GIVEN:** platform-supported desktop-file/background integration is allowed;
  virtual desktops, compositors, window managers, display servers, native-app
  hosting, Wayland replacement, and a full desktop environment are rejected.

The detailed routing and individual plans live in `FUTURE_SCOPE_LEDGER.md`,
`../games/`, `../lexicon/`, the Paint/Text Editor planning roots, frontend
context/desktop plans, and Orchestrator first-party-extension proposals.

## Malkuth suite and release horizons

- **GIVEN:** the unified suite/distribution is named **Malkuth**, because it
  governs the physical existence of files and reveals their provenance and
  location. Component and application names remain distinct.
- **GIVEN:** the mission is **Curious, Concise, Friendly.** The accepted full
  framing lives in ADR-012.
- **GIVEN:** after File Manager capabilities are dogfooded, public release work
  adds online documentation, native cross-platform installers, and a polished
  website using real builds and honest capability states.
- **GIVEN:** 1.0 focuses on getting the integrated system working and stable on
  physical Windows, Linux and macOS, fixing material bugs, and supplying basic
  installers/documentation.
- **GIVEN:** 2.0 preserves calls while tightening mutexes, memory, performance,
  correctness and coarse/duplicated machinery; it adds the hostile plugin
  wrapper and secure LAN-only tracking/remote-file availability.
- **GIVEN:** 3.0 adds formal verification, further hardening/platform support,
  decent touch, more configuration/fonts, and app-store signing/distribution.
- **DECIDED:** a Malkuth suite version pins independently versioned components
  in an evidence-bearing release manifest; it does not force one ABI version.

Release planning lives under `../malkuth/`.

## Accepted engine implementation direction

The full authoritative record is
`../decisions/ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md`.

- Object records and path bindings are separate; compact posting ordinals are
  generation-local.
- Exclusive approved-root shards live under lightweight volume manifests.
- Bounded immutable root generations and atomic manifests are the selected
  storage spine; codecs and accelerators remain measured choices.
- Generations commit filesystem-observation watermarks rather than an ordinary
  per-event engine journal. Gaps cause reconciliation.
- Deterministic exact/name/fuzzy/metadata/provider evidence tiers precede later
  voting fusion.
- Removable local external volumes offer on-volume full indexing or no
  persistent catalogue. Network drives and other machines use full-at-source
  plus local-coarse-catalogue semantics when explicitly admitted.
- Directories with at least 65,536 immediate children default to name/basic-
  metadata light mode, not exclusion.
- The accepted performance constitution supplies experiment rejection gates and
  becomes a measured claim only after its named workloads run.

## Search, CLI, and future plugin AI

- One search box initially.
- Match explanations should be concise.
- The initial Orchestrator API is a local CLI with human and structured output.
  A human, local AI tool, or developer agent invoking it has the same local-user
  authority for the admitted operation; Orchestrator does not classify the
  author of CLI input.
- “Plugin AI” is a future application capability designed alongside the plugin
  API. It does not describe agents developing the repository. Plugin AI and
  semantic facts are currently stubbed.
- The semantic fact engine remains under architect design; no fact operation is
  inferred in advance.
- Model execution location is user policy and not an initial feature.
- OCR, captions, people/places/dates/topics/summaries, temporal language, and
  assistant-style answers are later work.
- Orchestrator owns the path to secure, synchronizable information hives and
  multi-machine search/copy. Remote federation is a future mission, not
  permission for the local core to search the web.

## Handlers and previews

- File Manager's handler registry is internal only.
- Unknown types use native operating-system selection dialogues.
- Native preview providers may be reused, but only behind hard process/container
  isolation. File Manager displays their output; their crash is contained.
- Plugins may contribute information through preview, thumbnail, virtual-system,
  and search contracts. They may not replace or restyle File Manager controls.
- A future in-application plugin catalogue is permitted in principle, subject to
  a separate trust, signing, network, and capability design.

## Classic-shell decisions from anti-model review

- Finder's automatic scope widening is rejected.
- Finder's spatial/free-placement icon canvas is rejected.
- Finder's floating Quick Look window is rejected; preview belongs in the
  collapsible preview/properties pane.
- Modern Explorer's automatically populated Quick Access is rejected. Recent
  breadcrumb trails supply deliberate locality instead.
- Modern Explorer's split “short” and “legacy” context menus are rejected. Power
  users choose visible context-menu commands in Settings.
- A sparse icon-only command bar may exist as an option, not as the sole command
  surface.
- Provider-specific shell control replacement is rejected.
- Rounded/Mica-style modern Windows appearance is rejected; GUI.Forms owns its
  style.
- Color is relational: foreground and background are paired and derived
  together in OKLCH rather than selected as independent swatches. Classic
  Office Word ribbon font-color families are the preferred seed palette once a
  specific revision is pinned.
- GUI.Forms uses systematic physical depth—raised actions, inset inputs, etched
  groups, deeper instruments, and explicit plane shadows—without indiscriminate
  rainbow material or modern cards.
- Ordinary recyclable deletion has no confirmation. Classic Properties remains
  a positive model.

## Deep principles

- Filesystem content remains the source of truth. Indexes and hives are
  projections that must not impersonate it.
- Deterministic behavior outranks adaptive convenience.
- Richness belongs where it is intentional; the precise boundary between
  principled minimalism and glorious excess is now substantially constrained by
  the recorded visual, ceremonial, material, and relational-color verdicts.
