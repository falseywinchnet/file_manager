# M12p32 physical seams and proximity controls

Date: 2026-08-13

Status: **MEASURED**

## Question

Can GUI.Forms render the accepted File Manager pane boundary as a quiet
physical seam while retaining a substantially larger, accessible input target
and truthful proximity/drag/focus state, without changing pane layout on hover
or introducing an idle animation loop, while retaining the accepted short
collapse-tab interpolation?

## Reference judgment

**OBSERVED:** the accepted HTML uses a three-logical-pixel pane column, a
one-pixel highlight, a one-pixel lowlight, and a small `7 × 28` collapse tab.
Pointer proximity or keyboard focus grows the tab to `12 × 42`, raises its
contrast and adds shallow depth. The construction specification requires at
least nine logical pixels of hit width and admits seams thinner than three
pixels. The current frontend CSS retains the three-pixel visual gradient but
does not itself express the larger hit target.

**OBSERVED:** the pre-existing retained `SplitContainer` already supplied two
stable panels, a source-private `SplitterGrip`, three-logical-pixel paint inside
a nine-logical-pixel hit strip, minimum constraints, pointer capture, keyboard
resizing, focus transfer, collapse/restore, responsive accommodation, and a
quiet/expanded collapse actuator. Replacing it with a second public splitter or
a File Manager-specific grip would duplicate working behavior.

The missing reusable contract was an explicit physical thickness policy,
asymmetric hit extents, bounded atomic geometry validation, inspectable seam
state/geometry, drag cancellation, adjustable semantic actions, and a bounded
motion-policy-aware actuator interpolation.

## Admitted public contract

`SplitContainer` now exposes:

- `SplitSeamThicknessPolicy::{logical, device_pixel_hairline}`;
- `SplitSeamGeometry`, with visible thickness, independent before/after hit
  extents, declared minimum hit target, and hard public complexity limits;
- `SplitSeamState::{idle, near, hot, dragging, focused, disabled, collapsed}`;
- `SplitSeamSnapshot`, with authored geometry, orientation, local visible and
  hit/actuator bounds, display scale, physical visible-pixel count, transition
  duration/progress and active state;
- `set_splitter_geometry` and `splitter_seam_snapshot`;
- `splitter_transition_duration` and a bounded atomic
  `set_splitter_transition_duration`;
- semantic focus/increment/decrement/set-value in addition to the existing
  collapse/expand actions.

The legacy width setters remain compatibility shorthands. They publish through
the same geometry validation path.

Construction validates every field before mutation. Visible thickness is
bounded at 64 logical units, each invisible hit extension at 128, and the
declared minimum target at 256. A device-pixel hairline must satisfy its minimum
from invisible extents alone, so increasing display scale cannot silently make
the logical hit target too small.

## Geometry and interaction result

**MEASURED:** with `{hairline, before=5, after=7, minimum=12}`:

| display scale | logical paint | device paint | retained hit width |
|---:|---:|---:|---:|
| 1× | 1.0 | 1 px | 13.0 logical |
| 2× | 0.5 | 1 px | 12.5 logical |

The seam origin is rounded to the active device grid before layout publication,
so a nominal `100.26` logical distance becomes `100.5` at 2× rather than
producing a blurred subpixel hairline.

The panel boundary uses the effective visible thickness; the hit control is a
last-child overlay over both neighboring panel edges. Probes at the leading and
trailing overlap resolve to the seam, so there is no dead crack. At a container
edge, the complete hit strip shifts inward rather than being truncated.

**MEASURED:** pointer travel through the wide strip enters `near`; the central
actuator enters `hot`; neither changes existing keyboard focus. A press anywhere
on the adjustable strip enters explicit `dragging`, requests focus and captures
the pointer. Escape releases capture and restores the drag-start value.
Disabling or detaching a captured seam releases capture; detachment also clears
every retained proximity flag. Arrow keys, Shift+Arrow, semantic increment,
decrement and set-value all reach the same constrained distance model. No seam
state owns an idle timer or active surface. A state change retargets the
actuator from its currently presented geometry to quiet `7 x 28`, near
`9 x 34`, or engaged `12 x 42` geometry over the authored duration (90 ms by
default) with ease-out timing. Only an in-flight transition owns a frame lease;
completion, detach, zero duration or reduced-motion policy revokes it. Seam and
hit bounds never interpolate. The opt-in visual inspector polls independently
at 250 ms while it is open.

## Dedicated demoboard

`gui_forms_seam_proximity_lab` contains:

- accepted three-logical-pixel vertical seam with asymmetric hit geometry;
- one-device-pixel vertical and horizontal specimens;
- live idle/near/hot/drag/focused state readouts;
- disabled and user-collapsed specimens;
- first/second pane minimum constraints and collapse/restore;
- device-scale 1×/2× and text-scale 100/125/150% controls;
- semantic focus/collapse/disable controls;
- target selectors plus the public `VisualInspectorView` and pointer-transparent
  overlay;
- a named over-budget geometry rejection specimen.

The lab is ordinary public GUI.Forms composition. It has macOS and MinGW/Win32
mains, uses bundled font policy, and introduces no backend type or File Manager
pane policy.

## Automated evidence

M4 Mac mini Release configuration:

```text
gui_forms_core_tests                         pass
gui_forms_scrollable_control_tests           pass
gui_forms_input_controls_tests                pass
gui_forms_visual_inspection_tests             pass
gui_forms_theme_tests                         pass
gui_forms_split_container_tests               pass
gui_forms_layout_panel_tests                  pass
gui_forms_lifecycle_controls_tests            pass
gui_forms_visual_inspector_lab_tests           pass
gui_forms_seam_proximity_lab_tests             pass
gui_forms_seam_proximity_lab_font_policy       pass
11/11 passed
```

Focused tests cover 1×/2× logical-to-device thickness, asymmetric before/after
geometry, both overlap edges, trace paint width, atomic invalid/over-budget
rejection, proximity without focus theft, captured drag, Escape restoration,
disable and detach revocation, collapsed state, semantic range actions,
horizontal transposition, minimum resize, inspector chunk bounds, and absence
of a seam-owned idle wake. The transition test samples the 90 ms path at 45 ms,
proves in-flight actuator geometry and an unchanged seam/hit target, then proves
exact completion and lease revocation; reduced motion completes immediately.

The Windows cross-build target
`gui_forms_seam_proximity_lab_windows` compiles and links successfully against
the CPU-only MinGW/Skia projection.

## Native review

**MEASURED:** the Release macOS app was launched from Terminal inside the
logged-in Screen Sharing session. At 1x and 2x profiles, the physical specimen
remained a single device-pixel line while its cyan/magenta inspection overlay
showed the independently wider asymmetric hit strip. The three-logical-pixel
reference, horizontal transpose, disabled seam, and edge-collapsed seam stayed
visually distinct; the collapsed edge retained its proximity actuator without
leaving a dead crack.

Right-button pointer probes (used to move the remote pointer without starting
a primary drag) reproduced `NEAR` across the broad strip and `HOT` over the
centered actuator. A primary drag from the broad strip entered `DRAGGING` and
retained capture; Escape restored the start position and left the specimen in
`FOCUSED`. Right and Shift+Right then moved the focused seam through the same
constrained model. Collapse/restore and disable/enable commands changed state
without shifting neighboring lab geometry. Inspector target selection covered
the 3L, horizontal, disabled, and collapsed specimens; committed operations,
arranged/visual/clip geometry, generation, and zero pending damage remained
visible.

The 125% and 150% text profiles retained readable headings, readouts, controls,
rejection evidence, and inspector rows. The native zoom/minimum-size cycle kept
every specimen and control in bounds, and active/inactive/active transitions
did not introduce a stale hover or capture state. No native defect was found.

## Remaining boundary

- Host accessibility publication remains governed by the broader GUI.Forms
  semantic bridge. This slice proves the renderer-neutral role, numeric value,
  range and actions; it does not claim a new VoiceOver/UIA conformance run.
- Persisted pane identity/extent storage and DML/C ABI spelling remain outside
  this visual seam slice.
