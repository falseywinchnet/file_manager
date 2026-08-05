# T-HISTORY-COUPLING-1: deformation evidence must remain bound to one history

Status: **HYPOTHESIS with complete exact lift and least-obstruction proofs;
fixed-width phase family remains CANDIDATE**.

## Exact history lift

For `t` deletions, let `e_R` be a distinct formal basis element for every source
deletion set `R` of size `t`. Lift the occurrence polynomial to

```text
H_(x,k,p,t) = sum_(|R|=t) e_R tensor G_(x\R,k,p).
```

This object binds every surviving content/position/combination atom to the
deletion history that produced it. Applying the all-ones functional to the
history factor gives the unlabelled orbit marginal from T-DELETION-ORBIT-1.

## Multiplicative history projection

Choose source-position weights `lambda_0,...,lambda_(n-1)` and encode a deletion
history multiplicatively:

```text
chi_lambda(R) = product_(r in R) lambda_r.
```

Define the all-radius projected history polynomial

```text
O_(x,k,p)^lambda(z,s)
  = sum_R s^|R| chi_lambda(R) G_(x\R,k,p)(z).
```

For an occurrence `I`, let `B_j(I)` be the unselected source positions in its
gap `j`. Then

```text
O_(x,k,p)^lambda(z,s)
  = sum_(I : x_I=p)
      product_(j=0)^k product_(r in B_j(I)) (z_j+s lambda_r).
```

### Proof

Expand the product for one occurrence. Choosing `z_j` at position `r` means the
unselected source position survives in gap `j`; choosing `s lambda_r` means it
is deleted. A term choosing exactly the set `R` contributes
`s^|R| chi_lambda(R) z^(g-alpha(R))`, which is precisely the target occurrence
monomial for that history. Positions selected by `I` are absent from the
product, so histories destroying the occurrence contribute nothing. Sum over
occurrences. QED.

For `lambda_r=1`, the product becomes `product_j(z_j+s)^g_j`, recovering the
translation identity of T-DELETION-ORBIT-1. Nonconstant `lambda` retains a
projected trace of which source positions jointly generated every feature.

## Exact common-history criterion at degree one

Let target `y` have length `m=n-t`. Its degree-one exact positional atom at
target position `i` is

```text
f_i(y) = (pattern=y_i, gap=(i,m-i-1)).
```

For source `x`, let `S_x(f)` be the deletion histories `R` of size `t` whose
descendant `x\R` contains atom `f`. Then

```text
y is a t-deletion descendant of x
  iff intersection_(i=0)^(m-1) S_x(f_i(y)) is nonempty.
```

### Proof

If `y=x\R`, the same `R` supports every indexed symbol atom of `y`. Conversely,
if one `R` lies in every support set, `x\R` has symbol `y_i` at every target
position `i`; both objects have length `m`, so they are equal. QED.

Thus degree-one content plus exact position is sufficient for exact deletion
containment only when the feature evidence remains coupled to a common history.

## Least marginal-coupling obstruction

Unlabelled orbit marginals retain only the weaker conditions

```text
S_x(f_i(y)) is nonempty for every i.
```

They do not require one common history. The least obstruction is

```text
source x = bab
query  y = aa
t = 1.
```

Deleting the first source position produces `ab`, supporting query atom `a` at
target position zero. Deleting the last source position produces `ba`,
supporting query atom `a` at target position one. No deletion produces `aa`
because the source contains only one `a`.

Target length one cannot fail: it has only one indexed-symbol atom, so marginal
support and common-history support coincide. A strict deletion source must then
have length at least three. A one-symbol alphabet cannot fail because every
descendant is literal. The witness uses source length three, target length two,
and a binary alphabet, so it is least in those support coordinates.

The least content-balanced ordered obstruction is

```text
source x = aabb
query  y = ba
t = 2.
```

The source and query both contain the required symbols. Descendant `bb`
supports `b` at target position zero, and descendant `aa` supports `a` at
position one, but no subsequence of `aabb` has `b` before `a`. Length three
cannot realize both a later witness after `b` and an earlier witness before `a`
while forbidding `b` before `a`; length four is least for this balanced ordered
failure.

## Candidate compact repair

The exact history tensor is too large for the intended hash. The product formula
admits fixed scalar probes without enumerating descendants:

```text
address_(q,k,t)(x)
  = Q( sum_p sigma_q(p)
       [s^t] O_(x,k,p)^lambda_q(zeta_q,s) ).
```

Here `sigma_q` is a content sign/phase, `zeta_q` probes gap position, and
`lambda_q` binds all features from one deletion history to the same
multiplicative phase. Several independently addressable probes are intended to
make common-history contributions coherent while incompatible-history terms
interfere differently.

This is a **CANDIDATE**, not a proved compact intersection oracle. Its decisive
gate is whether a small, fixed probe family rescues common-history hard
negatives at equal serialized support without losing true descendants.
