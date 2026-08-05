# T-INFLUENCE-1: contribution-energy and atomic SNR capacity

Status: **HYPOTHESIS with complete conditional proofs; the required energy and
noise budgets are not yet instantiated**.

## Continuous/pre-quantized statement

For a declared set of `p` required atomic edits, let response energy be

```text
I_i = ||Phi(e_i x)-Phi(x)||^2
```

and total contribution energy `E=sum_i I_i`. Then

```text
min_i I_i <= E/p.
```

If every edit must meet SNR threshold `Gamma` against noise energy `N>0`, so
`I_i>=Gamma N`, necessarily

```text
p <= E/(Gamma N).
```

### Proof

The minimum of nonnegative numbers is at most their average. If every term is at
least `Gamma N`, their sum is at least `p Gamma N`; rearrange. QED.

For one required edit at each of `n` positions, take `p=n`. For all `a-1`
replacements per position, take `p=n(a-1)`.

## Normalized binary statement

Let `h(x) in {-1/sqrt(B),+1/sqrt(B)}^B`. If an edit flips `r_i` bits, then

```text
I_i = 4r_i/B.
```

The least nonzero energy is `4/B`. Reliable detection at energy `Gamma N`
requires

```text
r_i >= ceil(B Gamma N/4).
```

If total flip incidence `R=sum_i r_i`, then some edit flips at most `R/p` bits,
and supporting all edits at the threshold requires

```text
p <= R/ceil(B Gamma N/4).
```

### Proof

Each flipped normalized sign coordinate changes by `2/sqrt(B)` and contributes
`4/B` squared energy. Sum over flipped coordinates and apply the preceding
average argument to integer flip counts. QED.

## Smoothness incompatibility

If every atomic edit is required to have squared binary response strictly less
than `4/B`, every response must be zero because `4/B` is the smallest nonzero
value. A binary hash cannot be both nonconstant on each atomic edit and smoother
than its one-bit quantum.

## Epistemic dependency

No finite content limit follows until `E` or `R`, `N`, `Gamma`, the norm, and the
required edit family are fixed independently of the observed failure.
