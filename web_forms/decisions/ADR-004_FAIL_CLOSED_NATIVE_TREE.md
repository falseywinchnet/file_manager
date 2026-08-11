# ADR-004: Fail-closed typed native-tree dogfood

Date: 2026-08-10

Status: **DECIDED for the experimental dogfood target; production API spelling
and 0.1 compatibility remain open**.

Owner direction: the grand architect named the first complete generated native
control tree, inset shadows, complete box geometry, typography, decoration,
grid/table lowering, and focus-visible modality as the remaining honest gaps
and directed the interrupted Web.Forms work to resume.

## Question

May the independent manifest-backed GUI.Forms projections be composed into one
complete retained tree, and what must happen when any node is not exact?

## Constraints

- Stage 2 consumes accepted typed IR only. It does not parse HTML or CSS.
- No browser, DOM, CSS engine, JavaScript engine, or Python runtime ships in the
  product.
- Every retained node in a generated tree has an explicit hierarchical `id` and
  one typed C++ member. Normalized member-name collisions are errors.
- Generated product source remains C++17-compatible and uses no implicit
  typing, lambdas, closures, `std::function`, or `std::vector`.
- Construction uses only public GUI.Forms controls and recipes. Skia remains
  private to GUI.Forms.
- A Forms default cannot stand in for a refused CSS initial or computed value.
- The experiment is not a frozen application-facing ABI or a native/browser
  pixel-equivalence claim.

## Candidates

1. Keep independent projection fragments and defer all tree construction.
2. Ship a runtime descriptor interpreter which selects best-effort GUI.Forms
   defaults.
3. Generate a complete typed tree at build time only when every required
   material, state, layout, box, typography, and decoration record is exact.

## Measurements

**MEASURED:** candidate 3 emits compile-valid complete trees for the 7-node
button, 14-node breadcrumb, and both 38-node standard-shell theme generations.
All retained nodes have typed members and exact `StableId` strings.

**MEASURED:** a linked native probe constructs the 38-node Sapphire tree against
GUI.Forms, attaches it to a `Window`, and verifies source-order ownership, the
34px body inset, 1012px shell width, 650px minimum shell height, and live
210px-plus-fraction workspace tracks.

**MEASURED negative retained:** the first linked probe exposed GUI.Forms' 3px
default control margin inside the CSS tree, producing a 204px sidebar. Stage 2
now applies CSS initial zero margin and padding before authored box geometry;
the same probe then passes at 210px. This is the class of seam compile-only
descriptor tests did not expose.

**MEASURED:** inset shadow identity survives retained display recording/replay
without expanding damage outsets. Keyboard traversal and semantic focus expose
focus-visible; primary-pointer focus suppresses it without removing actual
focus; programmatic focus deterministically preserves current modality.

**MEASURED:** bounded nonsemantic `::before` and `::after` box decorations
compile into distinct retained owner paint layers around control content.

## Decision

Select candidate 3 for experimental dogfood. `generate-gui-tree` invokes the
bounded component projections, rejects the whole tree if any component record
is unavailable, constructs every control in DOM order, applies CSS initial and
computed state, attaches children in DOM order, and returns a `NativeForm` with
one strongly typed member per retained `id`.

Buttons expose ordinary GUI.Forms events through their typed members. The
`data-wf-command` mapping remains in the generation report; Web.Forms generates
no application behavior or translated JavaScript.

## Rejected options and failure modes

Candidate 1 no longer exercises the central nested-composition hypothesis.
Candidate 2 would ship a second layout/style runtime and would hide capability
loss behind target defaults.

Generation fails on a missing ID, normalized member collision, multiple roots,
unavailable component projection, inexact state recipe, or missing manifest
admission. Native visual comparison may still fail independently; compile and
structural success are not raster fidelity.

## Reversal path

The generator is disposable output over the versioned IR. A future denser tree
lowering may fold layout-only elements or replace the `NativeForm` spelling
without changing source HTML/CSS. Until an ABI decision freezes that surface,
applications must treat the experimental header as regeneration-owned.
