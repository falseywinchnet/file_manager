# T-POSTING-SUPPORT-1: logical membership charge for projected certificates

Status: **HYPOTHESIS with complete elementary counting proof; concrete engine
codec, cache, and update policy absent**.

## Posting object

For `N` exact engine records and each configured certificate/view/cell address
`a`, define exact posting list

```text
L_a subseteq {0,...,N-1}.
```

A record belongs to `L_a` exactly when its projected certificate mask occupies
that cell. Query candidate production intersects the lists named by its
certificate keys; exact records remain authoritative.

For one record `x`, define logical posting membership

```text
P(x) = sum_(c,s) |support M_(c,s)(x)|.
```

Double counting record/cell incidences gives the exact identity

```text
sum_x P(x) = sum_a |L_a|.
```

This is the codec-independent posting charge.

## Per-record support bounds

Let certificate `c` have `H_c` distinct exact patterns and view `s` have `K_s`
cells. Then

```text
1 <= |support M_(c,s)| <= min(H_c,K_s).
```

For nonempty sources,

```text
Q S <= P(x) <= sum_(c,s) min(H_c,K_s).
```

The lower bound is attained by total collapse. It is not an efficiency target:
the same collapse removes private influence and increases candidate load.

## Exact-list information lower bound

For a fixed address with document frequency `f`, there are `C(N,f)` possible
posting subsets. Any lossless representation that must distinguish all of them
without side information needs at least

```text
ceil(log2 C(N,f))
```

bits in the worst case for that list. This is a per-list bound. Correlations
between lists may permit joint compression, so their individual bounds may not
be added without a declared independence or coding model.

## Query predicates versus physical reads

A query names `Q S` logical membership predicates. Cardinality ordering and an
empty intermediate intersection can short-circuit later predicates. Cached
bitmaps, combined blocks, or implication between frozen lists can reduce
physical reads. Therefore `Q S` is a semantic predicate count, not a universal
I/O lower bound.

Any approximate posting representation reports its own candidate misses
separately. Omitting a nonredundant exact predicate from an intersection can
only admit more candidates; dropping records from a required posting can create
false negatives.

## Collision/resource nonmonotonicity

Coarsening projection merges cells. It can reduce dictionary cells and posting
memberships while increasing each surviving list's document frequency and the
final candidate intersection. Thus neither fewer lists nor fewer memberships
alone proves lower end-to-end work.

The required resource ledger is at least

```text
private/SNR support,
posting memberships and encoded bytes,
list document-frequency/tail distribution,
logical predicates and physical reads,
candidate count,
exact-verification work,
update amplification.
```

E10 gives only posting memberships for nine synthetic objects. It does not
instantiate the information bound or engine costs.

## Boundary

This theorem does not select sparse lists, dense bitmaps, delta coding, segment
layout, ordinal width, compaction, or cache policy. Those remain engine-private
measurements after a profile and workload freeze.
