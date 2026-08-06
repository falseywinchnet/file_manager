# DEMO-D3 retained inspector evidence

Date: 2026-08-06
Fixture: `file-manager-demoboard-001`, generation 86
Source base: `9b0d79977570d6a5ef7f4f2f9f15700b83a52200` plus the working-tree D3 slice
State: **MEASURED PARTIAL**

## Claim

The Folder surface consumes public GUI.Forms controls for the first bounded D3
inspector slice:

- `PropertyList` owns stable grouped read-only, text, and choice rows;
- preview/header and property rows participate in one scroll plane;
- Enter commits and Escape rolls back stock `TextBox` editors;
- duplicate, empty, and path-separator Name failures publish inline accessible
  validation and deterministic rollback;
- session Name and Opens-with changes project through stable object identity;
- preview disclosure retains and reuses the same header subtree;
- `SplitContainer` exposes a compact seam tab for pointer, Enter/Space, and
  semantic collapse/expand while preserving ordinary live drag outside it;
- optional pane minima/maxima prevent a user from donating space beyond the
  content's useful measure or crushing it below that measure; Selection is
  constrained to 288–420 logical pixels and fills that allocation, while the
  folder tree is capped at 360;
- pane collapse transfers focus, removes the hidden pane from semantic task
  order, and restores the remembered extent.

The reusable split state distinguishes programmatic, user, and automatic
accommodation collapse. The File Manager headless matrix covers 1450×850,
1200×760, 960×680, 720×520, 480×360, 300×240, and 150×150, including automatic
restore and a user override below threshold. A second matrix covers
100/125/150/200/225% logical text without conflating it with host display
scale. At 1800×1050, the title/workspace bands consume the entire allocation,
the object field grows in both axes, and the Selection pane remains attached to
the right edge; physical AppKit zoom shows the same full-window behavior.

Global reduced motion now changes cadence/excursion without stopping active
progress or easing surfaces. Semantic location, pane, option, and operation
feedback is clock-injected and deterministic; sound-off preserves the semantic
record while suppressing host gain.

This evidence does **not** close DEMO-D3. The host-display-scale and
high-contrast visual matrices, physical compact-size captures, preview
copy/drag, final seam proximity animation, and physical host cue observation
remain open.

## Automated evidence

- Native CMake build and full test run: **54/54 passed**.
- Focused tests:
  - `gui_forms_inspection_controls_tests`
  - `gui_forms_split_container_tests`
  - `gui_forms_file_manager_demoboard_tests`
- The File Manager test drives Selection past its useful maximum and verifies
  the pane stops at 420 while `PropertyList` fills the complete allocation.
- The File Manager test resizes the product from 1450×850 to 1800×1050 and
  verifies every product band fills, the central object field grows in both
  axes, and the right pane remains edge-attached.
- `gui_forms_layout_panel_tests` proves a `DockStyle::fill` child consumes its
  complete TableLayout cell at both 240×120 and 640×360 while its authored
  requested bounds remain unchanged.
- Installed-package proof: a fresh `GUIForms::Controls` consumer builds and
  runs against the installed `PropertyList`, validation API, and semantic split
  collapse/restore surface.
- Current Win64 proof: MinGW-w64 GCC 15.2.0 builds both
  `File Manager Demoboard.exe` and the CPU-only `gui_drawing_raster0.dll`
  against the pinned `gui_forms-cpu-windows-mingw` Skia archive. The initial
  current-revision attempt incorrectly reused the macOS archive path; correcting
  the cache to the existing PE/COFF archive closes that negative result without
  disabling Skia. An isolated-prefix Wine Gallery smoke then handled four
  retained clicks, captured a 1120x680 framebuffer, reported zero rejected host
  events, and closed/shut down once.

Host environment: macOS 14.8.7, Apple clang 16.0.0.26.6. The PE64 build used
MinGW-w64 GCC 15.2.0.

## Physical AppKit dogfood

`native-selection-properties-restored.png` records the restored Selection pane
after the following physical sequence:

1. focus `fm.object.obj-facade-study`;
2. press F2;
3. replace the complete Name value with `Facade QA.png` and press Return;
4. activate `fm.selection.collapse`;
5. verify the complete Selection subtree leaves the native accessibility tree;
6. activate ribbon `Properties`;
7. verify the split returns to distance `935`, and the edited object, heading,
   and property value remain coherent.

Native accessibility exposed `PropertyList` as a property grid/group hierarchy,
the Name editor as a settable text field, Opens with as a combo box, and both
workspace splits with current collapse/expand descriptions.

## Artifact hashes

```text
2581f99a163b81e5bf98d2099eb41c53696701b9e5da0631494ce5505076154d  native-selection-properties-restored.png
cbd06c525f82f07bf4cc441bb92272dc5bb85fdac7175a748b0826b8385fa8e9  File Manager Demoboard.exe
c76f5bc212b4728c389e4866545480193a5ecfed93ab15e2ee172142f8358a37  gui_drawing_raster0.dll
1fb8e4860592dffd7d5e2c1f9499c1845cfc4619532635883421480f84055152  wine-gallery-smoke.jsonl
e4db6fa96f0447c78bf74b7c2be0f7bd342e4c079a52bdca17922eebe2b53a3c  gallery-automation.bmp
88988bea222852c30e08a3629d9f929baacfec4844c85bb978dcc68e6f4d4add  PortsmouthRapids.ttf
f97d702778f5933b4ae138064a348499a6dba90d9e2e6bf75a6ab81a6730c03a  PortsmouthRapids-Bold.ttf
```

## Negative results and boundaries

- The first current-revision Win64 attempt was negative because the cache named
  the macOS Skia output directory. The linker consequently reported unresolved
  Skia symbols for `gui_drawing_raster0.dll`. The checked-in MinGW archive was
  present and valid; reconfiguration to that explicit target archive produced
  the current passing PE64 artifacts. This retained failure is configuration
  evidence, not a renderer fallback.
- A first Wine smoke attempt was contaminated by an already-running private
  compatibility specimen
  facade, which publishes the same GUI.Forms automation window class. The probe
  found that unrelated process before Gallery startup. The authoritative rerun
  used an isolated temporary Wine prefix and passed; no live specimen process
  was stopped or modified.
- Coordinate-based Computer Use targeting of the thin seam could not resolve
  the window position reliably; pointer activation is therefore proven by the
  headless routed-input test, while the physical collapse used the caption
  affordance. This is not claimed as a physical seam-pointer measurement.
- Native accessibility reported the retained surface as focused even while the
  model correctly routed F2 and text input to the Name editor. Editable value
  mutation was physically successful, but native focused-element publication
  remains an FM-A closure item.
- The File Manager projection is fixture-only and performs no filesystem rename
  or host handler enumeration.
