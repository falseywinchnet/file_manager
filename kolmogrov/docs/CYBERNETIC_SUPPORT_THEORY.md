# Cybernetic support theory: observations, controls, and deformation flow

Status: **CANDIDATE high-level theory with exact finite-state support theorems;
approximate perceptual closure unresolved**.

## 1. The support state answers a flow contract

A compressed state is sufficient only relative to:

- what must be observed;
- which control/action labels are visible;
- what transition or deformation outcomes must be predicted;
- the tolerated error/noise;
- the scale at which those statements hold.

Changing the action alphabet changes minimum support even when the object set
and observation do not change.

## 2. Exact minimum support

On a finite oracle, the coarsest observation-preserving strongly lumpable
partition is the exact minimum state space with closed controlled dynamics.
T-CYBERNETIC-SUPPORT-1 constructs it by partition refinement and proves
minimality.

This is a theorem-bearing answer to “are we using too much support?” for a
declared finite flow contract. A proposed representation uses excess exact
support when it has more cells than this partition; it uses insufficient exact
support when its cells merge different minimum blocks.

## 3. Control resolution is not metadata

T-ACTION-SUPPORT-1 exposes the boundary sharply:

- Hamming weight plus every position-labelled flip forces all `2^n` literal
  states.
- Hamming weight plus one uniformly averaged flip kernel needs only `n+1`
  states.
- Length/weight plus position-labelled deletion forces every literal string
  through the bound.
- Length/weight plus uniformly averaged deletion needs only the triangular
  `(length,weight)` state set.

Position labels carry information about affine directions. Demanding exact
response to each one defeats perceptual compression by definition. Hiding them
permits compression but gives up controlled prediction. The product contract
must choose consciously.

## 4. Occurrence measure as the Kolmogrov state object

For symbolic object `x`, let `mu_x` be the positive measure over canonical
gap-simplex witnesses, with separate coordinates for:

- content pattern;
- gap position;
- ordered combination/degree;
- object scale;
- channel/direction;
- explicit interaction family.

Deletion acts on this measure as a substochastic transport: witnesses selecting
the deleted position enter a destruction sink; every survivor preserves pattern
and moves gap mass by a standard-basis step. Including the sink makes the
occurrence flow mass-conserving. Substitution and transposition alter pattern
coordinates without gap transport according to T-EDIT-1.

This measure, rather than one already-fused vector, is the proposed fundamental
algorithmic support object.

## 5. Multi-axis scale lattice

Content, position, combination, and scale should not share one compulsory depth.
Use a multi-index scale

```text
m = (m_content, m_position, m_combination, m_scale, ...).
```

Axis contractions commute. T-TENSOR-SUPPORT-1 decomposes a refinement into:

- all-axis coarse support;
- single-axis detail;
- pairwise interaction detail;
- higher interaction detail.

The single semantic-cylinder route in `ALGORITHMIC_SUPPORT_BASIS.md` is a
serialization path through this lattice, not the underlying geometry.

E2's marginal residue failure is the empirical model: mod-2 and mod-3 marginals
lost which residues co-occurred. In the tensor theory, that missing information
is an interaction-detail block. A sparse coupling certificate retains selected
interaction support without fusing every axis.

## 6. Dynamic breakdown across scale

Let `C_m` aggregate the exhaustive occurrence state and `T_a` be a declared
deformation operator. Exact coarse closure requires a coarse operator satisfying

```text
C_m T_a = Tbar_(a,m) C_m.
```

The commutation residual is the scale's dynamic breakdown. Report it by action,
channel, scale, and required direction. The Jacobian/detail energy, conditional
relevant information, and transition residual remain different ledgers.

Approximate closure should admit a cell merger only when all of these pass their
declared gates:

1. transition residual under the task norm;
2. required-edit SNR;
3. thresholded affine direction support;
4. relevant conditional information loss;
5. collision/ambiguity behavior against exact anchors.

T-APPROX-CLOSURE-1 adds a necessary qualification: a distribution-weighted
residual controls expected Lipschitz readouts under that distribution, but can
be arbitrarily small while a rare protected state fails completely. Every
approximate merger therefore needs a statewise tail or protected-hard-negative
gate in addition to its average.

T-JOINT-FLOW-1 also shows that observation refinement is not dynamically
monotone. Degree-one content is closed under averaged binary edits, while the
first degree-two collision at length four is not. Support must be closed after
the observation is chosen; “more observable detail” is not automatically a
better compressed state.

## 7. What the theory predicts will work

- Exact anchors plus a lossy occurrence-flow state, rather than asking the hash
  to be identity.
- Multi-axis nested partitions, because deformations touch content, position,
  order, and scale differently.
- Explicit interaction details, because independent marginals lose coupling.
- Progressive coarse/detail coefficients before quantization.
- Per-action and per-scale residual accounting, because no single norm explains
  every breakdown.
- Averaged/equivalence-class deformation controls for perceptual candidate
  generation; exact position-resolved controls only in the authoritative layer.
- Least four-cell interaction cycles rather than an unexamined full tensor.
- Protected-state residual gates rather than average commutation error alone.
- Direction-conditioned progressive paths rather than one universal axis order.
- Interval-certified coarse quantization with an explicit uncertified outcome.

## 8. What remains conjectural

- the semantic partition trees that minimize task loss at fixed support;
- the smallest interaction blocks preserving collision rescue;
- an approximate lumpability theorem tied to retrieval recall and SNR;
- whether one route generalizes across modalities;
- a quantizer/margin code preserving the projective flow economically;
- a bridge from geometric contribution energy to relevant information or
  practical description complexity.
