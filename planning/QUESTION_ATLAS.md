# Grand architect question atlas

Answer by ID, in any order. “Never,” “later,” “plugin only,” “platform-native,”
and “I do not care” are all useful answers. If two answers conflict, the conflict
will be returned explicitly rather than harmonized in secret.

Answered constraints are transcribed in `ARCHITECTURE_INPUTS.md` and
`PRODUCT_NEGATIVE.md`. The atlas remains unchanged as the durable question set.

`★` marks the first interview round.

## A. The product it must not become

- **A001 ★** Which three existing file managers are the clearest anti-models,
  and what exact behavior condemns each?
- **A002 ★** What Finder behaviors make you angriest: spatial inconsistency,
  hidden paths, column view, search behavior, metadata, shortcuts, animations,
  window model, or something else?
- **A003 ★** Which post-Windows-7 Explorer changes must be prohibited?
- **A004 ★** Is a ribbon itself objectionable, or only the oversized,
  ever-changing modern ribbon?
- **A005 ★** Should a “Home,” “Recents,” “Recommended,” or activity landing page
  be forbidden, optional, or allowed if it is entirely local?
- **A006 ★** Are tabs desirable, tolerated, or a symptom of browser-thinking?
- **A007 ★** Are dual panes desirable, plugin territory, or forbidden as visual
  complexity?
- **A008 ★** Should cloud-provider integration be absent from the core even when
  the provider appears as an ordinary mounted filesystem?
- **A009 ★** Must advertising, account prompts, telemetry, tips, badges, and
  feature nags be categorically impossible?
- **A010 ★** What must never animate?
- **A011 ★** What must never make a sound?
- **A012 ★** Which automatic behaviors feel patronizing—grouping, sorting,
  renaming, view changes, “helpful” selections, opening new windows?
- **A013** Should the application ever rewrite or “clean up” metadata without a
  direct command?
- **A014** Should it ever infer that two files are duplicates and hide one?
- **A015** Should it ever display web-derived icons, thumbnails, descriptions,
  or metadata?
- **A016** Should the core contain archives-as-folders, optical burning, email,
  sharing, sync, or collaboration?
- **A017** Should network shares be unsupported, treated as ordinary mounts, or
  available only through plugins?
- **A018** Is device browsing (phones, cameras, MTP) core, adapter, plugin, or no?
- **A019** Do you reject virtual folders/saved searches, or only opaque ones?
- **A020** Should the program ever conceal files by default beyond explicit
  system/hidden-file policy?

## B. “Usable” and the first daily replacement

- **B001 ★** What exact workflow must the first usable version replace for you?
- **B002 ★** On which OS must dogfooding begin?
- **B003 ★** Which five operations must be excellent before you will use it
  daily?
- **B004 ★** Which failures would make you immediately return to Finder or
  Explorer?
- **B005** May the first usable build omit indexing entirely if navigation and
  operations are sound?
- **B006** May it omit previews? Search contents? Plugins? Semantic features?
- **B007** Is a single-window application sufficient for first dogfood?
- **B008** Must the first build be safe for irreplaceable data, or initially
  read-only/limited to test directories?
- **B009** What real directory should become the acceptance corpus?
- **B010** What is the minimum period of uninterrupted dogfood before a milestone
  counts as stable?

## C. Native, portable, and honest platform difference

- **C001 ★** When you say native, rank: startup, input feel, accessibility,
  controls, window chrome, menus, file dialogs, shell integration, memory, binary
  size, and visual appearance.
- **C002 ★** Should File Manager look nearly identical on every OS, or translate
  its grammar into each platform?
- **C003 ★** May the main file surface be custom rendered while text input,
  accessibility, drag/drop, and windows use native adapters?
- **C004 ★** Is a managed runtime acceptable if cold start and memory budgets are
  met?
- **C005 ★** Is a bundled web engine categorically forbidden?
- **C006** Are per-platform UI implementations acceptable if they share a core?
- **C007** Must plugins be binary-portable across all OSes?
- **C008** Must the CLI and API behave byte-for-byte identically across platforms,
  or expose platform capabilities honestly?
- **C009** Should macOS menu-bar conventions be followed even when they differ
  from Windows Explorer?
- **C010** Should Linux target one desktop integration first or remain shell-
  neutral?
- **C011** Are Wayland and X11 both day-one requirements?
- **C012** Are ARM64 Windows/Linux first-class at first release?
- **C013** What is the oldest supported OS generation on each platform?
- **C014** Must portable/no-install mode exist?
- **C015** May platform adapters be separately versioned and shipped?

## D. Process topology and failure containment

- **D001 ★** May a lightweight indexing service run persistently when the GUI is
  closed?
- **D002 ★** Must there be a mode with no daemon/service at all?
- **D003 ★** Should the GUI remain fully useful when the indexer is absent,
  crashed, stale, or rebuilding?
- **D004 ★** Is per-user service scope sufficient, or is a privileged/system
  service ever acceptable?
- **D005** Should file mutations be executed by the GUI, a broker, or a journaled
  operation service?
- **D006** Must previews always be out of process?
- **D007** Must semantic model inference always be out of process and killable?
- **D008** What is the maximum acceptable number of idle processes?
- **D009** Must every helper exit when the GUI and indexing are disabled?
- **D010** May multiple GUI windows share one service and cache?
- **D011** What state may survive a crash: navigation history, selection,
  operation journal, undo, search session?
- **D012** Should a plugin crash be visually reported, silently fall back, or
  disable that plugin?

## E. Navigation and window semantics

- **E001 ★** Classic tree at left: permanent, collapsible, optional, or replaced
  by something better?
- **E002 ★** Address bar: breadcrumbs, editable path, both, or command line?
- **E003 ★** Should clicking an already selected breadcrumb open its text path?
- **E004 ★** Back/forward history per window, per tab, global, or none?
- **E005 ★** Should Up be a first-class button and keyboard command?
- **E006** Should folders open in the same window by default?
- **E007** Do you want spatial windows that remember folder position/size/view?
- **E008** Should each folder remember its own view and sort?
- **E009** Are tree expansions persistent between sessions?
- **E010** Should typing while the file surface is focused select by name, search
  the folder, or open a command palette?
- **E011** Is a command palette desirable or too application-centric?
- **E012** Should paths accept shell abbreviations, environment variables, URIs,
  or only filesystem-native syntax?
- **E013** How should unavailable volumes remain represented?
- **E014** Should package/bundle directories be traversable explicitly?
- **E015** Should symlink traversal be visually obvious?

## F. File surface, density, and selection

- **F001 ★** Which default view: details, list, small icons, medium icons, tiles?
- **F002 ★** What Windows Explorer era most precisely defines ideal row density?
- **F003 ★** Single click selects and double click opens, without exceptions?
- **F004 ★** Should blank-area double-click do anything?
- **F005 ★** Do selection checkboxes belong anywhere?
- **F006 ★** Should full-row selection be default in details view?
- **F007** Rubber-band selection required day one?
- **F008** Middle click behavior?
- **F009** Should hidden files be dimmed, ghosted, or merely shown?
- **F010** Should extensions always be visible?
- **F011** Should folders sort before files, mix naturally, or be configurable?
- **F012** Should parent (`..`) appear as a row?
- **F013** How should millions of items degrade: virtual scrolling, paging,
  warning, or refuse?
- **F014** Should icon and thumbnail sizes be continuously zoomable or use fixed
  crisp steps?
- **F015** What selection survives sort, refresh, rename, and asynchronous loads?
- **F016** May rows reflow while metadata/thumbnails arrive?
- **F017** Should the UI ever show skeleton placeholders?
- **F018** Should empty whitespace be functional or simply calm?

## G. Sorting, grouping, columns, and folder models

- **G001 ★** Which sort keys are core: name, extension/type, size, modified,
  created, accessed, owner, tags, rating, dimensions, duration?
- **G002 ★** Natural numeric name order (`file2` before `file10`) by default?
- **G003 ★** Case-sensitive or locale-aware name ordering?
- **G004 ★** Is automatic grouping categorically forbidden?
- **G005** Should multi-key stable sorting be exposed?
- **G006** Must every sort state be expressible in the CLI/API?
- **G007** How should unknown/pending metadata sort?
- **G008** Can plugins add columns and sort keys?
- **G009** May a plugin sort require background extraction?
- **G010** Are folder-size calculations opt-in only?
- **G011** Should directory size ever appear as if it were ordinary metadata?
- **G012** Do you want persistent per-folder column layouts?
- **G013** Should settings be rule-based by folder kind, path, volume, or explicit
  per-folder only?
- **G014** Is “folder type inference” (pictures/music/documents) forbidden?

## H. File operations, conflicts, and undo

- **H001 ★** Must every destructive operation be undoable where platform
  facilities permit?
- **H002 ★** Delete means trash/recycle by default, with permanent delete an
  explicit separate command?
- **H003 ★** Should delete confirmation depend on reversibility rather than item
  count?
- **H004 ★** How should copy conflicts work: replace, keep both, skip, compare,
  apply-to-all—with which default?
- **H005 ★** Should operations begin instantly or show a review plan first?
- **H006 ★** Must batch rename always preview the complete mapping and collisions?
- **H007** Should undo survive application restart?
- **H008** Should copy/move operations survive GUI restart?
- **H009** How much operation history is retained, and is it sensitive data?
- **H010** May independent operations run concurrently?
- **H011** Should speed yield to storage politeness when the machine is busy?
- **H012** Do you want explicit verification/checksum modes for copies?
- **H013** How are partial results left after cancellation?
- **H014** Should an operation ever follow symlinks implicitly?
- **H015** Are secure erase or shredding explicitly out of scope?
- **H016** Should elevated operations be supported or handed to the OS?
- **H017** May plugins participate in mutation transactions?
- **H018** Must the AI API be unable to perform destructive mutations without a
  separately issued, expiring authorization token?

## I. Index scope, economy, and lifecycle

- **I001 ★** Which locations are indexed by default: home, selected roots,
  mounted local volumes, everything, or nothing until chosen?
- **I002 ★** Must removable volumes retain an offline catalogue?
- **I003 ★** Are external/network volumes categorically excluded from automatic
  indexing?
- **I004 ★** What idle CPU, memory, disk space, and writes/day are acceptable?
- **I005 ★** Should indexing stop on battery, low power, thermal pressure, or
  metered storage?
- **I006** Are file contents indexed by default or only names/metadata?
- **I007** Maximum file size for content extraction?
- **I008** Which file types must never be opened by the indexer?
- **I009** How are encrypted, sparse, compressed, package, and database files
  treated?
- **I010** Should ignored patterns resemble `.gitignore`, OS indexing policy, or
  a new grammar?
- **I011** Should project-local ignore files be honored?
- **I012** Must the index be human-inspectable/exportable?
- **I013** Must it be reproducible from the filesystem with no irreplaceable
  state?
- **I014** Is derived semantic metadata disposable and separately erasable?
- **I015** Should index corruption trigger automatic quarantine/rebuild or ask?
- **I016** Is eventual consistency acceptable, and how is staleness shown?
- **I017** Should queries combine live directory state with a stale global index?
- **I018** May indexing use extended attributes or sidecar files?
- **I019** Must the index never alter indexed files or directories?
- **I020** Should multiple users on one machine ever share index data?

## J. Search language and results

- **J001 ★** Should ordinary typing search names first, everything, or the current
  folder only?
- **J002 ★** Do you want one query box or distinct exact/filter/semantic modes?
- **J003 ★** Should the query language be visible and composable (`type:pdf`,
  `size>`, dates), or primarily conversational?
- **J004 ★** Must every result explain why it matched?
- **J005 ★** Should results include unavailable/offline files?
- **J006** Global results in the same file surface, a dedicated view, or a panel?
- **J007** Should search default to current folder, descendants, indexed roots,
  or remember last scope?
- **J008** Are saved searches core, plugin, or unwanted?
- **J009** Should search history persist?
- **J010** Must negative filters and exact literals be first-class?
- **J011** Should results ever be grouped by relevance bands?
- **J012** Should ambiguous results expose multiple interpretations?
- **J013** What should “no results” do—nothing, suggest query relaxation, or show
  exactly which constraint failed?
- **J014** Can plugins contribute result providers to the same list?
- **J015** If so, must remote/plugin results be visually and structurally
  segregated from local results?
- **J016** Should relevance alter ordinary folder sorting, or only search views?
- **J017** Is fuzzy filename matching allowed to outrank an exact metadata match?
- **J018** Should search results be stable across repeated identical queries while
  the index is unchanged?

## K. Local semantics and AI

- **K001 ★** Is all model inference required to occur locally, without network
  fallback?
- **K002 ★** May semantic indexing be opt-in per root, or should a lightweight tier
  be on by default?
- **K003 ★** What derived facts are acceptable: OCR, objects, captions, people,
  places, dates, topics, sentiment, document summaries?
- **K004 ★** Are face detection, face clustering, or person naming forbidden?
- **K005 ★** Are sensitive-document categories forbidden even if computed only
  locally?
- **K006 ★** Should the system learn personal phrases such as “winter of 28,” and
  where may that memory live?
- **K007 ★** Must the semantic system show exact supporting fragments/thumbnails
  for every claim?
- **K008** May user corrections train/adapt a local model, or only adjust explicit
  graph facts?
- **K009** Should model downloads be core update artifacts, optional packs, or
  plugins?
- **K010** Maximum acceptable model download, RAM, index time, and query latency?
- **K011** CPU-only minimum?
- **K012** May idle GPU/NPU acceleration be used?
- **K013** Should captions be searchable if confidence is low?
- **K014** How should the UI distinguish file text from AI-derived description?
- **K015** Should semantic state survive source-file deletion?
- **K016** Must deleting a root prove deletion of all derived vectors/captions?
- **K017** Should embeddings be exportable, inspectable, or treated as cache?
- **K018** May the system correlate files across unrelated roots?
- **K019** May temporal inference use file timestamps, photo EXIF, folder history,
  operation history, and application usage?
- **K020** What does “my files” include when the user has several accounts,
  removable disks, or shared mounts?
- **K021** Should the AI answer questions, or only compile queries and return file
  objects?
- **K022** Must the core work indefinitely with all semantic/AI components absent?

## L. CLI and AI-interrogatable API

- **L001 ★** Should the CLI command be literally `filemanager`, `fm`, both, or
  something else?
- **L002 ★** Human-readable output by default and structured output by flag, or
  structured-first?
- **L003 ★** Local socket/RPC API, command subprocess protocol, library API, or all
  as projections of one object model?
- **L004 ★** May the API mutate files, or begin read-only?
- **L005 ★** Must every mutation support dry-run and a stable plan ID?
- **L006** Stable file IDs or paths as primary operands?
- **L007** Should paths always be accepted for human convenience even if IDs are
  authoritative?
- **L008** Pagination vs streaming for huge lists?
- **L009** Event subscription for filesystem/index/operation changes?
- **L010** How are partial failures represented in batch operations?
- **L011** Must commands be idempotent?
- **L012** Should the API expose ranking components and query plans?
- **L013** Should the API expose plugin capabilities and provenance?
- **L014** Is a shell extension that changes the user's current directory useful
  or too magical?
- **L015** Should GUI state be interrogatable/controllable, or only filesystem and
  index state?
- **L016** Are natural-language commands allowed to directly create mutations?
- **L017** Must an AI caller identify itself and receive a narrower capability
  profile than a human CLI?
- **L018** Should all API calls be auditable locally, and for how long?

## M. Handlers, icons, and type identity

- **M001 ★** Is File Manager's handler registry authoritative only inside File
  Manager, or should it modify OS defaults?
- **M002 ★** Should it import OS associations automatically, on request, or never?
- **M003 ★** Can a handler be selected by extension, MIME/UTI, content sniffing,
  user rule, or all with explicit precedence?
- **M004 ★** Should “Open with” show OS handlers, internal handlers, and plugins in
  one list or segregated?
- **M005** Per-file, per-folder, per-workspace, and global overrides?
- **M006** Are executable/script files ever content-sniffed?
- **M007** Should icons come from the file, OS association, File Manager theme,
  plugin, or ordered fallback?
- **M008** Must icon packs be plugins/themes?
- **M009** Should executable icons be extracted in a sandbox?
- **M010** How should icon changes invalidate caches?
- **M011** Is a unified cross-platform type taxonomy desirable, or should native
  MIME/UTI/ProgID identities remain visible?
- **M012** Can users create friendly type names independent of OS names?

## N. Preview system

- **N001 ★** Preview pane right, bottom, detachable, overlay, or Quick-Look-style?
- **N002 ★** Is preview generated on selection, explicit key, hover, or never on
  hover?
- **N003 ★** Which preview types are indispensable day one?
- **N004 ★** Should native OS preview providers be reused despite inconsistent
  rendering and trust?
- **N005 ★** Must every third-party preview run out of process with timeout and
  memory limits?
- **N006** Can previews execute scripts/macros/active content?
- **N007** May previews make network requests?
- **N008** Should preview plugins receive raw bytes, a file descriptor, a copied
  sandbox file, or a narrow decoding service?
- **N009** Are previews cacheable on disk? Encrypted? Purged with source?
- **N010** How are huge files sampled?
- **N011** Should archives preview contents without mounting?
- **N012** Should media autoplay?
- **N013** Should audio preview emit sound immediately?
- **N014** Should preview failures ever create system dialogs?
- **N015** Can preview plugins add metadata/commands, or render only?

## O. Plugin system and trust

- **O001 ★** Which extension classes are actually wanted: preview, metadata,
  commands, panels, columns, icons, search, virtual filesystems, remote providers?
- **O002 ★** Which plugin powers must never exist?
- **O003 ★** Must plugins be out of process by default?
- **O004 ★** May plugins use the network if separately granted?
- **O005 ★** Is there ever an in-app plugin store, or only manual/local install?
- **O006 ★** Should unsigned development plugins require a conspicuous developer
  mode?
- **O007** Capability prompts at install, first use, or both?
- **O008** Can users grant access by root/folder/file type?
- **O009** Can plugins mutate files directly, or only request core operations?
- **O010** Should plugin UI be declarative, remote-rendered, native ABI, or
  prohibited outside fixed extension points?
- **O011** ABI stability horizon?
- **O012** Can plugins bundle runtimes/models?
- **O013** Resource quotas and timeouts?
- **O014** Update model: independent, core-managed, package-manager-managed?
- **O015** Must plugin uninstall prove cleanup of derived data?
- **O016** Should plugins be reproducibly packageable and checksum-pinned?
- **O017** How are abandoned plugins handled after API evolution?
- **O018** Can a plugin shadow a core command or file type?

## P. Visual language

- **P001 ★** Which exact elements come from NeXTSTEP: depth, monochrome controls,
  dock, menus, typography, window geometry?
- **P002 ★** Which exact elements come from Windows 7: glass, gradients, command
  bar, tree, details rows, selection glow, icons?
- **P003 ★** Which Watercolor traits matter: soft blue planes, bevels, tabs,
  compact title chrome, illustrative icons?
- **P004 ★** Which Encarta traits matter: saturated jewel tones, dimensional
  panels, sonic feedback, typography, information density?
- **P005 ★** Should the base look dark, light, or neither?
- **P006 ★** How much bevel/depth is delightful before it becomes skeuomorphic?
- **P007 ★** Are translucent/glass materials desired, or are opaque fast planes
  truer to the vision?
- **P008 ★** Custom title bars or native window chrome?
- **P009** Pixel-aligned bespoke icons or modern vector scaling?
- **P010** Should icons imitate an era or create a new consistent object language?
- **P011** What corner radius range is acceptable?
- **P012** Are shadows allowed inside controls? windows? popups?
- **P013** Should focus rings be vivid and permanent for keyboard navigation?
- **P014** Is dark mode a separate authored theme or automatic inversion?
- **P015** User theming: full skinning, curated palettes, accent only, none?
- **P016** Must themes preserve exact density and geometry?
- **P017** Should the interface ever use emoji as icons?
- **P018** How much text belongs on the main toolbar?
- **P019** Are large touch targets a mode rather than the desktop default?
- **P020** Do we embrace compact 4K density or scale physical size with DPI?

## Q. Motion and sound

- **Q001 ★** Name the actions that deserve click sounds.
- **Q002 ★** Mechanical, electronic, glassy, soft UI, or mixed sonic palette?
- **Q003 ★** Should sounds be on by default?
- **Q004 ★** Must repeated navigation clicks rate-limit or vary subtly?
- **Q005 ★** Are error sounds acceptable?
- **Q006** Separate volume from system effects?
- **Q007** Per-action sound packs?
- **Q008** Should destructive actions have a unique sonic confirmation?
- **Q009** Maximum motion duration for menus, panels, selection, navigation?
- **Q010** Should folder navigation crossfade, slide, snap, or simply redraw?
- **Q011** Is smooth scrolling desired or should wheel/keyboard movement be
  immediate and stepped?
- **Q012** Must reduced-motion disable everything nonessential?
- **Q013** Should background activity have any ambient animation?
- **Q014** Is a busy spinner acceptable, or should progress always be concrete?

## R. Accessibility and international behavior

- **R001 ★** Is full keyboard operation a day-one invariant?
- **R002 ★** Is screen-reader support a release gate from first dogfood or before
  public release?
- **R003 ★** High contrast and reduced motion day one?
- **R004 ★** Must all sounds have visible equivalents and all visual state have
  non-color cues?
- **R005** Minimum font scaling?
- **R006** Right-to-left UI requirement?
- **R007** Full IME and composition support day one?
- **R008** Locale-sensitive dates/sizes with a technical exact-value mode?
- **R009** How are Unicode-equivalent but byte-distinct names displayed?
- **R010** Should dangerous lookalike characters be indicated?
- **R011** Must shortcuts be remappable?
- **R012** Mouse, trackpad, keyboard, touch, pen priorities?

## S. Privacy, security, and auditability

- **S001 ★** Is zero telemetry—including crash upload—the invariant?
- **S002 ★** May updates check the network automatically?
- **S003 ★** Must the core be buildable and usable permanently offline?
- **S004 ★** Should query and navigation history default to off, memory-only, or
  persisted locally?
- **S005 ★** Should the index support encryption independent of full-disk
  encryption?
- **S006** Threat model: malicious files, malicious plugins, other local users,
  compromised account, forensic recovery?
- **S007** Should extracted text ever enter OS-wide search indexes?
- **S008** Should private roots be visually marked and excluded from semantic
  inference/history?
- **S009** Can the API expose file contents, or only metadata unless separately
  authorized?
- **S010** Must every AI/plugin read have a local audit trail?
- **S011** Should clipboard operations clear or remain ordinary OS behavior?
- **S012** Are thumbnails/previews of encrypted volumes retained when unmounted?
- **S013** Should crash dumps omit paths/content by construction?
- **S014** What update-signing and reproducible-build expectations apply?

## T. Performance and resource budgets

- **T001 ★** Cold launch target on a representative machine?
- **T002 ★** Warm launch target?
- **T003 ★** Time from navigation command to first correct rows?
- **T004 ★** Maximum keyboard-to-highlight latency?
- **T005 ★** Idle GUI memory target?
- **T006 ★** Idle indexer memory and CPU target?
- **T007 ★** Search p50/p95 target at one million, ten million, and one hundred
  million records?
- **T008 ★** Is binary size important independently of startup and memory?
- **T009** Maximum acceptable dropped-frame rate while scrolling?
- **T010** Maximum synchronous work on UI thread?
- **T011** Maximum index write amplification?
- **T012** Maximum index size as fraction of indexed content or record count?
- **T013** Should file operations yield to foreground applications?
- **T014** How old/slow a reference machine should guard the design?
- **T015** Is HDD performance a first-class guard?
- **T016** How should remote/removable latency appear without freezing?
- **T017** Must every expensive feature expose its resource consumption?

## U. Configuration, state, and registry

- **U001 ★** Plain-text inspectable configuration, structured database, or both?
- **U002 ★** Should settings sync ever exist outside plugins?
- **U003 ★** Is File Manager's “registry” one portable store or several explicit
  registries for handlers, icons, plugins, views, and semantic concepts?
- **U004 ★** Must every configuration change be scriptable through the CLI/API?
- **U005** Per-machine, per-user, per-workspace, and portable profiles?
- **U006** Should settings changes be journaled/undoable?
- **U007** Can plugins write arbitrary settings or only namespaced schema?
- **U008** How are migrations rolled back?
- **U009** Should reset preserve index data, handlers, plugin grants, or nothing?
- **U010** Is registry editing a first-class expert UI or CLI-only?

## V. Distribution, governance, and longevity

- **V001 ★** Open-source license intent?
- **V002 ★** Is Modern.Forms acceptable to fork and maintain if it proves the
  best substrate?
- **V003 ★** Do we prefer boring dependencies even if more custom UI code is
  required?
- **V004 ★** Must the core avoid dependencies controlled by a single vendor?
- **V005** Release cadence and stability channel model?
- **V006** Automatic updates, package managers, manual packages, or all?
- **V007** Backward compatibility promise for CLI/API/plugin ABI?
- **V008** Should the index format be stable, migratable, or disposable/rebuilt?
- **V009** Should project governance favor one coherent vision over broad feature
  voting?
- **V010** What contribution types are welcome or unwelcome?
- **V011** Is there a formal deprecation period?
- **V012** What happens when platform APIs disappear?

## W. The deepest architectural taste questions

- **W001 ★** Should the filesystem remain the sole source of truth, with every
  File Manager database treated as disposable projection?
- **W002 ★** Should tags/notes/custom metadata live inside files, sidecars,
  extended attributes, or an explicitly non-portable registry?
- **W003 ★** Is one universal “file object” model desirable, or should volumes,
  links, packages, search results, and plugin objects stay different types?
- **W004 ★** Which is worse: exposing platform differences or hiding them behind
  leaky abstractions?
- **W005 ★** Is deterministic behavior more important than adaptive convenience?
- **W006 ★** When speed and visual richness conflict, which wins and where?
- **W007 ★** When exact lexical search and semantic inference disagree, which is
  presented first?
- **W008 ★** Should File Manager make uncertainty visible, or quietly rank and
  let the user judge?
- **W009 ★** What powers may the GUI have that the AI API must not have?
- **W010 ★** What powers may a first-party plugin have that a third-party plugin
  must not have?
- **W011** Should all core behavior be expressible as commands over a stable model,
  with GUI as one client?
- **W012** Or should some interactions remain intentionally GUI-native and not
  abstracted into an API?
- **W013** Where do you want principled minimalism, and where do you want glorious
  excess?
- **W014** What single compromise would most betray the project even if it made
  delivery easier?
- **W015** What single constraint are you most willing to relax to make the first
  usable system arrive?
