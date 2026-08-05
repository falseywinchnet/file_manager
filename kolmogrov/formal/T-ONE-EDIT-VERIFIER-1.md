# T-ONE-EDIT-VERIFIER-1: matrix-free exact radius-one verification

Status: **HYPOTHESIS with complete elementary proof; native cost remains
unmeasured**.

## Relation

For two finite atom streams, accept exactly one of:

```text
equality;
one substitution;
one adjacent transposition;
one insertion;
one deletion.
```

The classifier returns no relation otherwise. This is the exact radius-one
verification seam for E11; it is not a perceptual ranking policy.

## Same-length algorithm

Scan aligned atom pairs once and retain at most the first two mismatch
positions.

- zero mismatches is equality;
- one mismatch is one substitution;
- two mismatches are an adjacent transposition exactly when the positions are
  adjacent and the two atoms cross-match;
- three mismatches or a failed cross-match are outside the relation.

Necessity follows because equality/substitution/transposition affect zero, one,
or two adjacent aligned positions respectively. Sufficiency follows by applying
the named edit at the retained position or positions.

## Cross-length algorithm

If lengths differ by more than one, reject. Otherwise scan the shorter and
longer streams with two indices. At the first mismatch, advance only the longer
index. A second mismatch rejects. If no mismatch occurs, the unmatched final
longer atom is the insertion/deletion.

Any one insertion shifts the remaining alignment by exactly one after its
location, so the single skip is necessary and sufficient. Reversing stream
roles distinguishes insertion from deletion.

## Work bound

The algorithm retains two indices, at most two mismatch locations, and one skip
flag. It allocates no edit matrix. For maximum input length `n`, it performs at
most `n` aligned atom comparisons and uses `O(1)` auxiliary state apart from the
fixed result object.

The bound is sharp: equality and an edit at the final atom require inspection
through the final aligned position. Any exact verifier for an unrestricted
alphabet must inspect that position, since changing only the uninspected atom
can change acceptance.

## Boundary

This removes quadratic dynamic-programming work only for the declared typed
radius-one seam. Larger edit budgets, block moves, normalization lineage, and
semantic equivalence require separate verifiers. Candidate production remains
responsible for keeping the number of exact verifications bounded.
