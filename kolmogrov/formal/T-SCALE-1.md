# T-SCALE-1: Banach contraction and Hilbert detail-energy law

Status: **HYPOTHESIS with complete paper proof; independent review and formal
encoding absent**.

## Definitions

Fix block factor `b >= 2`. Let `K_m=b^m` and
`X_m=R^(K_m)` with `l_p` norm. Partition `z in X_(m+1)` into `K_m` consecutive
blocks of size `b`.

For `1 <= p < infinity`, define

```text
(A_m z)_j = b^(1/p-1) sum_(r=0)^(b-1) z_(bj+r).
```

Define `(R_m y)_(bj+r)=b^(-1/p)y_j`. For `p=infinity`, let `A_m` be block
average and `R_m` block repetition.

## Banach statement

For every `p in [1,infinity]`:

1. `||A_m|| <= 1`;
2. `A_m R_m=I`;
3. `R_m` is an isometry;
4. compositions `A_(m<-M)=A_m...A_(M-1)` are contractions.

## Proof

For finite `p`, Hölder on one block gives

```text
|sum_r z_r| <= b^(1-1/p) (sum_r |z_r|^p)^(1/p).
```

Multiplying by `b^(1/p-1)`, raising to `p`, and summing blocks yields
`||A_m z||_p^p <= ||z||_p^p`. The `p=infinity` block average is bounded by the
block maximum. Direct substitution gives `A_m R_m y=y`. For finite `p`, each
block of `R_m y` contributes `b*b^(-1)|y_j|^p=|y_j|^p`; repetition preserves the
infinity norm. Composition of contractions is a contraction. QED.

## Hilbert statement

At `p=2`, `R_m=A_m^*`, `P_m=R_m A_m` is an orthogonal projection, and with
`D_m=I-P_m`:

```text
||z||_2^2 = ||A_m z||_2^2 + ||D_m z||_2^2.
```

For a projectively consistent tower `Phi_r=A_r Phi_(r+1)`:

```text
||Phi_M||^2 = ||Phi_m||^2
            + sum_(r=m)^(M-1) ||D_r Phi_(r+1)||^2.
```

## Proof

The definitions give the adjoint identity and `A_m A_m^*=I`. Hence `P_m` is
self-adjoint and idempotent. Its range and the range of `D_m` are orthogonal.
Moreover `||P_m z||=||A_m z||` because `R_m` is an isometry. Pythagoras gives
the one-level identity; recursive substitution telescopes it. QED.

## Scope

This theorem supplies a support-space tower. It does not prove that a particular
Kolmogrov feature map is projectively consistent, that scale blocks should be
contiguous, or that binary quantization commutes with `A_m`.
