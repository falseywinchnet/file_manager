# Web.Forms interview ledger 001

Date: 2026-08-10

Status: **GIVEN intake, recorded resolutions, and remaining CANDIDATE choices;
not itself an ADR**.

## Recorded architect direction

**GIVEN:** create Web.Forms as its own sibling project.

**GIVEN:** use a constrained, ordinary HTML-like authoring language which can
be designed visually in a browser and compiled into retained native C++ over
GUI.Forms/Skia.

**GIVEN:** preserve stable control identity so behind-code logic can address,
read, mutate, and subscribe to generated objects.

**GIVEN:** runtime behavior remains GUI.Forms-like, retained, imperative, and
stateful; it is not a browser DOM and not a frame-by-frame rebuild.

**GIVEN:** Athene DML is historical lineage rather than the new source syntax.
The accepted round-trip, metadata, stable-ID, and deterministic-order
requirements carry forward to Web.Forms.

## Resolution R1 — HTML/CSS source, no JavaScript

**GIVEN:** approved. Reject all authored JavaScript and do not try
to translate prototype simulation logic. Keep a finite declarative visual-state
matrix and connect real C++ handlers/models to generated typed IDs.

Why:

- the atlas JavaScript is fixture/review logic, not the source of its visual
  quality;
- translating general JavaScript would import an unbounded language, DOM
  mutation semantics, async/timer behavior, and a second application model;
- C++ handlers already match the intended Forms programming model;
- compile-time-only HTML/CSS can remain inspectable, deterministic, and
  fail-closed.

Necessary qualification: banning JavaScript must not ban `:hover`, pressed,
focus, selected, disabled, expanded, or popup-visible visual design. Those are
real GUI states, not simulated domain behavior.

## Resolution R2 — identity is selective and typed

**GIVEN:** require IDs on every retained/addressable object,
not every HTML fragment. Compile decorative fragments into their parent. Require
model keys for repeated virtual items. Generate and validate the parent tree
from DOM/control ownership.

The atlas demonstrates the need: it has 99 buttons/inputs but IDs on only seven,
while many important controls are class-only structures. A blind all-elements
ID pass would generate hundreds of meaningless C++ objects and make source
refactors unnecessarily breaking.

## Resolution R3 — do not overload ordinary classes

**GIVEN:** preserve `class` for style/component recipes. A reserved `wf-*` class
prefix can carry Boolean traits; typed values and relationships use
`data-wf-*`. `id` alone owns addressable identity. Style manipulation is not a
theming opt-out: `data-wf-style-exposure="baked|exposed"` controls whether
typed presentation properties are generated, and defaults to `baked`.

This keeps CSS useful in a normal browser, makes compiler settings visibly
namespaced, and prevents a spelling such as `selected` from ambiguously meaning
fixture state, CSS appearance, control behavior, and application data.

## Resolution R4 — compile the nested landscape, do not run a cascade

**GIVEN:** resolve containment, cascade, inherited values, custom properties,
surface ownership, component expansion, and state variants at build time into
interned records. Runtime theme/state changes choose a finite compiled record;
they do not query selectors. Transparent children reveal an explicit nearest
ancestor surface; `background` is not falsely redefined as inherited.

Use one behavioral control base plus visual recipe composition. Do not admit
arbitrary multiple behavioral inheritance.

## Resolution R5 — precompose static material, not the whole GUI

**GIVEN:** flatten shared backgrounds, frescoes, textures,
and decorative layers where exact, while retaining controls/text/focus/semantic
objects separately. This preserves precise damage and accessibility while still
removing repetitive browser-style paint structure.

## Owner questions

### WF001 — JavaScript and state — RESOLVED

Approved: no JavaScript, with finite native pseudo-state styling and an external
preview/capture harness.

### WF002 — class metadata — RESOLVED

Approved: ordinary visual classes plus reserved Boolean `wf-*` traits and typed
`data-wf-*` values; class tokens are not all compiler toggles.

### WF003 — ID grammar — RESOLVED

Approved: dotted hierarchical IDs, derived/validated parent IDs, and IDs only
for addressable retained objects.

### WF004 — component reuse

Should 0.1 admit bounded inert `<template data-wf-component>` expansion, or
defer reusable components until one atlas slice compiles without it?

### WF005 — SVG — RESOLVED FOR 0.1

SVG is admitted only as an indivisible local visual resource hosted by an
HTML/CSS-laid box. It is not a structural element language: SVG descendants do
not become controls, layout nodes, stable IDs, state owners, hit targets, or
selector subjects. Relational geometry such as breadcrumb chevrons remains CSS
decoration attached to the owning box so it relaxes with layout.

Stage 1 rejects inline `<svg>` trees in 0.1. A later resource lane may accept a
closed, script-free SVG file with an intrinsic `viewBox` and lower the whole
asset at build time to retained path records or provenance-tracked raster scale
variants. No SVG parser ships in the product.

### WF006 — CSS transitions

Should 0.1 allow only finite state-linked transitions, or reject transitions
entirely until static fidelity is proven?

### WF007 — fidelity — RESOLVED

Exact logical structure/geometry under a pinned profile is the target. Material
and typography rasterization are measured separately; literal pixel equality is
not promised.

### WF008 — compiler implementation — RESOLVED

ADR-003 selects a two-stage build-time Python compiler. Product output is
C++17-compatible orthodox source; Python contributes no runtime.

### WF009 — style freedom — RESOLVED

Raw material remains confined to backplanes/custom drawing. The real control
axis is `baked` versus `exposed` style properties; multiple-theme support is a
separate capability.

### WF010 — dynamic data

Candidate answer: no general binding grammar. Consider only the one-way,
expression-free virtual-item projection in `DATA_PROJECTION_001.md`; application
C++ owns models, validation, edits, and replacement snapshots.
