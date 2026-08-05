# T-ORTHANT-1: deletion containment as gap-orthant lifting

Status: **HYPOTHESIS with complete local and full-degree paper proofs; low-degree
converse rejected by retained counterexamples; formal encoding absent**.

## Witness compatibility

Let query witness `u` and record witness `v` have the same degree `k`. Define
`u <=_D v` when:

1. their symbol patterns are equal;
2. the record is at least as long as the query;
3. every record gap coordinate is greater than or equal to the corresponding
   query gap coordinate.

The excess gap mass is automatically the object-length difference.

## Local statement

`u <=_D v` if and only if some deletion set containing no selected position of
`v` transforms occurrence `v` into occurrence `u` while preserving its pattern.

### Proof

If such deletions exist, T-MULTIDEL-1 says source minus target gaps are a
nonnegative vector, establishing coordinatewise dominance and pattern equality.

Conversely, let `d_j = v.g_j-u.g_j`. Dominance makes every `d_j` nonnegative.
Delete any `d_j` unselected positions from gap region `j`. Their total equals
the record/query length difference because both gap sums equal object length
minus degree. Selected symbols and order survive, and the remaining gaps are
exactly those of `u`. QED.

## Object-level score

At degree `k`, form the bipartite graph between query and record occurrences
with edges `<=_D`. The reference score is maximum distinct matching size divided
by query occurrence count. Distinct matching prevents one record occurrence
from impersonating arbitrary query multiplicity.

## Soundness for deletion descendants

If the whole query is obtained from the record by deletions, the score is one at
every valid degree. The actual survivor lift is an injective matching by
T-MULTIDEL-1.

## Full-degree converse

Let query length be `m` and choose degree `k=m`. The query has one witness: its
whole pattern and all-zero gaps. A compatible record witness exists exactly
when that whole pattern occurs in increasing record indices. Therefore score
one at full degree is equivalent to literal ordered-subsequence containment.

## Low-degree limit

At `k<m`, independently compatible witnesses may require incompatible deletion
sets and need not assemble into one global query embedding. E4 searches and
retains the smallest such false positives. Consequently low-degree orthant
matching is a candidate-generation channel, not an identity or exact
containment oracle.

## Open direction

A hierarchy of degrees can expose how global consistency emerges: low degrees
provide stable, plentiful local witnesses; high degrees enforce shared context.
The unresolved problem is to choose or compress this ladder without hiding
which degree supplied each candidate.
