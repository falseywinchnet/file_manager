# T-DELETION-ORBIT-1: deletion descendants form an exact differential orbit

Status: **HYPOTHESIS with complete paper proof; independent review and compact
probe selection absent**.

## Occurrence polynomial

Let `x` have length `n`, fix degree `k`, and fix a content pattern `p` of length
`k`. For every occurrence `I` of `p`, let `g(I)` be its `k+1` gap coordinates
from T-GAP-1. Define the homogeneous occurrence polynomial

```text
G_(x,k,p)(z_0,...,z_k) = sum_(I : x_I=p) z^g(I).
```

Its degree is `d=n-k`. Let

```text
D = partial_(z_0) + ... + partial_(z_k).
```

For a source-position set `R`, write `x\R` for simultaneous deletion of those
positions with the order of all survivors retained.

## Exact orbit-sum theorem

For every `0<=t<=n-k`,

```text
sum_(R subseteq {0,...,n-1}, |R|=t) G_(x\R,k,p)(z)
    = (1/t!) D^t G_(x,k,p)(z).
```

Thus the complete unlabelled radius-`t` deletion orbit is a derivative of the
source support object, not an independently enumerated family.

### Proof

Fix one source occurrence with gap vector `g`. A deletion set preserves that
occurrence exactly when it contains only unselected positions. Let `alpha_j`
count deletions in gap `j`. For `|alpha|=t`, there are

```text
product_j C(g_j,alpha_j)
```

such deletion sets, and each produces target monomial `z^(g-alpha)`. Meanwhile,

```text
(1/t!) D^t
  = sum_(|alpha|=t) (1/alpha!) partial^alpha,
```

and

```text
(1/alpha!) partial^alpha z^g
  = product_j C(g_j,alpha_j) z^(g-alpha).
```

This is the same contribution for every source occurrence. Occurrences
selecting a deleted position contribute to neither side. Summing over source
occurrences proves the identity. QED.

## The whole orbit is a translation

Because `D` is directional differentiation along the all-ones gap direction,

```text
sum_(t=0)^(n-k) s^t
  sum_(|R|=t) G_(x\R,k,p)(z)
    = exp(sD) G_(x,k,p)(z)
    = G_(x,k,p)(z+s*1).
```

One translated occurrence polynomial therefore contains every deletion radius
as a coefficient in `s`. This is an exact algebraic identity, not a smoothness
analogy.

## Intrinsic contraction and survival mass

Uniformly average over all `t`-element deletion sets. Since
`t! C(n,t)=n^(underline t)`, define

```text
J_t G = D^t G / n^(underline t).
```

Then

```text
J_t G_(x,k,p)
  = average_(|R|=t) G_(x\R,k,p).
```

For a nonnegative coefficient polynomial,

```text
||J_t G||_1
  = ((n-k)^(underline t) / n^(underline t)) ||G||_1.
```

The factor is exactly the probability that a uniformly chosen `t`-deletion set
avoids a fixed degree-`k` occurrence. For signed content projections, the same
ratio is an `l_1` operator-norm upper bound.

At one deletion the survivor operator is `D/n`. Augment it with `k` typed
destruction outlets per source occurrence, one for deletion of each selected
slot. A monomial then sends gap mass `g_j/n` to survivor `z^(g-e_j)` and mass
`1/n` to each selected-slot outlet. The outgoing column mass is

```text
(sum_j g_j + k)/n = 1.
```

This is an occurrence-derived Banach contraction with explicit destruction,
not a generic block average imposed after the fact.

## The concealed closed state is a jet

Let `A_n=D/n` be one uniformly averaged deletion step. With the object length
decreasing after each step,

```text
J_t(A_n G) = J_(t+1)G.
```

Indeed,

```text
D^t(DG/n) / (n-1)^(underline t)
  = D^(t+1)G / n^(underline (t+1)).
```

Therefore

```text
(J_0G, J_1G, ..., J_TG)
```

is an exact deletion-flow state through radius `T`: one averaged deletion shifts
the components left. Its entire closure defect is the single omitted boundary
component `J_(T+1)G`. Applying any fixed linear address probe to every component
preserves the same shift law before quantization.

## Granular fuzzy tail

Place the radius components in a direct sum and weight radius `t` by `rho^t`,
with `0<=rho<1`. Since

```text
(n-k)^(underline t) / n^(underline t)
  <= ((n-k)/n)^t,
```

discarding all radii above `T` has coefficient-mass bound

```text
||tail_(>T)||_1
  <= ||G||_1
     * (rho*(n-k)/n)^(T+1)
     / (1-rho*(n-k)/n).
```

This supplies a construction-specific fuzzy scale: `rho` controls deformation
reach, `T` controls stored jet depth, and the bound names the exact discarded
orbit mass. It does not by itself guarantee retrieval separation.

## Boundary

The derivative orbit forgets which deletion set produced which evidence. It is
exact for the unlabelled sum or uniform average, but not for a protected
individual deletion history. T-HISTORY-COUPLING-1 identifies the least failure
and the history-bound lift required before compact projection.
