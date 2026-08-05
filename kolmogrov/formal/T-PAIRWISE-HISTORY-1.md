# T-PAIRWISE-HISTORY-1: pair coherence closes one deletion but not two

Status: **HYPOTHESIS with complete paper proof; compact character projection
still governed by T-SAMPLED-HISTORY-1**.

## One-deletion support intervals

Let source `x` have length `n`, query `y` length `n-1`, and let history `r` mean
deleting source position `r`. For query position `i`, define

```text
S_i = {r : (x\{r})_i = y_i}.
```

If `r<=i`, target position `i` comes from source position `i+1`; if `r>i`, it
comes from source position `i`. Therefore

```text
S_i
  = ({0,...,i}       when x_(i+1)=y_i)
    union
    ({i+1,...,n-1}   when x_i=y_i).
```

Each nonempty `S_i` is a prefix, a suffix, or the whole deletion-position line.
It is therefore an interval.

## Pairwise sufficiency theorem

The query is a one-deletion descendant of the source if and only if every pair
of indexed-symbol support sets intersects:

```text
y=x\{r} for some r
  iff S_i intersect S_j is nonempty for every i,j.
```

### Proof

A common deletion history trivially belongs to every pairwise intersection.
Conversely, pairwise-intersecting intervals on a finite line have a common
point: the largest lower endpoint is no greater than the smallest upper
endpoint. That deletion position lies in every `S_i`, so it produces every
query symbol at its exact target position and hence produces `y`. QED.

Thus exact radius-one containment needs only pair coherence over degree-one
positional atoms. No higher-order history product is required at the oracle
level.

## Least radius-two obstruction

Pairwise coherence is not sufficient for two deletions:

```text
source x = ababa
query  y = bbb.
```

The three relevant descendants

```text
bba, bab, abb
```

support respectively query-position pairs `(0,1)`, `(0,2)`, and `(1,2)`.
Every pair of query atoms therefore has a common two-deletion history. No
history supports all three because the source contains only two `b` symbols.

The obstruction is least by source length. A pairwise/global distinction needs
at least three query atoms, so target length is at least three. One deletion is
closed by the interval theorem, so at least two deletions are required. Hence
source length is at least `3+2=5`, attained by the binary witness above.

## Algorithm consequence

- Radius one: pair character coherence is the exact structural order; scoring
  need not iterate over higher-order frequency tuples.
- Radius two and above: pair coherence alone is unsound. Bind at least three
  query positions inside an occurrence atom or retain a declared higher-order
  coherence channel.

This selects semantic interaction order, not the number of character probes or
their quantization.

T-DELETION-CLOSURE-1 generalizes both parts: radius `t` closes exactly at order
`t+1`, with `(ab)^t a / b^(t+1)` as the sharp obstruction family.
