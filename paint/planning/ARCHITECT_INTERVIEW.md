# Paint architect interview

Status: **questions prepared; no answers inferred**.

The sibling asks these in two conversational rounds, with tradeoffs. It may
reorder or combine them to follow the architect's answers but may not replace
them with a generic product questionnaire.

## Round A — identity and document truth

### PA-Q01 — What survives a save?

**Recommended candidate:** a native Paint document can retain a shallow set of
placed composite objects, while ordinary PNG export flattens them. This permits
move/reuse without exposing a layers product.

- Gain: honest editable composites and reusable clipart.
- Lose: native format, migration, selection/object semantics and more memory.
- Alternative: every placement bakes immediately; radically simple, but drag/
  clipart composition becomes destructive.
- Alternative: PNG-only plus transient floating selection until the next tool;
  historically familiar, but cannot preserve composition across reopen.

Ask which behavior is desired, what “object” means, and when it becomes pixels.

### PA-Q02 — Clipart representation

Choices: alpha PNG only; native composite object; package containing PNG preview
plus editable object data; or both PNG and native flavors. Determine whether
clipart retains scale-independent shapes/text, provenance, palette, original
file link, or only pixels.

### PA-Q03 — First tool vocabulary

Ask individually about pencil, brush, eraser, fill, eyedropper, rectangular/free
selection, text, line/curve, rectangle/rounded rectangle/ellipse/polygon,
airbrush/stamp, magnifier, rotate/flip/stretch/skew, crop and canvas attributes.
For each: accept first release, later, or reject; identify the indispensable
classic behavior and any historical defect not to preserve.

### PA-Q04 — Color and transparency

Choose true RGBA throughout, opaque canvas plus transparent objects, indexed
palette mode, or multiple document modes. Decide primary/secondary color,
transparent selection, erase-to-background versus erase-to-alpha, alpha editing,
color-dialog depth and palette persistence.

Use `COLOR_DIALOG.md` to ask which invocation is modal versus modeless, exact
conversion/profile claims, gamut behavior, recent palette scope and whether
CMYK is informational or tied to admitted output profiles. Persistent color
panels are not an option under ADR-013.

### PA-Q05 — Undo promise

Choices: one action; bounded in-memory actions; memory-budgeted checkpoints; or
none. Clarify which operations are reversible, behavior after save, whether
undo survives close, and whether “no revision tracking” forbids only persistent
history or also normal edit undo.

### PA-Q06 — Window/document topology

Recommended candidate: one document per window, multiple ordinary windows, no
tabs. Ask whether a new/open command reuses an empty window and whether unsaved
documents block close. Persistent palettes/toolboxes and left/right panels are
already rejected; ask which secondary operations are modal or modeless owned
popup dialogs.

### PA-Q07 — Initial formats and decoders

Recommended candidate: PNG first; other decoders only through separately
hardened first-party/plugin paths. Ask about BMP, JPEG, GIF, TIFF, ICO, WebP and
platform clipboard formats, plus export versus editable-open distinctions.

### PA-Q08 — Collaboration meaning

Does “collab” mean composition between reusable objects/files, two local windows
working together, turn-taking on LAN, or simultaneous network painting? The
first is already admitted; network collaboration would require separate 2.0+
authority, conflict and threat design.

## Round B — interactions, integration, and release boundary

### PA-Q09 — Placement lifecycle

After a File Manager drop, is the content immediately floating/selectable,
placed at cursor and awaiting commit, linked to source, or copied and baked?
What happens when the source disappears or changes?

### PA-Q10 — Picker profiles

Which operations use open, open-many, import clipart, save-as and export? Should
Paint show hidden files, admit create-folder, expose preview, or use only a type
filter and object field? Which native-dialog fallback is acceptable?

### PA-Q11 — Safe write and conflicts

Choose atomic temp-and-replace where supported, direct write, or per-format
policy. Decide backup policy, external modification detection, overwrite dialog,
permission/xattr preservation, failed-export partial files and crash recovery.

### PA-Q12 — Text tool

Ask font selection, bundled/system fonts, shaping/scripts, point/pixel sizing,
alignment, antialiasing, edit-before-commit, text retained as an object versus
baked pixels, and whether text remains editable after reopening native files.

### PA-Q13 — Selection and transform grammar

Ask rectangular/free selection, handles, keyboard nudging, copy versus move,
transparent selection, resize interpolation, rotate angles, arbitrary rotation,
snap/grid and off-canvas behavior.

### PA-Q14 — Help and sound

Choose local topic window, greaseboard overview, per-tool F1, tooltips, animated
examples, click/tool sounds and silent/reduced modes. Identify which help is
bundled versus online documentation. Help is a popup/overlay, not a side panel.

### PA-Q15 — Accessibility and touch

Determine first-release keyboard reachability, semantic canvas/tool model,
screen-reader descriptions, high contrast, scale and reduced motion. Touch is a
Malkuth 3.0 horizon, but ask which pointer assumptions must be avoided now.

### PA-Q16 — Kid Pix influence

Ask which properties matter: playful stamps, sound, oversized tools, surprise,
patterns, compositional objects, child legibility, or something else. Reject
unbounded novelty, content store and inaccessible sound-only meaning unless
explicitly reversed.

### PA-Q17 — Printing, capture and acquisition

Ask individually about printing/page setup, scanner/camera, screenshot import,
paste from clipboard and drag from applications. None is inferred from classic
Paint precedent.

### PA-Q18 — First dogfood workflow and “done”

Name three real images the architect will create/edit, required formats, target
sizes, source paths, drag/clipart actions, expected time, failure recovery and
the moment Paint can replace the incumbent for those workflows.

## Interview output

After Round B, produce a decision ledger containing:

- accepted/rejected/deferred tool and format matrix;
- document/composite/clipart state model;
- file/picker/transfer/help boundaries;
- UI topology and direct-manipulation rules;
- correctness, performance, accessibility and dogfood workloads;
- required ADRs and current dependency blockers;
- exact smallest post-gate vertical slice.
