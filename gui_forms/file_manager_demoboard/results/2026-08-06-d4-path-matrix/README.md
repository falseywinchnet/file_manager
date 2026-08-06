# DEMO-D4 path matrix evidence

Date: 2026-08-06
Fixture: `file-manager-demoboard-001`, generation 86
Source base: `28b7b220434d01180ac1781f2ce60c87532bc18f` plus the working-tree D4 closure
State: **MEASURED PARTIAL**

## Claim

The File Manager product consumes public GUI.Forms controls for the bounded D4
path instrument:

- `AnchoredPopupLayer` is a renderer-free retained overlay with deterministic
  near/center/far alignment, above/below preference, vertical flip, viewport
  clamping, live resize resolution, click-away, and Escape requests;
- the browse state contains the complete drive-rooted current stack and exactly
  five recent complete stacks, including one explicitly offline volume;
- the open tail replaces the current stack with the stock public `TextBox`, a
  public `ListBox`, and exact stable completion IDs;
- a new query cancels the prior deferred `DispatchOperation`; only the current
  monotonic generation may update the retained completion model;
- Up/Down changes the active completion without moving text focus, Tab accepts,
  Enter resolves and navigates, and Escape returns to browse before closing;
- invalid fixture variables remain in the editor with visible and semantic
  explanation; click-away and final close restore terminal focus;
- a valid path transition updates title, breadcrumb, tree projection, content,
  status, and history through the existing navigation session.

This evidence does **not** close D4. Physical IME composition and text-range
bounds, an actually concurrent external completion provider, constrained-size
native captures, and correct native focused-element publication remain open.

## Native Computer Use proof

The ordinary application has two native roots: the product and an owned
controller. Computer Use's app-wide screenshot gathers both windows and scales
them into an overview; it therefore looked like the product had been pushed
off-screen even though AppKit reported both windows in valid side-by-side
bounds.

The `--product-only` review mode removes that ambiguity without changing the
product model. In that mode Computer Use measured the complete product AX tree
and successfully performed this sequence against public native semantics:

1. activate `fm.path.matrix.current.tail`;
2. set `fm.path.matrix.editor` to `$PROJECTS/N`;
3. observe `fm.path.matrix.completion.north-shore` as the sole completion;
4. press Tab and Return;
5. observe `fm.title.text` change to `File Manager  ·  North Shore`, the popup
   disappear, Back become enabled, and the status change to the North Shore
   fixture location.

The product surface is therefore usable by the same accessibility action,
set-value, and keyboard paths available to a human or agent. AppKit still
reports the retained surface rather than the stock editor as the focused native
element; that is an explicit D8 defect.

## Seven-surface prototype audit

All seven atlas tabs were opened and inspected: Folder, Search, Criteria,
Palettes, Icons, Styles, and DNA. They reduce to four reusable topology
families:

| Family | Atlas surfaces | Current GUI.Forms posture |
|---|---|---|
| split workspace with optional inspector | Folder, Criteria | **MEASURED PARTIAL** — table/dock/split/fill/collapse and dense property groups exist |
| pane-free correspondence ledger | Search | **OPEN** — variable-height virtual rows, pinned/hover inspection, and scroll-anchor preservation are not implemented |
| predicate rack over object projection | Criteria | **OPEN** — editable module composition, priority reflow, staged Apply, and progress projection remain D6 |
| master/detail laboratory | Palettes, Icons, Styles, DNA | **PARTIAL** — retained composition and materials exist; richer selectable specimens, provenance/failure states, atomic theme transactions, and audit-ledger virtualization remain D9 |

The answer to “is GUI.Forms already a visual superset of the prototype?” is
therefore **no**. The current engine has the necessary retained drawing and
composition base, but parity-and-beyond still requires reusable, product-neutral
capabilities rather than demoboard-only decoration:

- variable-height virtualized collections with stable expansion and scroll
  anchors;
- priority/overflow layout and dense master/detail composition;
- richer stateful material recipes, gradients, shadows, focus/selection layers,
  and high-contrast substitutions;
- size-specific icon resources plus provenance, loading, missing, and failure
  presentations;
- contained inspection overlays and inline evidence expansion;
- atomic theme/material transactions with state preservation.

Those gaps directly determine D5, D6, and D9 ordering. The D4 popup and stable
model identities are reusable prerequisites, not a visual one-off.

## Automated and portability evidence

- Native build and full test run: **55/55 passed**.
- Focused D4 tests cover placement flip/clamp, click-away, Escape request,
  resize re-resolution, focus scope restoration, exact path fixtures, stable
  suggestion IDs, deferred cancellation, Tab/Enter/Escape, invalid variables,
  navigation coherence, and deterministic capture-state entry.
- Win64 MinGW-w64 cross-build passed for `File Manager Demoboard.exe`, including
  the same public popup, dispatcher, controls, and model code.
- A clean build-local install followed by an external `find_package(GUIForms)`
  consumer build and run passed while constructing `AnchoredPopupLayer`, exact
  `ListBox` stable IDs, and button disclosure state from `GUIForms::Controls`.

## Native captures and hashes

Both images are 1269×768 Computer Use product-window captures on macOS 14.8.7.

```text
b56ee24fd91a5e2ac54cf750a6b5697426b5cc4649a14faf926e214020449c23  native-path-matrix-browse.jpeg
9d55346ab2ef8e36be4b313471d2d94e35e747faea701b8ee6cb4b2a2f65d6e4  native-path-matrix-editing.jpeg
1dda4ad14de4f352ce1cf9394696ccd458511553e7818f1258c047663c03c6c6  File Manager Demoboard.exe
```

## Negative results and boundaries

- A two-window Computer Use capture is a gathered thumbnail overview. It is
  retained as a host-inspection limitation, not interpreted as a product
  clipping or responsive-layout result.
- The completion fixture calculates matches synchronously and posts delivery
  through the real dispatcher. Cancellation and stale-generation rejection are
  measured; out-of-order worker/provider completion is not yet measured.
- Native set-value and keyboard navigation succeed, but AppKit's focused-element
  publication still identifies the retained surface rather than the editor.
- The popup is a current House/Sapphire implementation, not a frozen final
  visual design. The atlas audit deliberately records remaining visual-superset
  work instead of claiming screenshot parity.
- Every path and object is fixture-only. No real filesystem path is read,
  opened, or modified.
