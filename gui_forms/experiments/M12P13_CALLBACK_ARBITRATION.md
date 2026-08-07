# M12-P13 callback-safe paint, hit testing, semantics, and validation

Status: **MEASURED PARTIAL M12-P13** on 2026-08-06.

## Question

Can retained paint, hit testing, semantic projection, semantic action routing,
and bulk validation tolerate application callbacks that mutate ownership
without invalid iterators, stale targets, mixed semantic generations, or
unbounded recursion?

## Findings and rule

- **OBSERVED:** paint replay iterated live child and popup collections across
  `on_paint`; semantic projection iterated live children across descriptor and
  virtual-child callbacks; bulk validation held live child iterators across
  `Validating`; hit testing could return a control disposed by its own
  overridable local hit test.
- **DECIDED FOR THIS 0.x SLICE:** every callback-bearing traversal fixes a
  strong identity arbitration set, checks live parent/window membership before
  invocation and before consuming results, and never invokes an identity that
  an earlier callback removed from that set.
- **DECIDED:** semantic snapshots retry only when their retained semantic
  generation changes and fail after four attempts. Hit testing uses the same
  four-attempt stabilization bound and returns no target if callbacks never
  settle. Recursive semantic snapshot construction is rejected explicitly.

## Implemented center

- paint validates identity after visual-outset, child-viewport, application
  paint, and overlay callbacks; removed siblings and closed popup roots are
  skipped from strong snapshots;
- a control disposed or reparented by `on_paint` cannot receive overlay paint,
  publish a display chunk, or participate in former-parent summary state;
- hit testing snapshots z-ordered children and popup roots, rejects dead or
  reparented results, and retries after declared hit/geometry mutation;
- semantic projection snapshots roots/children, revalidates descriptor and
  virtual-child results, discards mixed-generation candidates, and bounds
  oscillation;
- semantic child-action traversal and popup arbitration revalidate identities;
- bulk validation snapshots each sibling set and skips a child removed by an
  earlier `Validating` callback;
- public `MetricsSnapshot` and deterministic JSON add
  `callback_arbitration_retries` and `callback_arbitration_limit_hits` for
  availability/diagnostic consumers.

## Verification

- paint removal skips the later sibling while completing and releasing the
  retained paint lease;
- self-disposal in application paint suppresses overlay invocation and chunk
  publication, and a following paint remains usable;
- hit-test self-disposal stabilizes to the underlying live target;
- semantic remove-plus-add retries to a snapshot whose generation equals the
  Window generation;
- a deliberately oscillating descriptor stops after four attempts, records
  one limit hit, releases the recursion guard, and permits a later snapshot;
- bulk validation does not invoke a later sibling removed by the first
  callback;
- existing core, popup, semantic, invalidation/damage, and validation suites
  pass in the normal, renderer-free, and combined ASan/UBSan builds; the
  affected core/control libraries also cross-compile strictly for Win64.

## Honest remainder

- randomized cross-family callback mutation and nested popup/menu mutation;
- scheduled-frame callback mutation of the frame-request collection;
- managed protected paint/semantic callback projection and physical host
  reentry qualification;
- ABI/host availability projection of arbitration diagnostics;
- native UIA/AT-SPI publication and assistive-technology task corpora.

This is a retained-core safety rule, not a promise that arbitrary callback
side effects are ordering-compatible with every historical WinForms accident.
