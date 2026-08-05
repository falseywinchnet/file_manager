# T-HISTORY-INFLUENCE-1: mutation contribution and saturation boundary

Status: **deterministic obstruction PROVED; expectation conditional on a
uniform independent tuple map; E13 influence MEASURED**.

For coupled history-key set `A(x)`, define mutation contribution energy

```text
Delta(x,x') = |A(x) symmetric_difference A(x')|.
```

For multiple semantic views, retain the per-view vector and its sum. A mutation
is hash-invisible exactly when every component is zero. This is a candidate
generation statement, not a relevance score.

## Deterministic lower boundary

No finite projected address has a positive universal lower bound on `Delta`.
Different fingerprints can collide, and occupancy union can cancel a removed
key through another history already occupying it. A claim that every
one-letter change moves the hash therefore requires exact/unbounded keys or a
restricted finite domain with an exhaustive obstruction audit.

This is the epistemic boundary of the one-letter criterion: choosing a clever
finite projection cannot rule out zero influence universally.

## Conditional signal decay

Let coupled address cardinality be `R=product_j B_j`, and let a source have `n`
histories. Under independent uniform placement, a history key is private within
its source with probability

```text
(1 - 1/R)^(n-1).
```

Expected private histories are

```text
n (1 - 1/R)^(n-1),
```

approximately `n exp(-(n-1)/R)`. A one-symbol substitution changes all but the
history deleting that symbol, so `n-1` histories carry potential signal. Their
observable set contribution decays on the same occupancy scale.

Private-history survival does not guarantee a particular `Delta`, because old
and new keys may cross-collide. It is nevertheless a necessary SNR diagnostic:
once the expected private fraction falls below a declared threshold, that
scale no longer supports individual history contributions.

## Operational gate

A configuration must declare maximum input length, minimum changed-key energy
for named mutation classes on the admitted development domain, maximum held-out
invisibility rate, address cardinality, conditional private-history floor, and
overflow behavior. The safe overflow behavior marks the hash outside its
supported envelope instead of silently claiming similarity fidelity.

## E13 instance

No same-length nonexact evaluation query was invisible at any tested scale. At
two 8,192-cell separable coordinates, changed cells across semantic views were
128 p50 and 144 p95. Case mutations changed only the literal channel, as the
filename profile declares.

These measurements show signal on that workload; they do not create a
deterministic minimum or consent a production threshold.
