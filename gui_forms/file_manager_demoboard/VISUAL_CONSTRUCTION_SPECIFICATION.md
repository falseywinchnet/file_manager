# Visual construction specification

Status: **GIVEN default direction with measured/native realization still open**.

This document specifies what the demoboard must build. The copied HTML and
screenshots are visual evidence; this document resolves their known historical
deltas for the native target.

## 1. Character

The interface is future-vaporwave retro from an unplaceable 1998–2007 lineage,
rendered at modern density and precision: “Microsoft Encarta 2000 Ultra HD
Deluxe Premier Edition” rather than a pixel-art skin.

It should feel:

- professional and industrial;
- classically deep rather than flat;
- fast, local, and mechanically legible;
- materially rich in geographically bounded regions;
- calm and white where files themselves need authority;
- crisp at native scale;
- neither dark-theme software nor candy vaporwave;
- neither generic modern web UI nor a literal clone of one historical OS.

## 2. House Composite

Default construction is a composite with explicit geographical ownership:

| Region | Source language | Native interpretation |
|---|---|---|
| title/identity | Watercolor | bounded sapphire-to-coral fresco, glass/specular edge, application/location identity |
| menu/ribbon | Office 2017/2019/2021 restraint + Office Pearl + Watercolor | white pearl command plane, grouped actions, shallow gloss, disciplined labels |
| navigation and pane chassis | Workshop Graphite | middle-value graphite texture, narrow seams, inset white wells |
| folder/search/selection information panes | Studio 2003 | compact captions, warm/neutral information stock, dense factual rows |
| object field | classic Explorer/desktop object field | white paper, colored objects, no cards around ordinary files |
| object/icon depth | classic iOS/material-object lineage | upper-left light, localized gradients, occlusion, restrained shadow, specular rim, size-specific drawing |
| status | technical desktop instrument | shallow pearl rail, separated factual cells, no decorative banner |

Depth explains physical role. Color explains semantic state. Do not use color
merely to fake a raised/inset relationship, and do not use depth as a substitute
for selection/status meaning.

## 3. Reference geometry

### 3.1 Product client

Reference logical client: **1450 × 850**. This excludes host shadow and the
separate controller. The historical captures are 1440×960 and include a review
strip; they establish ratios, not current product-client bounds.

The main grid at reference size:

| Band | Height | Behavior |
|---|---:|---|
| identity/title content | 40 | fixed logical height; host scale and text scale may remeasure |
| ribbon tab/menu line | 23 | fixed baseline at nominal scale |
| command shelf/ribbon | 66 | fixed nominal; priority collapse when narrow |
| navigation | 40 | fixed nominal; reflows before essential controls collapse |
| workspace | 657 | receives remaining height |
| status | 24 | fixed nominal; low-priority cells collapse when narrow |

The sum is 850 logical pixels. At large text, rows grow by measurement and the
workspace yields height.

### 3.2 Workspace columns

Folder and Criteria nominal grid:

| Column | Width | Rule |
|---|---:|---|
| folder tree | 218 | resizable; persisted by stable pane ID |
| left seam | 3 | visible width; hit target is at least 9 logical pixels |
| content | fluid | minimum useful field before priority collapse |
| right seam | 3 | visible width; hit target is at least 9 logical pixels |
| selection pane | 288 | resizable; one scroll plane |

Search grid:

- tree 218;
- seam 3;
- correspondence field consumes remaining width;
- no right seam or selection pane.

DNA board grid:

- no tree or selection pane;
- decision index 278;
- decision body fluid.

### 3.3 Navigation columns

Nominal navigation grid:

```text
Back 34 | Forward 34 | Up 34 | breadcrumb min 280/fluid | search min 240/34%
```

Gap is 5; outer horizontal padding is 7; vertical padding is 5. Each button is
28 high. Address and search wells are 28 high.

The breadcrumb well contains, in this order:

```text
quentin › Work › Projects › [open tail, flexible] [./ terminal, 30 minimum]
```

`./` is always at the end of the breadcrumb well, never at its beginning and
never inside the separate search well.

### 3.4 Object field

Nominal small-icon field:

- grid columns: auto-fill, minimum 86 logical pixels, equal fractional growth;
- row height: 78;
- column gap: 2;
- row gap: 4;
- padding: 10 top, 11 horizontal, 18 bottom;
- object icon: 42×42;
- name: 10 logical-pixel body role, 13 line-height, maximum two lines;
- object hit/selection rectangle fills the cell minus 3-pixel padding;
- selection: one colored boundary, tinted fill, and independent dotted focus
  indication when focused;
- ordinary items have no cards, drop shadows around cells, or background tiles.

Coarse size badge:

- only on eligible folder objects;
- anchored over lower-right icon region, not beside the filename;
- nominal 8-pixel metric role;
- compact embossed capsule, minimal color;
- exact size is available through inspection/properties;
- the badge is not a command, rank, or warning.

### 3.5 Result correspondence rows

Search rows are Outlook-like correspondence objects, not table rows and not
floating notification toasts.

Compact state:

- 45 logical pixels total height;
- 5 bottom gap;
- 1-pixel faint outline;
- 4-pixel subdued source/evidence rail;
- compact grid: 34 icon, fluid title, at least 82 factual hint, at least 56
  match percentage;
- icon 30×30;
- title 11; secondary line 9; percentage 16 metric role;
- no right preview pane.

Expanded state:

- 126 logical pixels total;
- expansion adds an 81-pixel body;
- left body begins after a 52-pixel icon/title alignment offset;
- body grid is fluid evidence plus 170-pixel plugin-information lane;
- metadata is a single factual string, 9-pixel metric role, ellipsized only when
  necessary;
- excerpt is 43 high, inset, selectable where supported, with matched
  substrings marked by restrained background emphasis;
- plugin lane uses a quiet separating rule and may contain restrained plugin
  indication tags;
- one row may be pinned; hover/focus can temporarily expand another only if
  variable-height anchoring remains stable.

### 3.6 Criteria rack

The current target is a horizontal predicate rack, not the historical
left-facet draft.

- console minimum height: 104;
- outer padding: 7 top, 9 horizontal, 8 bottom;
- title row: 9-pixel metadata with 10-pixel location name;
- modules form one horizontal rack and wrap/collapse by authored priority when
  needed;
- ordinary module: 210 wide, at least 66 high;
- module columns: 20 enable, fluid fields, 22 remove;
- field grid: `property | operator | value`, then a full-width state line;
- field gaps: 3; field inset: 4 vertical, 5 horizontal;
- staged expensive module uses a restrained violet material difference plus
  explicit `staged · expensive · not applied` text;
- action module uses remaining width and contains `+ module` and `Apply 1`;
- result field below is the same virtual object-field grammar as Folder;
- selection pane remains present.

The criteria rack must not look like a consumer-web chip collection. Modules
are compact instruments with explicit enablement, fields, cost/application
state, and remove action.

### 3.7 Selection pane

- caption is `Selection`, not `Preview` or `Preview & Properties`;
- nominal width 288;
- caption 27 high;
- content uses one owning vertical scroll plane;
- object name appears first, 12-pixel bold body role;
- preview frame nominal 174 high with one dark image surround;
- preview can collapse independently to a compact property-only pane;
- property groups use one separator and an optional disclosure arrow;
- property grid columns: 76 and fluid; gap 7; row gap 4;
- editable value is inset and participates in ordinary editor focus/validation;
- the entire pane can collapse through the thin seam;
- copying/dragging the preview is a real source behavior when its GUI.Forms
  dependency is available.

Do not label the pane `Preview`; the selected object already supplies context.

### 3.8 Path matrix overlay

Reference placement at full size:

- anchored below the navigation/breadcrumb region;
- prototype reference: top 169, left 109 within the product client;
- preferred width 860, bounded to available content width;
- overlays the workspace without resizing it;
- one dark crisp boundary, bounded shadow, pearl heading/current section;
- recent stacks are calm rows, not cards.

Internal geometry:

- heading at least 34 high;
- current block padding 8;
- current stack at least 33 high;
- breadcrumb node padding 9 left / 16 right;
- open tail minimum 54;
- editor at least 33 high with dark terminal material;
- completion rows at least 25;
- recent rows at least 31;
- exactly five recent stacks in the default fixture.

Common roots repeat in each recent stack by design. Do not compress them into a
single tree or a shared root heading.

## 4. Default Sapphire atmosphere

Use relational pairs, not isolated hard-coded control colors. Nominal sRGB
reference values:

| Token | Value |
|---|---|
| chrome highlight | `#fafdff` |
| chrome | `#e7edf6` |
| chrome middle | `#c9d4e5` |
| chrome low | `#aebbd1` |
| ordinary line | `#778cab` |
| dark line | `#3d5f8d` |
| ink | `#1d2d4b` |
| muted ink | `#63708a` |
| paper | `#ffffff` |
| soft paper | `#f3f6fb` |
| selection background | `#dbe6ff` |
| selection line | `#436fbd` |
| focus | `#294f91` |
| accent | `#3e61a3` |
| bright signal | `#df6b65` |
| success | `#4a795d` |
| title start | `#17347f` |
| title middle | `#3a68cb` |
| title endpoint | `#d96872` |
| title cyan glow | `#92d9ff88` |
| title violet glow | `#d6b2ff88` |
| title apricot glow | `#ffc38e88` |
| title edge | `#172d69` |
| title shadow | `#13275b` |

Every foreground/background pair must meet the product's contrast gate in its
actual state. High contrast replaces decorative material as necessary; it does
not merely intensify the same gradients.

## 5. Twelve atmosphere families

Geometry and construction remain constant while atmosphere changes. The table
lists title start/middle/end, paper, middle chrome, selection, and accent:

| ID | Family | Title triplet | Paper | Chrome middle | Selection | Accent |
|---:|---|---|---|---|---|---|
| 01 | Encarta Cobalt | `#07558d #2784b8 #65509a` | `#fff` | `#cedde8` | `#d7eaff` | `#356b9a` |
| 02 | Amethyst CRT | `#25195f #7647b5 #c64b9e` | `#fffefe` | `#d5c8e4` | `#eadcff` | `#70439a` |
| 03 | Miami Ledger | `#075965 #18a9ac #d75f9c` | `#fff` | `#c3dcda` | `#cff5ef` | `#1f7d80` |
| 04 | Orchid Relay | `#4b1e59 #98508e #dc6f83` | `#fffdfd` | `#dbc7d7` | `#f4dce9` | `#87436f` |
| 05 | Aqua Memory | `#075679 #1599ac #4b88c8` | `#fff` | `#c5dde2` | `#d3f2f4` | `#267884` |
| 06 | Apricot Modem | `#5b2c4c #be5d65 #e99a68` | `#fffefa` | `#dfcfbe` | `#f7e2d2` | `#8d4e60` |
| 07 | Mulberry Glass | `#2d1745 #78366f #9f5db9` | `#fffefe` | `#d4c3d6` | `#ecd9ef` | `#7b3d78` |
| 08 | Viridian Laser | `#075552 #26956e #4f6ec0` | `#fff` | `#c7dbd1` | `#d6efe3` | `#2d7963` |
| 09 | Sapphire Dusk | `#17347f #3a68cb #d96872` | `#fff` | `#c9d4e5` | `#dbe6ff` | `#3e61a3` |
| 10 | Rose Quartz | `#682143 #c4527e #6152aa` | `#fffdfd` | `#dbc6d1` | `#f4dce8` | `#934765` |
| 11 | Electric Iris | `#31206f #7042c3 #22aec5` | `#fff` | `#d0c9e1` | `#e5ddff` | `#6546a0` |
| 12 | Phosphor Violet | `#2b4247 #4c9571 #76509b` | `#fff` | `#cbd5d2` | `#dcebe3` | `#487662` |

Do not complete alternate atmospheres before Sapphire passes contrast, state,
and resource gates.

## 6. Construction families

These are review candidates independent from atmosphere:

1. Watercolor Instrument
2. House Composite — default
3. Studio 2003
4. Office Pearl
5. Aqua Technical
6. Workshop Graphite
7. Machined Workstation
8. QNX Precision
9. Object Desk
10. MSN Jewel
11. Delphi Laboratory
12. Sky Pearl

Only House Composite is required for primary completion. Every alternate family
must preserve identical control identity, commands, content state, semantic
state, selection, and focus. A construction switch replaces material recipes,
not the interface ontology.

## 7. House material recipes

### Title

- horizontal sapphire-to-coral gradient;
- three bounded radial glows: cyan near upper-left, violet near the middle/lower
  field, apricot near the endpoint;
- one bright lower specular keyline;
- crisp dark outer boundary;
- white title text with restrained one-pixel shadow;
- app mark in a small glass/plastic material well;
- no animated aurora.

### Ribbon

- pearl white-to-cool-chrome plane;
- subtle colored two-pixel identity filament along its top edge;
- groups separated by a single darker rule plus one light relief edge;
- ordinary command backgrounds transparent;
- hover creates a shallow cool raised material;
- press insets locally by roughly one logical pixel without moving surrounding
  content;
- group captions are small and quiet beneath controls;
- no dark ribbon background.

### Graphite chassis

- middle values around the `#4b5459` family, not near-black;
- directional texture amplitude below 3%; reference uses sparse 1-pixel marks
  over a 3–4-pixel period;
- texture only behind navigation/seams/structural chassis, never behind body
  copy or object names;
- three-pixel visible seams;
- seam hit targets expand invisibly;
- collapse tabs are small/flatter at rest and grow in width/height/opacity as
  pointer or keyboard focus approaches.

### Information panes

- light neutral/blue-gray stock;
- 27-pixel compact caption;
- factual content hierarchy, not card stacks;
- one line or change in material per true grouping boundary;
- warm paper may be used sparingly for explanation/audit regions;
- nested decorative borders are forbidden.

### Object field

- white/near-white paper;
- no structural texture;
- file color and selection carry identity;
- sparse fields remain grounded by pane boundary, alignment, and status—not
  cards or wallpaper.

## 8. Typography

Roles, not arbitrary font family calls:

| Role | Face direction | Nominal use |
|---|---|---|
| title/control | bundled Portsmouth Rapids | window identity, ribbon labels, compact control chrome |
| body/object | selected bundled Tahoma/Calibri-like humanist face | filenames, tree, properties, correspondence titles |
| metadata | bundled body face at smaller size | status, explanations, evidence metadata |
| metric | bundled body face with tabular figures if available | sizes, percentages, generations |
| terminal/path | admitted bundled monospace role | path editor, completions, factual excerpt where intentionally terminal-like |

The body-face choice remains gated. The HTML begins with Carlito only as a
specimen candidate. Native acceptance cannot silently approve a different host
font.

Nominal logical sizes from the reference range from 8 to 13 for dense controls,
with 16 for match percentages and 17 for DNA headings. Actual metrics come from
the selected bundled font pack. Never estimate widths by CSS or code-point
count.

Large text remeasures controls. Essential names and actions do not clip. Low
priority labels collapse only after reflow/growth according to the responsive
order below.

## 9. Icons and preview resources

- runtime format: PNG only;
- command icons: nominal 28×28 on the reference ribbon, with explicit smaller
  derivatives for menus/status;
- object icons: nominal 42×42 rendered from a size-specific 48-pixel source;
- tree icons: 17×17 from a hand-tuned small derivative;
- status icons: 14×14;
- navigation icons: 18×18;
- search row icons: 30×30;
- preserve upper-left lighting and controlled cast shadow;
- do not scale one large raster down to every size;
- do not use font icons or runtime SVG;
- every file records source, revision, license, derivative recipe, dimensions,
  and content hash.

The direction is Fluent Color semantics for commands/mid-size coverage plus
deeper original House Material objects where necessary. Fluent Emoji 3D is a
volume/material benchmark, not a license to ship playful emoji geometry.

The facade preview is a deterministic bundled fixture image, not decoded from a
user file. Search excerpts are text inside correspondence rows, not a separate
preview pane.

## 10. Visual states

Every interactive object must cover where applicable:

- normal;
- hot/proximate;
- pressed;
- keyboard focused;
- selected;
- selected + focused;
- default action;
- checked;
- expanded;
- disabled;
- unavailable;
- drop target/accepted;
- drop target/rejected;
- staged/pending;
- validation error;
- secondary-participating window;
- drag-proximate secondary window;
- genuinely deactivated board.

Inactive is not synonymous with unavailable. A non-key product window remains
readable and usable as a cross-window drag source/destination. Strong recession
is reserved for genuinely deactivated review/plugin boards.

State must remain locally evident without relying solely on title-bar tint,
color, motion, sound, or hover.

## 11. Responsive collapse

The user cannot resize below 150×150 logical pixels. Between reference width and
the minimum, the shell uses content-aware shrink then authored collapse:

1. reduce flexible whitespace and breadcrumb tail;
2. compact status copy while preserving local authority and one view-mode
   actuator;
3. shorten low-priority ribbon group captions;
4. collapse low-priority ribbon commands into group overflow, preserving
   commands in menus;
5. collapse selection pane to its seam tab;
6. collapse tree to its seam tab;
7. reduce breadcrumb to current tail plus `./`, keeping Back and Up;
8. reduce search scope caption, never the editable field itself;
9. switch correspondence/criteria layouts to vertical internal stacks;
10. at extreme size present one usable focused region plus explicit pane/menu
    access, never a crushed miniature of all panes.

Large-text collapse has its own priority ordering:

1. grow/reflow text-bearing controls;
2. wrap command groups where admitted;
3. replace nonessential visible labels with accessible icon commands only when
   names remain available to assistive technology and tooltip/focus help;
4. collapse secondary panes;
5. preserve current location, primary object field, and complete menu command
   vocabulary.

The exact breakpoints are measured from content. Do not hard-code browser media
query widths as architectural truth.

## 12. Motion and sound presentation

Motion is local, reactive, interruptible, and brief:

- seam-tab proximity: about 90 ms reference;
- correspondence expansion: about 90 ms reference;
- pane disclosure may settle over a short bounded transition if interruption
  and reduced-motion replacement are correct;
- navigation changes state immediately; motion never delays destination;
- no perpetual animation or idle frame loop;
- reduced motion substitutes an immediate state change and preserves all
  information.

Sounds default on but are emitted only for meaningful state/location changes:

- location changed;
- pane collapsed/expanded;
- option committed;
- transfer started/completed/failed;
- destructive fake action committed;
- blocking conflict appeared/resolved.

No sound for button press, menu opening, hover, pointer focus, or ordinary
selection. With sound off the visual and semantic result remains complete.

## 13. Visual prohibitions

Reject:

- black structural bars;
- wide separators between panes;
- card containers around normal files;
- redundant content summary bars;
- a `Preview` pane title;
- a search-side preview pane;
- colorful search-criterion tag soup;
- flat modern web buttons with no physical role;
- glass/gradient material applied uniformly everywhere;
- dark body fields;
- mobile topology or oversized rounded cards;
- retro pixelation;
- arbitrary host font substitution;
- one raster icon scaled to every size;
- and decoration that outranks selection, focus, failure, or object identity.
