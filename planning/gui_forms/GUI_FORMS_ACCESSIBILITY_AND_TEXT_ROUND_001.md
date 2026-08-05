# GUI.Forms accessibility and text foundation round 001

Date: 2026-08-05

Status: **owner verdicts recorded; native accessibility publication and owned
text-stack direction are GIVEN; implementation profiles and formal ADR remain
gated**.

This round records what File Manager needs from GUI.Forms before the native
surface is opened. It does not open Frontend 001, freeze an ABI, or make
accessibility metadata a new application data model.

## 0. Owner verdicts — 2026-08-05

The owner accepts each recommended option except where explicitly overridden.
This is not a rule that an unmentioned answer becomes option A; the selected
letter follows whichever option was recommended. Every recommendation in this
particular round happened to be A.

- **GIVEN:** A1A, A2A, A3A, A4A, A5A, and A6A.
- **GIVEN, parsed from the owner's `TB` alongside `T2C`:** T1B, selecting
  HarfBuzz now as the common shaper on every host. If `TB` did not mean `T1B`,
  this one parse must be corrected by the owner.
- **GIVEN:** T2C, selecting FreeType as the common glyph loader, hinter, and
  rasterizer on every host.
- **GIVEN:** T3A, interpreted through FreeType: do not force LCD/subpixel
  rendering globally; map supported host preferences into audited raster
  profiles and use grayscale where target geometry/compositing makes LCD
  assumptions unsafe.
- **GIVEN:** T4B, revised because all product UI fonts are now bundled: target
  exact layout geometry for a pinned font-pack and shaping/raster profile.
- **GIVEN:** T5A and T6A.
- **GIVEN owner revision:** do not use host `system-ui` for product UI text.
  Bundle Portsmouth Rapids for control/title roles and select a bundled
  Tahoma/Calibri-like humanist body face. Font coverage, production Portsmouth
  rights, fallback packs, and exact body-face choice remain gates.

## 1. Outcome of the investigation

The direction has two deliberately different seams:

1. one retained, renderer-free **semantic graph**, published through each host
   operating system's accessibility contract;
2. one renderer-free **positioned glyph-run contract**, backed by HarfBuzz,
   FreeType, and bounded bundled font packs.

This gives assistive technologies the native objects they expect while keeping
font selection, shaping, glyph geometry, hinting, and raster policy under
GUI.Forms control. Host preferences still inform scale, contrast, reduced
motion, focus, and admitted antialias profiles.

The consistency target should therefore be:

- the same typographic roles and hierarchy;
- the same Unicode and cluster correctness;
- stable baselines, line-height rules, and declared layout tolerances;
- the same semantic name, role, value, action, and logical order;
- and pinned, profile-qualified glyph raster results at the final CPU edge.

Exact layout geometry is now a target for one pinned font-pack/shaper/raster
profile. Pixel equality is qualified by scale and antialias profile because LCD
geometry, alpha composition, user accommodation, and the final host compositor
still vary. “Same profile” may be exact; “every display and preference” is not.

## 2. Inputs already fixed or observed

### Owner intent

- **GIVEN GAT-G01:** File Manager supports blind and disabled users through the
  operating system's native accessibility facilities and established best
  practices. Accessibility is part of the ordinary product, not a separate
  visual mode.
- **GIVEN GAT-G02:** File Manager does not adopt a cross-platform special-purpose
  accessibility framework. The exact Linux system-protocol binding policy is
  still a question below.
- **GIVEN GAT-G03, revised:** Portsmouth Rapids is the bundled control/title
  face. A separately selected bundled Tahoma/Calibri-like humanist face serves
  content and explanatory roles. Missing clusters fall back through bounded
  bundled packs without restyling the whole control.
- **GIVEN GAT-G04:** The product is CPU-only. The host compositor may use the
  GPU, but GUI.Forms has no GPU rendering capability or resource model.
- **GIVEN GAT-G05:** Text correctness is staged, but text storage, indices, and
  contracts may not encode Latin-only assumptions.

### Repository evidence

- **OBSERVED:** `HostCapability::accessibility`, `Dirty::accessibility`, and
  semantic dirtiness already exist in the retained kernel.
- **OBSERVED:** the master plan already describes stable semantic identities,
  roles, names, descriptions, values/states, actions, relationships, order,
  bounds, text ranges, and virtual children. VoiceOver, UI Automation, and
  AT-SPI tests remain OPEN.
- **OBSERVED:** stock controls invalidate semantic state, but the production
  semantic-node model and native publishers do not yet exist.
- **OBSERVED:** `TextShaper` and `FontFallbackResolver` already make shaping and
  fallback renderer-neutral services. Unicode 17 grapheme segmentation has
  passed the repository's 766-case UAX #29 corpus.
- **OBSERVED:** the current macOS Skia experiment uses Core Text font discovery,
  `SkFont::Edging::kAntiAlias`, and `drawSimpleText`; it does not provide the
  complete shaping policy.
- **OBSERVED:** the CoreGraphics comparison uses `CTLine`/`CTLineDraw`; the
  Windows proving host uses GDI `TextOutW` with `CLEARTYPE_QUALITY`. These paths
  prove pixels, not a selected cross-platform typography system.
- **OBSERVED:** the renderer comparison found visible metric and antialiasing
  differences and has no complete objective multilingual text-quality corpus.

## 3. Accessibility architecture

### 3.1 One semantic truth, three native publications

```text
retained controls + stable model identities
                 |
                 v
        renderer-free semantic snapshot
 role · name · state · value · actions · relations
 order · focus · bounds · text ranges · virtual children
          /              |                 \
         v               v                  v
 Windows UIA       macOS NSAccessibility   Linux AT-SPI2
```

The platform objects are projections, not the semantic authority. A screen
reader action routes back to the same command or control mutation used by a
keyboard or pointer. A visual repaint does not rebuild semantic identity.

On Windows, a custom window exposes a UI Automation provider. Simple elements
provide properties and control patterns; complex controls such as the folder
tree and virtual file collection publish fragment roots and navigable fragment
children. On macOS, drawn controls without individual `NSView` backing are
represented by `NSAccessibilityElement` objects adopting the appropriate
role-specific protocols and posting native notifications. On Linux/BSD, the
equivalent publication is AT-SPI2's accessible hierarchy and role-specific
Action, Component, Selection, Text, Value, Table, and related interfaces.

This is native support even though the retained graph is portable. Native does
not mean every visual item must become an HWND or NSView.

### 3.2 “Optional metadata” must not mean optional accessibility

The existing GUI.Forms wording permits controls to instantiate without authored
semantic metadata. That remains useful and compatible with GAT-G01 only under
this interpretation:

- every supported stock control has a complete default semantic adapter;
- the control's ordinary properties provide its default role, value, state, and
  actions;
- application metadata may improve names, relationships, help, or domain
  meaning;
- a custom control must either publish a semantic adapter or explicitly declare
  itself presentational before it can pass the File Manager release gate;
- and turning off tooltips, help overlays, inspection, or test automation cannot
  turn off native accessibility publication.

Thus metadata is optional for construction; accessibility correctness is not
optional for shipping File Manager.

### 3.3 File Manager-specific semantic demands

| Visual object | Native semantic obligation |
|---|---|
| Ribbon | Toolbar/group structure, commands with names, shortcut/mnemonic, checked/expanded state, and logical order independent of paint chunks |
| Folder tree | Outline/tree semantics, expandable nodes, selection, stable virtual children, level/set position, and action parity with keyboard commands |
| File content | List/grid/table semantics according to view, stable item identity across sorting and virtualization, selection, column relations, rename/edit ranges, and coarse size badge separated from exact inspection text |
| Breadcrumb and `./` matrix | Navigable path components, current location, expansion state, recent-location groups, editable path range, autocomplete status, and an unambiguous announcement when location changes |
| Preview/properties | Semantic content appropriate to the preview type, property name/value relationships, selectable/copyable text where offered, and no redundant “preview” label merely because a pane is visible |
| Search result toast | Result identity, match percentage as evidence rather than instruction, compact excerpt, expansion state, source/reason metadata, and plugin indications without color-only meaning |
| Criteria virtual folder | Editable criteria relationships, validation state, result collection, and the same preview semantics as a real folder representation |
| Drag and drop | Source selection, available operation, target identity, accepted/rejected state, progress, conflict dialog, and final state change communicated without relying on sound or color alone |

Accessibility reading order follows task logic, not raw draw order. A collapsed
pane leaves the accessible tree when it ceases to participate; a visually
de-emphasized but still usable pane remains fully present. Virtualization may
limit native proxy allocation, never the logical child set or stable identity.

### 3.4 System accommodations are host inputs

Native screen-reader publication is only one accessibility surface. The host
adapter must also report, and the retained system must react to:

- display scale and Windows text-scale independently;
- high-contrast/forced-color and contrast preferences;
- reduced motion and animation policy where the OS exposes it;
- keyboard-only focus, focus visibility, access keys, and full command parity;
- color-vision-safe redundant cues;
- magnification-safe reflow and inspectable bounds;
- and state changes that remain understandable when product sounds are muted.

The existing 150-by-150 minimum window size prevents accidental unusable
geometry; it does not excuse clipping when text scale grows. Content-aware
shrink and priority collapse need a large-text ordering distinct from ordinary
window shrink.

### 3.5 Accessibility gates

1. Headless semantic snapshots are deterministic and keyed by stable IDs.
2. The same action invoked by keyboard, pointer, and assistive technology reaches
   one command path and has one state-transition trace.
3. One-million-item tree/list fixtures expose logical virtual children without
   allocating a native proxy for every item.
4. Focus, selection, expand/collapse, value, live progress, and location-change
   notifications are tested for coalescing and order.
5. Editable text exposes caret, selection, composition, text attributes, and
   character/cluster geometry without splitting graphemes.
6. Primary scenarios are completed with Narrator, VoiceOver, and Orca, as well
   as keyboard alone. Inspectors and automated checks supplement rather than
   replace those runs.
7. High contrast, 225% Windows text scale, platform display scales, reduced
   motion, and muted sound have screenshot plus semantic acceptance fixtures.

## 4. Text, shaping, hinting, and rasterization

### 4.1 What “font hinting” actually covers

The browser mockup collapses several systems behind CSS and the browser's text
stack. GUI.Forms must name them separately:

```text
UTF-8 text
  -> grapheme, paragraph, bidi, script and language analysis
  -> font selection and per-cluster fallback
  -> shaping: characters become ordered glyph IDs + advances + offsets
  -> line layout, wrapping, caret and selection geometry
  -> hinting/grid fitting: outlines are adjusted for the device pixel grid
  -> rasterization: glyph masks, grayscale or LCD/subpixel coverage
  -> color-managed compositing into the CPU window surface
```

ClearType is a Windows LCD/subpixel rasterization family. GDI is an older
drawing and text API. DirectWrite covers font discovery, analysis, shaping,
layout, metrics, and several rendering paths. None of those names alone defines
the whole pipeline.

HarfBuzz, by contrast, is principally a shaper: it turns a buffer with direction,
script, language, font, and features into glyph IDs, cluster mappings,
advances, and offsets. It deliberately hands actual drawing to a raster or
graphics library.

FreeType loads outlines, executes native hints or an auto-hinter, and rasterizes
grayscale or LCD masks. Fontconfig describes Linux font matching and user
rendering preferences such as antialiasing, hinting, hint style, DPI, scale, and
subpixel geometry.

### 4.2 Selected boundary and remaining comparisons

Do not compare “Skia versus Core Text versus ClearType” as if they were peers.
The owner has selected the implementation family at each seam. The remaining
comparisons are validation/reference lanes, not backend selection polls:

| Seam | GIVEN direction | Reference/control | What must still be measured |
|---|---|---|---|
| Shaping | HarfBuzz on all hosts | DirectWrite/Core Text corpus output | glyph/cluster correctness, caret behavior, OpenType coverage, maintenance |
| Font selection/fallback | pinned bundled role/font catalogue | declared missing-pack and last-resort cases | cluster coverage, locale/script packs, update generation, determinism |
| Rasterization | FreeType on all hosts | current Skia/CoreGraphics/GDI/DirectWrite proving outputs | small-size clarity, profile behavior, alpha/transforms, remote display, throughput, dependency weight |
| Layout invariants | exact geometry for pinned font/shaping profile | changed scale, text scale, locale, antialias profile | reflow stability, truncation, localization, cache invalidation |

The selected direction is:

- keep the current renderer-neutral `GlyphRun` contract;
- implement the renderer-neutral shaper service with HarfBuzz;
- load, hint, and rasterize the bounded bundled faces through FreeType;
- use Portsmouth for covered control/title clusters and the selected bundled
  body face for content roles;
- resolve uncovered clusters only through the declared bundled fallback order;
- qualify glyph caches and raster goldens by scale/antialias profile;
- and retain DirectWrite/Core Text as quality/correctness references rather
  than runtime authority.

This gives GUI.Forms substantially more control over geometry and minimal-build
behavior. The claim of a smaller vulnerability surface remains a **HYPOTHESIS**
until exact FreeType/HarfBuzz modules, font tables, fuzz coverage, and dependency
inventories are measured.

### 4.3 Platform accommodation over the owned raster stack

#### Windows

DirectWrite is a correctness and visual reference, not the selected runtime
rasterizer. GUI.Forms maps supported Windows smoothing preference into audited
FreeType grayscale/LCD profiles. LCD output is permitted only where the target
is opaque, axis-aligned, and has known subpixel geometry; transparent or
intermediate surfaces use grayscale. GUI.Forms does not globally force
`CLEARTYPE_QUALITY` or promise DirectWrite-identical pixels.

Windows text scale is separate from display DPI. DirectWrite and GDI do not
natively apply the Windows text-scale preference to custom controls, so the host
must observe the system text-scale change and GUI.Forms must remeasure/reflow.

#### macOS

Core Text remains a shaping/metrics/quality reference and host text-input
integration point. Product UI faces and final glyph masks come from the bundled
pack and FreeType. The macOS adapter supplies display scale and relevant
accommodation signals without silently substituting a host font.

#### Linux/BSD

Fontconfig is not product font-selection authority. Where present it may inform
an audited user antialias/hint/subpixel preference profile; minimal builds use a
documented GUI.Forms default. GUI.Forms does not hardcode RGB subpixels globally:
output can be grayscale, RGB/BGR, rotated, scaled, remote, or composited in ways
that invalidate the assumption.

### 4.4 Cross-platform visual consistency rules

1. Typography is addressed by semantic role: window title, ribbon command,
   control label, body, metadata, editable value, and monospace instrument.
2. Logical size and line height come from role tokens. Pixel size follows host
   scale and accessibility text scale.
3. Layout consumes measured shaped runs. It never estimates by code-point count
   or an ASCII average width.
4. Baselines and line boxes are stable; glyph origins may retain fractional X
   positioning. Do not snap every glyph independently.
5. Borders and icon edges may snap to the device grid independently of text.
6. Fallback is selected per indivisible cluster. Missing characters may not
   disappear, split a grapheme, or silently become a box without a diagnostic.
7. The same pinned font, HarfBuzz version/input, FreeType version/profile, and
   logical scale target exact layout geometry across hosts. Deviations are
   measured failures or explicitly qualified profile differences.
8. Text is never captured from the web mockup as an image reference. The mockup
   expresses hierarchy, density, and material intent, not native pixel truth.
9. Golden screenshots are qualified by OS, renderer profile, scale, and font
   inventory. Semantic and metric goldens carry more cross-platform authority
   than raster equality.

### 4.5 Typography test matrix

The shaper/raster decision is not earned by “the quick brown fox.” The corpus
must include:

- Latin at the actual 8–13 logical-pixel dense UI roles, plus larger titles;
- combining marks, emoji variation sequences, ZWJ emoji, symbols, and missing
  Portsmouth clusters;
- Arabic, Hebrew, Devanagari, Thai, CJK, Hangul, mixed bidi, and mixed fonts;
- filenames with normalization differences, leading dots, extension ambiguity,
  long paths, and truncation/ellipsis;
- caret traversal, selection, hit testing, composition, copy, and accessible
  text ranges over every complex case;
- 1.0, 1.25, 1.5, 2.0, and accessibility text scaling, including monitor moves;
- grayscale and supported LCD modes on opaque and translucent surfaces;
- light, dark/graphite, high-contrast, and selected/unselected states;
- remote-session/headless behavior where available;
- p50/p95/p99 shape, cache, raster, and damaged-region costs;
- and side-by-side blind review at native zoom, never enlarged nearest-neighbor
  screenshots alone.

## 5. Collaboration questions

Answer by ID in any order, for example `A1B, A2A, T1C`. The recommendation is a
working hypothesis, not an implied verdict.

### Accessibility

**A1 — shipping obligation for stock and custom controls — RESOLVED A**

- **A — Recommended:** every stock control has native semantics by default;
  File Manager custom controls must attach or implement a semantic adapter before
  release. Authored labels/help remain optional augmentation. This reconciles
  ordinary Forms construction with a genuinely accessible product.
- **B:** only controls with explicitly authored metadata appear to assistive
  technology. Simpler engine rules, but an omitted label can erase a usable
  control and accessibility will rot.
- **C:** require semantic metadata before any control can instantiate. Strongest
  construction-time enforcement, but hostile to WinForms familiarity,
  prototyping, and purely presentational controls.

**A2 — what “no special library” means on Linux — RESOLVED A**

- **A — Recommended:** target the native AT-SPI2 D-Bus contract and permit a
  thin system binding/generated D-Bus binding; do not adopt GTK or a portable
  accessibility framework. This reduces hand-written protocol risk while
  preserving the native boundary.
- **B:** speak the AT-SPI2 D-Bus XML directly with the existing general D-Bus
  transport only. Fewer accessibility-specific link dependencies, but GUI.Forms
  owns more versioning, caching, and protocol mistakes.
- **C:** link `libatspi` directly as the Linux system API. Conventional and
  typed, but it introduces the AT-SPI client-side library and its GObject
  dependency; whether it is the correct provider-side seam needs a proving
  spike.

**A3 — publisher lifetime — RESOLVED A**

- **A — Recommended:** the semantic graph always exists for supported controls;
  each native publisher creates/cache-proxies on demand when its OS accessibility
  system requests them. No user-facing accessibility off switch.
- **B:** build the semantic graph only when an assistive client is detected.
  Lower idle memory, but risks first-query races and makes inspection/testing a
  different execution path.
- **C:** make accessibility a user preference. Lower work when disabled, but a
  user who needs it can be locked out before reaching the setting.

**A4 — very large virtual folders — RESOLVED A**

- **A — Recommended:** expose the complete logical collection with stable item
  IDs and materialize native proxies lazily; preserve focus/selection across
  recycling and sorting.
- **B:** expose only visible rows plus a count. Cheaper, but screen-reader users
  receive a viewport rather than the folder and cannot navigate equivalently.
- **C:** expose paged artificial groups. Bounds allocations but invents a
  navigation structure sighted users do not encounter.

**A5 — visual order versus task order — RESOLVED A**

- **A — Recommended:** semantic order is an explicit task-order graph, usually
  aligned with visual order but independently reviewable. This handles ribbon,
  matrix expansion, toasts, overlays, and collapsed panes coherently.
- **B:** use retained child order. Simple and deterministic, but paint/layout
  refactors can silently change reading order.
- **C:** derive order from final screen coordinates. Visually intuitive until
  overlays, columns, RTL, popups, and virtual children make geometry ambiguous.

**A6 — system accommodations — RESOLVED A**

- **A — Recommended:** system high contrast, text scale, display scale, reduced
  motion, and focus visibility are authoritative host inputs. Local themes may
  specialize them but cannot suppress them.
- **B:** only display scale and screen readers are initially mandatory; defer
  other accommodations. Faster first slice, but bakes visual assumptions into
  controls and layout.
- **C:** provide a File Manager accessibility theme/preferences panel. More
  local control, but duplicates OS state and violates the ordinary-product
  intent if treated as the primary path.

### Text and rasterization

**T1 — shaping comparison — RESOLVED B**

- **A — Recommended experiment:** compare HarfBuzz-everywhere against
  DirectWrite/Core Text native shaping on one pinned multilingual corpus;
  preserve the service seam until metric, cluster, caret, and maintenance
  evidence selects one.
- **B — SELECTED:** select HarfBuzz now on all platforms. Strong portable
  cluster behavior and one implementation; still validate the exact bundled
  font tables, editor geometry, and OpenType behavior.
- **C:** select native shapers on Windows/macOS and HarfBuzz on Linux now. Best
  platform affinity, but three correctness surfaces and more cross-host metric
  drift.

**T2 — final glyph rasterization — RESOLVED C**

- **A — Recommended experiment:** native/platform-aligned CPU raster edge per
  host behind one glyph-run contract; measure against the portable renderer.
  This respects user font settings and platform character.
- **B:** Skia grayscale everywhere. Smaller policy matrix and visually calmer
  cross-platform output, but it can ignore platform smoothing preferences and
  does not reproduce ClearType/Core Text.
- **C — SELECTED:** FreeType everywhere. Maximum raster control and
  reproducibility for bundled faces; GUI.Forms owns profile, preference,
  packaging, cache, and security integration.

**T3 — Windows ClearType policy — RESOLVED A, mapped to FreeType profiles**

- **A — SELECTED/REINTERPRETED:** do not force it globally. Read supported host
  preference and map it to an audited FreeType LCD/grayscale profile; allow LCD
  only on suitable opaque, axis-aligned targets and use grayscale where
  alpha/transforms/display geometry make subpixel assumptions unsafe.
- **B:** always request ClearType for maximum classic Windows crispness. Strong
  period character, but color fringes and broken translucent composition become
  product behavior.
- **C:** always use grayscale. Predictable compositing and cross-platform
  resemblance, but unnecessarily gives up a user-selected Windows rendering
  mode on suitable surfaces.

**T4 — definition of consistent — RESOLVED B, revised for all-bundled fonts**

- **A — Recommended:** consistent roles, sizes, cluster behavior, baselines,
  line boxes, and tolerance-bounded layout; OS-qualified raster goldens.
- **B — SELECTED/REVISED:** exact layout geometry for the pinned bundled
  control, body, and fallback pack under one declared shaping/raster profile;
  scale and antialias profiles remain explicit qualifiers.
- **C:** pixel-identical text on every display and accommodation setting. Even
  with pinned fonts and rasterization, LCD geometry, scale, contrast, alpha
  composition, and user accommodation make this too broad.

**T5 — large text behavior in the dense shell — RESOLVED A**

- **A — Recommended:** text scale participates in measurement; controls grow or
  reflow, then low-priority ribbon/pane content collapses by a declared
  accessibility-specific priority order. Essential names and actions never
  clip silently.
- **B:** scale text inside fixed controls up to a cap, then ellipsize with full
  accessible names. Preserves geometry but can make the visible product
  difficult for low-vision users.
- **C:** scale the whole window bitmap. Simple and magnifier-like, but sacrifices
  working area, sharpness, and content-aware layout.

**T6 — browser mockup authority — RESOLVED A**

- **A — Recommended:** treat it as hierarchy/material evidence only. Build a
  native typography specimen showing every role, state, scale, script, and host;
  approve those native captures independently.
- **B:** tune native output until it visually matches the browser at one
  reference machine/scale, then allow drift. Gives a concrete starting target
  but can overfit WebKit/Chromium metrics.
- **C:** use browser screenshots as pixel goldens. Deterministic, but tests the
  wrong text engine and undermines the no-bundled-browser architecture.

## 6. Proposed next evidence slice after answers

No File Manager implementation is needed for this slice. GUI.Forms can build a
renderer-neutral “text and semantics bench” containing:

1. a retained folder tree, virtual file list, editable breadcrumb, ribbon
   commands, preview properties, and expandable search result represented only
   by deterministic fixture data;
2. a serializable semantic snapshot and action trace;
3. the multilingual text corpus with recorded glyph IDs, clusters, positions,
   line boxes, carets, and fallback faces;
4. native publisher probes for one custom button, tree, virtual list, editable
   text, value/progress object, and transient result notification;
5. OS/scale/profile-qualified raster captures from the owned HarfBuzz/FreeType
   path plus DirectWrite/Core Text/reference comparison lanes;
6. a review sheet at native scale plus measured shape/raster/damage costs;
7. negative fixtures for missing semantics, invalid ranges, stale virtual IDs,
   inaccessible color-only state, and forced LCD rendering on alpha surfaces.

The slice answers whether the selected implementation families satisfy the
seams. A numbered ADR still records the measured profile, failure modes,
dependency/asset versions, reversal path, and owner approval before ABI freeze.

## 7. Primary references consulted

- Microsoft, [Server-Side UI Automation Provider Implementation](https://learn.microsoft.com/en-us/dotnet/framework/ui-automation/server-side-ui-automation-provider-implementation)
- Microsoft, [Interfaces for UI Automation Providers](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-interfaces)
- Microsoft, [Rendering DirectWrite](https://learn.microsoft.com/en-us/windows/win32/directwrite/rendering-directwrite)
- Microsoft, [Text scaling](https://learn.microsoft.com/en-us/windows/apps/develop/input/text-scaling)
- Microsoft, [Accessibility checklist](https://learn.microsoft.com/en-us/windows/apps/design/accessibility/accessibility-checklist)
- Apple, [Custom Controls accessibility](https://developer.apple.com/documentation/appkit/custom-controls)
- Apple, [NSAccessibilityProtocol](https://developer.apple.com/documentation/AppKit/NSAccessibilityProtocol)
- Apple, [Core Text](https://developer.apple.com/documentation/coretext/)
- GNOME, [Architecture of the accessibility stack](https://gnome.pages.gitlab.gnome.org/at-spi2-core/devel-docs/architecture.html)
- GNOME, [AT-SPI2 API](https://docs.gtk.org/atspi2/)
- HarfBuzz, [Getting started](https://harfbuzz.github.io/getting-started.html)
- HarfBuzz, [Clusters](https://harfbuzz.github.io/clusters.html)
- FreeType, [Glyph retrieval and rendering modes](https://freetype.org/freetype2/docs/reference/ft2-glyph_retrieval.html)
- FreeType, [Subpixel rendering](https://freetype.org/freetype2/docs/reference/ft2-lcd_rendering.html)
- Fontconfig, [`fonts.conf` reference](https://fontconfig.pages.freedesktop.org/fontconfig/fontconfig-user.html)
