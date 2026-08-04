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
- Navigation uses breadcrumbs plus a terminal `./` control that turns the path
  into editable text.
- Search has its own field to the right. The combined command/path/search field
  idea is deferred in favor of an unmistakable classic split.
- A compact dynamic ribbon/control shelf is allowed when its structure is
  intelligible and its controls are well designed.
- Custom title bars.
- Interface sounds enabled by default; exact sonic grammar remains open.

## Navigation and organization

- Home is the actual user home directory. Desktop is ordinarily navigable.
- Clicking in the breadcrumb path opens a dropdown of recently used folder
  trails. Each trail is itself rendered as breadcrumbs; any ancestor can be
  clicked and drilled down, giving developer work several parallel recent
  locality trees without tabs.
- An editable path appears only through the terminal breadcrumb button or by
  clicking the margin after the last breadcrumb.
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

## Search and external AI

- One search box initially.
- Match explanations should be concise.
- AI-facing API begins read-only and returns file objects plus condensed,
  interrogatable index information.
- Personal assistants live elsewhere and call this API.
- Model execution location is user policy and not an initial feature.
- OCR, captions, people/places/dates/topics/summaries, temporal language, and
  assistant-style answers are later work.
- The architecture should preserve a path to a secure, synchronizable information
  hive and multi-machine search/copy. Remote federation is a future mission, not
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
