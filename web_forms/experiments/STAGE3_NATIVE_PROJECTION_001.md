# Web.Forms Stage 3 native projection boundary 001

Date: 2026-08-13

Status: **OBSERVED implementation boundary; MEASURED generated slices**

## Result

No additional File Manager-specific visual composition pass is required.
Web.Forms now owns the static appearance and retained topology that previously
had to be reconstructed after generation:

- ordered layered shell/title/shelf/location/status materials;
- authored title drag-region identity;
- fixed/remaining responsive application bands;
- typed command overflow groups, priorities, actuator and drop-down controls;
- settings and three-pane workspace topology through three generated
  `SplitContainer` instances;
- pane minima/maxima, fixed/collapse policy, enlarged seam hit geometry and
  90 ms transition metadata;
- command image presentation metadata and the authored title mark;
- explicit connected navigation-stock topology, without coordinate inference;
- bounded source-local 1x/2x PNG bytes, deterministic generated `ImageList`
  registration, and authored command image assignment.

File Manager now aliases the generated workspace split controls instead of
clearing and rebuilding the workspace. Its remaining child replacement calls
target explicit live-content hosts: application menu, breadcrumb/path editor,
search editor, tree, object view, preview media, selection property list and
settings/services pages. Those controls depend on filesystem, Orchestrator or
session state and correctly remain application C++.

The command `ImageList` is no longer late application surgery. Stage 1 validates
canonical PNG headers, exact logical/density dimensions, source containment,
per-file and aggregate byte budgets, consistent list size and key/density
identity. Resource bytes participate in the source digest. Stage 2 embeds the
bytes, registers 1x/2x variants through GUI.Forms' PNG-only decoder, binds the
list to authored controls, and keeps it alive in the generated `NativeForm`.
File Manager C++ still creates dynamic object/tree and preview image lists
because their keys are selected from live filesystem and preview models.

## Remaining work

There is no blocking native Stage 3 capability gap for the accepted prototype.
The 11-profile browser/native matrix now exercises ordinary, 720-wide and
150x150 clients; 100/125/150/200 percent text; active/inactive; hover, pressed,
focused and disabled; split collapse; and command overflow. Every profile has
exact source digest, state name, authored stable-ID set, parentage, control kind,
connected topology and local-resource identity.

What remains is verification and preview precision:

1. use the fidelity oracle to reduce measured geometry/typography deltas where
   they are visible and important;
2. run Windows native visual/accessibility review in addition to the green
   MinGW compile/link projection;
3. keep dynamic compound controls inside their authored hosts rather than
   teaching Web.Forms filesystem or Orchestrator semantics;
4. decide whether the raw browser preview must itself execute non-CSS
   `data-wf-*` layout semantics. At 150x150, native priority collapse and
   command overflow are correct while ordinary HTML/CSS remains a static
   rendering of the 150x150 source and does not run the retained priority
   solver. The oracle records 31 state mismatches rather than hiding that
   difference. A future browser preview adapter belongs in Web.Forms; it is not
   another GUI.Forms primitive.

The ordinary profile has zero structure, state and semantic-material mismatch
nodes. It still has 69 strict geometry, 23 typography and 32 eight-point raster
probe mismatch nodes. These measurements identify layout/font/resource-preview
precision work; they do not justify new compositing breadth.

Advanced masks/blending are not on this list because the accepted source has no
such operation, the ordinary profile has no raster-only residual, and the few
root/shell-only raster differences in constrained profiles sample descendant
layout/state differences rather than an isolated paint primitive. Command
execution and overflow-menu composition are not on this list because they are
application authority, not static visual projection.
