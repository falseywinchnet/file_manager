# DEMO-D0/D1 first native shell evidence

Date: 2026-08-06

State: **MEASURED PARTIAL**

## Claim

The opt-in first-party consumer constructs a deterministic catalogue-001 File
Manager product shell using public GUI.Forms controls. At the 1450×850 logical
model size, the six shell bands resolve to 40/23/66/40/657/24 and the workspace
uses retained thin split panes. The same revision launches and paints through
the AppKit/Skia CPU host.

## Environment

- macOS 14.8.7 (Darwin 23.6.0), arm64;
- GUI.Forms source worktree on 2026-08-06;
- bundled Portsmouth Rapids evaluation faces plus Carlito/Cousine specimen
  body/terminal faces;
- CPU Skia renderer; no GPU backend;
- Sapphire Dusk / House Composite first-pass material;
- nominal text scale and ordinary contrast/motion.

## Reproduction

```text
cmake -S gui_forms -B gui_forms/build \
  -DGUI_FORMS_BUILD_FILE_MANAGER_DEMOBOARD=ON
cmake --build gui_forms/build --target gui_forms_file_manager_demoboard
ctest --test-dir gui_forms/build -R file_manager_demoboard --output-on-failure
```

The install proof used `cmake --install` into a clean build-local prefix, then
configured and ran `file_manager_demoboard/tests/installed_consumer` against
only `find_package(GUIForms CONFIG)` and `GUIForms::Controls`.

## Capture

`native-folder-sapphire-house-current-screen.png`

SHA-256:
`34b831501fb0d6581a51b3ee3086f35aa3afa2d95ecbf711fe6d42c62cbb5534`

The capture is the largest placement admitted by the current display, not the
required exact 1450×850 golden. The separate review controller is not present.

## Observed behavior

- exact shell-band geometry and catalogue identity pass headlessly;
- native semantic tree exposes ribbon, navigation, editable search, tree rows,
  split groups, status, and stable IDs;
- native accessibility `set_value` edits the search field and paints its real
  caret/value;
- the same product model and public Windows launcher cross-compile as the
  static Win64 `File Manager Demoboard.exe` target;
- the Folder visual composition includes a selected fixture object,
  deterministic preview, factual properties, and status authority;
- after one full retained present, a reset metrics window reports zero damage,
  zero measure/arrange/paint passes, zero active surfaces, and no pending frame.

## Negative result retained

The first native launch painted only the workspace. The cause was not the
renderer: `TableLayoutPanel` correctly preserves child preferred sizes and
margins, while the first consumer revision had supplied zero-size band
preferences. The consumer now declares exact band preferences, and a geometry
test rejects recurrence.

## What this does not prove

This evidence does not establish DEMO-D1 completion, reference-size pixel
parity, responsive collapse, native controller/owned windows, a real TreeView
or virtual object model, shared command authority, Search/Criteria surfaces,
outbound drag, physical screen-reader completion, or filesystem behavior.
