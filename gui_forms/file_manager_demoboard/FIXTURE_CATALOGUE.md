# Deterministic fixture catalogue

Status: **GIVEN demoboard data; not product truth, filesystem evidence, search
quality evidence, or plugin contract**.

All fixtures compile into the demoboard or a bounded data-only demoboard pack.
The runtime never enumerates the host filesystem. Paths are intentionally
macOS-shaped because the visual reference was authored that way; portable host
tests may project volume labels without teaching GUI.Forms that these path
semantics are universal.

## 1. Catalogue identity

```text
catalogue_id: file-manager-demoboard-001
catalogue_generation: 86
clock_epoch: 2026-08-05T09:14:00-05:00
home_display_name: quentin
home_path: /Users/quentin
projects_path: /Users/quentin/Work/Projects
```

Timestamps are rendered relative to the injected clock only.

## 2. Volumes and roots

| ID | Display name | Fixture path | State | Factual metadata |
|---|---|---|---|---|
| `vol-macintosh-hd` | Macintosh HD | `/` | online/current | `412 GB` summary label |
| `vol-archive-04` | Archive 04 | `/Volumes/Archive 04` | offline in default mixed fixture | stored catalogue record available |

The offline volume remains visible and navigable as a stored record where the
scenario permits. It never becomes a real mount check.

## 3. Folder tree

Default visible hierarchy:

```text
Local
└── quentin                         node-home-quentin       expanded
    ├── Desktop                     node-desktop            collapsed
    ├── Documents                   node-documents          collapsed
    ├── Work                        node-work               expanded
    │   ├── Projects                node-projects           selected, expanded
    │   ├── Reference               node-reference
    │   └── Field Notes             node-field-notes
    ├── Pictures                    node-pictures           collapsed
    └── Downloads                   node-downloads          collapsed
Volumes
├── Macintosh HD                   node-volume-primary     collapsed
└── Archive 04                     node-volume-archive     collapsed, offline
```

Additional children used by path/search/navigation fixtures:

```text
Projects/
  Orchard Study/
    Billing/
    Schedules/
  North Shore/
    Final/
  Print Masters/
Documents/
  Legal/
    2026/
Pictures/
  Scans/
Reference/
  Materials/
Archive 04/
  Reference/
    Material Library/
  2025/
    Templates/
```

Every node has stable ID, parent ID, display name, fixture path, volume ID,
child-presence state, availability, icon role, and optional stored-record state.

## 4. Projects folder objects

Default sort is display name ascending for the product model even if reference
placement is authored for composition. The exact reference sequence is retained
as `reference_order` for visual-golden setup.

| ID | Name | Kind | Size | Icon role | Extra |
|---|---|---|---:|---|---|
| `obj-file-manager` | File Manager | folder | 1.27 GB allocated | folder-blue | 284 objects; indexed 09:12; badge `1.3G` |
| `obj-orchard-study` | Orchard Study | folder | 250 MB allocated | folder-green | badge `250M` |
| `obj-field-recordings` | Field Recordings | folder | 3.6 GB allocated | folder | badge `3.6G` |
| `obj-north-shore` | North Shore | folder | 725 MB allocated | folder | badge `725M` |
| `obj-print-masters` | Print Masters | folder | 90 MB allocated | folder | badge `90M` |
| `obj-facade-study` | Facade Study.png | PNG image | 2.8 MB | image | default selection; 1600×1000; Display P3 |
| `obj-material-notes` | Material notes.txt | plain text | 18 KB | document | UTF-8 fixture |
| `obj-palette-study` | palette-study.css | stylesheet | 31 KB | code | UTF-8 fixture |
| `obj-survey-plates` | Survey plates.pdf | PDF document | 6.4 MB | pdf | no actual PDF decoding |
| `obj-room-tone` | Room tone 03.aiff | audio | 48 MB | music | no audio-file decoding |
| `obj-delivery-archive` | Delivery 2026-07.zip | ZIP archive | 12.7 MB | archive | no archive decoding |
| `obj-invoice-quartz` | invoice quartz.txt | plain text | 42 KB | document | search rank 1 source |
| `obj-map-detail` | Map detail 17.tif | TIFF image | 14.2 MB | image | preview fixture may be unavailable |
| `obj-index-fixture` | index fixture.json | JSON document | 9 KB | code | fixture-description object |

`reference_order` is the table order above. Tests use it for the default
1450×850 capture, then use explicit sort commands to prove stable identity.

## 5. Object inspection and properties

### File Manager folder inspection

```text
Folder · 284 objects · 1.27 GB allocated · indexed 09:12
```

This is hover/focus inspection, not a property card in the field.

### Facade Study.png

| Group | Property ID | Name | Value | Editable |
|---|---|---|---|---|
| Identity | `prop-kind` | Kind | PNG image | no |
| Identity | `prop-location` | Location | `~/Work/Projects` | no |
| Identity | `prop-size` | Size | 2.8 MB on disk | no |
| Image | `prop-dimensions` | Dimensions | 1600 × 1000 | no |
| Image | `prop-profile` | Profile | Display P3 | no |
| Image | `prop-created` | Created | August 2, 2026 | no |
| Editable | `prop-name` | Name | Facade Study.png | yes, session only |
| Editable | `prop-handler` | Opens with | Preview | yes, session only |

Preview description:

```text
Architectural facade study: a symmetrical pale building with two blue window
groups, green ground plane, warm horizon rule, blue sky, and a small sun at the
upper right.
```

The eventual runtime preview resource is a pinned PNG derivative of the
reference specimen. It does not decode a filesystem PNG.

## 6. Fake handlers

| ID | Display name | Availability |
|---|---|---|
| `handler-preview` | Preview | available/default for image fixture |
| `handler-image-lab` | Image Laboratory | available fixture |
| `handler-text-edit` | TextEdit | available for text fixture |
| `handler-none` | No assigned handler | explicit unavailable choice for tests |

No host applications are enumerated or launched.

## 7. Search fixture

Query:

```text
invoice quartz
```

Scope:

```text
Projects and descendants
```

Result generation: 86. Seven logical results exist; the reference viewport
shows five without requiring every row to be realized.

### Results

| ID | Object/display | Match | State | Why matched |
|---|---|---:|---|---|
| `result-invoice-text` | invoice quartz.txt | 98% | current, default pinned | exact filename “invoice quartz”; current catalogue record |
| `result-invoice-pdf` | Invoice 0428.pdf | 91% | current | filename “invoice”; extracted text “quartz” |
| `result-quartz-image` | quartz-countertop-final.png | 82% | current | filename substring “quartz”; no content claim |
| `result-correspondence` | Client correspondence.rtf | 76% | current | extracted text contains “invoice” and “quartz” |
| `result-pages-offline` | Invoice quartz sample.pages | 61% | source unavailable/stored | coarse stored record; volume offline; filename evidence only |
| `result-stone-schedule` | Stone schedule.csv | 58% | current | row text mentions quartz; related invoice identifier |
| `result-quartz-order` | Quartz order.msg | 54% | stale metadata | stored message subject contains quartz; invoice relation from stale extractor record |

### Rank 1 expanded evidence

Metadata:

```text
Projects / invoice quartz.txt · exact filename “invoice quartz” · current
catalogue record
```

Excerpt:

```text
Received the revised invoice for the North Shore quartz samples. Reference
QZ-0428…
```

Plugin information:

```text
Local index
Text extractor
Generation 86 · direct filesystem identity
```

### Rank 2

Location: `Projects / Orchard Study / Billing`

Excerpt:

```text
Material schedule: honed white quartz. See approved invoice 0428 for quantities…
```

Plugin facts: `Local index`, `PDF text`, `Evidence is inspectable; rank is
advisory.`

### Rank 3

Location: `Projects / North Shore / Final`

Excerpt:

```text
Raster preview: pale stone counter surface, final presentation crop,
3840 × 2160.
```

Plugin facts: `Local index`, `Image metadata`, `No semantic provider
contributed.`

### Rank 4

Location: `Projects / North Shore`

Excerpt:

```text
Could you confirm the quartz sample before we approve the final invoice?
```

Plugin facts: `Local index`, `RTF extractor`.

### Offline result

Location: `Archive 04 / 2025 / Templates`

Excerpt:

```text
Excerpt unavailable. Match derives from the stored filename only.
```

Plugin facts: `Local index`, `Stale evidence retained and labelled.`

Search percentages and evidence are fixtures. They do not measure or validate
the Engine's ranking.

## 8. Criteria virtual folder

Identity:

```text
virtual-folder-project-images-2026
Projects
31 objects from Projects and descendants
```

### Default modules

| ID | Enabled | Property | Operator | Value | Cost/application |
|---|---|---|---|---|---|
| `criteria-kind-images` | yes | Kind | is | Images | live · inexpensive |
| `criteria-modified-2026` | yes | Modified | during | 2026 | live · inexpensive |
| `criteria-content-facade` | yes | Content | resembles | Facade | staged · expensive · not applied |

### Result objects visible in reference

- `obj-facade-study` — Facade Study.png, selected;
- `obj-map-detail` — Map detail 17.tif;
- `criteria-obj-quartz-elevation` — Quartz elevation.png;
- `criteria-obj-orchard-plate` — Orchard plate 03.tif;
- `criteria-obj-north-shore-18` — North shore 18.png.

The remaining 26 logical objects use deterministic IDs and generated fixture
names stored in the catalogue. They are not created from the host filesystem.

### Additional add-module menu fixtures

- Name contains `study` — live/inexpensive;
- Size is greater than `10 MB` — live/inexpensive;
- Source volume is `Archive 04` — live/inexpensive, source offline;
- Content resembles `Quartz` — staged/expensive;
- Plugin indication is `Approved` — staged/producer-dependent.

Changing modules selects predeclared result-set fixture generations. It does not
evaluate arbitrary predicates or similarity.

## 9. Path matrix fixture

Current stack:

```text
Macintosh HD > Users > quentin > Work > Projects
```

Current path:

```text
/Users/quentin/Work/Projects
```

Recent stacks, newest first:

1. `Macintosh HD > Users > quentin > Work > Projects > Orchard Study`
2. `Macintosh HD > Users > quentin > Documents > Legal > 2026`
3. `Archive 04 > Reference > Material Library`
4. `Macintosh HD > Users > quentin > Pictures > Scans`
5. `Macintosh HD > Users > quentin > Work > Reference > Materials`

The third recent path is allowed to navigate to a stored/offline fixture state
that clearly reports unavailable volume authority.

Variables:

| Input | Resolution |
|---|---|
| `~` | `/Users/quentin` |
| `$HOME`, `${HOME}` | `/Users/quentin` |
| `$PROJECTS`, `${PROJECTS}` | `/Users/quentin/Work/Projects` |

Invalid examples:

- `/Users/quentin/Work/Missing` — no fixture destination;
- `$UNKNOWN/Projects` — unknown variable;
- an unpaired surrogate/malformed input through the C ABI test — rejected before
  model mutation.

## 10. Status/failure conditions

Controller condition choices:

| ID | Visible outcome |
|---|---|
| `condition-normal` | default current catalogue/navigation |
| `condition-offline-volume` | Archive 04 offline; stored result/path remain labelled |
| `condition-stale-index` | generation 85 stale label; no claim of current content |
| `condition-preview-unavailable` | preview region shows factual unavailable state; properties remain |
| `condition-empty-folder` | calm empty object field with location intact |
| `condition-million-items` | virtual collection count 1,000,000; bounded realization; generated fixture identities |
| `condition-transfer-running` | status plus operation/drawer fixture progresses on fake clock |
| `condition-collision` | deterministic conflict task dialog |
| `condition-merger` | hierarchical drawer with child operations |
| `condition-capability-missing` | selected public GUI.Forms capability labelled unavailable |

## 11. Operation fixtures

### Same-volume copy

```text
operation_id: op-copy-facade-to-orchard
source: obj-facade-study
destination: node-orchard-study
negotiated_action: copy
duration: 450 ms fake clock
conflict: none
dialog: none
```

### Cross-volume copy

```text
operation_id: op-copy-delivery-to-archive
source: obj-delivery-archive
destination: node-volume-archive/reference
volume_state: controller may make online for scenario
duration: 4.0 s fake clock
dialog: cross-volume task dialog before start
```

### Collision

```text
operation_id: op-copy-facade-collision
source: obj-facade-study
destination: node-orchard-study
existing_name: Facade Study.png
choices: Replace | Keep both | Skip | Cancel
```

### Merger

Parent: `op-merge-project-delivery`.

Children:

- `op-child-images` — 14/31, running;
- `op-child-documents` — 8/8, complete;
- `op-child-audio` — 0/4, paused;
- `op-child-conflict` — `Survey plates.pdf`, needs decision.

All outcomes are session records. Reset restores catalogue state.

## 12. Sounds

Semantic cue IDs only; actual resource mapping belongs to admitted GUI.Forms
presentation/resource policy.

| Cue ID | Fixture events |
|---|---|
| `cue.location.changed` | successful navigation |
| `cue.pane.changed` | pane or preview disclosure committed |
| `cue.option.committed` | view/sort/theme/handler/criterion commit |
| `cue.operation.started` | fake transfer/expensive apply start |
| `cue.operation.completed` | fake operation completion |
| `cue.operation.failed` | fixture failure |
| `cue.conflict.presented` | blocking collision/merger decision appears |
| `cue.destructive.committed` | fake delete/replace committed |

No cue exists for generic button, menu, hover, focus, or ordinary selection.

## 13. Review board catalogue

### Atmospheres

`cobalt`, `amethyst`, `miami`, `orchid`, `aqua`, `apricot`, `mulberry`,
`viridian`, `sapphire`, `rose`, `iris`, `phosphor`.

### Constructions

`watercolor`, `house`, `studio`, `office`, `aqua-tech`, `workshop`,
`machined`, `qnx`, `object-desk`, `msn-jewel`, `delphi`, `sky-pearl`.

### Icon-study candidates

- Fluent Color — MIT; closest usable command/mid-size base;
- Fluent Emoji 3D — MIT; volume/material benchmark;
- Meteocons Fill — MIT; lighting technique reference only;
- AppIcon Forge — MIT tool; construction study, no supplied product artwork;
- House Material 48 — original target extension; license/provenance gate.

Candidate rows are planning facts. The demoboard must not bundle a candidate
asset merely because its name appears here.

## 14. Million-item fixture

Logical items are generated by stable arithmetic identity without one million
heap-owned controls:

```text
id: large-item-%07u
name: Sample Object %07u.dat
kind: deterministic cycle over folder/document/image/archive
size: deterministic bounded function of item index
```

The generation formula is test data, not a product filesystem or indexing model.
Only realized rows/items request display snapshots.
