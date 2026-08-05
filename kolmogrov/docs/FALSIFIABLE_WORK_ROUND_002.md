# Falsifiable work round 002: five least-support boundaries

Status: **HYPOTHESIS-bearing research round; no architecture decision**.

Date: 2026-08-05.

## 1. Joint occurrence/edit flow

Object tested: ordered subsequence multiplicities through degree two under
uniform deletion, substitution, and adjacent transposition with edit position
hidden.

Least obstruction:

```text
0110 and 1001
```

They first collide at length four and have different uniform-deletion laws over
the same observation. No shorter binary obstruction exists. Through length
four, splitting this one cell is both necessary and sufficient for exact flow
closure.

Result: **REJECTED** the idea that a finer occurrence observation necessarily
remains closed under an averaged edit contract. Refinement can expose dynamic
differences and increase required support.

Proof object: `formal/T-JOINT-FLOW-1.md`.

## 2. Two-axis interaction support

Object tested: separate mod-2 and mod-3 gap-address marginals.

Least obstruction: a nonzero signed joint measure with both marginals zero
requires four cells, forming an alternating two-by-two rectangle. The E2 string
pair contains exactly such a rectangle in its `bb` occurrences:

```text
x: [[0,1],    y: [[1,0],
    [1,0]]        [0,1]].
```

Result: one checkerboard interaction coefficient rescues this collision pair.
This establishes the least repair for the pair, not a universal interaction
basis.

The four-cell rectangles span the complete marginal-invisible subspace. For a
frozen obstruction family, minimum interaction support becomes a hitting
problem over the rectangle coordinates on which each obstruction is nonzero.
This supplies an exact selection object while leaving held-out generalization
falsifiable.

Proof object: `formal/T-INTERACTION-MIN-1.md`.

## 3. Approximate closure versus task loss

Object tested: distribution-weighted total-variation commutation residual.

Least obstruction: three fine states, with two merged states taking different
coarse transitions. Giving the exceptional state probability `epsilon` makes
the optimal average residual `epsilon` while its conditional recall is zero.
Three states are necessary: two to merge and one distinct target cell.

Result: **REJECTED** average dynamic residual as a stand-alone breakdown gate.
It controls expected Lipschitz readouts under the same distribution, but it
does not control rare-state, protected-class, or worst-case recall.

Proof object: `formal/T-APPROX-CLOSURE-1.md`.

## 4. Equal-support lattice paths

Object tested: two commuting binary axis contractions, compared at the same
two-coordinate intermediate and four-coordinate final support.

Least obstruction: a mass-conserving edit secant needs two atoms. A two-atom
position secant is annihilated by content-first truncation but partly retained
by position-first truncation; a two-atom content secant reverses the preference.

Result: commuting contractions make the final state path-independent, not the
progressively truncated state. **REJECTED** a universal path order derived from
commutativity alone.

Proof object: `formal/T-PATH-ORDER-1.md`.

## 5. Quantization after flow

Object tested: whether fine quantized values determine a coarse contraction
sign.

Least obstruction: two coordinates. For any `eta>0`, `(1+eta,-1)` and
`(1,-1-eta)` have the same fine signs and opposite coarse signs. One positive
contraction coordinate cannot fail this way.

Result: the exact replacement is an interval certificate. A coarse bit is
emitted only when the contracted feasible interval lies wholly on one side of
zero; otherwise it is explicitly uncertified. Certificate bytes are support
and must be charged.

Proof object: `formal/T-QUANT-CERT-1.md`.

## Revised algorithmic-support candidate

The five obstructions narrow the candidate to this sequence:

1. Construct the exhaustive occurrence measure and declared edit kernels.
2. Choose the observation contract; compute or characterize its least exact
   flow-closed refinement.
3. Allocate content, position, combination, and scale support independently.
4. Add only interaction contrasts that repair named marginal-invisible
   obstructions.
5. If exact support is too large, merge cells under both average and protected
   statewise residual gates; charge the task consequence separately.
6. Serialize a path only after required affine direction energies determine
   which axis details must arrive first.
7. Quantize last, carrying interval/margin certificates and an explicit
   uncertified state.

## Confidence after the round

Strong confidence attaches to the boundaries, not yet to retrieval performance:

- exact minimum support is flow-contract relative;
- interaction loss begins in four-cell cycles;
- average residual cannot protect rare states;
- path order matters before the common finest state;
- two-cell contractions require amplitude information for exact quantized
  ancestry.

Moderate confidence attaches to the constructive synthesis: least closed
occurrence support plus a sparse cycle basis, protected approximate merging,
direction-conditioned progressive order, and interval-certified quantization.

Low confidence remains on the actual semantic partitions, how many interaction
cycles generalize, the task distribution and protected set, and whether the
result beats equal-support controls on retrieval. Those are the next empirical
objects; aggregate evaluation is warranted only after their contracts are
frozen.
