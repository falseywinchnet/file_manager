# File Manager frontend design brief 001 — revision 007

Status: **accepted supporting synthesis for the Frontend 001 design
constitution; individual CANDIDATE entries remain open and implementation still
requires the ADR-004 opening events**.

Date: 2026-08-05. Supersedes revision 006 by applying the nineteen GIVEN owner
directions in
[`DESIGN_DNA_VERDICTS_007.md`](DESIGN_DNA_VERDICTS_007.md). It preserves the
corrected graphite House Composite, approved color/icon directions, and
inspectable Design DNA laboratory.

Companion artifact:
[`frontend-concept-atlas.html`](frontend-concept-atlas.html).

**Superseding owner correction (2026-08-07):** the ordinary path trail is now a
vertically dense continuous chevron cascade with a first-order inline editor.
The pulled-down path matrix described in revision 007 is no longer the primary
editing model. See
[`../OWNER_DIRECTION_2026-08-07.md`](../OWNER_DIRECTION_2026-08-07.md).

## Purpose

Turn the accepted product shape and recorded visual verdicts into a coherent
daily-work surface for Frontend 001. This brief does not select a GUI.Forms API,
frontend source structure, icon license, font package, or final numerical theme.
The HTML is a deterministic planning instrument built from CSS and inline SVG;
it does not dogfood GUI.Forms and contains no image-generated material.

## Pooled program knowledge

### Product composition

- **GIVEN:** one location-oriented window; no tabs and no dual pane.
- **GIVEN:** the left folder tree and right preview/properties pane are persistent
  concepts but fully collapsible.
- **GIVEN:** the center is the only file-content surface. Its default view is
  small icons; list/details and criteria views remain deliberate modes.
- **GIVEN:** one vertically dense continuous chevron breadcrumb precedes a
  separate current-subtree search field. Its first-order editing operator lives
  inline in the trail rather than opening a path matrix.
- **GIVEN:** activating path editing preserves the trail's retained identity and
  row geometry while exposing command-line-like text entry and autocomplete.
  Canonical paths and safe shell conveniences are accepted; the resolved
  canonical destination is shown before explicit navigation.
- **CANDIDATE:** recent full breadcrumb trails may survive behind a separate
  history affordance. Their count, placement, and repeated-root treatment are
  no longer frozen and may not displace the inline editor.
- **GIVEN:** the shelf is compact, labelled, and task-specific. It is not a
  modern oversized ribbon or an icon-only strip. Move/Copy, Delete, View, Sort,
  and Properties remain permanent; New Folder is background-context only and
  Rename/Open With are object-context only.
- **GIVEN:** ordinary resizing is content-aware before panes and command groups
  collapse by priority. The window may not shrink below 150 × 150 logical units.
- **GIVEN:** Portsmouth Rapids is restricted to titles and control chrome.
  Content and explanatory text use a bundled Tahoma/Calibri-like humanist body
  face selected through the GUI.Forms font-pack gate. Uncovered clusters use
  bounded bundled fallback packs rather than arbitrary host fonts.
- **GIVEN:** selection is single-click and opening is double-click.
- **GIVEN:** the file surface remains useful without the engine or Orchestrator.
- **GIVEN:** search never silently widens beyond the displayed folder subtree.
- **GIVEN:** Home is the real home directory. Drive-rooted and Home-rooted trees
  are honest structures; Quick Access, Libraries, This PC, Recommended, and
  provider-branded aliases are excluded.

### Authority and failure presentation

- **DECIDED:** filesystem/platform identity is authoritative; displayed paths
  are addresses, not durable identities.
- **DECIDED:** the engine owns catalogue observations and generation-bound query
  results. Its evidence channels remain distinct and inspectable.
- **DECIDED:** Orchestrator owns handler, command, settings, capability, provider,
  plugin, and audit semantics across project boundaries.
- **GIVEN:** the frontend owns window, pane, navigation, selection, search
  presentation, preview/properties composition, and degraded-state presentation.
- **GIVEN:** a stale, rebuilding, absent, or quarantined engine can reduce search
  quality but cannot prevent live folder navigation.
- **GIVEN:** plugins contribute validated data or declarative commands, never
  controls, styles, native windows, or callbacks into the frontend.
- **OBSERVED:** the current engine API names ready, partial, stale, unavailable,
  generation, evidence, warnings, and cursor states. The mockup uses these
  meanings, not the current Go data layout.
- **OBSERVED:** the Orchestrator registry is still paper/importing-v0. Any screen that
  implies finalized plugin, handler, or provider payloads is illustrative only.

### GUI.Forms consumption boundary

- **GIVEN:** GUI.Forms is retained, custom-rendered C++ with disciplined damage,
  synchronous UI-thread state mutation, explicit ownership, and no GPU path.
- **GIVEN:** PNG is the only renderer/resource-core image decoder currently
  admitted. SVG in this HTML is a planning convenience; selected production
  icons would need pinned PNG derivatives and source/license provenance.
- **OBSERVED:** GUI.Forms currently proves basic retained controls, physical
  panel styles, input routing, damage, and experimental ABIs, but lacks the
  production text editor, selected HarfBuzz/FreeType integration, IME,
  scrolling, tree/list/menu, accessibility publisher, and layout families
  required by File Manager.
- **DECIDED:** the old Gate F0 is superseded by ADR-004 and ADR-006. Frontend
  001 source may begin only after Orchestrator Core 1.0 is live, the named
  GUI.Forms go-ahead is recorded, and the architect explicitly starts it.

## Visual grammar

### House character

**GIVEN:** the target is “future-vaporwave retro straight out of somewhere
between 1998 and 2007 that can't be placed anywhere and is HD, snappy, and makes
nice clicky sounds,” with “Microsoft Encarta '00 ultra HD deluxe premier edition
vibes.”

**HYPOTHESIS:** the daily surface should feel like a precise information
instrument housed inside an illustrated object: classically deep, professional,
industrial, optimistic, and unusually resolved. This is not a generic retro
skin. The white file field is the instrument; the blue-violet title fresco
identifies the object; physical chrome communicates action and editability; rich
material icons give stored objects their identity.

The resulting grammar is:

1. unequivocal window and pane geometry; the exact square/soft edge grammar is
   now a comparison axis rather than a settled family;
2. one global light direction;
3. raised actions, inset inputs, etched groups, and explicit plane shadows;
4. color concentrated in title fresco, selection, object icons, and tiny status
   signals rather than spread across structural rails;
5. stable labels beside or below command icons;
6. white object-centered content with no cards, alternating-row wallpaper, or
   luminous category headers;
7. dense factual typography with modest breathing room rather than either Motif
   compression or contemporary luxury spacing;
8. immediate state changes with no navigational choreography.

### Redundancy law

**GIVEN:** the first pass presented too much repeated information.

**CANDIDATE:** every persistent fact receives one primary home:

| Fact | Primary home |
|---|---|
| current location | breadcrumb/path instrument |
| search scope | search well suffix |
| sort order | labelled shelf command |
| view mode | status-line view actuator |
| selection and object count | left status cell |
| index/generation/degraded state | local-navigation status at bottom |
| selected-object identity and properties | right selection pane |
| search why/where/excerpt | expanded result toast only |

The folder-content summary strip is removed. The selection pane does not label
itself “Preview”; its image already establishes that role.

### Proposed spatial tokens

These are **CANDIDATE mockup values**, not GUI.Forms constants.

| Role | Candidate value | Reason |
|---|---:|---|
| Title fresco | 40 px | enough identity without becoming a semantic banner |
| Classic menu | 22 px | explicit commands without a second toolbar |
| Command shelf | 64 px | labelled, bounded task controls |
| Path/search row | 40 px | distinct navigation and query instruments |
| Tree default | 218 px | useful hierarchy without source-list dominance |
| Preview default | 288 px | preview plus editable property groups |
| Status line | 24 px | selection, item count, index state, view controls |
| Small-icon object | 82 × 72 px | high-color object plus two-line label |
| Details row | 26 px | factual density with clear focus geometry |

### Color relations — owner approved direction

The mockup now tests twelve authored vaporwave pair families recorded in
[`VAPORWAVE_PALETTE_ATLAS_003.md`](VAPORWAVE_PALETTE_ATLAS_003.md): Encarta
Cobalt, Amethyst CRT, Miami Ledger, Orchid Relay, Aqua Memory, Apricot Modem,
Mulberry Glass, Viridian Laser, Sapphire Dusk, Rose Quartz, Electric Iris, and
Phosphor Violet.

**GIVEN:** retain all twelve families and use **Sapphire Dusk** as the default.
Their numerical tokens remain subject to final contrast and state gates. The
central file canvas remains white or warm pearl; atmospheric chroma is
concentrated in the title fresco, chrome tint, selection relation, focus, icons,
and rare signals.

### Structural construction axis

Color and styling are now independent. State `06 Styles` holds Sapphire Dusk
constant while applying twelve strongly different construction families from
[`STYLE_FAMILY_ATLAS_004.md`](STYLE_FAMILY_ATLAS_004.md): Watercolor
Instrument, House Composite, Studio 2003, Office Pearl, Aqua Technical,
Workshop Graphite, Machined Workstation, QNX Precision, Object Desk, MSN Jewel,
Delphi Laboratory, and Sky Pearl.

All twelve are **CANDIDATE**. They vary command orientation and grouping, edge
grammar, physical depth, pane captions, tool density, and structural light/dark
balance. They do not vary product topology or filesystem authority.

**GIVEN:** House Composite is now the primary styling direction. It uses
Watercolor identity and Office Pearl/Word ribbon anatomy above the navigation
boundary, Workshop Graphite for navigation and structural chassis below the
ribbon, and Studio 2003 hints inside folder, search, and selection/properties
panes. **GIVEN correction:** that chassis is a middle-value, subtly textured
graphite rather than black, and visible pane separators are 3 px or thinner.
The pointer hit target may exceed the visible seam. The exact CSS values remain
candidate mockup values. Official Microsoft ribbon references and the
accepted/rejected cue extraction are recorded in the style atlas.

### Design constitution

Revision 006 introduced
[`DESIGN_DNA_006.md`](DESIGN_DNA_006.md), the **DECIDED Frontend 001 visual
constitution**. Its individual candidate and hypothesis records remain open;
its sixteen principles make visual behavior derivable rather than merely
consistent by imitation. Its governing promises are: objects before
containers; one location is one place; honest structure; material that explains
behavior; an immediate probable path; locally evident state; justified
redundancy; geographically budgeted richness; density without cramped targets;
honest time; relational and scarce color; familiar behavior with authored
appearance; relationship-preserving adaptation; visual performance; domesticated
external contributions; and explicit, reversible exceptions.

Every nuance is reviewed as one object with one status, principle, rule,
deterministic specimen, evidence lineage, failure test, and owner verdict. This
does not elevate an individual visual candidate into architecture or a
GUI.Forms contract.

Revision 007 records the first extensive owner dogfood round in
[`DESIGN_DNA_VERDICTS_007.md`](DESIGN_DNA_VERDICTS_007.md). It promotes the
named behavior and composition directions to **GIVEN** while leaving their
rendering metrics, platform adapters, sound assets, responsive breakpoints, and
unbuilt state specimens open.

## Mockup states

### 01 — Folder / small icons

The baseline daily view. It tests the fixed single-window topology, physical
command shelf, dense chevron path cascade with inline editing and autocomplete,
honest tree roots, high-color objects, indexed folder-size badges, embedded
preview, and collapsible panes. The historical screenshot's pulled-down path
matrix is no longer authoritative.
Folder-size marks now show allocated size only after indexing: no mark at or
below `1M`, then compact embossed values rounded by `5M` below 100 MB, `25M`
through 1000 MB, `100M` from 1 GB to 1 TB, and `100G` above 1 TB. Exact size
moves to inspection. Unindexed locations do not invent either folder sizes or
thumbnails. Hover or keyboard focus may expose a richer factual inspection
without selecting or opening the object.

### 02 — Search / expandable result correspondence

The content surface becomes a stack of shallow Outlook-like correspondence
toasts. Search has no separate preview pane. A collapsed toast states file
identity, ordinary facts, and match percentage but withholds location and match
reason. Hover expansion reveals a plain metadata sentence, matched substrings or
criteria, and a short inline excerpt. Semantic/provider indications have a
separate plugin-information lane at the right edge of the expanded toast. They
are neutral tags there; the evidence string itself is not broken into colorful
chips. A stale or unavailable result remains factual and locally visible.

### 03 — Criteria / virtual-folder instrument

**GIVEN:** criteria use a shallow instrument rack above an otherwise ordinary
virtual folder. Each compact predicate module exposes field, operator, value,
enabled state, removal, and local validation while keeping source location
anchored in Projects. Inexpensive changes update live; expensive changes stage
visibly for Apply. The former lens metaphor is rejected. Because this is a
virtual-folder representation, the Selection pane remains.

### 04 — Vaporwave palette atlas

Twelve clickable review cards apply their complete pair family to the same
window. The cards are a review instrument, not a proposed file-browser content
mode.

### 05 — Icon depth laboratory

The shortlist now prioritizes Fluent Color, Fluent Emoji 3D, Meteocons Fill,
AppIcon Forge as a construction study, and an original house-material family.
**GIVEN:** the direction is approved: Fluent Color supplies the external
semantic/coverage base and an original House Material layer supplies the deeper
visible objects, with Fluent Emoji 3D and Meteocons retained as material
references. Packaging, pinned provenance, derivative policy, and the original
art license still require their declared gates.

### 06 — Structural style atlas

Twelve clickable candidates materially rebuild the surrounding instrument while
Sapphire stays fixed. This is a review surface, not a proposed file-browser
content mode and not an architecture selection.

### 07 — Design DNA laboratory

Forty-seven individual decision panes dogfood the constitution across frame,
commands, navigation, panes, objects, search, time/failure, and adaptation. The
index can be searched from the ordinary query well. Each pane presents one
candidate direction, its governing principle, a bounded specimen, confidence
and design debt, evidence, failure/reversal rule, and a locally saved review
verdict. Arrow keys and the previous/next controls move decision by decision;
`?state=dna&decision=DNA-V04` deep-links a review object. Verdicts remain review
notes, not ADRs.

## MIT-only icon-system basis

**GIVEN:** Fluent Color plus an original House Material depth layer is the
approved direction. The remaining entries are supporting references or coverage
reserves. “Repository says MIT” is not sufficient by itself: every selected
revision still needs a file-level provenance manifest and an audit for separately
licensed brand marks or imported assets.

| System | Proposed role | Visual fit | Main issue |
|---|---|---|---|
| Microsoft Fluent UI System Icons | primary command/status skeleton | strongest professional and familiar control vocabulary; filled weights survive small sizes | intentionally flat; cannot lead object identity |
| Phosphor Icons, duotone weight | alternate command skeleton | multiple weights and two-tone hierarchy adapt well to physical button plates | friendlier/softer than the industrial target |
| VSCode Symbols by Miguel Solorio | file/folder taxonomy reference | compact, colored, serious developer-tool register; actual repository is MIT | contemporary and flat; optimized for a code tree rather than large file objects |
| Microsoft Fluent Emoji | material and lighting study only | MIT artwork includes unusually resolved color and three-dimensional material cues | literal emoji are too playful and semantically wrong for direct adoption |
| Tabler Icons, filled subset | semantic coverage reserve | extremely broad, disciplined 24 px grid, explicit MIT license | generic web-product character and insufficient depth |

**REJECTED for the MIT-only constraint:** `vscode-icons/vscode-icons` and
Microsoft's separate `vscode-icons` artwork repository. Their code may be MIT,
but the icon/content licenses are CC BY-SA and CC BY respectively. Neither is an
MIT-only artwork source.

**GIVEN direction:** use Fluent Color for semantic coverage and an original
32/48 px House Material family for folders, files, volumes, archives, media,
applications, and degraded states. Fluent Filled and Phosphor remain command
reserves; VSCode Symbols remains a taxonomy inventory; Fluent Emoji and
Meteocons remain references for light, volume, material separation, and edge
finish—not sources of literal product semantics. Exact asset mapping,
derivatives, and package boundaries remain gated.

## Remaining visual questions

The owner round closes the prior activation, tree-root, command-membership,
preview-scroll, folder-size form, typography-role, criteria-form, recent-count,
and path-expansion questions. The following remain **CANDIDATE**:

1. **VIS-FE-004:** Should everyday control density lean toward Studio 2003,
   QNX/Delphi micro-tools, or the larger Encarta object shelf within the accepted
   Office Pearl structure?
2. **VIS-FE-009:** Should unavailable/offline status remain a small object badge
   plus factual text, or use a stronger whole-row treatment?
3. **VIS-FE-012:** Is a separate dark atmosphere needed for first dogfood, or
   should light, host-high-contrast, and local-high-contrast modes close first?
4. **VIS-FE-014:** Does House Composite read as one authored object, or is the
   pearl-to-graphite transition still too visibly assembled?
5. **VIS-FE-017:** Does the corrected graphite carry enough mechanical identity
   without approaching a dark theme?
6. **VIS-FE-018:** What keyboard operation resizes a pane when its visible seam
   remains a hairline and its pointer target is larger?
7. **VIS-FE-020:** How should a coarse folder-size badge disclose stale versus
   current indexed evidence without becoming a second status system?
8. **VIS-FE-021:** What exact interaction sounds, concurrency limits, and
   repetition rules implement the accepted state/location-only vocabulary?

## Explicit exclusions preserved

The mockup does not introduce tabs, dual panes, a source-list Home, automatic
grouping, cards, provider-branded navigation, web search, floating preview,
rounded/Mica chrome, skeleton loading, injected plugin controls, or a second
search index. No visual flourish is allowed to outrank startup, memory, input,
or accessibility budgets.
