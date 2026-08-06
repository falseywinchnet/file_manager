# Future Malkuth application consumer profile

Date: 2026-08-06.

Status: **GIVEN consumer topology under ADR-013 plus CANDIDATE capability
planning; not an implementation authorization or ABI freeze**.

This profile complements `FILE_MANAGER_CONSUMER_CAPABILITY_PROFILE.md`. It does
not change the active GUI.Forms milestone plan. It records capability classes
future Paint, Text Editor, Games, and trusted host dialogs will eventually need
so the framework does not accidentally equate every substantial application
with File Manager's panel topology.

## Topology law

- Persistent left/right panels are a File Manager and embedded picker/browser
  product decision.
- Paint, Text Editor, Games, and other ordinary Malkuth applications have one
  primary document/canvas/board surface.
- Secondary tools use ordinary owned modal or modeless popup dialogs, or a
  bounded composited greaseboard overlay.
- GUI.Forms may implement panel controls generally, but it must not require a
  dock/panel host to obtain help, color selection, character selection, find/
  replace, game rules, hints, results or settings.
- Plugins/providers never instantiate GUI.Forms objects or native windows.

## Owned dialog requirements

- application/document owner identity and transient/modal relationship;
- modal disable scope without whole-process assumptions;
- modeless owner tracking, owner close teardown and no orphan windows;
- focus capture/return, default/cancel, keyboard traversal and accessibility
  parentage;
- per-monitor placement, scale and edge avoidance;
- nested-dialog limit and deterministic close ordering;
- synchronous-friendly API behavior without blocking the native event loop;
- cancellation, host shutdown and test-adapter terminal results;
- headless fixture clock and no dependence on actual window activation;
- no background repaint or timer while idle.

## Paint profile

- bitmap/canvas custom control and bounded local damage;
- modal/modeless detailed Color dialog with sliders, numeric fields, swatches
  and custom color field;
- CMYK/OKLCH/RGB/hex editing and validation presentation without GUI.Forms
  owning document color semantics;
- Attributes, Resize/Skew, font/text and Help dialogs;
- drag/clipboard and alpha-bearing drawing operations from the Paint plan.

## Text Editor profile

- mature multiline plain-text editor and decoration spans;
- owned Find/Replace dialog that can remain modeless safely;
- virtualized Characters dialog with glyph grid/list, search, detail and insert;
- Help/file-information/encoding-choice dialogs;
- dialog insertion routed through one document command/undo path.

## Games profile

- retained card/board/grid primitives and custom drawing without a scene engine;
- event-driven animation commands with test clock, cancellation, coalesced local
  damage and reduced-motion replacement;
- input during animation as explicit per-consumer policy;
- repeated grid/cell/peg controls, overlapping hit targets, z-order and drag;
- owned New Game, Rules, Hint, Result and local-statistics dialogs;
- sound hooks with independent disablement and no sound-only meaning;
- accessible virtual board/card/cell children and keyboard equivalents;
- zero perpetual frame loop or idle animation requirement.

## Trusted File Manager/plugin-host dialogs

- Checksums progress/result/expected-value dialog;
- Archive password, extraction destination, collision, progress and failure
  dialogs;
- Image Converter format/quality/matte/options dialog;
- all content comes from trusted presentation models; no worker control/layout
  injection.

## Evidence gates

For each profile, require headless lifecycle/input/accessibility/damage traces,
owned-modal/modeless focus fixtures, reduced-motion substitutions, resource
limits, native first-platform inspection and clean public-package consumption.
One attractive demo does not establish cross-platform dialog ownership or game
animation correctness.
