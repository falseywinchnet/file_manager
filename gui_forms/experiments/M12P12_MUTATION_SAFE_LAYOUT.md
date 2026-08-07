# M12-P12 mutation-safe retained layout

Status: **MEASURED PARTIAL M12-P12** on 2026-08-06.

## Question

Can application-overridable measurement and arrangement mutate the retained
tree without invalidating framework iterators, assigning stale geometry, or
leaving the bounded scheduler unusable?

## Evidence and rule

- **OBSERVED:** the window scheduler, base Dock/Anchor engine, FlowLayoutPanel,
  TableLayoutPanel, and ScrollableControl iterated live child storage across
  overridable `measure`/`get_preferred_size` calls.
- **HYPOTHESIS CONFIRMED FOR THE NAMED CORPUS:** a callback removing or
  reparenting a later sibling could invalidate that traversal and could invoke
  or assign a control that no longer belonged to the layout owner.
- **DECIDED FOR THIS 0.x SLICE:** layout traverses a strong identity snapshot.
  It checks live parent/window membership before invoking a callback and again
  before committing its result. A control added during a callback enters a
  following bounded pass; a removed, reparented, or disposed identity receives
  no stale callback or slot from its former owner.

This is retained-core behavior, not a demo workaround or platform policy.

## Implemented center

- protected `snapshot_layout_children` and `is_current_layout_child` seams for
  reusable custom retained layout controls;
- mutation-safe recursive Window measure/arrange traversal;
- post-callback alive/attachment checks before damage, bounds-change events,
  or descendant traversal;
- snapshot/revalidation in base AutoSize, Dock, Anchor, FlowLayoutPanel,
  TableLayoutPanel, and ScrollableControl extent calculation;
- role-identity revalidation in Card and MasterDetailView measurement;
- later additions retain dirty state and are consumed by the next scheduler
  pass instead of changing the current arbitration set.

## Verification

- native core removes a later sibling during measure, reparents a later sibling
  during arrange, disposes the active callback target, and adds a new child;
- FlowLayoutPanel and TableLayoutPanel prove a sibling removed by the first
  measurement callback is never measured or assigned from the stale snapshot;
- a 32 by 32 leaf graph defers 1,024 geometry mutations under one root
  suspension and commits in at most two arrange passes with zero bounded-pass
  limit hits;
- focused core/layout/composition/scroll tests pass in the normal build;
- the same four focused suites pass in the renderer-free build and under the
  combined ASan/UBSan build.

## Honest remainder

- property/model randomized mutation across deeper layout-family mixtures;
- mutation during semantic, hit-test, and paint replay callback families;
- public availability projection for hard convergence/pass-limit state;
- baseline/RTL/font-scale/DPI rounding and independent WinForms ordering;
- DML construction and reorder transaction policy.

This record does not claim complete WinForms layout parity or a general
performance result from the designer-scale correctness corpus.
