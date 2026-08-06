# File Manager frontend and dogfood sequence

Status: **future delivery order**.

## Frontend 001 / F1 — deterministic shell

Using GUI.Forms, the live Core 1.0 Orchestrator bootstrap, and fake later
provider ports, render the classic location window:
custom title bar, menu/control shelf, breadcrumbs plus terminal path editor,
search field, collapsible tree, content surface, and preview/properties pane.
Prove keyboard, selection, focus, resize, collapse and shutdown traces.
Engine and later-provider states are explicitly simulated in this slice.
Deterministic tests replay Orchestrator's canonical Core fixtures, but product
startup uses the live authority. See [`FRONTEND_001.md`](FRONTEND_001.md).

## F2 — sandbox filesystem navigator

Navigate a disposable macOS root with list/small-icon views, sorting, inline
rename, drag/drop, deletion and one-step undo according to the operation oracle.
No real Home/root mutation.

## F2P — reusable Document Picker in parallel with navigation

Extract the bounded browser model, selection controller, and GUI.Forms picker
composition described in [`DOCUMENT_PICKER_SURFACE.md`](DOCUMENT_PICKER_SURFACE.md).
Exercise open, bounded multi-open, folder selection, save-as, app-scoped hidden
visibility, cancellation, and native fallback against disposable roots. This is
implemented alongside the real navigation substrate so it cannot become a
second divergent file browser.

## F3 — real engine

Connect exact object/navigation metadata, indexed folder sizes, current-subtree
search, deterministic result updates, unindexed live fallback and unavailable
states. Exercise Kolmogrov-ready candidate envelopes without waiting for final
research quality.

## F4 — later Orchestrator user services

Consume settings, handler resolution, context declarations, preview/thumbnail
workers, provider lanes, semantic-memory query envelopes, local audit and
degraded/restart behavior. Plugin code never enters the frontend process.

Dogfood the trusted built-in checksum and `Open Command Line Here` commands
before first-party plugin commands so command identity, contextual visibility,
settings, cancellation and host-owned dialogs have a known trusted reference.

## F5 — macOS package and protected dogfood

Install signed/notarized artifacts, setup root consent, run the visible index
agent, preserve Finder as recovery, and use File Manager daily on admitted roots.
Keep destructive/fault campaigns inside explicit sandboxes until their gates
close.

## F6 — daily replacement threshold

The program becomes the architect's ordinary file browser when it can reliably
perform navigation, search, list/icons, drag/drop, rename, delete/undo, open-with,
preview/properties and configured context commands without service stalls or
data-authority confusion.

## F7 — Windows and Linux

Port platform adapters and packaging against the same Orchestrator/engine/GUI.Forms
contracts. Platform differences remain explicit; the visual/application model
does not fork by default.

## Later first-party backbone proofs

After File Manager and its picker package are usable, a minimal classic-Paint
descendant exercises open/save-as, PNG/clipart transfer, drag from File Manager,
popup Color conversion and application-local help. A plain-text/configuration
Text Editor then exercises hidden-file picker profiles, exact text encodings/
newlines, popup Characters and Find/Replace dialogs, restrained color hints,
and non-regex find/replace without tabs or RTF coercion.

The planning-only Games collection exercises cards, grids, custom boards,
bounded animation, owned dialogs, deterministic solvers and reduced motion.
Lexicon proves a typed exact-definition provider and supplies the pinned
Shakespeare corpus to Crossword without entering Engine's file catalogue.

Archive Viewer then proves virtual hierarchy, external-engine supervision,
embedded browser reuse and trusted extraction destination; Image Converter
proves create-new host-mediated transformation. These remain separately gated
and do not expand Frontend 001's opening predicate.
