# E11 result: semantic protection is two rows; useful support is four; unseen directions remain

Status: **MEASURED on two disjoint generated families of 288 records and 180
typed queries each under Unicode 16.0.0; not real-filename relevance or native
engine evidence**.

## Row-support result

The development mutation graph induced 51 literal, 45 folded, and nine
structural nonzero flattened differences. Their ranks were 18, 16, and six.
Every view contained the least one-row obstruction from
T-PROTECTED-KERNEL-1, and the compiler reached zero development erasures at
depth two. Because one row was proved impossible and two were constructed,
two is the exact semantic minimum for each declared development graph.

That minimum was not enough for retrieval. On the development split:

| Depth | p95 candidate fraction | p95 false candidates | hard-decoy admission |
|---:|---:|---:|---:|
| 1 | 100.00% | 287 | 88.00% |
| 2 | 55.56% | 159 | 55.56% |
| 3 | 8.33% | 23 | 18.44% |
| 4 | 2.08% | 5 | 6.44% |
| 5 | 1.39% | 3 | 5.33% |
| 6 | 1.39% | 3 | 4.00% |

The predeclared development gate first passed at four rows. Thus this workload
separates three support quantities:

```text
two rows   minimum named-difference support
four rows  minimum development candidate-tail support
six rows   completion of the tested unseen atom-difference support
```

They are not interchangeable uses of “minimum hash size.”

## Disjoint mutation failure

At the two-row protected depth, the evaluation graph still erased 13 of 48
literal differences and nine of 42 folded differences. At the selected
four-row depth it still erased four literal and eight folded differences.
Every tested evaluation difference became nonzero only at depth six.

Structural differences generalized at depth two because the nine-edge
evaluation difference set coincided at the coarse role-code level. Literal and
folded behavior did not. The protected functional prefix therefore does not
generalize by itself; unseen response came from the balanced representative in
the orthogonal ambient coset.

This rejects the strong form of K-H45 in which development obstruction
protection alone was expected to outperform a support-balanced route.

## Equal-depth evaluation controls

All four-row layouts retained the exact source and the complete exact
radius-one ambiguity class for every substitution, transposition, insertion,
deletion, and case query. This is the expected recall direction of
T-SYMMETRIC-CERTIFICATE-1 and the two directional cross-length plans.

| Layout | Candidate p50 / p95 / max | False p95 | Hard-decoy admission |
|---|---:|---:|---:|
| low-bit affine | 2 / 10 / 20 | 9 | 21.11% |
| balanced rows | 2 / 4 / 4 | 3 | 12.89% |
| protected coset | 2 / 6 / 12 | 5 | 13.78% |

The protected rows beat the rejected low-bit route but lost to the balanced
control on every displayed held-out tail. Protection is consequently a guard,
not a sufficient selection objective.

Increasing the protected-coset depth from four to six reduced evaluation p95
false candidates from five to one, but hard-decoy admission only moved from
13.78% to 12.22%. This was a pre-verification candidate rate: E11 did not
establish whether those particular siblings were exact one-edit neighbors.
E12 later adds genuine one-edit digit/extension siblings and separates exact
verification from conditional relevance policy. More cells alone cannot supply
either distinction.

## Posting and verification charge

At the selected four-row protected layout, including forward descendant masks
and reverse exact keys:

```text
logical memberships                    78.51 / record
nonempty addresses                     515
hybrid posting payload                 51.16 bytes / record
single-record mini-segment payload    157.03 bytes mean
query lists read                        51 p50, 56 p95
query posting payload                1636 bytes p50, 2001 bytes p95
candidate exact comparisons             34 p50, 65 p95
```

The mini-segment number is payload only: one tag plus one local ordinal byte per
membership. It excludes a dictionary, framing, durability, and later
compaction, so it is not an engine write-amplification claim.

Depth increased hybrid payload per record monotonically:

```text
depth        1      2      3      4      5      6
bytes      20.48  30.98  41.04  51.16  57.19  60.71
```

Query bytes did not increase monotonically because finer cells shortened the
lists that were read. This is the concrete nonmonotonicity predicted by
T-POSTING-SUPPORT-1: support, encoded index size, and verification work trade
against one another rather than sharing one scalar optimum.

## Iteration and latency boundary

The reference query performs only these data-dependent loops:

```text
ITERATE three admitted source lengths                 [fixed: n-1,n,n+1]
  ITERATE three views x at most eight certificates    [fixed]
    ITERATE occupied query cells                      [bounded by 2^depth]
      READ one posting
  INTERSECT cardinality-ordered predicate unions      [early empty exit]
ITERATE candidate records
  SCAN at most max filename length once               [no edit matrix]
```

Warm Python reference medians were roughly 0.28--0.40 ms/query across the
four-row controls, but repeated runs produced materially different p95 tails
and one 18 ms process stall. Those timings establish neither native latency nor
cache behavior. The valid algorithmic result is the absence of descendant
enumeration and quadratic edit verification; K1 still needs a native engine
measurement with cold/warm state and directory metadata included.

## Consequence

The exact-length query system is now coherent enough for an experimental
adapter: it has complete radius-one candidate recall on the generated split,
honest reverse handling when the query is longer, bounded iteration, and a
concrete payload control.

Production projection selection is not closed. The next row objective must be
minimax across independent mutation graphs or must reserve an explicit ambient
coverage code. It may not select rows solely by fitting one protected graph.
Real consented filename relevance, rank policy, native cache/I/O, segment
metadata, deletion/update behavior, and scale beyond 288 records remain open.
