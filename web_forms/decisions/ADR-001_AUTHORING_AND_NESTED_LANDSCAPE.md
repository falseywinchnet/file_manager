# ADR-001: bounded HTML/CSS and the nested landscape

Date: 2026-08-10

Status: **DECIDED — approved by the grand architect on 2026-08-10**.

## Question

Which parts of the browser authoring model are authoritative, and how must they
survive lowering into retained GUI.Forms objects without shipping a browser?

## Constraints

- Source must remain ordinary browser-valid HTML/CSS inside a versioned,
  fail-closed profile.
- General JavaScript, DOM mutation, and simulated application behavior are not
  admitted.
- The result is retained, imperative native GUI.Forms, not immediate mode.
- IDs preserve addressable object identity; C++ owns domain behavior.
- The browser's nested composition is a required semantic input, not merely a
  screenshot reference.
- Raw material styling is confined to static backplanes/custom drawing.
- Logical geometry is the fidelity target; typography and material raster are
  measured separately.

## Workload and evidence

**OBSERVED:** the concept atlas obtains density and surface continuity from a
nested HTML landscape.

**OBSERVED (owner report):** the native File Manager breadcrumb-history list
has unwanted vertical gaps because its rows were loosely placed over the rest
of the interface.

**OBSERVED (owner report):** the SDRSharp reflection experiment still contains
control backgrounds that differ from their surrounding surface after hours of
manual composition.

These are retained negative results. They falsify loose visual overlay as an
adequate translation method.

## Candidates

1. Translate arbitrary HTML/CSS/JavaScript and ship browser semantics.
2. Treat HTML/CSS only as a visual sketch and hand-compose native controls.
3. Compile a bounded HTML/CSS tree, cascade, and finite state matrix into a
   retained nested layout/surface/state graph.

## Decision

Select candidate 3.

The compiler computes each retained node's intrinsic properties and its derived
ambient context. Containment, content boxes, gaps, clipping, typography,
effective enablement/visibility, theme roles, accommodation, and surface
ownership are preserved in the lowered graph. Runtime state selects among
finite precompiled records; it does not run a CSS selector engine.

CSS `background` is not redefined as an inherited property. A child instead has
one explicit surface mode: reveal the nearest ancestor surface, paint its own
named surface, or be statically baked into its ancestor. Every visible pixel
must therefore have one nearest surface owner. Compiler diagnostics reject
unowned holes and unintended default-control backgrounds.

Ordinary controls use named material roles. Raw colors, gradients, textures,
and similar material remain confined to static backplanes or explicit custom
drawing. A control's style exposure is separately `baked` or `exposed`; theme
multiplicity is not implied.

## Rejected candidates

Candidate 1 imports unbounded execution and a second application/runtime model.
Candidate 2 is the method that produced the recorded density and background
failures and does not preserve authoring economy.

## Consequences and failure modes

- Absolute/top-layer placement requires an explicit owning popup/overlay
  relationship; it cannot escape containment silently.
- A browser preview is conformant only under the pinned profile and logical
  geometry oracle.
- GUI.Forms must expose public layout, surface, state, and style-record
  capabilities sufficient to instantiate the compiled graph.
- If the first atlas slice still requires manual background repair or row
  placement, the lowering or GUI.Forms capability is incomplete; the compiler
  may not hide that gap with approximations.

## Reversal path

The source profile and typed construction IR are versioned. A later ADR may add
a bounded feature or replace a lowering family while retaining fixtures and
source maps. Adding JavaScript or a shipped browser requires a new program-level
decision because it reverses a hard product boundary.

