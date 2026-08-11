# Nested landscape model 001

Date: 2026-08-10

Status: **DECIDED principle under ADR-001; MEASURED experimental native-tree
composition under ADR-004**.

## The object being preserved

HTML's useful contribution is not merely cascade syntax. It is that every node
lives inside a nested layout, paint, typography, clipping, and state landscape.
Web.Forms compiles that landscape into retained GUI.Forms records.

For each node, the compiler records:

```text
effective context = parent ambient context
                  + applicable inherited CSS values
                  + resolved custom properties and named roles
                  + local layout and surface declarations
                  + control recipe
                  + intrinsic finite state variant
```

This formula does not claim that every CSS property inherits. The compiler
implements the admitted CSS property's actual inheritance and initial-value
rule, then records provenance for the computed value.

## Four coupled graphs

1. **Containment/layout:** nested content and padding boxes own child placement,
   gap, clip, scroll, and density. A row sequence is laid out as one sequence;
   it is not independently splattered onto canvas coordinates.
2. **Surface ownership:** a node reveals its nearest ancestor surface, paints
   one named local surface, or is statically baked into the owning surface.
3. **Effective state:** visibility, enabledness, focus eligibility, theme,
   density, accommodation, and finite control state combine through typed rules.
4. **Reference/overlay:** labels, descriptions, popups, and top-layer content
   may refer outside containment, but always name an owner and anchor.

## Surface invariant

Every visible pixel has exactly one nearest surface owner before child content
and decoration are painted. Transparent children reveal that surface. A stock
control default fill cannot silently introduce a second background.

The compiler reports at least:

- no ambient surface owner for a visible region;
- an opaque control default that conflicts with its compiled surface role;
- absolute geometry escaping its containing layout;
- popup/top-layer content without an owner and anchor;
- discontinuous manually positioned children where a declared sequence owns
  density and gap;
- state variants that expose a surface or clip not present in the base graph.

**MEASURED dogfood slice:** Stage 1 records each retained parent and one of
`reveal-parent`, `own-surface`, or `baked-into-parent`. The native layout pass
projects 2/2 button-board, 4/4 breadcrumb-board, and 11/11 standard-shell flex
containers into retained direction, wrapping, gap, padding, main/cross
alignment, and grow relationships. This removes fixed-frame child coordinates
from those container algorithms. GUI.Forms' opt-in `Control` background layer
also proves parent reveal and child surface ownership without a wrapper panel.
The fail-closed constructor applies those edges to the same controls that own
layout and identity. A linked 38-node native probe verifies the resulting
shell, workspace, sidebar, and content containment. It also retained a failed
first result: Forms' default 3px margin created a 204px sidebar until the
generator explicitly applied CSS initial zero margins/padding before authored
geometry. The passing result is therefore an end-to-end structural and logical-
geometry measurement, not a claim inferred from available primitives.

A provenance inspector must be able to answer which ancestor, recipe, rule, and
state supplied each effective layout/style value.

## Style exposure

Style exposure is independent from surface ownership and theming:

- `baked` (default): the compiler emits immutable style records; application
  code receives behavior, state, value, and handler surfaces but no mutable
  presentation handle.
- `exposed`: the compiler emits a typed style handle containing only the
  properties the profile and GUI.Forms manifest declare dynamically mutable.

Neither mode creates a second theme. Multiple compiled themes are a separate
future capability.

## Fidelity consequence

The accepted target is exact logical structure and geometry under a pinned
profile, with declared tolerance for rounding. Native/browser typography and
material rasterization are scored separately. A visually pleasing result with
different nesting, gaps, clipping, or ownership is not conformant.
