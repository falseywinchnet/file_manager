# M11h — reusable progress animation styles

Status: **MEASURED PARTIAL**. Date: 2026-08-06.

## Question

Can GUI.Forms expose visually distinct professional progress effects as
reusable retained control behavior, with bounded scheduling and deterministic
reduced-motion behavior, rather than embedding animation in a demo painter?

## Implemented

`ProgressBarVisualStyle` now includes three reusable animated families:

- `pulse` / `luminance_pulse`: a broad five-stop luminance band travelling
  slowly forward across the determinate fill;
- `marching_stripes`: clipped diagonal classic stripes with a continuous phase;
- `laser_etch`: a horizontally extended five-stop repeated phase that shifts
  vertically, a four-stop energized leading edge, a compact white-hot radial
  corona and core, compact phase-shifted flame lobes, and a deterministic field
  of short ember flecks. Long linear particles were rejected after dogfood
  because they read as tendrils.

Horizontal and vertical controls share the same public model. The laser field
changes axis correctly with orientation. `ProgressBarAnimationAppearance`
owns all effect colors plus pulse extent, laser edge extent, and phase pitch as
one validated value. Invalid finite/range input rejects the whole transaction.

Every style uses the existing `MotionPolicy`, phase-preserving pause/resume,
hidden-surface quiescence, one active-surface lease, and deadline scheduler.
Reduced motion remains visibly alive with calmer spark geometry and cadence;
pause and disable revoke the lease. Static blocks/continuous modes have no idle
animation work.

## Measured gates

- `gui_forms_animation_tests` records renderer-neutral painter commands. It
  requires a moving five-stop luminance gradient, multiple clipped stripe
  bands, a repeated five-stop vertical laser phase, a four-stop edge, five full
  seven short sparks, three calmer reduced-motion sparks, balanced clips, and
  lease revocation
  on transition to continuous.
- Appearance tests prove exact round trip and atomic rejection of invalid pulse,
  edge, and pitch records.
- `gui_forms_showcase_interaction_tests` proves that the ranges page is composed
  only from reusable public controls and that its expanded content fits.
- The rebuilt native Complete Showcase ranges page exposes all six style labels
  and six stable progress semantics. Direct inspection observed marquee motion,
  marching stripes, and the laser phase/edge presentation without semantic-tree
  churn.

## Honest boundary

These styles do not claim pixel identity with a Windows theme, GPU particle
effects, or a separate general-purpose animation graph. Sustained cadence and
damage-volume measurements across Win32/Wine/Linux remain open. Spark placement
is deliberately deterministic so retained traces and screenshots are
reproducible. Sound cues remain an orthogonal host service and are not coupled
to progress paint.
