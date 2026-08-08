# File Manager Design DNA owner verdicts 007

Status: **GIVEN owner direction for Design DNA 006. These verdicts promote
named visual and interaction rules; implementation mechanisms, numerical
breakpoints other than the stated window minimum, renderer tokens, and service
contracts remain gated elsewhere.**

Date: 2026-08-05.

Companion records:

- [`DESIGN_DNA_006.md`](DESIGN_DNA_006.md)
- [`FRONTEND_DESIGN_BRIEF_001.md`](FRONTEND_DESIGN_BRIEF_001.md)
- [`frontend-concept-atlas.html`](frontend-concept-atlas.html)

## Verdicts

### DDV-007-01 — Participation, activation, and deactivation

**GIVEN:** ordinary secondary File Manager windows are still participating
objects. They may be drag sources or destinations even when they do not own
keyboard focus. Use a modest reduction in title chroma and shadow—between the
former quiet and strong-recession candidates—plus dynamic proximity cues when
a drag approaches. Never make a secondary participating window appear disabled.

“Inactive” is therefore split into three states:

1. **key** — owns keyboard focus;
2. **secondary but participating** — not key, still readable and drop-capable;
3. **deactivated/unavailable board** — genuinely nonparticipating, such as a
   disabled plugin-store board, with a stronger unavailable treatment.

Drag proximity may temporarily restore destination emphasis without transferring
keyboard focus. Exact host activation behavior remains platform-owned.

### DDV-007-02 — Content-aware resize and minimum window

**GIVEN:** initial shrinkage is content-aware: spacing, pane allocation, command
group composition, and preview extent adapt while preserving relationships.
When the shell becomes too narrow or short, regions collapse by declared
priority. The window has a hard minimum of **150 × 150 logical window units** so
an accidental resize cannot create a zero-control or irrecoverable window.

The DPI-to-device-pixel mapping and exact collapse breakpoints remain
implementation candidates. At no size may controls merely shrink below their
usable targets to preserve a screenshot proportion.

### DDV-007-03 — Command epistemics and permanent shelf

**GIVEN:** command placement follows what the command acts upon, not frequency
alone.

- The permanent shelf holds broad current-scope or selected-object operations:
  Move/Copy, Delete, View, Sort, and Properties.
- **New Folder** belongs to the folder/background context menu.
- **Rename** and **Open With** belong to object context menus.
- Menus remain the complete non-contextual vocabulary.

A command is not promoted merely because another file manager exposes it on a
toolbar. Context-only placement is deliberate product behavior, not hidden
overflow.

### DDV-007-04 — Pane collapse actuator

**GIVEN:** use a small, flat mechanical grip on the seam at rest. Pointer
proximity or keyboard focus increases its size, contrast, and depth before
activation. The invisible hit target may be wider than the visible grip.

It must remain discoverable without hover and must expose the same expanded
state to keyboard users. Exact animation and hit geometry remain candidates.

### DDV-007-05 — Typography roles

**GIVEN:** **Portsmouth Rapids** is the narrow humanist face for titles and
control chrome. **GIVEN owner revision:** content, explanatory text, object
labels, properties, search evidence, and editable values use a shipped
Tahoma/Calibri-like humanist body face. Portsmouth Rapids never becomes the
body face. HarfBuzz shapes, FreeType hints/rasterizes, and bounded bundled packs
supply per-cluster fallback; arbitrary host font availability is not product
authority.

Exact body-face selection, font-pack coverage, Portsmouth production rights,
hint profiles, and metric generations remain owned by the GUI.Forms typography
gate.

### DDV-007-06 — Conventional drag and drop

**GIVEN:** drag and drop behaves like the host file manager and operating
system. File Manager adds no theatrical house overlay or novel operation
grammar. Previews are drag sources; tree rows, folder objects, other File
Manager windows, and external programs may be sources or destinations through
ordinary platform transfer conventions.

An explicit window appears only when:

- source and destination are on different volumes; or
- the destination already contains a colliding object.

Same-volume, noncolliding operations remain direct. Cross-process payload and
operation semantics remain platform-adapter work, not a visual invention.

### DDV-007-07 — Error and merge surfaces

**GIVEN:** ordinary blocking errors and file collisions use a material modal
dialog. Complex directory mergers use a persistent operation drawer where
multiple conflicts can be inspected and resolved without serial modal churn.

Both must name the authoritative failing object or subsystem. The threshold
between a single collision and a complex merge remains a workflow candidate.

### DDV-007-08 — Selection pane scrolling

**GIVEN:** the Selection pane uses one ordinary scroll plane. Its preview is
collapsible into a compact identity header so properties can occupy the pane
without introducing competing nested scroll ownership.

### DDV-007-09 — Match percentage

**GIVEN:** retain the percentage. It is a compact disclosure of what the
retrieval system thinks, not advice about what the user should conclude. Pair it
with inspectable evidence and avoid probability language, recommendation
language, or visual treatment that converts a score into authority.

### DDV-007-10 — Criteria instrument rack

**GIVEN:** criteria use an instrument rack. Each predicate is a compact module
with field, operator, value, enabled state, removal, and local validation. The
results remain an ordinary virtual folder with visible source derivation and a
Selection pane.

The former “lens” form is **REJECTED** as the primary editing metaphor.

### DDV-007-11 — Live and staged criteria

**GIVEN:** inexpensive predicate changes update the virtual folder live.
Expensive predicates are visibly staged and require Apply. Cost must be known or
declared; the interface may not silently pause while pretending an expensive
change is live.

### DDV-007-12 — Interaction sound grammar

**GIVEN:** interaction sound defaults **on** and marks changes in state or
location, not raw control activation.

Sound-bearing events include pane collapse/restore, directory change, file
operation state, and setting-value changes. Button presses, menu opening,
hover, focus movement, and other actions that have not themselves changed state
remain silent. All visual meaning remains complete with sound disabled.

Exact sound assets, duration, volume, concurrency, and repeated-event
coalescing remain candidates.

### DDV-007-13 — Two high-contrast authorities

**GIVEN:** when high contrast comes from the host operating system, File Manager
yields almost completely to host-native colors and metrics. When the user
selects File Manager's local high-contrast mode, the house system performs a
semantic reconstruction with minimized color, no texture dependence, and
explicit boundaries, focus, selection, and authority.

### DDV-007-14 — Relationship-derived scaling

**GIVEN:** text and targets scale fully. Hairlines, shadows, texture, and optical
details scale according to their function rather than by uniform multiplication.
Content-aware reflow and priority collapse occur before text or controls become
unusable.

### DDV-007-15 — No mirrored application topology

**GIVEN:** File Manager does not offer a mirrored RTL interface. The tree remains
left, Selection remains right, and application geography remains fixed. Text is
translated and bidirectionally isolated; filesystem path syntax and filename
segments retain their correct logical ordering.

### DDV-007-16 — Default tree root

**GIVEN:** the folder tree opens Home-rooted. Honest Volumes navigation remains
immediately available without synthetic roots or provider aliases.

### DDV-007-17 — Coarse folder-size marks

**GIVEN:** allocated folder size appears only after the indexer knows it. The
physical embossed badge uses compact `M`, `G`, and `T` notation with standard
coarse rounding:

| Allocated size | Permanent object mark |
|---:|---|
| at or below 1 MB | no size; ordinary folder icon; inspect for detail |
| above 1 MB and below 100 MB | nearest 5 MB, such as `35M` |
| 100 MB and below 1000 MB | nearest 25 MB, such as `250M` |
| 1 GB through below 1 TB | nearest 100 MB, compactly expressed around `1G`, `1.1G`, and so on |
| 1 TB and above | nearest 100 GB, compactly expressed in `T`/`G` magnitude |

The badge is a coarse indexed observation, not a falsely live exact quantity.
Exact allocated size belongs to inspection.

**OBSERVED record repair:** earlier planning preserved “indexed folder sizes”
and a badge-versus-text question, but did not preserve the architect's coarse
threshold/inspection rule. This verdict restores that rule explicitly. The
freshness mark remains a candidate; this round restores the thresholds and
rounding vocabulary.

### DDV-007-18 — Recent path count

**SUPERSEDED 2026-08-07:** the path matrix no longer owns manual path editing.
If recent full trails survive behind a separate history affordance, their count
and row form return to **CANDIDATE** status. The first-order edit surface is the
inline editor in the dense chevron breadcrumb cascade; see
[`../OWNER_DIRECTION_2026-08-07.md`](../OWNER_DIRECTION_2026-08-07.md).

### DDV-007-19 — Safe path conveniences

**GIVEN:** manual path entry accepts canonical platform paths plus safe
conveniences such as home expansion, environment variables, quoting, and
escaped spaces. Before navigation, the editor shows the resolved canonical
destination. Ambiguous, missing, or unauthorized expansions remain explicit and
do not navigate silently.

## Remaining mechanism questions

These verdicts deliberately leave the following as **CANDIDATE** work:

1. the exact moderate-recession values and drag-proximity signal for secondary
   participating windows;
2. resize breakpoints and the content-priority order between 150 × 150 and the
   ordinary shell size;
3. keyboard geometry and timing for the proximity-expanding seam actuator;
4. stale/indexed notation for coarse folder-size marks;
5. criteria cost classification and staged-Apply threshold;
6. the directory-merge drawer's batching and conflict-resolution model;
7. interaction-sound assets, mixing, and repetition limits;
8. bundled body-face selection, script-pack coverage, production font rights,
   and the HarfBuzz/FreeType metric/raster profile.
