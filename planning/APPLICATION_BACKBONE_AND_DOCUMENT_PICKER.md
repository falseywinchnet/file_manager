# First-party application backbone and document picker

Date: 2026-08-06.

Status: **GIVEN application direction; CANDIDATE projection mechanics pending
cross-project negotiation**.

## Terminology

The ordinary generic name is **file chooser** or **file picker**. Windows calls
its modern native family the Common Item Dialog; GUI.Forms exposes Forms-shaped
`OpenFileDialog`, `SaveFileDialog`, and `FolderBrowserDialog`; macOS supplies
`NSOpenPanel` and `NSSavePanel`. The macOS panel is Finder-like system UI, but it
is an AppKit service rather than a Finder application window.

File Manager needs a broader first-party object called the **Document Picker**:
a bounded file-selection surface assembled from the same navigation and object
model as File Manager. A picker operation is a **File Selection Session**. The
picker is not a miniature independent file manager and does not become a generic
filesystem mutation API.

## Architect direction

- **GIVEN:** GUI.Forms, Engine, Kolmogrov, and Orchestrator continue converging;
  File Manager frontend implementation remains deferred for several more steps.
- **GIVEN:** after GUI.Forms reaches a decent consumption state, File Manager and
  its reusable open/save-as picker representation are implemented together.
- **GIVEN:** the same Orchestrator/GUI.Forms backbone will support more than File
  Manager.
- **GIVEN:** the first additional proving application is a deliberately small
  classic-Paint descendant, followed by a classic plain-text/configuration
  editor named **Text Editor**.
- **GIVEN:** Paint emphasizes direct bitmap editing and alpha-capable reusable
  clipart/composites rather than a layers panel or revision-history product.
- **GIVEN:** File Manager file/preview objects may be dragged into an active
  Paint canvas; Paint composites may be saved as ordinary reusable clipart and
  used to construct later canvases.
- **GIVEN:** Text Editor is a plain-text and configuration editor, not an RTF
  editor, WordPad descendant, browser-like tab workspace, or modern Notepad
  clone. It needs find/replace, modest format color hints, and useful access to
  hidden configuration files.
- **GIVEN:** picker visibility preferences, including hidden-file display, may
  differ by host application. They are not forced to follow File Manager's
  global view preference.
- **DECIDED under ADR-013:** persistent left/right panels belong only to File
  Manager and this embedded picker/browser projection. Paint, Text Editor,
  Games, and other applications use owned popup dialogs for secondary tools.

## Application surface law

The Document Picker may retain File Manager's bounded tree/content/preview
composition because it is the same navigation model presented for selection.
That exception does not make a panel-based application shell part of the shared
backbone.

- Paint uses a canvas window plus owned dialogs, including the detailed Color
  selector/converter and Help.
- Text Editor uses a text window plus owned dialogs, including Find/Replace,
  Characters and Help.
- Games use a board/table/playfield window plus owned New Game, Rules, Hint and
  result dialogs.
- Providers and plugins supply typed data only and never contribute controls,
  dialogs, panels or native windows.

GUI.Forms may expose panels as general controls, but each product's accepted
topology governs whether they can appear. Reuse of a widget inside a dialog does
not create a reusable dock/panel framework requirement.

## Recommended responsibility split

This split is **CANDIDATE** until the affected projects reply.

| Owner | Responsibility |
|---|---|
| GUI.Forms | Retained controls, owned/custom modal sessions, focus restoration, keyboard/IME/accessibility, application help hooks, drag/drop and clipboard mechanics, native common-dialog fallback |
| File Manager frontend/shared surface | Breadcrumb/tree/object-field composition, live directory model, selection controller, filters, save-name field, validation presentation, house style, degraded states |
| Orchestrator | Stable application identity, caller capability/availability, per-application picker profile, selection-session policy, route choice, settings namespace, handler/type information, result envelope, local help-provider registration |
| Engine | Optional bounded name/search evidence and file-object anchors; never required for ordinary picker navigation |
| Host application | Operation intent, accepted file types, initial location, actual document parsing/writing, unsaved-change policy, application-specific help content |
| Platform adapter | Filesystem enumeration/identity, access handles or security-scoped grants where required, native common-dialog fallback, OS drag/clipboard projection |

Orchestrator remains headless. “Orchestrator's GUI” means a first-party control
surface over Orchestrator contracts. Its semantic settings/service model may be
shared by a standalone control-center shell and File Manager's settings area;
the daemon does not own a window or send arbitrary controls/layout to clients.

## Picker delivery choices

### A — native common dialogs only

Gains mature OS sandbox grants, platform familiarity, and minimal initial code.
Loses house consistency, File Manager dogfood, application-specific browsing
behavior, and reusable object/preview integration. Retain as fallback.

### B — dedicated out-of-process picker host

Gains one independently updated implementation, isolation from host crashes,
and a future path for untrusted or ABI-incompatible clients. Loses effortless
window-modal ownership, adds startup/IPC/focus complexity, and risks looking
like a detached application. Retain as a later route.

### C — link the entire File Manager application into every host

Gains maximum reuse superficially. Loses a bounded dependency, startup economy,
and authority clarity; invites host-specific conditionals into File Manager.
**REJECTED** as the reusable design.

### D — shared in-process picker surface plus Orchestrator session

The host links a small first-party `DocumentPickerView`/controller package built
with the same GUI.Forms consumption snapshot as File Manager. Orchestrator
provides policy, application profile, availability, and session/result
semantics. This gives true owned modality and direct retained composition
without duplicating File Manager or putting GUI code in the daemon.

**CANDIDATE recommendation:** D for File Manager, Paint, and Text Editor; A as
universal fallback; B later for sandboxed, untrusted, or incompatible clients.

## File Selection Session

A session request needs bounded, typed fields rather than arbitrary UI
overrides:

- stable requesting `ApplicationId` and application-instance/session identity;
- purpose: open one, open many, select folder, create new document target, save
  as, export, import clipart, or choose auxiliary resource;
- allowed object kinds and selection cardinality;
- accepted type identifiers, extensions, and display filters;
- initial location as a capability-scoped location hint, not ambient authority;
- proposed filename and default extension for save/export;
- whether an existing target is allowed and which component owns overwrite
  confirmation;
- visibility profile: inherit application preference, show hidden, or hide
  hidden;
- admitted picker commands such as create folder, rename, delete, preview, and
  search—defaulting to the smallest set required by the purpose;
- route preference: house picker preferred, native fallback allowed, or a
  separately justified strict route;
- deadline/cancellation identity, result budget, locale/theme generation, and
  capability context.

The caller may narrow its registered permissions in a request. It may not gain
filesystem mutation, content inspection, hidden-object disclosure, or plugin
execution by adding an unknown override.

The result distinguishes cancel, accepted, denied, unavailable, stale,
version-mismatch, and validation failure. An accepted result contains exact
object/path snapshots plus a platform access token/handle when required. It does
not claim that the selected object still exists or is unchanged when the host
later opens it.

### Save correctness

The picker chooses and validates a destination; Paint or Text Editor performs
the write. Save-as must report whether the target existed, its observed identity
and generation when available, the chosen extension, and the user's overwrite
decision. The writer revalidates immediately before atomic replacement. A
picker result is not a durable reservation unless a future platform-specific
reservation contract explicitly supplies one.

## Per-application hidden-file policy

Hidden visibility and access authority are separate:

- **GIVEN:** File Manager, Paint, and Text Editor may have different defaults.
- A stable signed/bundled application identity owns a settings namespace.
- The profile records `inherit`, `show`, or `hide`, plus whether the user may
  change it in that application's picker.
- A session-local toggle changes only that session unless the user explicitly
  chooses to remember it for the host application.
- Showing a hidden name does not grant read/write access; access still follows
  filesystem and sandbox authority.
- Search, filtering, recents, and filename completion obey the same effective
  visibility rule so hidden names do not leak through a side channel.

**CANDIDATE default:** Paint hides hidden files; Text Editor shows them or asks
once during its first configuration-oriented picker use; File Manager retains
its own explicit view preference.

## Help ownership

Three distinct things should not collapse into one “help provider”:

1. **GUI.Forms `HelpProvider` mechanics:** stable per-control/context help IDs,
   F1/help-key routing, tooltip and described-by relations, and a composited
   greaseboard overlay scoped to the owning window or bounded File Manager
   panel. It does not imply a Help side panel in every application.
2. **Application help corpus:** Paint or Text Editor owns its bundled, local,
   versioned topics, illustrations, command descriptions, and context map.
3. **Orchestrator help registry:** resolves `ApplicationId + HelpTopicId +
   locale + version` to an available local provider/resource pack and reports
   unavailable or incompatible honestly.

Paint's immediate help can therefore live inside its package and render through
GUI.Forms in an owned popup dialog or greaseboard overlay. Orchestrator registers
and locates it so a future shared Help Viewer or File Manager control surface
can open the same topic. No web lookup, remote content, persistent application
Help panel, or plugin-supplied native window is implied.

## Orchestrator control surface

The same settings/service GUI can appear standalone and inside File Manager if
the shared object is a bounded presentation model, not arbitrary server-driven
UI. Orchestrator may publish typed groups, setting descriptors, service rows,
availability, command IDs, validation, and help-topic IDs. A first-party
GUI.Forms view/controller package supplies layout and house rendering in either
shell. Plugins may contribute namespaced schemas only through future admitted
contracts; they never contribute controls, styling, native windows, or code to
the trusted renderer.

## Paint transfer and clipart

- GUI.Forms must complete outbound drag initiation, lazy/promised payloads,
  clipboard ownership, and Windows/Linux adapters; inbound bounded payloads are
  already measured partially.
- File Manager advertises ordinary file references and exact object snapshots.
- Paint accepts file references and admitted image/clipart flavors. It owns
  decode/import policy; GUI.Forms' PNG decoder does not become a general image
  ingestion service.
- A Paint native canvas format may retain movable compositional objects without
  exposing a layers panel or general revision history. Exported flattened images
  remain separate from reusable alpha-bearing clipart objects.
- Reusable clipart should be ordinary files with a registered type/handler so it
  can be browsed, previewed, dragged, copied, and indexed like any other file.
- Large or delayed transfer data uses a bounded lazy stream/file promise rather
  than copying unbounded bytes into one drag message.
- Orchestrator may register application/type/transfer capabilities, but it is
  not on the pointer-move or paint hot path and does not own canvas bytes.

## Text Editor picker profile

Text Editor needs a first-party application profile optimized for configuration
work:

- plain-text open/save with explicit encoding and newline/BOM observation;
- no implicit RTF conversion;
- hidden-file visibility independent of File Manager;
- filesystem navigation and opening that do not require indexing;
- optional lightweight language/color-hint provider selected by file type;
- find/replace without regular-expression requirements;
- no document tabs.

The architect's term “whitecards” is retained as **OPEN terminology**; if it
means wildcard matching, the exact `*`, `?`, escaping, case, and newline rules
need a small separate text-search contract.

## Unresolved decisions

1. Whether first-party picker UI code lives in a frontend-produced shared
   library, a new application-surfaces project, or another independently
   versioned package.
2. Whether an out-of-process picker host is needed for first release or only
   after third-party/sandboxed consumers exist.
3. Which picker commands exist in each purpose profile; especially rename,
   delete, create-folder, preview, and search.
4. Whether Text Editor defaults to showing hidden files or prompts once.
5. Whether save overwrite confirmation is rendered by the shared picker or by
   each host application while preserving one semantic result.
6. Paint's native canvas and clipart formats, flattening rules, object/group
   semantics, and decoder/plugin boundary.
7. Whether the Orchestrator control surface receives a standalone shell in the
   first application wave or only File Manager embedding.

The surface-law question is not open: non-File-Manager applications do not gain
persistent left/right panels. Their interview may choose modal versus modeless
owned dialogs for each secondary operation.
