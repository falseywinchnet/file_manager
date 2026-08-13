# M12-P31 — Explicit connected-control painting

Date: 2026-08-13

Status: **MEASURED on renderer-neutral retained traces, native macOS build,
and physical macOS Screen Sharing review**.

## Question

Can separate retained controls form one Office Pearl physical instrument without
losing their individual focus, semantics, command routing, disclosure boundary,
or hit target?

## GIVEN constraints

- Adjacency is authored. GUI.Forms must not infer a connection from arbitrary
  tree coordinates.
- Interior corners and duplicate borders disappear; outer corners and one seam
  per boundary remain.
- Hover, press, checked, default, focus, disabled, and disclosure state remain
  local to one retained control.
- The API and retained trace remain renderer-neutral and CPU-only.
- The File Manager prototype is evidence for the Office Pearl control grammar;
  GUI.Forms does not acquire File Manager commands or ribbon policy.

## Implemented seam

**OBSERVED:** `ConnectedControlTopology` records axis, ordered index, and count.
`count >= 2`, `index < count`, a defined axis, and a 256-member bound are
validated. `nullopt` is the explicit standalone state. The derived public
positions are standalone, leading, middle, and trailing.

**OBSERVED:** `connect_button_group` accepts an explicit ordered span of
`ButtonBase` controls. It preflights the complete span before mutation and
rejects null/disposed members, duplicates, mixed parents/windows, a mixed
existing axis, and invalid group size. It never sorts or inspects coordinates.

**OBSERVED:** `resolve_connected_control_visual_geometry` returns a
renderer-neutral clip, expanded paint rectangle, position, and optional leading
seam. Material painting clips an expanded rounded surface at joined edges; this
removes interior round corners and the material's duplicate interior border.
Every non-leading member then owns exactly one leading seam. Joined-edge shadow
outsets are suppressed because those pixels are clipped.

**OBSERVED:** `ButtonBase` owns the topology independently from control type.
Ordinary `Button`, `DropDownButton`, and button-appearance `CheckBox` consume the
same path. `CheckBoxAppearance::button` changes only painting: check-box role,
checked state, activation, focus, and hit identity remain intact. Split/menu
buttons retain their internal disclosure boundary and actions.

## Demoboard

`gui_forms_connected_controls_lab` contains:

- an isolated baseline;
- horizontal two-, three-, and five-member instruments;
- a vertical three-member instrument;
- push + split and menu/dropdown tails;
- default/focused, checked, selected, pending, disabled-interior, hover, and
  pressed states;
- one mixed-axis formation attempt visibly rejected with zero member mutations;
- a public `VisualInspectorView` and pointer-transparent overlay with targets
  for isolated, leading, interior, trailing/split, disabled, and vertical
  members.

The native bundle is `GUI.Forms Connected Controls Lab.app`. A Windows host main
and target are present under the existing MinGW build gate.

## Reproduction

Environment: nearby Apple M4 Mac mini, arm64 macOS, Release build, pinned
CPU-only Skia and bundled GUI.Forms font pack.

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/cmake -S gui_forms -B gui_forms/build \
  -DCMAKE_BUILD_TYPE=Release \
  -DGUI_FORMS_BUILD_GALLERY=ON \
  -DGUI_FORMS_SKIA_PREBUILT=ON \
  -DGUI_FORMS_SKIA_OUT=gui_forms/build/skia-cpu-release

/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/cmake --build gui_forms/build --target \
  gui_forms_connected_controls_tests \
  gui_forms_connected_controls_lab_tests \
  gui_forms_connected_controls_lab \
  --parallel 10

/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/ctest --test-dir gui_forms/build --output-on-failure \
  -R 'gui_forms_(connected_controls|basic_controls|theme|material|visual_inspection)_tests|gui_forms_connected_controls_lab_tests|gui_forms_connected_controls_lab_font_policy'
```

**MEASURED:** the final focused seven-test set passed 7/7 in 0.73 seconds after all
named binaries were built.

**MEASURED:** deterministic traces prove:

- leading/middle/trailing expanded paint rectangles at exact expected bounds;
- one clip over each connected member and one leading seam for each non-leading
  member;
- no claimed shadow outset across a clipped joined edge;
- atomic rejection of invalid topology, duplicate members, mixed parents, and
  mixed axes;
- exact seam hit ownership (`x < boundary` belongs to the leading member and
  `x == boundary` to the trailing member), with no overlap or dead crack;
- separate Tab focus stops and semantic nodes for button, checked check box, and
  split disclosure;
- hover, pressed, checked, disabled-interior, default, and focus traces keep
  seam cardinality stable.

## Physical review

**MEASURED:** the native arm64 bundle was launched with `/usr/bin/open` from
Terminal in the logged-in M4 Aqua session and inspected through Screen Sharing.
The two-, three-, and five-member horizontal sets and the three-member vertical
set kept one continuous silhouette: interior radii disappeared, every boundary
showed one seam, the outer leading/trailing corners remained rounded, and the
isolated control retained its full outline. The split disclosure separator
remained distinct from the group seam.

**MEASURED:** target selectors moved the pointer-transparent overlay among the
isolated, default, checked, split-tail, and vertical-middle controls. The public
inspector reported the exact requested/arranged/absolute/visual/effective-clip
bounds, local viewport, state, display operation count, current chunk, resolved
theme, and window damage count for each separate member while the physical
silhouette remained connected.

**MEASURED:** mouse hover remained local; Return activated the focused checked
member; Tab moved the focus cue from default to checked and through the
five-member set; the disabled trailing and disabled interior specimens did not
accept focus; and Alt-Down independently opened and closed the split tail and
five-member menu tail without duplicating a seam or changing outer geometry.
Selected, pending, checked, default/focus, disabled, active, and inactive paints
were all visibly coherent. Resize preserved every joined boundary.

**OBSERVED:** shrinking the lab to its former 1080-logical-pixel minimum made
the inspector column illegible, although the connected controls remained
correct. This is a demoboard layout limit, not a connected-paint defect; both
native mains now enforce a 1280-pixel minimum so the inspector retains a useful
width. The lab process was then closed without disturbing other running apps,
and its temporary launch symlink was removed.

## Scope of inference and limits

- **MEASURED:** the retained renderer-neutral geometry, state traces, M4 build,
  font bundle policy, and headless semantics pass the named workloads.
- **MEASURED:** native macOS pixels satisfy the connected silhouette, state,
  disclosure, focus, inactive/active, resize, and inspection workload above.
- Win32 source/target presence is not a Windows native capture or accessibility
  result.
- Connection does not provide command-shelf layout, overflow, menus, or product
  policy. It is one reusable paint/topology primitive consumed by those future
  compositions.
- Visual recipes should keep a coherent outer radius across state variants.
  The resolver deliberately preserves the recipe selected for each segment; it
  does not rewrite theme state policy.
