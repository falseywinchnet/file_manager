# Theory assessment 001: what is likely to work

Status: **evidence-weighted research judgment, not an architecture decision**.

Date: 2026-08-05.

## Outcome

The strongest current direction is a two-layer system of ideas:

1. exact external identity and source anchors;
2. a lossy, progressively refinable measure of occurrence flow used only to
   generate and rank perceptual candidates.

The fundamental lossy object should be the exhaustive occurrence measure before
fusion, not one fixed vector. Its scale should be a multi-axis lattice, not one
universal depth. Its minimum support should be defined by the observations and
deformations it must predict, not by hash width alone.

This is the construction I would continue. I am confident in its accounting
framework and exact finite boundaries. I am not yet confident that any concrete
semantic routing or fixed-width code will achieve the retrieval objective.

## The proposed object

For object `x`, construct a positive measure `mu_x` over canonical occurrence
witnesses

```text
(content, gap position, ordered combination, object scale,
 direction/channel, interaction family, source anchor).
```

Edits act as transport plus destruction on this measure. A deletion destroys
the occurrences selecting its position and moves each surviving gap vector by
a standard-basis step. Substitution and transposition alter incidence/pattern
coordinates in the exact classes already proved in T-EDIT-1. A destruction sink
can make the augmented transport mass-conserving.

Apply separate nested aggregations along the content, position, combination,
scale, direction, and interaction axes. A support scale is then a multi-index

```text
m=(m_content,m_position,m_combination,m_scale,...),
```

and a serialized progressive code is only a chosen path through those indices.
At any chosen cell structure, test whether declared edit kernels close on the
cells. Exact closure is lumpability. Approximate closure is measured by the
commutation residual and is not accepted without separately declared task,
information, SNR, and collision gates.

## Strong confidence: proved structure or exact finite boundary

### Exact identity must remain outside the perceptual state

**GIVEN**, and reinforced by the support results. If the lossy state is required
to answer every exact, position-labelled mutation, it reconstructs literal
identity in the binary control systems. Keeping exact records authoritative is
not merely conservative system design; it is what permits the perceptual layer
to compress its action contract.

### Occurrence geometry is a sound research oracle

**HYPOTHESIS with strong formal and finite support**. The gap-simplex witness is
bijectional with an ordered occurrence. Its deletion, multi-deletion,
substitution, and transposition laws are exact, and E1/E3 exhaustively checked
the declared finite domains. I expect it to work as the inspectable reference
state from which compressed candidates are judged. That is not yet evidence
that it is the best practical retrieval basis.

### Multi-axis coarse/detail accounting will work mathematically

**HYPOTHESIS with complete paper proof**. Complementary projections on distinct
axes commute, and the tensor blocks give an exact orthogonal decomposition into
coarse state, single-axis details, and interaction details. This resolves a
conceptual mistake in the original scalar cylinder: one depth is not
fundamental. The remaining risk is semantic, not algebraic—whether useful
occurrence states admit economical concrete axis projections.

### Minimum exact support is relative to a flow contract

**HYPOTHESIS with complete paper proof and MEASURED finite support**.
T-CYBERNETIC-SUPPORT-1 gives the unique coarsest observation-preserving partition
closed under declared actions. E6 reproduces the sharp control-resolution
boundary exactly. This is the cleanest answer currently available to “are we
using too much support?” on a finite oracle.

### Progressive pre-quantized detail is support-neutral at finest dimension

**HYPOTHESIS with complete paper proof**. Coarse coefficients plus all successive
orthogonal detail coefficients telescope to exactly the finest scalar
dimension. Progressive access need not duplicate every scale before metadata,
quantization, margins, or indexes are counted. Quantized sign-only inheritance
does fail, so the statement must remain pre-quantized or margin-qualified.

### Atomic influence gives a necessary breakdown law

**HYPOTHESIS with a conditional proof**. If total required-edit response energy
is bounded by `E_total`, every one of `p` required atoms needs SNR at least
`Gamma`, and noise is `N`, then necessarily

```text
p <= E_total/(Gamma N).
```

This is a real upper boundary once the budgets are independently fixed. It is
not a sufficient capacity theorem and supplies no numerical content limit by
itself.

## Moderate confidence: mechanisms worth pursuing

### Sparse interaction detail

**HYPOTHESIS**. E2 proves that useful coupling can live outside separate
marginals: mod-2 and mod-3 address summaries collide where their joint address
does not. T-TENSOR-SUPPORT-1 identifies a place for that missing information as
an interaction-detail block. I expect selected low-order interactions to rescue
many such failures more economically than full fusion. We do not yet know which
interactions, how many, or whether the advantage survives an equal-bit control.

### Approximate lumpability as the compression objective

**HYPOTHESIS**. Exact partition refinement becomes literal support when the
control alphabet is too expressive. The useful next object is therefore the
smallest partition or linear projection satisfying simultaneous bounds on:

- deformation commutation residual;
- required-direction Jacobian support;
- atomic-edit SNR;
- conditional relevant-information loss;
- collision ambiguity after exact verification.

This looks more faithful than optimizing vector reconstruction error alone,
because it asks whether the compressed state preserves the flow used by the
task. The missing theorem is a bound from residual to candidate recall or task
loss under a declared state distribution.

### Averaged or equivalence-class deformations in candidate generation

**HYPOTHESIS with an exact toy-model mechanism**. E6 shows a large support drop
when position-labelled edits are replaced by an averaged edit channel: at the
largest checked deletion bound, 511 exact states become 45 flow states. I expect
perceptual retrieval to benefit from modelling edit classes rather than exact
edit addresses. The risk is loss of discriminative position evidence; exact
anchors and independent position channels must measure that loss rather than
assuming invariance is beneficial.

### Orthant containment and a degree ladder

**HYPOTHESIS with MEASURED finite evidence**. Exact deletion descendants remain
sound, full degree recovers exact subsequence containment, and low degrees
sharply reduced false parents in E4. I expect the degree ladder to be a useful
directional candidate channel. Low degree is demonstrably not a global
embedding certificate.

### Semantic cylinder routing

**CANDIDATE**. Structurally derived routes provide nested partitions, auditable
collisions, and progressive detail. I expect some routes to work, but confidence
is only moderate-to-low because no route has yet been instantiated and compared
against random, residue, lexical, and task-trained controls at equal support.

## Low confidence or deliberately open

- **Universal cross-modality basis:** no evidence says one content/position
  route serves text, images, audio, and arbitrary files at comparable quality.
- **One fixed width:** no selected noise floor, recall target, distribution,
  edit family, or conditional-information budget exists, so no numerical
  `n(B)` is justified.
- **Energy as information:** orthogonal detail energy and conditional relevant
  information both telescope, but one does not determine the other without a
  probabilistic observation model.
- **Static universal axis schedule:** different deformations may require
  incompatible refinement orders. A frozen route could still work, but this is
  an empirical question.
- **Automatic sparsity of interactions:** tensor decomposition names interaction
  blocks; it does not prove most are negligible.
- **Novelty:** no prior-art search was performed. The mathematics used as
  controls is classical in character, and the integrated construction remains
  unverified for novelty.

## What I would not pursue now

- A single fused fixed vector as the first mathematical object.
- One scalar scale path treated as the underlying geometry.
- Fine sign bits used as exact parents of coarser sign bits.
- Exact position-resolved mutation prediction inside the perceptual hash.
- Marginal entropies added as if they were independent contribution credit.
- A universal content limit derived from bit width without a declared loss and
  noise model.

Each has either an exact counterexample, an exact support explosion, or missing
assumptions substantial enough that optimization would be premature.

## Five falsifiable steps completed

These five steps were executed in
`docs/FALSIFIABLE_WORK_ROUND_002.md` using least-support obstructions. The list
below is retained as the originating program, not as outstanding work.

1. Build the exact finite occurrence-transition system for deletion,
   substitution, and transposition together; compute its coarsest partitions
   under several explicitly weakened observation/control contracts.
2. Instantiate two independent axis partitions on gap witnesses and verify the
   tensor interaction accounting against the E2 marginal-coupling failure.
3. Define a distribution-weighted approximate commutation residual, then plot
   support size against residual, hard-negative collisions, and exact-verified
   recall. Do not tune the gate after inspecting the test split.
4. Compare multiple paths through the same multi-index lattice at equal finest
   coefficient count. This tests whether scalar routing order matters only for
   progressive truncation, as the theory predicts.
5. Add quantization after the pre-quantized flow is understood; report margin
   failures and uncertified coarse bits rather than hiding them in aggregate
   recall.

The theory should be rejected or restricted if small-support projections cannot
simultaneously keep edit-flow residuals and hard-negative collision rates low.
That failure would be informative: it would show that the desired perceptual
equivalence itself requires more interaction or position support, rather than
that the implementation merely needs tuning.
