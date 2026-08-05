# T-PROTECTED-KERNEL-1: minimum binary support for named deformation directions

Status: **HYPOTHESIS with complete finite proof; the row representative and
held-out protected graph remain configuration inputs**.

## Protected difference object

Flatten a degree-`m`, `w`-bit atom pattern into the vector space

```text
V = F_2^(mw).
```

Let `D` be a finite nonempty set of named nonzero pattern differences. A
`d`-row binary projection `L:V -> F_2^d` protects `D` exactly when

```text
L(delta) != 0 for every delta in D.
```

Equivalently, `ker L` is disjoint from `D`. The semantic minimum row support is

```text
d*(D) = min { codim K : K <= V and K intersection D = empty }.
```

This is the exact minimum-support object. It does not count dense occupancy
bits, postings, or useful candidate selectivity; those are later charges.

## Bounds

Let `r=rank(span D)`. Restricting `L` to that span is sufficient, so a basis
coordinate map gives

```text
d*(D) <= r.
```

Choose `d` independent uniformly random binary functionals. For each fixed
nonzero `delta`, the probability that all rows vanish is `2^-d`. The union
bound gives positive probability of avoiding every element of `D` whenever
`2^d>|D|`. Therefore

```text
d*(D) <= min(r, ceil(log2(|D|+1))).
```

This is an existence bound, not a claim that a particular greedy compiler
attains the minimum.

If protected pattern vertices form a graph whose edges are the named
differences, `L` is a proper coloring by at most `2^d` cells. Hence every
protected clique of size `q` gives the independent lower bound

```text
d*(D) >= ceil(log2 q).
```

## Least one-row obstruction

One binary row can protect any one or two distinct nonzero differences. For two
distinct vectors `u,v`, they are linearly independent over `F_2`, so the linear
conditions `f(u)=f(v)=1` have a solution.

Three differences first obstruct one row. Take

```text
D = {u, v, u+v}
```

for independent `u,v`. If one row were nonzero on all three, linearity would
force

```text
f(u+v) = f(u)+f(v) = 1+1 = 0,
```

a contradiction. Two coordinate rows protect the set, so `d*(D)=2`. This is
the least-support witness both in difference count and row depth.

## Ambient representative freedom

Only the restriction of a row to `W=span D` affects protected response. If rows
`a,b` have the same restriction to `W`, then

```text
a-b belongs to W^perp
```

and they act identically on every protected difference. Thus every selected
functional on `W` has an entire affine coset of ambient representatives.

This separates two compilation jobs:

1. select the smallest protected functional prefix on `W`;
2. choose representatives inside the corresponding cosets to spread support
   over unprotected atom bits and improve held-out behavior.

The second operation cannot damage the named development guarantees, but it
has no theorem of held-out relevance. `compile_protected_binary_rows` uses the
deterministic balanced control as a coset representative and audits the exact
protected restriction after lifting.

## Source-Jacobian lift

For a protected atom rewrite with exact code difference `e`, each certificate
slot supplies the flattened difference

```text
delta_s = e << (s w).
```

A protected transposition of atoms `a,b` in two selected slots supplies the
joint difference with `a xor b` in both slots. These columns protect event-level
pattern distinction. They do not prove occupancy-mask response because exact
history redundancy and exchange inside already occupied cells remain possible
under T-SYMBOL-JACOBIAN-1.

## Breakdown across row depth

For an ordered row family `L_1,...,L_d`, record

```text
Z_j = |{delta in D : (L_1,...,L_j)(delta)=0}|.
```

`Z_j` is monotone nonincreasing and the first `j` with `Z_j=0` is the semantic
protection depth of that row order. Increasing depth contracts no prior cell
distinction because the earlier code is a prefix. Candidate fanout may still
need a depth strictly larger than `d*(D)`.

## Boundary

The theorem answers the minimum rows required not to erase a finite declared
difference graph. It does not select `D`, a filename relevance policy, a noise
threshold, the number of occupancy cells needed for retrieval, or a posting
layout. A development graph can be perfectly protected while a disjoint graph
collapses; that generalization question is empirical and must be reported
separately.
