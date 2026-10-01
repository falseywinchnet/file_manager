# Mac synchronous paint phase instrumentation

Status: independently reviewed instrumentation committed as `945f9a7`;
native run `36878654190` succeeded on all three platforms, including the Mac
fixture. No rendering-strategy change or CPU improvement claim.

## Evidence and measurement boundary

**MEASURED consumer evidence:** SwiftEdit 700157e with SDK 0322371 reports all
eight hidden dialogs at zero paints/deadlines in both five-second phases. Nine
main-editor blinks now damage 342 logical area units rather than 4,796,820.
Focused process CPU remained 1.77252% of one core, compared with earlier
1.8294–2.14822% observations. These short observations are not a controlled
attribution of cost to a particular subsystem.

**OBSERVED sample:** the supplied three-second native sample includes 2,963 of
2,986 main-thread samples waiting in mach messages, one Window paint sample,
and AppKit/Core Animation commit and backing-store paths. It cannot establish
causation or accurately apportion the remaining low-duty-cycle work.

**OBSERVED source:** each presentation creates a provider over the full raster,
a color space and image, then submits the image to the full logical destination
under the current native graphics clip. This does not establish that Core
Graphics copies or converts every source pixel.

## Private diagnostics

Six per-view fixed aggregates record completed synchronous phases, each with
calls, total nanoseconds, maximum nanoseconds and a saturation flag:

| JSON phase prefix | Included operations |
|---|---|
| `paint_raster_prepare` | Resize, pending damage, image synchronization, begin frame |
| `paint_retained` | Window paint, including command recording/replay and raster work |
| `paint_raster_finish` | Live-surface composition and end frame |
| `paint_cg_setup` | Provider, color space and CGImage creation |
| `paint_cg_draw` | Graphics-state save, blend/transform, DrawImage, state restore |
| `paint_cg_release` | Image, color-space and provider releases |

The private host JSON also reports the latest native dirty area, frame damage
bounding area, submitted CG destination area, context clip bounding area, and
declared source-buffer bytes. Areas are logical coordinate square units; source
bytes include row stride. The clip is a bounding rectangle, not the exact clip
shape. These fields do not measure traffic, physical copies, asynchronous work,
or presented-pixel area.

All timing is steady-clock elapsed wall time. It is not thread or process CPU
time. Phases do not cover work deferred until after drawRect returns, nor are
their sums a whole-process cost. A throwing phase is absent from the completed
aggregate and the existing native callback fault counter must be checked.
Setup without successful image creation can outnumber draws. Saturated results
cannot support interval subtraction. Clock reads and clip queries add overhead;
capture a matching instrumented baseline before judging later changes.

No sample history, hot-loop allocation, logging, new worker, public API, or new
timer is introduced. Strings are built only when diagnostics are requested.
Existing exposure, resizing, receipt acceptance, occlusion and failure paths
retain their operation order. Release timing adds a timestamp immediately before
the same image/color-space/provider release sequence.

## Native fixture and proposed controlled experiment

The existing owned-window fixture now verifies advancing phase counts during a
visible caret interval, unchanged counts and durations during the settled hidden
interval, unsaturated aggregates, maximum <= total duration, paired healthy CG
setup/draw/release calls, and reported destination equal to actual logical view
bounds. The fixture retains its earlier initial-hidden, explicit UI timer,
show/hide, caret and shutdown assertions. It prints complete host snapshots and
has no timing-performance threshold.

**CANDIDATE next experiment; not implemented or an architecture decision:**

1. Run the instrumented production blank-document probe on a fixed Mac, scale,
   window size and focus state. Capture per-window phase deltas alongside process
   CPU seconds, completed frames, actual damage, and display-chunk rebuilds.
   Use longer intervals and repeated alternating runs; report distributions,
   not a comparison of one favorable sample with one earlier sample.
2. In an owned native fixture, settle a static blank editor, then compare equal
   cadence and equal damaged rectangles in two modes: invalidate the control's
   narrow rectangle to force command rebuild, or request native exposure of that
   rectangle with unchanged retained commands. Both modes submit identical
   static pixels through the same normal presentation path. Revoke only this
   fixture's autonomous caret schedule before starting the controlled cadence;
   do not suppress application timers or change production behavior.
3. Verify matching native draw counts, submitted image/destination/clip extents,
   and raster damage between modes. Verify the intended difference in chunk
   rebuild counts. Compare retained-phase time against CG setup/draw/release and
   total process CPU. If extents or cadence differ, reject that comparison.
4. Treat remaining process CPU outside measured synchronous phases separately.
   Native stacks may be collected in separate intervals to avoid mixing profiler
   overhead into CPU results. Only after these measurements should a cropped
   presentation or image-reuse prototype be proposed, with exposure/resize,
   antialiasing/scale, ownership, and failure regressions. No GPU path is proposed.

## Exact source review and validation

Reviewed all authored hunks in `src/host/macos/application/macos_host.mm` and
`tests/macos_idle_visibility_tests.mm` against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`: explicit types and conversions, const
inputs, named execution, initialized fixed storage, main-thread access,
non-retained native borrows, completed/failure distinctions, saturation, native
release ordering, and no per-element instrumentation. Unchanged legacy blocks
and unrelated host methods are not claimed compliant. No known house-style
violation remains in the authored scope.

The exact portable aggregate was extracted and compiled on Shadow with C++20,
`-Wall -Wextra -Wconversion -Wsign-conversion -Werror`. Accumulation of 17 ns and
3 ns, maximum, JSON field names, and overflow saturation checks passed. The
temporary source and executable are
`C:/Users/Shadow/file_manager/gui_forms/.build/mac-phase-aggregate-check.cpp` and
`C:/Users/Shadow/file_manager/gui_forms/.build/mac-phase-aggregate-check.exe`.
The source copies the current `struct MacPaintPhase final` through its closing
brace, stopping before the following anonymous-namespace closing comment; only
standard includes and the described main-function checks are added. The exact
file-based verification command, run from `C:/Users/Shadow/file_manager`, was:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
g++ -std=c++20 -Wall -Wextra -Wconversion -Wsign-conversion -Werror gui_forms/.build/mac-phase-aggregate-check.cpp -o gui_forms/.build/mac-phase-aggregate-check.exe
& ./gui_forms/.build/mac-phase-aggregate-check.exe
```

Both compilation and execution returned zero. This is not an Objective-C++
compile or native rendering test. `git diff --check` passed. Existing native
fixture target requires no CMake change. Coordinator owns review, commit, push,
and native CI; A2 remains frozen and untouched.

The coordinator independently verified both source hashes and reviewed the full
two-file diff, including aggregate bounds, initialization, diagnostic formatting,
native resource lifetimes, completed-phase failure semantics, and fixture stage
ordering. No defect was found in the authored scope. Native Objective-C++ compile
and fixture execution remain pending CI; portable aggregate checks do not replace
them. This commit admits measurement only, not a rendering optimization.
The coordinator also independently compiled and ran the saved portable aggregate
check with the strict command above; both completed successfully.

SwiftEdit's evidence collector reported all six Mac fixture snapshots parsed.
Visible stage 1 to 2 added one draw: retained 53,083 ns, CG setup 5,291 ns,
CG draw 60,458 ns, release 2,708 ns, raster preparation 7,833 ns and finish
166 ns. Hidden stage 3 to 4 left all phase counts and times unchanged; no
saturation or native callback faults were reported. These are attributed native
fixture observations, not a measurement of SwiftEdit process CPU or a complete
cost attribution. The longer provider-owned surrogate remains separate.
