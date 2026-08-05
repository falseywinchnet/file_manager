# T-TENSOR-SUPPORT-1: multi-axis coarse/detail and interaction decomposition

Status: **HYPOTHESIS with complete finite-Hilbert paper proof; concrete semantic
axis bases and independent review absent**.

## Statement

Let `H=H_1 tensor ... tensor H_d`. On each axis let `P_i` be an orthogonal
coarse projection and `D_i=I-P_i` its detail projection. For every subset
`S subseteq {1,...,d}`, define

```text
Q_S = tensor_i (D_i if i in S else P_i).
```

Then:

1. every `Q_S` is an orthogonal projection;
2. `Q_S Q_T=0` for `S!=T`;
3. `sum_S Q_S=I`;
4. every state has the unique orthogonal decomposition
   `z=sum_S Q_S z`;
5. `||z||^2=sum_S ||Q_S z||^2`.

`S=empty` is the all-axis coarse state. Singleton subsets are axis-specific
details. Subsets of size at least two are explicit interaction details.

## Proof

On each axis, `P_i` and `D_i` are orthogonal complementary projections. Tensor
products of self-adjoint idempotents are self-adjoint idempotents. If `S!=T`,
some axis uses `P_i` in one product and `D_i` in the other, whose product is
zero; hence the tensor products are orthogonal. Expanding

```text
tensor_i(P_i+D_i)
```

gives `sum_S Q_S`, while every factor equals `I`, proving the identity sum.
Orthogonal decomposition and Pythagoras follow. QED.

## Commuting axis contractions

An axis contraction acting as `A_i` on factor `i` and identity elsewhere
commutes with every contraction on a different axis. Therefore a multi-index
scale `m=(m_1,...,m_d)` is fundamental; a one-dimensional refinement schedule
is merely a path through this lattice.

## Dimension

If `p_i=rank(P_i)` and `q_i=rank(D_i)`, then

```text
dim(range(Q_S)) = product_(i in S) q_i * product_(i not in S) p_i.
```

The dimensions sum to `product_i(p_i+q_i)=dim(H)`.

## Research consequence

Content, position, combination, and scale marginals correspond to selected
axis-coarse/detail blocks. Coupling certificates are not ad hoc cross-features:
they are selected interaction blocks `Q_S`. Omitting all multi-axis details
retains marginals and can lose exactly the coupling observed in E2.

## Boundary

The theorem supplies orthogonal accounting, not a semantic choice of axes or a
claim that the true feature measure factors statistically. Entropic interaction
does not inherit this nonnegative orthogonal decomposition automatically.
