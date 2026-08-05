# T-GAP-1: gap-simplex occurrence bijection

Status: **HYPOTHESIS with complete paper proof; independent review and formal
encoding absent**.

## Construction

Fix a sequence `x` of length `n` and a degree `k` in `[0,n]`. An occurrence is
a strictly increasing index tuple

```text
I = (i_1, ..., i_k),  0 <= i_1 < ... < i_k < n.
```

Its content pattern is `(x[i_1],...,x[i_k])`. Its position is the gap vector

```text
g_0 = i_1
g_j = i_(j+1) - i_j - 1       for 1 <= j < k
g_k = n - i_k - 1.
```

For `k=0`, the sole gap vector is `(n)`.

## Statement

The map from an occurrence index tuple `I` to its gap vector `g` is a bijection
between degree-`k` occurrences in length `n` and the discrete simplex

```text
Delta(n-k,k+1) = {g in N^(k+1) : sum(g_j) = n-k}.
```

Consequently the gap vector plus object size and degree reconstructs the exact
occurrence indices. Adding the pattern retains content and order without
fusing them into the position coordinates.

## Proof

Every defined gap is nonnegative because the indices are strictly increasing
and lie inside the object. Telescoping the definition gives

```text
g_0 + ... + g_k = n-k.
```

For the inverse, set `i_1=g_0` and recursively
`i_(j+1)=i_j+g_j+1`. Nonnegative gaps make the indices strictly increasing.
The simplex sum makes the final right boundary gap exactly `g_k`, so the final
index is less than `n`. Substituting the constructed indices into the forward
definition returns every original gap coordinate. Conversely, the recurrence
applied to a forward gap vector returns every original index. The maps are
mutual inverses. QED.

## Significance and limit

This is an exact uncompressed coordinate system over occurrences. It is not a
claim of perceptual quality or novelty relative to all external literature.
The project's contribution target begins with the deletion law and later
multi-address projections built on this separation.
