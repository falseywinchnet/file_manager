# T-JACOBIAN-SCALE-1: affine response flow through scale

Status: **HYPOTHESIS with complete conditional paper proof; concrete Kolmogrov
support map, thresholds, and independent review absent**.

## Finite-edit Jacobian

For required edits `e_1,...,e_p`, define the secant matrix at scale `m` by

```text
J_m(x)[:,i] = Phi_m(e_i x)-Phi_m(x).
```

For a differentiable relaxed object space, the same statements hold for the
Fréchet derivative restricted to a declared finite-dimensional direction
subspace.

## Exact projective flow

If `Phi_m=A_m Phi_(m+1)` for every object, then

```text
J_m(x) = A_m J_(m+1)(x).
```

### Proof

For every edit column,

```text
Phi_m(e_i x)-Phi_m(x)
= A_m Phi_(m+1)(e_i x)-A_m Phi_(m+1)(x)
= A_m J_(m+1)(x)[:,i].
```

Collect columns. QED.

Because `A_m` is a contraction, every coefficient direction `v` satisfies

```text
||J_m v|| <= ||J_(m+1) v||.
```

Thus coarse scale cannot create pre-quantized affine response energy under an
exact contraction tower.

## Exact Hilbert breakdown

At `p=2`, with detail projector `D_m` from T-SCALE-1:

```text
||J_(m+1)v||^2 = ||J_m v||^2 + ||D_m J_(m+1)v||^2.
```

This is the exact response-energy loss across one scale. Iteration decomposes a
direction into its coarsest response plus every intervening detail contribution.

## Thresholded directional support

For noise/separation threshold `tau>0`, define

```text
d_m(tau) = #{singular values sigma_i(J_m) >= tau}.
```

Then `d_m(tau)<=rank(J_m)<=K_m`. A scale claiming to support `d` affine
directions above threshold must have `d_m(tau)>=d`, hence `K_m>=d`.

In Hilbert spaces, every ordered singular value is nonincreasing under exact
coarsening:

```text
sigma_i(J_m) <= sigma_i(J_(m+1)),
d_m(tau) <= d_(m+1)(tau).
```

Indeed `A_m^*A_m <= I`, so
`J_(m+1)^* A_m^*A_m J_(m+1) <= J_(m+1)^*J_(m+1)` in positive-semidefinite
order. Eigenvalue monotonicity gives the squared singular-value inequalities.
Equality is allowed; the theorem asserts no strict loss.

## Approximate projective flow

If


```text
Phi_m(x)=A_m Phi_(m+1)(x)+r_m(x),
```

then each edit column obeys

```text
J_m[:,i]=A_m J_(m+1)[:,i] + r_m(e_i x)-r_m(x).
```

Therefore an observed coarse response contains contracted fine response plus a
measurable projective-residual response. The residual must not be silently
called fine-scale information.

## Boundary

The binary hash has no useful classical Jacobian. These laws apply before
quantization or to declared finite-edit differences after quantization. The
threshold `tau` must derive from a noise and task model, not from the desired
direction count.
