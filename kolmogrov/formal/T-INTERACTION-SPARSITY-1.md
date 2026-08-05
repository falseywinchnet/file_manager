# T-INTERACTION-SPARSITY-1: universal interaction fidelity needs full dimension

Status: **HYPOTHESIS with complete finite linear proof; relevant interaction
family absent**.

## Setup

Let measures live on `A x B`, with `|A|=p>=2`, `|B|=q>=2`. Let `K` be the
linear space of signed measures whose row and column marginals are zero.
T-INTERACTION-MIN-1 gives

```text
dim(K)=(p-1)(q-1).
```

Let a linear interaction certificate `L` have `r` scalar outputs. Call it
universally faithful when any two measures with the same marginals and the same
certificate are equal.

## Statement

Every universally faithful linear certificate has

```text
r >= (p-1)(q-1).
```

The full rectangle-basis coordinates attain equality.

## Proof

Universal faithfulness is exactly injectivity of `L` restricted to `K`.
Rank-nullity requires `r>=dim(K)`. If `r<dim(K)`, choose nonzero
`Delta in K intersect kernel(L)`. Start from any strictly positive probability
measure `mu_0` and take sufficiently small `eta>0`; then

```text
mu_+=mu_0+eta Delta,
mu_-=mu_0-eta Delta
```

are distinct nonnegative probability measures with equal marginals and equal
certificates. QED.

## Least failure of nontrivial sparsity

For a `2 x 2` product, the interaction space has dimension one, so one scalar is
already full interaction support. The `2 x 3` product is the least product with
interaction dimension two; every one-scalar linear certificate misses a
nonzero marginal-invisible difference.

## Task-restricted replacement

If all relevant signed differences are known to lie in a subspace `W subseteq
K`, universal fidelity on that declared family needs at least `dim(W)` linear
coordinates and can attain that dimension. For a finite named obstruction set,
coordinate-selected rectangle certificates instead form the hitting problem in
T-INTERACTION-MIN-1.

Thus interaction sparsity is never automatic. It must come from a restricted
task family, low-dimensional relevant span, tolerated ambiguity, or a finite
protected obstruction set.

## Falsification gate

A universal sparse-interaction claim fails by rank whenever its certificate
dimension is below `(p-1)(q-1)`. A task-restricted claim fails when a relevant
held-out difference lies in the certificate kernel.
