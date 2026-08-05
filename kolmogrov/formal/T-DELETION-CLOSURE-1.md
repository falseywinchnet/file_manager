# T-DELETION-CLOSURE-1: radius `t` closes at interaction order `t+1`

Status: **HYPOTHESIS with complete finite-sequence paper proof and exhaustive
binary measurement through source length ten**.

## Offset representation

Let source `x` have length `n`, let query `y` have length `n-t`, and index query
positions by `i`. Define the allowed deletion-offset set

```text
A_i = {d in {0,...,t} : x_(i+d)=y_i}.
```

The query is a radius-`t` deletion descendant of the source exactly when there
is a nondecreasing selection

```text
d_0 <= d_1 <= ... <= d_(n-t-1),    d_i in A_i.
```

If a deletion history produces the query, `i+d_i` is the source position that
survives into target position `i`; the number of earlier deletions is
nondecreasing. Conversely, a nondecreasing selection makes `i+d_i` strictly
increasing. Those `n-t` source positions are the kept positions, and their
complement is a `t`-deletion history producing `y`.

The same statement holds for a subset of query positions: its atoms share a
history exactly when their allowed offsets admit a nondecreasing selection.
Any partial nondecreasing offset sequence extends across omitted query
positions by holding its last value until the next selected value.

## Bounded obstruction theorem

If the full query is not a deletion descendant, some subset of at most `t+1`
query positions has no common deletion history.

### Proof

Run the least-offset process from left to right. Start at offset zero. At query
position `i`, choose the least member of `A_i` no smaller than the current
offset. If no such member exists, the process fails. This greedy choice cannot
destroy a solution: every later offset must be at least the current one, so the
least available value leaves every possible continuation available.

Retain only positions at which the chosen offset strictly increases, followed
by the position where the process fails. There are at most `t` strict increases
because the offset lies in `{0,...,t}`, and one failure position, hence at most
`t+1` retained positions.

The first retained constraint forces its increased lower bound. Inductively,
each later retained constraint, after the preceding lower bound, forces the
next increased value. The failure constraint has no allowed value at or above
the last forced bound. Those retained atoms therefore have no common history.
QED.

Consequently, when the target has at least `t+1` positions,

```text
y is a t-deletion descendant of x
  iff every (t+1)-position query atom has a supporting history.
```

If the target is shorter, the required order is the full target length.

## Sharpness and least obstruction

For every `t>=1`, set

```text
x = (ab)^t a,
y = b^(t+1).
```

The source contains only `t` copies of `b`, so no deletion history produces the
query. Remove query position `h`, however, and map its remaining positions to
the `t` source copies of `b` using offsets

```text
d_i = i+1  when i<h,
d_i = i-1  when i>h.
```

These offsets are nondecreasing and lie in `{0,...,t}`. Thus every `t`-atom
subset has a common history although the `t+1` atoms do not. Interaction order
`t+1` is necessary.

The witness is least by source length: exposing an order-`t` versus
order-`t+1` difference requires at least `t+1` target atoms, and a radius-`t`
source is longer by `t`, so `n>=2t+1`.

The first three instances are

```text
aba      -> bb,
ababa    -> bbb,
abababa  -> bbbb.
```

## Retrieval consequence

The theorem fixes semantic combination degree, not fixed-width probe count.
An exact degree-`t+1` certificate for every target-position subset closes the
deletion predicate, but uses `C(n-t,t+1)` certificates. A bounded affine
schedule retains only selected certificates, never rejects a true descendant,
and leaves a measurable false-candidate residual.

For one scheduled degree-`m` certificate, all observable histories collapse to
the weakly increasing offset vectors

```text
0 <= d_1 <= ... <= d_m <= t.
```

There are `C(t+m,m)` such states, independent of source length. The active
certificate builder therefore iterates observable offset states, not deletion
sets.

## Coverage and content-length support bound

Let a fixed schedule contain `Q` degree-`m` certificates over a target of length
`L`. If some target position occurs in no certificate, changing only that
symbol cannot change any certificate address. Therefore one-change
observability requires

```text
Q*m >= L.
```

Requiring `c` independently addressable witnesses per position strengthens this
to `Q*m>=cL`. With an exact alphabet-`A` pattern bitset for each certificate,
the ideal payload is `B=Q*A^m` bits, hence

```text
B >= ceil(cL/m) A^m.
```

Deletion exactness imposes `m>=min(t+1,L)`. This is a hard support/length law for
the exact certificate representation: a fixed payload cannot remain sensitive
to every one-symbol change as content length grows without bound. Hashing the
pattern axis can reduce bytes only by admitting an explicit collision and
rescue term; it does not repeal the incidence bound.

## Boundary

This is an exact symbolic deletion theorem. Insertions, substitutions,
transpositions, content hashing, certificate quantization, and cross-length
schedule compatibility remain separate claims. Complete certificates are an
oracle support object; a fixed schedule is only a candidate index.
