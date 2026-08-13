# Authoritative control regions

Date: 2026-08-13

Status: **GIVEN first axiom; derived rules are CANDIDATE pending Web.Forms and
GUI.Forms contract decisions**.

## 1. First axiom

**GIVEN:** an authoritative control region is a region with absolute control of
everything within it.

For GUI.Forms planning, “absolute control” means presentation and interaction
jurisdiction. The region controls the composition, visual ordering, interactive
ordering, clipping, overlap, child grants, focus relationships, and transitions
inside its bounds. It does not acquire application-domain authority such as
filesystem access or service policy.

A child authoritative system may receive a nested subregion. That authority is
delegated by the parent and cannot override the parent's decision about the
child's placement or relationship to siblings. Authority therefore nests
without becoming ambiguous.

## 2. Geometry owned by a region

A retained element may have several related but distinct geometries:

- authored/layout bounds;
- visual bounds, including shadows and permitted overflow;
- interactive/hit bounds;
- clipping or mask geometry;
- focus and accessibility geometry;
- projected geometry in another authority region.

The authoritative parent grants and resolves these geometries. A child cannot
silently paint or receive input outside the grant merely because its painter or
hit tester permits it.

## 3. Parent-owned order

**GIVEN:** the rendered z-order and interactive z-order of composed controls are
baked into their parent container.

They are distinct ordered decisions:

- rendered z-order decides which visual contribution appears above another;
- interactive z-order decides which child owns a shared or overlapping point.

The two orders may differ, but neither may emerge accidentally from unrelated
tree traversal. Both must be deterministic, source-addressable where admitted,
and visible in inspection output.

For an overlap boundary, the parent must choose exactly one interactive owner
or a documented split rule. There may be no dead crack and no multiply executed
activation.

## 4. Parent-owned relationships

Siblings do not negotiate overlap, seams, anchoring, or transitions directly.
The nearest authoritative parent containing the complete relationship owns it.

Derived candidate rules:

1. A child's intrinsic system supplies content, semantic behavior, local state,
   and preferred geometry.
2. The parent assigns child order, role, placement, overlap, clipping, and
   shared-boundary ownership.
3. Leading/middle/trailing roles derive from the parent's ordered membership,
   not coordinate inference by each child.
4. Seams and joins belong to the relationship and have one declared owner.
5. Focus entry, traversal, dismissal, and restoration across siblings follow
   the same authority boundary.
6. A transition involving two or more children belongs to their nearest common
   authoritative parent.

## 5. Projection and overflow

A popup, menu, tooltip, or drawer may extend beyond the parent's ordinary
layout bounds. That does not suspend the authority model.

The parent must do one of two things:

- include the possible visual and interactive extent in its authoritative
  region; or
- project a named child into an ancestor overlay/native-host region through an
  explicit grant.

An explicit projection records:

- logical owner;
- projection host;
- anchor relationship;
- rendered and interactive layer;
- clip/overflow policy;
- focus transfer and restoration;
- dismissal rules;
- lifetime and native-window ownership where applicable.

Projected visual placement does not change the logical owner or give the child
authority over unrelated overlay siblings.

## 6. Breadcrumb application

**GIVEN:** breadcrumbs live inside an authoritative parent container that
decides their order.

```text
breadcrumb region
├── ancestor segment systems
├── current segment system
├── ./ operator system
└── location-history drawer
```

The breadcrumb parent owns:

- ancestor/current/operator sequence;
- complete chevron overlap geometry;
- rendered ordering of overlapping tips and faces;
- interactive ownership of every shared point;
- seam and backshadow continuity;
- leading/middle/trailing/current/terminal roles;
- drawer anchor, extent, projection layer, and clipping;
- the transition from the trail to the slide-out history drawer;
- focus transfer into the drawer and restoration to the operator;
- dismissal through command, pointer, Escape, owner deactivation, or disposal.

The `./` operator exposes activation/disclosure semantics. It does not privately
own the drawer transition. The parent changes its composed state—for example,
`history-open`—and the relationship between the trail and drawer emerges from
that parent state.

The individual breadcrumb button system owns its local label, face, hover,
press, focus, and activation behavior. It does not decide how it overlaps the
next sibling.

## 7. Candidate inspection snapshot

An authoritative region snapshot should expose at least:

```text
region stable/style identity
authority parent and projection host
ordered child membership and derived roles
layout, visual, hit, clip, focus, and projected bounds
rendered z-order
interactive z-order and overlap owner
active parent state and transition
focus owner and restoration target
committed display chunks and hit-test decision trace
```

This information must remain renderer- and platform-neutral. Native adapters
may add diagnostic facts without changing the portable decision.

## 8. Failure conditions

The authority model fails if:

- a child paints over an unrelated sibling without a parent grant;
- hit ownership is inferred from whichever recursive walk happens to run last;
- z-order changes input behavior without an explicit interactive-order change;
- a popup or drawer escapes clipping through an undocumented special case;
- each breadcrumb segment computes its own overlap from coordinates;
- the `./` button constructs and animates a private drawer that the parent
  cannot style or inspect;
- disposal, reparenting, interruption, or scale changes leave a stale projected
  authority or focus owner.

## 9. Required experiments

1. A headless overlapping-sibling fixture independently varies render and hit
   order and proves exactly one owner for every boundary point.
2. A breadcrumb fixture resolves two, three, five, and truncated segment sets
   without coordinate inference in the child systems.
3. A drawer fixture proves open, close, interruption, reversal, focus transfer,
   Escape dismissal, deactivation, reparenting, and disposal.
4. Inspection reconstructs the parent decision without private renderer or host
   data.
5. macOS, Windows, and Linux hosts consume the same portable authority result;
   native window differences remain adapter capability facts.
