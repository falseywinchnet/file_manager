# T-PROGRESSIVE-SUPPORT-1: coarse-plus-detail support has finest dimension

Status: **HYPOTHESIS with complete Hilbert-space paper proof; semantic routing,
quantization, and independent review absent**.

## Statement

Under T-SCALE-1 at `p=2`, let `A_m:X_(m+1)->X_m` be the coisometry,
`R_m=A_m^*`, and `W_m=ker(A_m)`. Then:

1. `X_(m+1)=range(R_m) direct-sum W_m` orthogonally;
2. every `z_(m+1)` has unique decomposition
   `R_m A_m z_(m+1)+D_m z_(m+1)`;
3. `dim(W_m)=K_(m+1)-K_m`;
4. through level `M`, one coarse coefficient block plus every detail block has
   total dimension exactly `K_M`;
5. truncating after level `m` is orthogonal projection and its squared error is
   the sum of discarded detail energies.

## Proof

T-SCALE-1 makes `P_m=R_m A_m` the orthogonal projection onto `range(R_m)` and
`D_m=I-P_m` its orthogonal complement. For a coisometry,
`ker(A_m)=range(R_m)^perp`, proving the direct sum and unique decomposition.
Rank-nullity gives

```text
dim(W_m)=K_(m+1)-rank(A_m)=K_(m+1)-K_m,
```

because `A_m R_m=I` makes `A_m` surjective. Dimension telescopes:

```text
K_0 + sum_(r=0)^(M-1)(K_(r+1)-K_r)=K_M.
```

Orthogonality across recursively refined detail spaces gives the truncation
error identity from repeated Pythagoras. QED.

## Quantization boundary

The theorem counts real scalar coefficients. It does not say that one bit per
coefficient is adequate, that detail coefficients share a quantizer, or that
coarse signs can be recovered from fine signs. T-QUANT-SCALE-1 still applies.
Progressive binary support must store/co-derive coarse coefficients and detail
coefficients with declared margins and exact serialized-bit accounting.
