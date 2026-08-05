# T-STREAMING-JET-1: the deformation address has a linear streaming recurrence

Status: **HYPOTHESIS with complete paper proof and MEASURED reference timing;
optimized native implementation absent**.

## Factorized probe contract

Fix maximum occurrence degree `K`, deletion radius `T`, and one address probe.
The probe supplies:

- content weights `eta_j(symbol)` for selected slot `j`;
- gap weights `zeta_j` for the current gap after `j` selected symbols;
- multiplicative history weights `lambda_r` for source position `r`.

For a selected occurrence `I=(i_1,...,i_k)`, its content phase factors as

```text
product_(j=1)^k eta_j(x_(i_j)).
```

The history-bound address polynomial is

```text
A_k(s)
  = sum_(I, |I|=k)
      product_(j=1)^k eta_j(x_(i_j))
      product_(r notin I)
        (zeta_(#{i in I : i<r}) + s lambda_r).
```

Its coefficient `[s^t]A_k` is the factorized content/gap probe of the
radius-`t` history superposition from T-HISTORY-COUPLING-1.

## Streaming recurrence

After reading the first `r` source positions, let `F_(r,j,u)` be the accumulated
weight of paths that selected `j` positions and deleted `u`. Initialize

```text
F_(0,0,0)=1
```

and all other states to zero. For source symbol `x_r`,

```text
F_(r+1,j,u)
  = zeta_j F_(r,j,u)
    + lambda_r F_(r,j,u-1)
    + eta_j(x_r) F_(r,j-1,u),
```

where negative indices are zero and `eta_j` is one-based. Then

```text
F_(n,k,t) = [s^t] A_k(s).
```

### Proof

Every source position has exactly one of three roles in a contributing term:

1. it is unselected and retained in the current gap, contributing `zeta_j`;
2. it is unselected and deleted, contributing `s lambda_r`;
3. it is selected as content slot `j`, contributing `eta_j(x_r)`.

The three predecessor states in the recurrence are disjoint and exhaustive for
those roles. Induction over source positions therefore sums every occurrence
and compatible deletion set exactly once with the required product weight.
Extracting the state with final selected degree `k` and deletion count `t`
gives the coefficient. QED.

## Iteration collapse

For `Q` independently addressable probes, a direct history/occurrence oracle
visits

```text
sum_(t=0)^T C(n,t) C(n-t,k)
```

occurrences at one degree. Occurrence-product evaluation removes the history
loop but still visits `C(n,k)` source occurrences.

The recurrence visits at most

```text
n (K+1) (T+1) Q
```

state/probe cells and uses

```text
2 (K+1) (T+1) Q
```

working scalars. One table emits every degree `0<=k<=K`; separate passes by
degree are unnecessary when the probe weights share the declared prefix law.

This is linear in object length for fixed `K`, `T`, and `Q`. It does not iterate
over deletion histories or occurrences.

## Fast-form consequences

- Store the two state slabs contiguously and reuse them; the hot path allocates
  nothing.
- Lay probes in structure-of-arrays order so the `Q` dimension is the vectorized
  inner operation.
- Precompute `lambda_r` or generate polynomial moment phases by finite
  differences; do not exponentiate in the hot loop.
- Specialize small frozen `K,T` configurations so radius and degree loops can be
  unrolled.
- Quantize only after the full pre-quantized recurrence; early quantization
  breaks the exact sum/product identity.

For frozen radius `T`, a rectangular specialization may assign every one of the
`n(K+1)(T+1)` cells, including unreachable zero boundary cells. This preserves
the recurrence while removing reachability bounds and the radius-loop branch.
E8 measures the radius-two reference form. T-HISTORY-QUOTIENT-1 supplies a
different run-block recurrence when history multiplicity is quotient rather
than retained.

## Boundary

The collapse requires factorized content weights, gap weights determined by the
current selected-slot count, and multiplicative history phases. An arbitrary
table over complete patterns or deletion sets does not fit this state without
additional dimensions. That restriction is part of hash identity and must face
equal-support retrieval controls.
