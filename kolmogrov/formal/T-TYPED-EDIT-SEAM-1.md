# T-TYPED-EDIT-SEAM-1: same-length Jacobians and cross-length spans

Status: **HYPOTHESIS with complete paper boundary; lineage-conditioned
cross-length schedule transport unselected**.

## Same-length rewrite

Let source streams `x,x'` have the same length and differ only on position set
`J`. For certificate `c`, replace T-SYMBOL-JACOBIAN-1's affected states by

```text
U_(c,J) = {d in Omega : some selected source position lies in J}.
```

Every state outside `U_(c,J)` is unchanged, so the same proof gives

```text
|R_c|+|A_c| <= 2|U_(c,J)|,
E_(c,s,J) <= |R_c|+|A_c|.
```

A substitution has `|J|=1`. An adjacent transposition has at most two changed
positions and is therefore an exact two-column finite secant inside one fixed
configuration. Cancellation between the two positions is possible and must be
measured from the joint rewrite; adding their individual energies is only an
upper bound.

### Least transposition redundancy

At radius one and degree two, take

```text
x  = aaab,
x' = aaba
```

by transposing positions two and three, and certificate subset `(0,2)`. Its
three offset states expose `aa,ab,ab` before and `ab,aa,aa` after, so both exact
idempotent sets are `{aa,ab}`. The transposition participates twice but the
certificate response is zero before projection.

This is least in source length: a nontrivial adjacent transposition needs two
different atoms, radius-one degree-two certificates first exist at length
three, and direct evaluation of their sole three pair states changes the set.
Length four attains redundancy. Thus separate single-position responses cannot
be added to infer the joint transposition response.

## Length-changing edits are not vectors in that space

If insertion or deletion changes source length from `n` to `n+1`, the target
length, certificate subsets, schedule identity, payload meaning, and often
payload width change. The expression

```text
h_(n+1)(x') - h_n(x)
```

is undefined unless a cross-configuration transport map is declared. Padding
bytes or aligning storage offsets does not create semantic coordinate identity.

The existing deletion certificate already supplies a different valid object:
a directional relation from a length-`n` stored source to a length-`n-t` query.
It proposes candidates and exact verification determines membership. This
cross-length span need not be represented as Hamming distance between two stored
hashes.

## Least universal-transport obstruction

Take the one-atom stream `x=(a)` and length-two streams obtained by inserting
one atom on the left or right. A transport depending only on old and new lengths
would have to map old position zero to new position one for left insertion and
to new position zero for right insertion. No single map can do both.

One old position and one insertion are minimal. Therefore any exact positional
transport must depend on edit lineage/location, average or hide position, or
accept residual breakdown. Length alone never selects it.

## Typed mixed-edit composition

A mixed edit path is represented as a typed sequence:

```text
same-length rewrite
  -> within-configuration Jacobian/secant

insertion or deletion
  -> lineage-conditioned cross-configuration span
  -> directional certificate retrieval

another same-length rewrite
  -> Jacobian in the new configuration
```

These contributions may be reported together but are not summed as if they
shared one coordinate basis. If a future length-band family declares transport
maps, it must expose their commutation residual with every protected edit path.

## Consequence for supported length bands

The earliest honest engine experiment can compile schedules per exact source
length inside a bounded band and query only source lengths allowed by the typed
edit budget. This costs bounded iteration over candidate lengths but avoids a
false cross-length metric. A band-shared address becomes admissible only after
its lineage/path residual is derived and measured.

## Boundary

This theorem does not select length bands, edit costs, insertion locations,
query planning, or mixed-edit ranking. It prevents same-length Hamming energy
from being silently extended across incompatible representations.
