# Text Editor architect interview

Status: **questions prepared; no answers inferred**.

The sibling asks these over two conversational turns with gains, losses and
alternatives. Answers are recorded before implementation planning is promoted.

## Round A — byte/text truth and window behavior

### TE-Q01 — Encoding detection and default

**Recommended candidate:** BOM first; strict UTF-8 when valid; otherwise present
an explicit encoding choice with a reversible preview rather than guessing and
saving silently.

- Gain: no accidental conversion of configuration files.
- Lose: occasional prompt and more explicit state.
- Alternatives: UTF-8-only refusal; locale fallback; heuristic detection with
  confidence. Ask which encodings must be first-class in 1.0.

### TE-Q02 — Malformed and binary-looking input

Choose refusal, read-only escaped/hex-adjacent display, replacement-character
view without save, or explicit lossy open. Define NUL, invalid sequences, very
long lines, control characters and save behavior.

### TE-Q03 — Newline and BOM preservation

Recommended candidate: observe and preserve original LF/CRLF/CR and BOM unless
the user invokes an explicit conversion. Ask about mixed newlines, new-file
defaults per platform/app, status indication and final-newline behavior.

### TE-Q04 — Safe save

Choose atomic sibling-temp/replace where supported versus direct write. Ask
about permissions, owner/group, executable bits, ACLs, xattrs/resource forks,
symlinks, hard links, sparse files, file identity, fsync strength, failed temp
cleanup and save-as versus save differences.

### TE-Q05 — External modification

Choices: prompt with reload/overwrite/save-as; read-only conflict state;
automatic reload only when unmodified; merge/diff. Determine detection cadence,
renames/replacements, deleted files and what happens while a dialog is open.

### TE-Q06 — Document/window topology

Recommended candidate: one document per window, no tabs; open-many creates
multiple windows. Ask empty-window reuse, reopen-last behavior, duplicate opens,
window geometry/history, unsaved close and application shutdown.

### TE-Q07 — Undo/recovery promise

Choose bounded in-memory undo, memory-budgeted history, one action, or another
model. Clarify save boundary, reopen persistence, crash recovery/autosave and
whether configuration editing argues against hidden recovery files.

### TE-Q08 — Large-file boundary

Ask representative sizes/line lengths. Compare full UTF-aware editable model,
paged/mapped editing, read-only large-file mode, and explicit refusal. Define
memory/latency budgets and when color hints/find/undo disable or degrade.

## Round B — search, presentation, integration and dogfood

### TE-Q09 — “Whitecards” grammar

Confirm whether the term means wildcard matching. If yes, decide `*`, `?`,
character classes, escaping, case, whole-word, selection scope, newline crossing
and replacement captures. A small visible wildcard language is different from
regex and needs exact examples.

### TE-Q10 — Find/replace interaction

Choose classic modal dialog, modeless owned popup, or hybrid modal/modeless
popup. Persistent panels and inline sidebars are rejected under ADR-013. Decide
wrap-around, incremental highlighting, replace-next/all,
selection-only, case/word options, history and zero-length match behavior.

### TE-Q11 — Color hints

Ask first formats individually: JSON, TOML, YAML, INI, XML, shell, PowerShell,
batch, C/C++/Rust/Go/Python/JavaScript, Markdown and plain logs. Decide token
categories, comments/strings/errors, theme dependence, on/off control, stale
parsing and whether hints ever imply validation.

### TE-Q12 — Editor chrome

Ask status bar, line/column, encoding/newline indicator, dirty mark, wrap,
line numbers, whitespace marks, indentation guides, ruler, minimap and toolbar.
The recommended candidate is a menu, text field, small factual status line and
temporary find surface—no IDE furniture.

### TE-Q12A — Characters dialog

Using `CHARACTERS_DIALOG.md`, decide the Unicode data source/version, system or
bundled font scope, search fields, blocks/categories, displayed encodings,
recents/favorites, multi-code-point sequences and behavior when the document
encoding cannot represent the selected character. The surface is an owned popup,
not a side panel or separate application.

### TE-Q13 — Text behavior

Decide wrapping default, tabs-as-tabs versus spaces, tab width, automatic
indent, smart quotes/dashes (recommended reject), trim trailing whitespace,
final newline, word-boundary behavior, font roles and zoom.

### TE-Q14 — Picker and hidden files

Choose Text Editor's default: show hidden, ask once, or inherit a neutral app
profile. Decide remember/session toggle, extensionless files, package/bundle
navigation, open-many, native fallback and recent locations.

### TE-Q15 — Protected files

Recommended first release: show permission failure and safe recovery/save-as;
do not broker sudo/admin editing. Ask whether a later first-party privileged
helper is ever desirable and what exact workflow would justify it.

### TE-Q16 — Handler integration and CLI

Ask which file types Text Editor registers internally, whether it may become an
OS default through explicit Orchestrator/platform action, `text_editor <path>`
CLI behavior, wait-for-close option and line/column addressing.

### TE-Q17 — Help, accessibility and sound

Ask local help topics, greaseboard overview, F1/context mapping, keyboard-only
requirements, screen-reader text ranges, high contrast, scale, reduced motion,
caret/selection visibility and whether any sounds belong in an editor. Help is
an owned popup or overlay, not a persistent panel.

### TE-Q18 — First dogfood workflow and “done”

Name real configuration/text files, encodings, hidden paths, sizes, search/
replace operations, external-change and permission cases. Define the point at
which Text Editor replaces TextEdit/modern Notepad for those workflows.

## Interview output

After Round B, produce:

- exact byte/text/newline/BOM and save state model;
- accepted/rejected/deferred UI and editing matrix;
- wildcard/literal search grammar with examples;
- color-hint format/provider list and failure behavior;
- large-file, external-change, picker/help/handler boundaries;
- performance/correctness/accessibility/dogfood workloads;
- dependency gate delta, required ADRs and smallest post-gate vertical slice.
