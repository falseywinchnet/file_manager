# T-PROJECTION-REFINEMENT-1: nested projection, private witnesses, and breakdown

Status: **HYPOTHESIS with complete finite paper proofs; independent review and
File Manager atom policy absent**.

## Object

Fix a certificate degree `m`, radix `k>=2`, and a finite set `S` of distinct
complete content patterns. A projection view `s` assigns each pattern a finite
ordered digit stream. Its depth-`d` bucket is the first `d` digits, encoded so
that removing the last digit is reduction modulo `k^d`.

One concrete family assigns an integer affine code

```text
c_s(a_0,...,a_(m-1)) = sum_i w_(s,i) a_i.
```

and takes

```text
pi_(s,d)(a) = c_s(a) mod k^d.
```

The dense count vector `N_(s,d)(S)` records the number of patterns in each of
the `k^d` buckets. The candidate mask is its thresholded occupancy
`M_(s,d)=1[N_(s,d)>0]`. A query pattern passes the view when its bucket bit is
occupied.

A second family flattens bounded atom codes into bits, applies an ordered binary
linear transform, and uses its first `d` output bits. If the full transform is
invertible, the finest view is exact, while the output-row order controls every
coarser prefix. The proofs below need only the prefix relation, not linearity.

The theorem does not select the atom codes, weights, binary rows, or their
order. They are representation identity and must be frozen separately.

## Exact projective contraction

For a fine residue `r mod k^(d+1)`, its coarse parent is `r mod k^d`. Therefore

```text
N_(s,d)(j) = sum_(q mod k^d = j) N_(s,d+1)(q).
```

This is an exact linear projective map. Its induced operator on signed count
differences is nonexpansive in unnormalized `l1`: grouping coordinates and the
triangle inequality give

```text
||C(u)-C(v)||_1 <= ||u-v||_1.
```

Thresholded masks obey the corresponding OR contraction. Every differing
coarse occupancy cell requires at least one differing fine child, hence their
unnormalized Hamming distance is also nonexpansive.

Thus a `k^(d+1)`-bit view contracts exactly into its `k^d`-bit predecessor. The
fine view is not a separate unrelated hash.

## Recall and candidate monotonicity

If a query pattern belongs to `S`, its bucket is occupied at every depth. Rich
content projection therefore introduces no false negative into a certificate
that was true before projection.

If a query passes at depth `d+1`, its fine occupied bucket has an occupied
coarse parent, so it passes at depth `d`. Consequently the candidate sets form
a nested sequence

```text
Candidates_(d+1) subseteq Candidates_d.
```

Refinement can remove false candidates but cannot remove an exact certificate
member. This statement is per certificate; bounded affine schedules retain
their pre-existing positional residual.

## Private-witness theorem

Call pattern `p in S` private in view `s` when no other pattern in `S` shares
its stored bucket. Let `v(p)` be the number of views in which `p` is private.

Deleting `p` from the exact pattern set changes exactly `v(p)` bits across the
concatenated occupancy masks: its bit clears precisely in the views where its
cell had count one. It is completely invisible to the compressed occupancy
hash exactly when

```text
v(p)=0.
```

Let view `s` contain `K_s=k^(d_s)` bits and let total dense support be
`B=sum_s K_s`. One bucket can be private to at most one pattern. If every one
of the `H=|S|` patterns must own at least `c` private witnesses, then

```text
cH <= B.
```

This is a necessary support bound, independent of random-collision assumptions.
It is not sufficient: bad projection weights can collide even above the bound.

Under a separately declared bit-flip/noise energy `N>0` and required atomic
SNR `Gamma`, the occupancy signal for deletion of pattern `p` is `v(p)` in
squared binary `l2` energy. Requiring `v(p)/N>=Gamma` forces

```text
c = ceil(Gamma N),
H <= floor(B/c).
```

The SNR statement is conditional; it does not select `N` or `Gamma`.

## Exact breakdown measure

For one view define

```text
collision_excess = H - |support(N_(s,d))|,
plural_mass = sum_(j: N_j>=2) N_j.
```

Both are nonincreasing under refinement until projection-code collisions stop
splitting. Across views, the decisive atomic breakdown is

```text
invisible_mass = |{p in S : v(p)=0}|.
```

These quantities expose why support failed. Candidate precision alone cannot
distinguish an unlucky query distribution from a source pattern that has lost
all bit influence.

## Least-support obstructions

### One projected cell

Two distinct patterns `p,q` in one bucket are the least invisibility witness.
Removing either leaves the occupancy bit set. One pattern cannot collide with
another and is necessarily private.

### Slot marginals are not a pattern certificate

For degree two, let a source certificate observe patterns

```text
S = {(0,0),(1,1)}.
```

Independent first-slot and second-slot membership both accept query `(0,1)`,
although the joint pattern is absent. Degree one has no cross-slot provenance
to lose, and one source pattern cannot assemble two coordinates from different
witnesses. This is the least joint-pattern obstruction.

### Even separating views can mix witness provenance

Over residues modulo three, take the two whole-pattern views

```text
pi_1(u,v)=u+v,
pi_2(u,v)=u+2v.
```

Their joint linear map is injective. Nevertheless source set
`S={(0,0),(1,0)}` independently accepts query `(2,1)`: its first view matches
`(0,0)` and its second matches `(1,0)`. Thus an injective vector code for one
pattern does not make independent per-view set membership exact.

The least repair is to bind view values to one witness, for example by storing
their joint cell. That dense repair costs the product rather than the sum of
view cardinalities. A cheaper provenance tag remains a candidate compression
and must expose its own collisions. Exact engine verification is therefore not
optional.

## Alphabet and position consequences

For a fixed surrounding context, varying one slot over `A` protected atom
values forms an `A`-clique in the one-letter mutation graph. Any stored
projection color that must change for every such mutation needs at least `A`
joint colors, consistent with T-OBSTRUCTION-CODE-1. This does not require
`A^m` colors because it asks for one-letter separation, not unique identity of
every degree-`m` pattern.

Pattern projection does not change T-DELETION-CLOSURE-1's positional incidence
bound `Qm>=cL`. A production configuration must satisfy both axes:

```text
position support: Qm >= c_position L,
content support:  sum_s k^(d_s) >= c_pattern H per certificate,
interaction:      m >= t+1 for exact radius-t deletion evidence.
```

## Boundary

The result applies to idempotent certificate membership over already-canonical
atoms. It does not choose Unicode normalization, tokenization, filename/path
views, affine weights, view depths, posting layout, or SNR thresholds. A source
symbol mutation can alter several certificate pattern sets; lifting the
private-pattern bound to a tight per-symbol Jacobian is the next formal step.
