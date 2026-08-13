# Web.Forms browser/native fidelity oracle 001

Date: 2026-08-13

Status: **MEASURED**

## Purpose

The oracle compares one accepted Web.Forms source in a real Chromium layout
against the generated GUI.Forms retained tree. It records stable identity,
authored parentage, bounds, effective clip, visible/enabled/focus/input state,
text and baselines, semantic material composition, connected/resource identity,
and eight raster samples per visible node. The comparison keeps structure,
state, geometry, typography, semantic material and raster differences separate.

The native side consumes GUI.Forms' public visual-inspection snapshot. Source-
private children of compound controls, such as split panels and grips, are
excluded and their authored descendants are reparented to the nearest authored
ancestor. This prevents implementation topology from becoming a false source
structure difference while leaving the private controls covered by their own
GUI.Forms tests.

## File Manager result

At the pinned `1450 x 850`, 1x, reference-state profile, browser and native
snapshots have identical source digests, state names and stable-ID sets. All
authored materials match semantically: ordered solid/linear/radial/repeating
fill kinds, borders, radii, shadows and keylines report zero material-mismatch
nodes. All 119 authored nodes also report zero structure and zero state
mismatches. The outer six application bands are emitted by `ResponsiveTrackPanel`,
and all three authored two-pane relationships are emitted as retained
`SplitContainer` instances.

Geometry and typography differences remain intentionally visible in the
report: 69 nodes exceed the strict 0.51-logical geometry/clip tolerance and 23
exceed the strict typography tolerance; 50 nodes match all measured dimensions.
Maximum bounds/clip delta is 37.479167 logical units. The 301-logical maximum
baseline delta belongs to browser-clipped/hidden text and is retained rather
than laundered into a parity claim. These are toolkit/browser measurement
differences, not silently folded into material support. Eight-point raster
sampling reports 32 nodes above the summed-RGBA tolerance of 120, with a worst
distance of 552. Those are actual pixels, kept separate from semantic material
agreement. Every one of the 32 raster-mismatch nodes also has a geometry or
typography mismatch; there is no raster-only residual pointing to an absent
blend or mask operation.

The complete matrix contains 11 profiles: ordinary, 720-wide, 150x150,
125/150/200-percent text, inactive, hover, pressed, focused and disabled.
Every profile preserves exact source digest and authored structure. Ordinary,
all large-text profiles, inactive and all interaction profiles have zero state
and semantic-material mismatches. The 720 profile records 19 state mismatches
because native split collapse is active while raw HTML does not execute the
authored `data-wf-*` split policy. The 150x150 profile records 31 state and five
clipped-material mismatches while proving that both command groups collapse and
the explicit overflow actuator appears. The HTML source itself now accepts a
150x150 viewport; the remaining difference is policy execution rather than a
hidden 1080-pixel CSS floor. The oracle reports those differences; it does not
declare the raw browser a simulator for GUI.Forms-only metadata.

Replay:

```sh
python3 -m web_forms.src.web_forms_compiler.cli capture-browser-fidelity \
  frontend/ui/boards/file_manager/file_manager.wf.html \
  --viewport 1450x850 --output /tmp/file-manager-browser.json
python3 web_forms/tools/build_native_fidelity_probe.py \
  frontend/ui/boards/file_manager/file_manager.wf.html \
  --unit file_manager_sapphire --gui-forms-build gui_forms/build
python3 -m web_forms.src.web_forms_compiler.cli normalize-native-fidelity \
  /tmp/file-manager-native-inspection.json \
  --projection /tmp/file-manager-projection/file_manager_sapphire.gui_tree.json \
  --output /tmp/file-manager-native.json
python3 -m web_forms.src.web_forms_compiler.cli compare-fidelity \
  /tmp/file-manager-browser.json /tmp/file-manager-native.json \
  --output /tmp/file-manager-fidelity-report.json

python3 web_forms/tools/measure_fidelity_matrix.py \
  frontend/ui/boards/file_manager/file_manager.wf.html \
  --native-probe web_forms/.build/native_fidelity_probe/file_manager_sapphire/native_fidelity_probe \
  --projection web_forms/.build/native_fidelity_probe/file_manager_sapphire/file_manager_sapphire.gui_tree.json \
  --font-directory gui_forms/assets/fonts \
  --output-dir web_forms/.build/fidelity_matrix/file_manager_sapphire
```

## Masks and blending verdict

**OBSERVED:** neither the accepted File Manager HTML/CSS nor its generated
material graph requires an arbitrary mask, blend mode, isolated compositing
group, backdrop filter or color-matrix operation. The visual construction is
expressible with ordered clipped fills, radial/linear/repeating gradients,
inset/outset shadows, borders, keylines and ordinary opacity. Adding a mask or
blend API now would be speculative library expansion, not fidelity repair.

At constrained and 200-percent profiles, the two raster-only IDs are the root
and shell composite probes. Their samples cross descendants which already have
geometry, state or material differences; they do not isolate a missing paint
operation. If a later accepted specimen requires one, it must enter with a
minimal source grammar, renderer-neutral retained semantics, bounded
complexity, Skia/Core Graphics equivalence probes, damage/outset rules and an
explicit fallback or rejection policy. It should not enter as an untyped CSS
escape hatch.
