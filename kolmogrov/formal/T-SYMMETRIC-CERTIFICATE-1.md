# T-SYMMETRIC-CERTIFICATE-1: bounded common-descendant candidate retrieval

Status: **HYPOTHESIS with complete recall proof and least false-candidate
obstruction; held-out filename selectivity absent**.

## Symmetric target relation

For same-length streams `x,y` of length `n`, let `D_t(x)` be the set of exact
length-`n-t` descendants formed by deleting `t` source positions. The symmetric
common-descendant relation is

```text
D_t(x) intersection D_t(y) != empty.
```

If `x,y` differ by `s<=t` substitutions, delete the mismatch positions from
both and, when `s<t`, delete the same additional matched positions; this proves
the relation. An adjacent transposition of two unequal atoms also has a common
one-deletion descendant. The relation is a candidate-positive edit control, not
a complete perceptual label.

## Certificate-overlap candidate

For a common degree-`m` certificate schedule, build exact idempotent pattern
sets `S_c(x)` and `S_c(y)`. Accept a candidate when

```text
S_c(x) intersection S_c(y) != empty
```

for every retained certificate `c`. With projected occupancy masks, replace set
intersection by a nonzero bitwise intersection in every certificate/view.

## Recall theorem

If `z` belongs to both `D_t(x)` and `D_t(y)`, then for every certificate subset
`I_c`, the selected pattern `z|I_c` belongs to both exact pattern sets. Every
exact certificate intersection is therefore nonempty.

Projection maps the same complete pattern to the same cell, so projected masks
also intersect. Consequently complete or bounded schedules and any deterministic
whole-pattern projection retain every exact common descendant. Schedule omission
and projection collision can only add candidates.

## Converse failure and least obstruction

At radius one and length four, take

```text
x = aaba,
y = babb.
```

Their one-deletion descendant sets are

```text
D_1(x) = {aba,aab,aaa},
D_1(y) = {abb,bbb,bab},
```

and are disjoint. The target length is three, so the complete degree-two
schedule contains subsets `(0,1)`, `(0,2)`, and `(1,2)`. Every corresponding
certificate intersection contains pattern `ab`; the candidate test accepts.
Different certificates borrow `ab` from incompatible descendant histories.

This obstruction is least. A radius-one common descendant needs source length
at least three. At length three the target has length two and degree two is the
full target, so its single certificate intersection is exactly the common-
descendant predicate. Length four is the first scale with a proper degree-two
cover, and the witness attains failure.

Full target degree `m=n-t` restores exactness because one certificate stores
the complete descendants themselves, but its pattern universe and build/index
support grow with content length. Fixed degree therefore remains a front index.

## Query algorithm without descendant enumeration

The record index stores a posting for every occupied certificate/view/cell.
For a same-length query:

```text
ITERATE Q certificates
  ITERATE occupied query cells H_(q,c,s) per view
    READ posting
  UNION cells within the certificate/view
INTERSECT certificate/view unions
EXACTLY VERIFY surviving records
```

Source and query mask construction already iterates the fixed
`C(t+m,m)` offset states; it does not enumerate `C(n,t)` descendants. Logical
query predicates are bounded by

```text
sum_(c,s) |support M_(c,s)(query)|
 <= sum_(c,s) min(H_(q,c),K_s).
```

This is larger than the `Q S` one-sided short-query path and is charged in the
posting/resource ledger.

## Typed retrieval consequence

- shorter query versus longer source: use one key per certificate/view under
  the directional descendant predicate;
- same-length fuzzy query: union occupied query cells per certificate/view,
  then intersect;
- insertion/deletion between stored objects: query the exact source lengths
  allowed by T-TYPED-EDIT-SEAM-1;
- every path ends in exact record verification and may retain ambiguity.

## Boundary

The theorem supplies candidate recall, not ranking or protected distinction.
Filename digits, extensions, and literal hard negatives can satisfy the same
common-descendant relation as intended typo positives. Independent literal,
folded, structural, numeric-jet, and exact-verification evidence decide their
later ranking or rejection.
