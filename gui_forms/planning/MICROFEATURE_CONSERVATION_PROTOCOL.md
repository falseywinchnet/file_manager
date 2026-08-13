# Microfeature conservation and custodial interview protocol

Date: 2026-08-13

Status: **GIVEN need; CANDIDATE planning and regression protocol**.

## 1. Problem

Broad visual resemblance does not conserve a design language. A control can
retain its approximate color, location, and function while losing the small
features that make it recognizable, legible, clickable, and related to its
neighbors.

**OBSERVED by the grand architect during native dogfood, 2026-08-13:**

- the File Manager breadcrumbs still appear as half-chevrons;
- they have no backshadow and therefore do not visibly read as buttons;
- clicking the `./` operator does not reveal chevron/location history;
- the implemented approximation therefore does not conserve the prototype's
  breadcrumb system.

This is not adequately repaired by adding three breadcrumb paint properties.
It is the first falsifying case for recursively addressable structure, style,
relationships, and parent authority.

## 2. Conservation unit

A microfeature is a small but design-significant rule whose loss changes one or
more of:

- recognition;
- perceived affordance;
- hierarchy or depth;
- relationship to adjacent controls;
- interaction prediction;
- state legibility;
- theme identity;
- accessibility or keyboard understanding.

Examples include a complete chevron silhouette, an underside shadow, singular
seam, disclosure glyph, overlap ownership, slide direction, pressed locality,
focus restoration, and interruption behavior.

The conservation unit is not a screenshot pixel alone. Every admitted
microfeature must trace through:

```text
custodial interview rule
    -> authored structural element/relationship/state
    -> style selector/property/asset
    -> Web.Forms validated lowering
    -> GUI.Forms retained authority/layout/input decision
    -> committed display and semantic result
    -> native visual and interaction evidence
```

## 3. Required custodial questions

Before implementing or revising a composed control, the interview ledger must
answer the applicable questions.

### 3.1 Recognition and anatomy

1. What makes this object recognizable rather than merely similar in color and
   size?
2. What are its complete outer and internal silhouettes?
3. Which faces, caps, seams, shadows, textures, glyphs, and labels are distinct
   style-addressable parts?
4. Which details make it visibly interactive?
5. Which details are essential DNA and which are one theme's optional
   ornament?

### 3.2 Relationship and authority

6. Which authoritative parent contains the complete system?
7. What is the ordered child membership and which roles derive from that order?
8. Where do children overlap, join, share a boundary, or project elsewhere?
9. What are the rendered and interactive orders at each overlap?
10. Who owns seams, clipping, overflow, focus traversal, and dismissal?

### 3.3 State and interaction

11. What changes for hover, press, capture, focus, current, selected, disabled,
    inactive-window, high-contrast, and reduced-motion states?
12. Does a state remain local to one child or alter a parent relationship?
13. What commands and semantic handles does program code receive?
14. What opens, closes, interrupts, reverses, or restores a projected surface?
15. What keyboard, pointer, accessibility, and semantic-action paths must
    produce the same result?

### 3.4 Theme and source conservation

16. Is every required visual part or relationship present in authored HTML?
17. Can CSS and admitted local images express the required appearance without a
    product-specific C++ paint branch?
18. Can a materially different theme use the same semantic structure and
    behavior?
19. Which selector/part/state names are compatibility promises?
20. What must fail compilation rather than lower approximately?

### 3.5 Scale, layout, and failure

21. What happens under truncation, narrow bounds, large text, different scale,
    localization, right-to-left layout, and missing assets?
22. What happens during reparenting, disposal, deactivation, interrupted
    animation, and unavailable commands?
23. What minimum geometry remains clickable and what visual affordance survives
    compression?
24. Which fallback preserves truth without silently changing the control into a
    different design?

## 4. Conservation ledger

Each composed control or system receives a table with at least these fields:

| Field | Required content |
|---|---|
| Rule ID | Stable interview/design identifier |
| Status | GIVEN, OBSERVED, MEASURED, HYPOTHESIS, CANDIDATE, REJECTED, or DECIDED |
| Source evidence | Interview answer, prototype/vision-board locator, or accepted design record |
| Structural owner | HTML element, part, slot, or parent relationship |
| Style owner | Selector, state, pseudo-part, property, or local asset |
| Runtime owner | GUI.Forms region/control/relationship contract |
| Program seam | Generated handle, command, collection, event, or none |
| Visual oracle | Exact silhouette/material/typography/geometry condition |
| Interaction oracle | Pointer, keyboard, semantic action, focus, and dismissal condition |
| Inspection oracle | Retained decision and display/semantic facts that must be visible |
| Native evidence | Host, scale, text size, states, and result |
| Known exclusions | Explicitly unsupported variants; never implied fidelity |

The ledger is a source artifact, not a prose claim made after implementation.

## 5. Regression layers

No single layer closes conservation.

1. **Source/profile tests** prove required elements, relationships, selectors,
   states, and assets are admitted and unsupported approximations fail closed.
2. **Generated-tree tests** prove stable identity, parentage, slots, ordered
   membership, and semantic handles.
3. **Retained authority tests** prove layout, overlap, z-order, hit ownership,
   focus, projection, and transitions.
4. **Display-trace tests** prove required materials, clips, shadows, seams,
   glyphs, and authored order.
5. **Raster correspondence** compares semantic probes and whole specimens at
   named scales without claiming that a tolerance proves interaction.
6. **Native dogfood** exercises actual visual recognition and interaction on
   every promoted host.
7. **Theme substitution** proves the system is addressable rather than one
   hard-coded skin.
8. **Documentation inspection** proves a theme author can find the anatomy and
   contracts without private source archaeology.

## 6. Breadcrumb corrective case

### 6.1 Required anatomy

- an authoritative breadcrumb parent region;
- complete leading, middle, current, and terminal button systems;
- full interlocking chevron silhouettes rather than divider-like half shapes;
- one parent-owned overlap and seam rule;
- a visible back/underside shadow that establishes button depth;
- local normal, hover, pressed, focused, current, and disabled presentation;
- a terminal `./` operator with a disclosure affordance;
- a parent-owned location-history drawer and transition.

### 6.2 Required behavior

- every segment remains an independently focusable/activatable navigation
  command unless explicitly disabled;
- overlap points have one deterministic hit owner and no dead crack;
- activation state affects only the intended segment while relational depth
  remains coherent;
- activating `./` opens the parent's history state and drawer;
- history is distinguishable from ancestry and inline path editing;
- keyboard and semantic disclosure actions match pointer activation;
- Escape, selection, owner deactivation, and disposal dismiss through the
  parent and restore focus deterministically;
- interrupted or reversed transition leaves no stale hit surface.

### 6.3 Required style addressability

At minimum, the source/style contract must address:

```text
breadcrumb region
segment system
segment face and content
leading/middle/trailing/current roles
chevron tip/overlap relationship
seam
backshadow
hover/pressed/focused/current/disabled states
terminal operator label and disclosure
history drawer, anchor, layer, and open/closed transition
```

Names and syntax remain a Web.Forms decision. The list is a capability and
conservation requirement.

### 6.4 Promotion gate

The breadcrumb regression is closed only when:

1. the grand architect recognizes the native system as complete button stock,
   not half-chevron decoration;
2. the backshadow and state changes visibly establish clickability;
3. `./` opens the parent-owned history drawer;
4. the same structure supports at least two materially different CSS theme
   treatments without breadcrumb-specific product C++ painting;
5. source, retained inspection, display trace, raster, and native evidence agree
   on every ledgered rule.

## 7. Planning consequence

Future control work must not use “functional” or “vaguely resembles the
prototype” as a promotion state. When the necessary addressability does not
exist, the work item is a design-language/GUI.Forms capability gap. When the
addressability exists but the source style is wrong, it is a consumer/Web.Forms
style gap. When source intent cannot be lowered truthfully, it is a compiler
profile gap. The ledger must name which boundary failed.
