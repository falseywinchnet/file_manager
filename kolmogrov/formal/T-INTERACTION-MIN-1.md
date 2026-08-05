# T-INTERACTION-MIN-1: least marginal-invisible interaction support

Status: **HYPOTHESIS with complete paper proof and an E2 witness; independent
review absent**.

## Statement

Let `mu,nu` be finite measures on a product `A x B` with identical `A` and `B`
marginals, and let `Delta=mu-nu` be nonzero. Then `Delta` has at least four
nonzero joint cells. Equality is possible only on a two-by-two rectangle with
alternating signs and equal absolute mass:

```text
+q  -q
-q  +q
```

up to row/column ordering and global sign.

Thus four joint cells are the least support on which coupling can change while
both marginals remain fixed. One checkerboard interaction coefficient detects
the equality case.

If `A,B` are finite, these four-cell rectangles span the entire
marginal-invisible subspace, whose dimension is

```text
(|A|-1)(|B|-1).
```

## Proof

Represent each nonzero cell of `Delta` as an edge between its `A` row and `B`
column. Zero row and column sums imply that every incident vertex has degree at
least two; a degree-one vertex could not balance its nonzero edge. A finite
bipartite graph of minimum degree two contains a cycle, and its shortest
possible cycle has four edges. Hence support is at least four.

With exactly four edges, the support graph is one four-cycle. Zero sum at each
of its four vertices forces alternating signs with one common magnitude. The
displayed rectangle attains the bound. QED.

## Rectangle basis proof

Fix reference cells `a0 in A`, `b0 in B`. For every `a!=a0,b!=b0`, define

```text
R_(a,b) = e_(a,b)-e_(a,b0)-e_(a0,b)+e_(a0,b0).
```

Every `R_(a,b)` has zero marginals. For any `Delta` with zero marginals,

```text
Delta = sum_(a!=a0,b!=b0) Delta(a,b) R_(a,b).
```

The nonreference/nonreference cell `(a,b)` occurs in only its own basis
rectangle, proving both the identity there and linear independence. The zero
row/column sums force the reference row, reference column, and corner values to
match as well. Hence the rectangles form a basis and give the stated
dimension. QED.

## Least interaction inside the E2 failure

For the E2 strings

```text
x = aaabbabbaaa
y = bbaaaaaaabb,
```

fix degree-two pattern `bb`. Use the complete mod-2 gap tuple as row and the
complete mod-3 gap tuple as column. On rows

```text
a0=(0,0,1), a1=(1,0,0)
```

and columns

```text
b0=(0,2,1), b1=(1,2,0),
```

the joint occurrence counts are exactly

```text
mu_x = [[0,1],       mu_y = [[1,0],
        [1,0]]               [0,1]].
```

Their row and column counts agree, but the checkerboard interaction has
opposite sign. This is a least-support kernel element inside the larger E2
collision, not merely a large-string collision report.

## Consequence

The minimum repair for this pair is one interaction contrast, not the entire
joint address. Whether one contrast generalizes is a separate empirical claim.
An interaction selector fails when a held-out marginal collision has zero value
on every retained contrast but nonzero task consequence.

For a frozen family of collision obstructions, sparse interaction selection is
therefore a hitting problem: each obstruction names the rectangle-basis
coordinates on which its signed difference is nonzero; select the fewest
coordinates hitting every required obstruction. This is an exact finite support
objective. Generalization to unseen obstructions remains unproved.
