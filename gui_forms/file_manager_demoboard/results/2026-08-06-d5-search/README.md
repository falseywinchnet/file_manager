# DEMO-D5 Search correspondence evidence

Date: 2026-08-06
Fixture: `file-manager-demoboard-001`, generation 86
Source base: `28b7b220434d01180ac1781f2ce60c87532bc18f` plus the working-tree D5 slice
Environment: macOS 14.8.7 arm64; MinGW-w64 GCC 15.2.0 cross-build
State: **MEASURED PARTIAL**

## Claim

The File Manager product now consumes a reusable public GUI.Forms
`CorrespondenceView` for the Search surface. It provides:

- seven stable logical fixture results without one retained control per item;
- sparse 45/126-pixel variable heights and logarithmic visible-row lookup;
- a delayed hover expansion, independent keyboard focus, independent
  selection, and one replaceable persistent pin;
- scroll-anchor restoration across height changes, including an expansion
  above the viewport and a distinct pointer target;
- bounded realization and semantic children for the default action,
  percentage, factual metadata, excerpt, and plugin-information lane;
- restrained case-insensitive evidence marks inside excerpts;
- explicit offline and stale presentations;
- a pane-free Search topology that keeps the folder tree and removes the
  Folder object/Selection projection from visible semantics;
- deterministic query debounce with cancellation/generation rejection;
- available-result activation back through the normal Folder navigation
  session and unavailable-result failure in place.

This evidence does **not** close D5. Selectable excerpt text ranges, physical
hover-delay measurement, provider-backed million-result storage, row-local
damage rectangles, native large-text/high-contrast captures, and correct
AppKit focused-element publication remain open.

## Native Computer Use proof

The product-only native window was driven through the same pointer, keyboard,
set-value, and accessibility paths available to a human or agent:

1. the Search field committed `invoice quartz` and projected Search without a
   Selection pane;
2. one pointer click pinned `fm.result.result-invoice-text`;
3. Down expanded `fm.result.result-invoice-pdf` by keyboard while the first row
   remained pinned;
4. Return activated the focused available result and navigated to
   `Orchard Study` through the ordinary retained Folder history;
5. committing the existing query reopened Search with correspondence state
   retained;
6. three Down presses focused and expanded the offline Pages result;
7. Return left Search visible and published the exact unavailable-volume
   reason.

After the navigation fixtures were widened, the final replay activated
`Stone schedule.csv`, populated North Shore with four fixture objects, selected
`fm.object.search-stone-schedule`, and projected its kind, size, name, and
status through the ordinary Folder Selection pane before reopening Search.

The physical accessibility tree exposed `fm.result.<id>.activate`,
`.percentage`, `.metadata`, `.excerpt`, and `.plugins` for expanded rows. As in
D4, AppKit still reports the retained root rather than the true model focus as
the focused native element; keyboard dispatch nevertheless reaches the focused
correspondence correctly.

## Prototype viewport finding

The interactive HTML atlas was opened and all seven surface tabs were inspected
again. It is one tall review page: a review-control strip sits above one product
window whose specified client is 1450x850. In a 1269x768 browser viewport the
product therefore continues below the screen. That is page overflow, not the
native product pushing its own content off-screen.

The earlier native two-window capture was a different limitation: Computer Use
gathered the product and controller into one overview. `--product-only` removes
that ambiguity and is the authority for product geometry.

## Visual-superset posture after D5

GUI.Forms is **not yet a proven visual superset** of the atlas, although the
daily-product composition base is now substantially stronger.

| Visual/composition axis | Current evidence | Remaining proof |
|---|---|---|
| responsive shell, docking, tables, splits, collapse | measured partial | priority ribbon overflow and more physical compact captures |
| pane-free expandable correspondence | measured partial | excerpt text ranges, physical hover timing, finer damage |
| overlays and contained inspection | measured partial | richer shadow/material primitive and constrained native matrices |
| criteria predicate rack | open | D6 editable modules, staged Apply, wrap/priority behavior |
| palette/style laboratories | partial base only | atomic material/theme transaction and state preservation |
| icon/material laboratory | partial base only | size-specific resources, provenance and failure presentations |
| DNA master/detail laboratory | partial base only | selectable virtual decision index and audit/verdict composition |
| rich material vocabulary | partial | retained Painter gradient/path/rounded/shadow vocabulary or an admitted GUI.Drawing bridge |

The ordinary retained layouts can compose the atlas structures, but claiming a
visual superset requires the remaining adjustment axes to be public and
measured—not merely reproducible with demoboard-specific paint code.

## Automated and portability evidence

- D5-focused collection and demoboard tests pass.
- A 10,000-record headless collection keeps realization bounded and owns zero
  per-item controls.
- The external installed-package consumer builds and runs after constructing
  and activating `CorrespondenceView` through `GUIForms::Controls` only.
- The Win64 `File Manager Demoboard.exe` cross-build passes with the same public
  model and controls.
- The final full native suite run passed 55/55. An earlier run passed 54/55;
  the AppKit repeated active-surface scheduled-wake stress case failed once,
  passed on the immediate isolated retry, and passed in the final full run.
  This timing flake is retained as a negative result.

## Native captures and hashes

All three captures are 1269x768 product-only Computer Use images.

```text
488d2a4e0e6da6df58eddeffb8053b2145b10c15cae267db31a8672309670c53  native-search-pinned-focus.jpeg
eb71cd2dc980b4540f169f287949e468eb93ad05e79a7ff68afca69a7c6a6e42  native-search-two-expanded.jpeg
76a2dc26f084da1033cc6213d0638ed24603b105bafc89dc7a35085a91e5602a  native-search-offline-expanded.jpeg
db6eb4cd72293b040e345bcfb037ce3f5608921ec8ef4315aefdbc84ca29ab60  File Manager Demoboard.exe
```

## Negative results and boundaries

- “Five of seven shown” is a fixture/status statement while all seven logical
  items remain present and virtualized; the current 850-pixel product viewport
  can visibly fit all seven at the default expansion mix.
- Evidence marks are paint fragments inside one factual excerpt string; the
  virtual excerpt is not yet an editable/selectable native text range.
- Model storage is a vector. Sparse expansion and realization are bounded, but
  a provider-backed million-result interface is not yet implemented.
- Correspondence invalidation is confined to the collection control, not yet a
  row rectangle, because the public invalidation seam is control-granular.
- The query fixture returns a stable seven-result corpus. It proves debounce
  ordering and surface transition, not search ranking or filesystem authority.
- No real file, index, volume, or plugin was read or executed.
