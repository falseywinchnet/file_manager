# E12 result: coverage wins selection; split guards mix witnesses; certificate support dominates

Status: **MEASURED on three disjoint generated families of 288 records and 216
queries each under Unicode 16.0.0; not consented relevance or native engine
evidence**.

## Development+tuning guard

The union guard protected the development and tuning literal/fold graphs only
at row four; the structural graph closed at row two. The evaluation graph still
contained two invisible literal and two invisible folded differences at depth
four. At depth six, one literal evaluation difference remained invisible.

Adding a second protected graph therefore did not create an ambient guarantee.
It enlarged the guarded span and pushed its own semantic support from E11's two
rows to four, without closing the third graph.

## Frozen selection

At equal 16-cell dense support per certificate/view, tuning produced:

| Layout | Candidate p95 | False p95 | Hybrid bytes/record |
|---|---:|---:|---:|
| balanced coverage-4 | 12 | 7 | 61.40 |
| union guard-4 | 16 | 15 | 58.38 |
| split guard3 + coverage3 | 13 | 11 | 106.44 |

The balanced coverage layout was frozen. The guard used slightly fewer posting
bytes because it collapsed more patterns, but its candidate tail was worse.
The split layout had the same dense mask cells as coverage-4 yet almost doubled
posting payload: two independent address families duplicate reverse keys and
occupancy memberships. Equal dense support is not equal index support.

The tuning certificate sweep selected eight certificates:

| Certificates | Candidate p95 | False p95 | Hybrid bytes/record |
|---:|---:|---:|---:|
| 2 | 40 | 39 | 17.28 |
| 4 | 32 | 26 | 32.68 |
| 8 | 12 | 7 | 61.40 |

Radius-one recall remained one throughout, as the theorem predicts. Useful
selectivity—not recall—forced certificate support.

## Evaluation

The frozen coverage-4/eight-certificate configuration retained every source
and exact radius-one ambiguity parent. Its evaluation tail degraded to 16 p95
candidates and 15 p95 false candidates, so the tuning threshold did not
generalize.

| Layout | Dense cells | Candidate p95 | False p95 | Bytes/record |
|---|---:|---:|---:|---:|
| coverage-3 | 8 | 27 | 26 | 41.45 |
| frozen coverage-4 | 16 | 16 | 15 | 49.42 |
| union guard-4 | 16 | 12 | 11 | 48.10 |
| split guard3 + coverage3 | 16 | 12 | 11 | 81.35 |
| coupled guard3/coverage3 | 64 | 12 | 10 | 52.61 |

The unselected guard happened to improve the evaluation p95 but lost the
selection split, so it is not promoted. The separate split did not justify its
posting cost. The 64-cell coupled control improved mean false candidates but
barely changed the displayed tail. This locates much of the residual beyond
projection-cell width: incompatible certificate histories and the eight-
certificate schedule remain active limits.

## Provenance residual

The separate guard/coverage intersection admitted candidates rejected by the
coupled pair-cell control on 46 of 216 queries. Split-only candidates were:

```text
0 p50, 4 p95, 6 p99, 9 maximum, 0.898 mean.
```

This is the empirical form of T-GUARD-COVERAGE-1's least obstruction. Separate
addresses can satisfy guard and coverage through different exact patterns.
They remain recall-safe, but they are not joint-witness evidence.

## Genuine policy negatives

The strengthened generator includes exact queries whose stored number or
extension siblings are genuine one-edit neighbors. Under the frozen layout,
46.30% of named hard decoys entered the candidate set; 24.00% of those admitted
decoys were exact one-edit neighbors. Every verified hard decoy carried typed
digit/extension evidence.

However, every intended separator-deletion source also carried
`policy_sensitive=true`. A universal typed-evidence veto would therefore reject
one entire intended transformation class. Typed evidence is sufficient for a
declared conditional rank policy, not a universal relevance decision.

This also corrects E11's interpretation: its hard-decoy number was a
pre-verification candidate rate and did not establish that its particular
siblings were exact one-edit negatives.

## Liveness and dead support

Deleting 29 of 288 ordinals required a 36-byte dense liveness bitmap. Applying
it after candidate construction preserved source and ambiguity recall for every
live expected source. The immutable postings retained 2,409 dead memberships
until compaction.

This confirms T-SEGMENT-LIVENESS-1's semantic path but also exposes its resource
cost: exact deletion is cheap; reclaiming dead posting mass is a separate
compaction problem.

## Conclusions

- K-H48's union/split guard variants are rejected as general projection
  selectors on this round.
- Balanced coverage remains the honest frozen control, not an accepted
  production row family.
- Eight certificates are the minimum tested tuning support; the evaluation
  regression shows that even this schedule is not frozen for K1.
- Projection refinement alone no longer looks like the dominant remaining
  lever. Certificate directions, common-history coherence, and typed ranking
  evidence deserve the next support.
- Universal posting precombination is not a viable escape from occupied-cell
  iteration; T-OCCUPANCY-QUERY-1 proves the `2^B-1` predicate obstruction.
