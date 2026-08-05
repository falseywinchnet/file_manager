# File Manager frontend design brief 001

Status: **CANDIDATE visual synthesis; no frontend architecture or implementation
permission implied**.

Date: 2026-08-05.

Companion artifact:
[`frontend-concept-atlas.html`](frontend-concept-atlas.html).

## Purpose

Turn the accepted product shape and recorded visual verdicts into a coherent
daily-work surface before Gate F0. This brief does not select a GUI.Forms API,
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
- **GIVEN:** breadcrumbs and an explicit terminal `./` path editor precede a
  separate current-subtree search field.
- **GIVEN:** the shelf is compact, labelled, and task-specific. It is not a
  modern oversized ribbon or an icon-only strip.
- **GIVEN:** selection is single-click and opening is double-click.
- **GIVEN:** the file surface remains useful without the engine or Oracle.
- **GIVEN:** search never silently widens beyond the displayed folder subtree.
- **GIVEN:** Home is the real home directory. Drive-rooted and Home-rooted trees
  are honest structures; Quick Access, Libraries, This PC, Recommended, and
  provider-branded aliases are excluded.

### Authority and failure presentation

- **DECIDED:** filesystem/platform identity is authoritative; displayed paths
  are addresses, not durable identities.
- **DECIDED:** the engine owns catalogue observations and generation-bound query
  results. Its evidence channels remain distinct and inspectable.
- **DECIDED:** Oracle owns handler, command, settings, capability, provider,
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
- **OBSERVED:** the Oracle registry is still paper/importing-v0. Any screen that
  implies finalized plugin, handler, or provider payloads is illustrative only.

### GUI.Forms consumption boundary

- **GIVEN:** GUI.Forms is retained, custom-rendered C++ with disciplined damage,
  synchronous UI-thread state mutation, explicit ownership, and no GPU path.
- **GIVEN:** PNG is the only renderer/resource-core image decoder currently
  admitted. SVG in this HTML is a planning convenience; selected production
  icons would need pinned PNG derivatives and source/license provenance.
- **OBSERVED:** GUI.Forms currently proves basic retained controls, physical
  panel styles, input routing, damage, and experimental ABIs, but lacks the
  production text editor, shaping backend, IME, scrolling, tree/list/menu,
  accessibility publisher, and layout families required by File Manager.
- **GIVEN:** Gate F0 remains closed. No frontend source, build system, generated
  binding, package, or product asset should be created from this work.

## Visual grammar

### House character

**HYPOTHESIS:** the daily surface should feel like a precise instrument housed
inside a restrained illustrated object. The white file field is the instrument;
the blue-violet title fresco identifies the object; shallow physical chrome
explains what can be pressed or edited.

The resulting grammar is:

1. square, unequivocal window and pane geometry;
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

### Color relations

The mockup tests two relational families already admitted by the color record:

- **Harbor:** cool blue working chrome, white work field, blue-violet selection,
  and occasional orange activity/status.
- **Ergonomic:** parchment work field, teal text/focus, cobalt title structure,
  and warm gold as the rare active accent.

Both preserve the same geometry and relief. This tests theme separation from
layout rather than treating a palette swap as a second product.

## Mockup states

### 01 — Folder / small icons

The baseline daily view. It tests the fixed single-window topology, physical
command shelf, recent-trail breadcrumb affordance, honest tree roots, high-color
objects, indexed folder-size badges, embedded preview, and collapsible panes.

### 02 — Search / evidence ledger

The content surface becomes a dense result ledger without opening a separate
page. Scope remains explicit in the path row. Match explanations are short and
channel-labelled. A partial/stale source is visible but not promoted to an
alarm banner. Stable selection is represented independently of row order.

### 03 — Criteria / intentional virtual fanout

An explicit `Type → Year` criteria trail sits inside the current location. It
does not change or group the real hierarchy and does not masquerade as a saved
search. The user can close one criterion at a time and return to the folder.

### 04 — Theme and icon laboratory

The same geometry is shown with an alternate relational palette, followed by
an icon-system shortlist. This is a selection surface, not a claim that icon
licensing or the final house pack is resolved.

## Open icon-system shortlist

All entries are **CANDIDATE** and require a pinned revision plus a per-file
license manifest before product use.

| System | License/status | Visual fit | Main issue |
|---|---|---|---|
| Tango Icon Theme 0.8.90 | public-domain release | strongest restrained, cross-platform baseline; excellent at 16–48 px | small and historically finite catalogue; house extensions would be substantial |
| KDE Oxygen Icons | LGPL-3.0-or-later | strongest ready-made high-color, glossy, material object family | large catalogue and stronger KDE personality; source/relink/notice obligations require care |
| Papirus | GPL-3.0 | broad and maintained; useful freedesktop naming coverage | flatter and more contemporary than the accepted material grammar; copyleft policy needs explicit approval |
| Silk | CC BY 2.5; authoritative package source must be re-pinned | exceptional compact 16 px command/status vocabulary | canonical project site was unavailable during this pass; too small and web-administration-coded for primary file objects |
| House pack over Tango naming | new artwork; license to be selected | exact control over object identity, size quantization, rare types, and cross-platform consistency | highest illustration workload; must not become an unbounded icon-replacement project |

**CANDIDATE recommendation:** use Tango naming and coverage as the semantic
baseline, prototype the daily surface with a small audited Oxygen subset, and
commission a house object pack only for the high-frequency file/folder/drive and
File Manager-specific states. This deliberately remains a proposal until the
architect chooses the visual/licensing tradeoff.

## Questions for the grand architect

These questions are useful but do not block the first visual comparison.

1. **VIS-FE-001:** Which daily baseline is closer: Harbor's cool white precision
   or Ergonomic's parchment/teal warmth?
2. **VIS-FE-002:** Should the title fresco remain visible in every window state,
   or collapse to a nearly plain inactive band?
3. **VIS-FE-003:** Should the first tree open Home-rooted or drive-rooted, while
   retaining the other as a one-click mode?
4. **VIS-FE-004:** Is the 64 px labelled shelf the right everyday density, or
   should labels move beside 20–24 px icons to reduce height further?
5. **VIS-FE-005:** Which actions deserve permanent shelf positions? The mockup
   assumes Back, Up, New Folder, Rename, Move/Copy, Delete, and View.
6. **VIS-FE-006:** Should preview and properties share one scroll plane, or should
   the preview retain a bounded top region while only property groups scroll?
7. **VIS-FE-007:** In small-icon view, should a folder's allocated-size mark be
   a physical badge on the object or a subordinate text line below the name?
8. **VIS-FE-008:** Do search match explanations belong in a dedicated column,
   under the filename, or only in the preview/properties pane?
9. **VIS-FE-009:** Is unavailable/offline status best expressed by a small object
   badge plus factual status text, as shown, or by a stronger full-row treatment?
10. **VIS-FE-010:** Which icon tradeoff should lead the next round: Tango's
    public-domain restraint, Oxygen's material richness, or a house pack that
    borrows only freedesktop naming?
11. **VIS-FE-011:** What production typeface family should replace the temporary
    system-font stack? Portsmouth Rapids remains Gallery evaluation material,
    not a File Manager packaging decision.
12. **VIS-FE-012:** Is a dark theme required for first dogfood, or may the visual
    system first complete light and high-contrast modes without a dark primary
    surface?

## Explicit exclusions preserved

The mockup does not introduce tabs, dual panes, a source-list Home, automatic
grouping, cards, provider-branded navigation, web search, floating preview,
rounded/Mica chrome, skeleton loading, injected plugin controls, or a second
search index. No visual flourish is allowed to outrank startup, memory, input,
or accessibility budgets.
