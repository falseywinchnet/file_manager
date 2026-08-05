# File Manager design DNA 006

Status: **DECIDED visual constitution for Frontend 001 under ADR-004, assembled
from GIVEN owner direction, accepted local verdicts, and OBSERVED external
design-framework discipline. Its individual CANDIDATE and HYPOTHESIS records
remain unresolved; it does not select GUI.Forms APIs or implementation tokens.**

Date: 2026-08-05.

Companion surface:
[`frontend-concept-atlas.html`](frontend-concept-atlas.html), state `07 DNA`.

Latest owner verdicts:
[`DESIGN_DNA_VERDICTS_007.md`](DESIGN_DNA_VERDICTS_007.md).

## Purpose

This document changes the unit of visual design from a screenshot preference to
an inspectable decision. File Manager should be coherent for the same reason a
good operating system is coherent: each visible behavior descends from a small
set of promises, every exception is named, and a new surface can be derived
without copying an old screen.

**GIVEN:** Design DNA 006 is the derivation and review authority for Frontend
001. Every nuance must remain individually reviewable. An attractive but
unprincipled choice is design debt. A principled choice without evidence remains
a hypothesis or candidate. A visual decision does not become architecture until
its proper decision gate is satisfied.

## Epistemic pipeline

Every visual decision record uses this path:

```text
owner constraint / observed need
    → governing principle
    → candidate rule
    → deterministic specimen
    → counterexample and accessibility audit
    → owner verdict
    → measured implementation gate, if implementation is later authorized
```

Each record contains:

| Field | Meaning |
|---|---|
| ID and object | one reviewable visual or behavioral question |
| Status | GIVEN, OBSERVED, MEASURED, HYPOTHESIS, CANDIDATE, REJECTED, or DECIDED |
| Principle | the higher promise that constrains the answer |
| Rule | what future surfaces should do by default |
| Evidence | local verdict IDs, program constraints, or named external guidance |
| Specimen | a deterministic comparison, including inactive/error/focus states |
| Failure test | a condition that disproves or rejects the rule |
| Reversal | which token, asset, or composition boundary changes if rejected |

No record may use “modern,” “clean,” “native,” “intuitive,” “fast,” or
“professional” as its justification. Those words require a named mechanism or
measurement.

## The sixteen governing principles

### DNA-01 — Objects before containers

Files, folders, volumes, results, properties, and operations are the subjects.
Chrome explains or acts on them; it does not become the subject. Color and
material richness concentrate in object identity and exceptional state.

### DNA-02 — One location is one place

A window represents one current location. Path, tree, content, selection, and
status must agree about that place. Search and criteria may produce virtual
folders, but their derivation remains visible.

### DNA-03 — Structure must be honest

Visual boundaries correspond to actual differences in role, ownership, or
layer. No separator exists merely to decorate. No two semantic regions are
merged merely to appear minimal.

### DNA-04 — Material must explain behavior

Raised means actionable; inset means editable or instrumented; calm plane means
content; shadow means real overlap. The global light is consistent. Depth is a
behavioral vocabulary, not a skin.

### DNA-05 — The probable path is immediate

Common operations are visible, direct, and require little memory. Less probable
power remains available through menus, contextual surfaces, autocomplete, or
inspectable expansion. This is KDE's “simple by default, powerful when needed”
and Windows' “design for the probable” translated into File Manager.

### DNA-06 — State is locally evident

Selection, keyboard focus, editability, activity, unavailability, and authority
appear where they matter. Success normally changes the affected object instead
of producing a redundant announcement.

### DNA-07 — Redundancy requires independent value

The same fact may appear twice only when the representations answer different
questions or survive different failure modes. Full recent breadcrumb stacks are
valid redundancy because each row proves an independent destination. Repeated
item counts in adjacent bars are not.

### DNA-08 — Richness is geographically budgeted

Daily content stays calm. The title may carry identity, icons may carry object
material, instruments may become deeper, and rare ceremony may become a world.
Richness is assigned to loci instead of sprayed across every surface.

### DNA-09 — Dense is not cramped

Density removes travel and memory load; cramped layouts reduce target clarity,
legibility, or grouping. Every reduction in space must preserve a hit boundary,
reading rhythm, and focus path.

### DNA-10 — Time must remain honest

State changes promptly. Motion explains causality, continuity, or activity; it
never delays navigation for spectacle. Progress distinguishes known work from
unknown work and exposes exact stages when available.

### DNA-11 — Color is relational and scarce

Foreground/background pairs are the primitive. Color communicates hierarchy or
semantic state and never acts as the sole carrier of meaning. Neutral structure
protects the salience of selection, objects, warnings, and live data.

### DNA-12 — Familiar behavior, authored appearance

Keyboard, pointer, selection, editing, menus, focus, drag/drop, and window
behavior remain conventional across target platforms. File Manager may have a
custom visual voice without inventing gratuitous interaction grammar.

### DNA-13 — Adaptation preserves relationships

DPI, font size, locale, RTL, contrast, reduced motion, window size, inactive
state, and alternate color schemes may alter metrics. They may not destroy the
relationships that explain hierarchy or authority.

### DNA-14 — Performance is part of appearance

The interface should feel attached to input. Immediate press feedback, stable
geometry, bounded invalidation, and absence of decorative startup work are
visual requirements. Polish cannot borrow from responsiveness.

### DNA-15 — Every external contribution is domesticated

Plugins contribute validated facts, commands, and object kinds. The house
system assigns their controls, materials, language, icons, failure treatment,
and accessibility. No plugin imports a competing visual constitution.

### DNA-16 — Exceptions are explicit and reversible

An exceptional material, animation, or topology names its scope and reason.
Tokens and specimens identify the reversal seam. Accumulated unnamed exceptions
are a failed design system.

## Primary-source framework survey

The following sources are used for decision discipline, not visual imitation.

| Framework | OBSERVED discipline | File Manager adoption | Boundary/rejection |
|---|---|---|---|
| Apple HIG | hierarchy, harmony, consistency; focus and selection are explicit states; toolbars group deliberate frequent actions | relationships between content, control, selection, and focus must survive scale and state | current Apple materials, translucency, and device geometry are not the house style |
| Windows desktop/Fluent | design for the probable; place commands on the right surface; feedback should be natural and nonredundant; coherence spans layout, interaction, visual style, window behavior | probability governs command prominence; content, menu, ribbon, context, and dialog have different command budgets | web-like cards, disappearing affordances, Mica as identity, and touch density are not imported |
| KDE | simple by default, powerful when needed; customization supports diverse workflows; status messages are minimized and actionable; exhaustive modality testing | visible common path plus inspectable expert power; explicit error ownership; test keyboard, pointer, color, text size, sound-off, screen reader, reduced motion | configurability does not justify unstable defaults or toolbar accretion |
| GNOME | design for people; make each view simple; reduce effort; be considerate; progressively disclose less-common actions | prevent overload and interruption; shorten language; keep common actions close | single-purpose minimalism does not erase professional file-management power |
| elementary OS | text is brief and translatable; ellipsis predicts further input; disabled versus hidden has semantic consequences; feedback escalates from toast to infobar to dialog; icons are redrawn per size | establish writing, visibility, feedback, and icon-size rules before styling | header-bar topology, icon-only preference, and automatic-save policy are not inherited |
| Enlightenment/EFL | theme, layout, and animation data are separated through a theme abstraction; widget styles can vary without changing the widget's purpose | supports the hypothesis that semantic role and visual construction are separate axes | EFL architecture and animated jewel excess are not selected dependencies or defaults |
| RISC OS Style Guide | “style” includes appearance, functionality, integration, keyboard conventions, and consistency that accelerates transfer learning | visual governance covers behavior, language, input, and interoperability—not CSS alone | RISC OS desktop topology and historical metrics are not copied |
| classic Macintosh lineage | direct manipulation, visible results, consistency, user control, forgiveness, and concrete metaphor established the constitutional model for GUI decisions | object identity, immediate feedback, undo, and learnable placement are first-order rules | literal classic-Mac chrome or single-button assumptions are irrelevant |
| Office/Word ribbon | stable tabs/groups/commands, group captions, visible common actions, and controlled overflow | command shelf uses quiet group structure and labelled priority | flat monoline iconography and generic document-app commands are rejected |

Primary references:

- [Apple Human Interface Guidelines](https://developer.apple.com/design/human-interface-guidelines)
- [Apple focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection/)
- [Apple toolbars](https://developer.apple.com/design/human-interface-guidelines/toolbars)
- [Windows: design for probable desktop behavior](https://learn.microsoft.com/en-us/windows/win32/uxguide/how-to-design-desktop-ux)
- [Windows commanding basics](https://learn.microsoft.com/en-us/windows/apps/design/basics/commanding-basics)
- [Windows design principles](https://learn.microsoft.com/en-us/windows/apps/design/design-principles)
- [KDE HIG](https://develop.kde.org/hig/)
- [KDE app philosophy](https://develop.kde.org/hig/kde_app_design/)
- [KDE status changes](https://develop.kde.org/hig/status_changes/)
- [KDE accessibility audit](https://develop.kde.org/hig/accessibility/)
- [GNOME design principles](https://developer.gnome.org/hig/principles.html)
- [elementary OS layouts](https://docs.elementary.io/hig/widgets/creating-layouts)
- [elementary OS feedback](https://docs.elementary.io/hig/widgets/providing-feedback)
- [elementary OS iconography](https://docs.elementary.io/hig/reference/iconography)
- [Enlightenment theme documentation](https://www.enlightenment.org/docs/start)
- [RISC OS Style Guide](https://www.riscosopen.org/zipfiles/platform/common/StyleGuide.3.pdf)

## Design-layer constitution

Visual values descend in this order. A lower layer may refine but not contradict
a higher one.

```text
product truth and authority
  → interaction promise
    → information hierarchy
      → spatial topology
        → control/material role
          → state grammar
            → color pair
              → typography/icon detail
                → motion and sound
```

This ordering prevents a color theme from changing topology, a plugin from
changing interaction, or a bevel from implying an action where none exists.

## Nuance decision ledger

The ledger is deliberately granular. “Direction” is the current principled
default. “Gate/debt” names what still prevents closure.

### Frame, layer, and activation

| ID | Object | Status | Direction | Gate/debt |
|---|---|---|---|---|
| DNA-F01 | one-location window | GIVEN | one current place; no tabs or dual content panes | multi-window history behavior still needs specimen |
| DNA-F02 | active title identity | GIVEN | Watercolor fresco carries application/place identity | inactive version unresolved |
| DNA-F03 | window participation state | GIVEN | distinguish key, secondary-but-participating, and genuinely deactivated surfaces; secondary windows recede moderately and regain destination emphasis on drag proximity | exact recession and drag-proximity cues need side-by-side tests |
| DNA-F04 | outer keyline | CANDIDATE | one dark keyline states the window boundary | test high contrast and dark host desktops |
| DNA-F05 | window corner | CANDIDATE | small 3–4 px optical softening; internal role edges remain crisp | current values not scale-derived |
| DNA-F06 | frame shadow | CANDIDATE | shadow only proves window overlap; no ambient glow | measure legibility on varied wallpapers |
| DNA-F07 | popup shadow | CANDIDATE | deeper than window internals, proportional to actual overlap | path matrix is current specimen |
| DNA-F08 | z-order vocabulary | HYPOTHESIS | keyline → local occlusion → cast shadow maps increasing layer | incomplete dialog/menu specimens |

### Ribbon and commands

| ID | Object | Status | Direction | Gate/debt |
|---|---|---|---|---|
| DNA-C01 | tab rail | GIVEN | tabs select command categories, not content locations | keyboard traversal and overflow unresolved |
| DNA-C02 | ribbon field | GIVEN | quiet Office pearl plane under Watercolor identity | exact pearl texture unmeasured |
| DNA-C03 | permanent commands | GIVEN | shelf holds Move/Copy, Delete, View, Sort, and Properties; New Folder is background context, Rename/Open With are object context | command-disabled and multi-selection states need specimens |
| DNA-C04 | group captions | CANDIDATE | small bottom captions make command geography learnable | test legibility at 125–200% DPI |
| DNA-C05 | group separators | CANDIDATE | one hairline only where command purpose changes | remove if caption/spacing is sufficient |
| DNA-C06 | large command icon | CANDIDATE | size follows recognition/frequency value, not importance theater | hierarchy not yet authored per command |
| DNA-C07 | command labels | GIVEN | visible on ordinary shelf commands | responsive collapse policy unresolved |
| DNA-C08 | hover state | CANDIDATE | shallow local pearl lift; no global color wash | full input-state matrix missing |
| DNA-C09 | pressed state | GIVEN | plate moves optically down and darkens lower edge | material rule; verify at small sizes |
| DNA-C10 | destructive command | CANDIDATE | ordinary until invoked; destructive semantics in icon/text and confirmation/undo policy | trash versus permanent delete needs workflow decision |
| DNA-C11 | disabled command | CANDIDATE | remains discoverable when prerequisite is understandable; hide when context makes it meaningless | per-command prerequisites absent |
| DNA-C12 | responsive composition | GIVEN | content-aware reflow first, then priority collapse; enforce a 150 × 150 logical-unit minimum | exact breakpoints and priority sequence unresolved |

### Navigation and place

| ID | Object | Status | Direction | Gate/debt |
|---|---|---|---|---|
| DNA-N01 | back/forward/up | GIVEN | stable leading navigation cluster | forward disabled state missing |
| DNA-N02 | path/search separation | GIVEN | address and query are adjacent but independently bounded | narrow-window behavior unresolved |
| DNA-N03 | terminal `./` position | GIVEN | terminal action at end of path well | icon/text accessible name required |
| DNA-N04 | full-root matrix | GIVEN | overlay exposes current full root and a user-configurable recent count defaulting to 5 full stacks | preference bounds and overflow behavior unresolved |
| DNA-N05 | path editing | GIVEN | terminal-like editor accepts canonical paths and safe conveniences, previews the resolved canonical destination, then navigates explicitly | platform expansion syntax and invalid-resolution treatment need specimens |
| DNA-N06 | breadcrumb segments | CANDIDATE | tangible text segments navigate immediately | truncation and network roots unresolved |
| DNA-N07 | repeated roots | GIVEN | accepted exception because each recent row is independently parseable | rows must stay skimmable at depth |
| DNA-N08 | tree root mode | GIVEN | Home-rooted default with honest immediate Volumes mode | retained-state behavior after first launch unresolved |
| DNA-N09 | search scope suffix | GIVEN | scope is always stated in the query well | scope-change interaction unresolved |
| DNA-N10 | navigation chassis | GIVEN | subtle graphite identifies wayfinding machinery below ribbon | current texture is a visual candidate |

### Panes and separators

| ID | Object | Status | Direction | Gate/debt |
|---|---|---|---|---|
| DNA-P01 | tree/content/selection topology | GIVEN | tree and selection panes are collapsible; center is only content surface | minimum sizes unresolved |
| DNA-P02 | pane separator | GIVEN | corrected to a 3 px or thinner graphite mechanical seam; never a black rail | pointer hit area may exceed visible width |
| DNA-P03 | collapse actuator | GIVEN | small flat seam grip at rest; pointer proximity or keyboard focus increases size, contrast, and depth | exact hit geometry and transition remain candidates |
| DNA-P04 | pane caption | GIVEN | Studio 2003 warm-neutral factual strip with a tiny identity key | exact capitalization and height unresolved |
| DNA-P05 | section header | CANDIDATE | compact ruled label groups facts without becoming a luminous category banner | current tree bars may still over-separate |
| DNA-P06 | tree row | CANDIDATE | 24 px factual row; hierarchy comes from indent and disclosure | font/DPI/RTL tests absent |
| DNA-P07 | selection/properties scroll | GIVEN | one scroll plane; preview collapses into a compact identity header | collapsed/expanded persistence and focus behavior need specimens |
| DNA-P08 | direct property edit | GIVEN | editable values look inset before activation and focus distinctly during edit | commit/cancel/error states absent |
| DNA-P09 | pane background | CANDIDATE | low-chroma warm-neutral plane distinguishes tools from white content | contrast pairs not frozen |
| DNA-P10 | graphite texture | GIVEN | corrected subtle value texture replaces black; texture must disappear before text clarity suffers | raster/noise implementation not selected |

### Content objects and selection

| ID | Object | Status | Direction | Gate/debt |
|---|---|---|---|---|
| DNA-O01 | file field | GIVEN | white or near-white object-centered work plane | object grounding at sparse counts needs refinement |
| DNA-O02 | object grid | CANDIDATE | compact regular cells with stable label baselines | keyboard spatial navigation not represented |
| DNA-O03 | object icon family | GIVEN | Fluent Color coverage plus original House Material depth | asset production/provenance gated |
| DNA-O04 | icon sizes | CANDIDATE | redraw at 16/24/32/48; do not scale one master | benchmark set not drawn |
| DNA-O05 | object label | GIVEN | centered, maximum two lines, full name on focus/inspection | truncation policy unresolved |
| DNA-O06 | folder-size badge | GIVEN | allocated size only after indexing: none at ≤1M; round by 5M below 100M, 25M through 1000M, 100M from 1G to 1T, and 100G above 1T; use compact embossed `M/G/T` marks such as `1G` | freshness notation remains unresolved; exact allocated size is inspection-only |
| DNA-O07 | selection | GIVEN | semantic color pair plus explicit boundary | active/inactive and high-contrast set incomplete |
| DNA-O08 | keyboard focus | GIVEN | distinct from selection and visible without color alone | current dotted outline needs DPI evaluation |
| DNA-O09 | rich hover inspection | GIVEN | hover may disclose materially richer object information without changing selection or opening the object | keyboard-focus equivalent and complete specimen absent |
| DNA-O13 | indexed thumbnails | GIVEN | object thumbnails exist only when the index supplies them; unindexed locations retain ordinary type/material icons | stale, rebuilding, and unavailable thumbnail states need specimens |
| DNA-O10 | drag source/target | GIVEN | follow ordinary host file-manager/OS behavior with no novel house overlay; preview, tree, windows, and external programs participate; show a window only for cross-volume or collision cases | platform transfer adapters and modal specimens remain absent |
| DNA-O11 | details row | CANDIDATE | factual 26 px rhythm; no alternating wallpaper by default | complete state specimen absent |
| DNA-O12 | empty folder | CANDIDATE | calm object field with one factual sentence and relevant next action only | current atlas has no empty-state specimen |

### Search and virtual folders

| ID | Object | Status | Direction | Gate/debt |
|---|---|---|---|---|
| DNA-S01 | search result form | GIVEN | shallow correspondence-like result strip; no separate preview pane | keyboard/touch expansion needs explicit control |
| DNA-S02 | collapsed result | GIVEN | identity, ordinary facts, match metric | whether terse evidence remains visible is open |
| DNA-S03 | expansion trigger | CANDIDATE | hover may preview, but focus/click must pin expansion | current mockup is hover-dependent |
| DNA-S04 | match percentage | GIVEN | retain percentage as a neutral disclosure of what retrieval thinks, paired with inspectable evidence and no recommendation/probability language | exact score derivation remains engine-owned |
| DNA-S05 | evidence sentence | GIVEN | plain metadata string explains where and why | truncation and copy behavior unresolved |
| DNA-S06 | inline excerpt | GIVEN | small local excerpt replaces search preview pane | binary/media fallback unresolved |
| DNA-S07 | plugin indications | GIVEN | neutral tags in a separate information lane | taxonomy and overflow unresolved |
| DNA-S08 | unavailable result | GIVEN | stays visible, factual, and labelled by source availability | action to reconnect/inspect absent |
| DNA-S09 | criteria surface | GIVEN | instrument rack above an ordinary virtual folder with visible derivation and Selection pane | module density and responsive layout need testing |
| DNA-S10 | criteria editing | GIVEN | each module exposes field, operator, value, enable, remove, and local validation; cheap changes are live, expensive changes visibly stage for Apply | cost classification and staged-state grammar unresolved |

### Status, failure, progress, and time

| ID | Object | Status | Direction | Gate/debt |
|---|---|---|---|---|
| DNA-T01 | selection status | GIVEN | count and size live in left status cell | multi-selection phrasing absent |
| DNA-T02 | engine authority status | GIVEN | generation/degraded state lives locally at bottom | state vocabulary needs contract snapshot |
| DNA-T03 | success feedback | CANDIDATE | change affected object; toast only when result would otherwise be invisible | undo model unresolved |
| DNA-T04 | error ownership | GIVEN | material modal for ordinary blocking errors/collisions; persistent operation drawer for complex directory mergers; always name the authoritative failing object/subsystem | modal/drawer specimens and complexity threshold absent |
| DNA-T05 | known progress | GIVEN | phase, exact component, measured amount, optional trace | grammar accepted; daily-operation specimen absent |
| DNA-T06 | unknown progress | GIVEN | one authored activity object plus current stage; no fake percentage | grammar accepted; activity art absent |
| DNA-T07 | completion | GIVEN | explicit settled state may linger briefly | grammar accepted; duration unmeasured |
| DNA-T08 | motion | GIVEN | reactive, direct, local; no navigational choreography | boundary accepted; duration/easing tokens unresolved |
| DNA-T09 | sound | GIVEN | default on; sound state/location changes such as pane collapse, navigation, file-operation state, and setting changes; buttons, menus, hover, and focus alone are silent | assets, volume, mixing, and coalescing unprototyped |
| DNA-T10 | undo | HYPOTHESIS | prefer reversible operation feedback over routine confirmation | filesystem transaction semantics not settled here |

### Color, type, texture, and icon detail

| ID | Object | Status | Direction | Gate/debt |
|---|---|---|---|---|
| DNA-V01 | atmosphere families | GIVEN | twelve relational palettes; Sapphire default | complete state-pair contrast gate pending |
| DNA-V02 | structural family | GIVEN | House Composite zoning | seam coherence still under review |
| DNA-V03 | global light | GIVEN | upper-left light across all physical materials | icon/ribbon consistency audit pending |
| DNA-V04 | graphite | GIVEN | corrected to middle-value textured graphite, not black | texture frequency and contrast unmeasured |
| DNA-V05 | pearl | GIVEN | quiet warm/cool pearl supports ribbon hierarchy | must not read as blank modern flatness |
| DNA-V06 | accent allocation | GIVEN | selection, identity, objects, rare status; not every rail | governing principle; token coverage incomplete |
| DNA-V07 | typeface | GIVEN | Portsmouth Rapids for titles/control chrome; a bundled Tahoma/Calibri-like humanist face for content and explanatory text; bounded bundled per-cluster fallback | body-face selection, production rights, coverage packs, HarfBuzz/FreeType profile remain implementation work |
| DNA-V08 | type hierarchy | CANDIDATE | size changes are modest; weight, alignment, and placement carry hierarchy | scale and localization testing absent |
| DNA-V09 | text case | CANDIDATE | sentence case for explanation; compact title case for commands; uppercase only for tiny structural labels | audit current inconsistencies |
| DNA-V10 | icon perspective | CANDIDATE | consistent mild material perspective for objects; commands may be frontal | House Material drawings absent |
| DNA-V11 | status glyph | CANDIDATE | simple base + specific overlay + text; color is redundant | final status inventory absent |
| DNA-V12 | texture budget | CANDIDATE | texture below 3% local contrast on structural planes, absent behind body text | needs measurement and renderer experiment |

### Adaptation, accessibility, and governance

| ID | Object | Status | Direction | Gate/debt |
|---|---|---|---|---|
| DNA-A01 | keyboard | GIVEN | every action and region reachable; focus path visible and predictable | atlas interactions incomplete |
| DNA-A02 | pointer | GIVEN | click targets may exceed visible hairline seams; drag feedback explicit | hit regions not represented |
| DNA-A03 | touch | CANDIDATE | behavior remains available; optional density adaptation may enlarge targets without changing topology | target platform priority unresolved |
| DNA-A04 | screen reader | GIVEN | object, role, state, value, relationship, and action names are published | GUI.Forms publisher gate closed |
| DNA-A05 | high contrast | GIVEN | host-requested mode yields nearly completely to host-native treatment; local mode reconstructs house semantics with minimized color and no texture dependence | neither mode has a complete specimen |
| DNA-A06 | reduced motion | GIVEN | transitions become immediate/static without losing state | no motion specimen yet |
| DNA-A07 | scale/font size | GIVEN | text and targets scale fully; optical details scale by function; content-aware reflow and priority collapse precede clipping | current fixed metrics fail this audit |
| DNA-A08 | localization/RTL | GIVEN | translate and bidi-isolate text/path segments but retain fixed application geography; File Manager does not offer a mirrored topology | locale, bidi, and path specimens absent |
| DNA-A09 | themes | GIVEN | atmosphere may alter color pairs; construction may alter style; neither changes meaning or authority | boundary accepted; hard-coded colors remain in prototype |
| DNA-A10 | plugin visual boundary | DECIDED | plugins never contribute controls or styles | program boundary; declarative contribution schema unresolved elsewhere |
| DNA-A11 | asset provenance | GIVEN | every external or derived asset is revision-pinned and licensed | gate accepted; final manifest absent |
| DNA-A12 | visual performance | GIVEN | no decorative work delays first useful folder or input feedback | future baseline/workload required |

## Current unprincipled-debt register

These are not hidden behind polished screenshots.

1. **Typography implementation:** Portsmouth Rapids versus bundled body roles
   are GIVEN, but platform mapping, fallback, hinting, shaping, and metrics are
   not demonstrated.
2. **Responsive geometry:** content-aware reflow, priority collapse, and the
   150 × 150 minimum are GIVEN; the atlas still uses fixed metrics and has no
   derived breakpoints.
3. **Command state coverage:** permanent and context-only membership is GIVEN,
   but selection, multi-selection, disabled, and narrow-window states are not
   completely drawn.
4. **Hover dependence:** search expansion currently works best with a pointer;
   keyboard focus, click pinning, and touch disclosure need equal specimens.
5. **Match percentage:** the visual precision may overstate ranking calibration.
6. **Criteria modules:** the instrument-rack direction is GIVEN, but cost,
   add/edit/error/keyboard, live, and staged-Apply states are incomplete.
7. **Collapse actuators:** the proximity-expanding flat grip is GIVEN; hit
   geometry, keyboard timing, and reduced-motion behavior remain unmeasured.
8. **Scrolling:** several mockup regions hide overflow; this is not a legitimate
   scrolling decision.
9. **Participation states:** key, secondary-but-participating, drag-proximate,
   and genuinely deactivated surfaces do not yet form a complete state matrix.
10. **Drag and drop:** conventional host behavior is GIVEN, but no source,
    destination, cross-window, cross-process, cross-volume, or collision state
    is drawn.
11. **Dialogs and errors:** modal ordinary failures and a complex-merge drawer
    are GIVEN, but confirmation, undo, collision, permission, handler, and
    degraded-service specimens remain absent.
12. **Empty/loading:** empty folder, unavailable preview, cold engine, rebuild,
    and slow volume states are absent.
13. **Texture:** graphite texture is CSS-authored by eye, not frequency/contrast
    measured against target renderers and display scales.
14. **Sound production:** the state/location-only vocabulary and default-on
    policy are GIVEN; assets, mixing, coalescing, volume, and silence testing
    remain absent.
15. **Token integrity:** the planning CSS contains hard-coded values that do not
    yet descend cleanly from semantic roles.

Each debt item receives a standalone pane in the DNA laboratory before it can
silently harden into product behavior.

## Decision review order

The laboratory should not review random cosmetic atoms. The next sequence is:

1. layer and role grammar;
2. active/inactive/focus/selection state matrix;
3. responsive topology and pane collapse behavior;
4. command probability and overflow;
5. typography and density;
6. object grid, drag/drop, and details view;
7. search expansion and relevance honesty;
8. criteria construction;
9. failure, progress, and undo;
10. accessibility equivalence;
11. motion and sound;
12. texture, color, and final ornamental refinement.

This sequence prevents late visual polish from legitimizing an unresolved
behavioral foundation.
