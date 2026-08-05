# T-HISTORY-MOMENTS-1: bounded deletion histories admit multiplicative moment codes

Status: **HYPOTHESIS with complete finite-field paper proof; compact character
subfamily and retrieval value absent**.

## History moment code

Let a deletion history be a `t`-element source-position set

```text
R subseteq {0,...,n-1}.
```

For prime `p`, define its first `t` power sums in `F_p`:

```text
c_p(R)_j = sum_(r in R) r^j mod p,    1<=j<=t.
```

## Exact bounded-range statement

If

```text
p > max(n-1,t),
```

then `c_p` is injective on the `t`-element deletion histories.

### Proof

Let `s_j` be the power sums and `e_j` the elementary symmetric functions of the
positions in `R`, with `e_0=1`. Newton's recursion gives, for `1<=m<=t`,

```text
m e_m = sum_(j=1)^m (-1)^(j-1) e_(m-j) s_j.
```

Because `p>t`, every `m` is invertible in `F_p`. The first `t` power sums
therefore determine every `e_m`, hence the monic polynomial

```text
P_R(X) = product_(r in R)(X-r)
       = X^t-e_1 X^(t-1)+...+(-1)^t e_t.
```

Equal moment codes give equal polynomials and equal root multisets in `F_p`.
Because `p>n-1`, the source positions embed distinctly in `F_p`; the original
integer position sets are equal. QED.

## Serialized support consequence

An explicit exact code uses `t` residues modulo `p`, hence

```text
B_history = t ceil(log_2 p)
```

bits before headers. Choosing the least admissible prime makes the exact
bounded-range cost scale as `t log n`, not as an `n`-bit deletion mask. This is
a history-identity bound, not yet the cost of a perceptual superposition hash.

For `t=1`, any modulus `p<n` aliases source positions `r` and `r+p` whenever
both exist. Thus a fixed exact singleton-history residue has an unavoidable
content-length range. Multiple smaller moduli may rescue one another while
remaining independently addressable, but their joint range and coupling must
be stated.

## Compatibility with history superposition

Let `omega` be a `p`th root of unity and choose a frequency vector
`a=(a_1,...,a_t)` in `F_p^t`. The character of a history moment code is

```text
chi_a(R)
  = omega^(sum_j a_j c_p(R)_j)
  = product_(r in R)
      omega^(sum_j a_j r^j).
```

Therefore it has the multiplicative position form required by
T-HISTORY-COUPLING-1, with

```text
lambda_(a,r) = omega^(sum_j a_j r^j).
```

The phase-bound deletion orbit for any such character is computable by the
same occurrence product formula without enumerating deletion histories.

The complete character family separates all admissible moment codes. A small
fixed character subfamily is only a perceptual candidate and may collide; exact
records remain authoritative.

## Least examples

- For the least `bab`/`aa` obstruction, `t=1`, and modulus `p=3` distinguishes
  the three possible deletion positions.
- For the content-balanced `aabb`/`ba` obstruction, `t=2`; modulus `p=5`
  distinguishes all two-position histories through the two residues
  `(sum r, sum r^2)`.

These examples establish exact history identity, not that every character is
needed or that a chosen fixed-width projection yields adequate SNR.

T-HISTORY-QUOTIENT-1 adds a distinct boundary: from radius two, making every
identical-descendant history share one cancellative product code can force
different descendants to collide. Exact position-history moments remain valid;
they cannot be converted into the full outcome quotient merely by character
selection.

## Falsification gate

A claimed exact history range fails when positions alias modulo `p`, when
`p<=t` makes the Newton recursion noninvertible, or when fewer recorded moment
coordinates admit two histories with the same code. A compact perceptual claim
fails when named hard negatives remain coherent across every selected character
channel at the declared width.
