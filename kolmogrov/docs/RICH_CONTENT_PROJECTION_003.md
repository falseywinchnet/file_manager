# Rich-content projection 003: coupled refinement with visible breakdown

Status: **CANDIDATE projection algebra with a paper support bound and narrow
four-symbol measurements; canonical File Manager atom policy unresolved**.

## The object

Keep the deletion certificate geometry unchanged. Replace its exact `A^m`
pattern mask with a dense occupancy view over complete pattern tuples:

```text
canonical atom stream
  -> degree-m certificate pattern
  -> coupled projection cell in [0,k^d)
  -> one occupancy bit
```

“Coupled” is essential: all `m` atom identities enter one cell. Storing slot
marginals independently loses which values belonged to the same witness.

Every projection emits an ordered radix digit stream. The `k^(d+1)`-cell mask
contracts into the `k^d`-cell mask by OR-ing children with the same prefix.
Count vectors contract linearly by summation. This makes scale granular and
exact: more support refines existing addresses rather than replacing them with
unrelated hashes.

## Two candidate code families

### Affine residue tower

```text
pi_d(a_0,...,a_(m-1)) = sum_i w_i a_i mod k^d.
```

It is cheap and every depth is automatically compatible. Its unresolved edge
is a permanent integer kernel: if two patterns have the same unmodulated sum,
no amount of refinement separates them.

### Ordered binary Jacobian

For atoms bounded to `b` code bits, flatten the `mb` pattern bits, apply an
ordered binary linear map, and store the first `d` output bits. A full-rank
`mb`-row completion is exact at `2^(mb)` cells. Coarse row order is selected
against protected mutation directions and private-witness breakdown.

E9 showed that arbitrary dense rows are not enough. The active candidate is an
**obstruction-compiled row order**, not “use a random invertible mixer.”

## Exact support and breakdown law

For one source certificate let `H` be its number of distinct observable
patterns. A pattern has a private witness in one view when it is the only
pattern in its cell. Removing that pattern changes the occupancy hash exactly
once for each view in which it is private.

If every pattern needs `c_pattern` private bits across views whose dense support
is `B_cell`, then T-PROJECTION-REFINEMENT-1 proves

```text
B_cell >= c_pattern H.
```

For deletion radius `t` and degree `m`,

```text
H <= min(A^m, C(t+m,m)).
```

Combining this content bound with the positional coverage law gives the
necessary total payload condition

```text
Q >= ceil(c_position L/m),
B_total = Q B_cell,
B_total >= ceil(c_position L/m) c_pattern H_required.
```

`H_required` is a protected bound, not automatically the worst combinatorial
value. This replaces the blunt exact-mask cost `Q A^m` with a declared collision
capacity while retaining a theorem-bounded failure measure.

Under declared bit-noise energy `N` and atomic SNR threshold `Gamma`, require

```text
c_pattern >= ceil(Gamma N).
```

Until `N` and `Gamma` are frozen, private-witness counts and retrieval quality
remain separate ledgers.

## Build and query shorthand

Let `R=C(t+m,m)` be the monotone offset-state count and let `S` be the number of
coupled projection views.

```text
BUILD
ITERATE Q certificates                                      [bounded schedule]
  ITERATE R monotone offset states                          [fixed by t,m]
    GATHER m atoms                                          [unroll]
    ITERATE S coupled views                                 [small/frozen]
      ACCUMULATE m affine terms or binary rows              [unroll/SIMD]
      SET one occupancy bit                                 [idempotent]

QUERY
ITERATE Q certificates                                      [bounded]
  GATHER one m-atom pattern                                 [unroll]
  ITERATE S coupled views                                   [small/frozen]
    COMPUTE one cell
    LOOK UP one posting
INTERSECT postings in cardinality order                     [short-circuit]
VERIFY exact engine records                                 [authoritative]
```

There is no `A^m` loop and no deletion-history loop. Reference build work is
`O(Q R m S)` with `Q sum_s k^(d_s)` dense payload bits. For the current radius
two/degree-three profile, `R=10`.

## What E9 changes

On exhaustive four-symbol length-seven sources, the exact control used 64 bits
per certificate. At 16 bits:

- a path-biased mixed-radix prefix produced 644.69 candidates/query;
- one balanced affine cell produced 257.90;
- a private-witness-selected binary quotient produced 260.58 with a much lower
  maximum, 421 rather than 704;
- two independent 8-cell views regressed to 383.46;
- four independent 4-cell views regressed to 3,122.70 despite jointly
  identifying each individual pattern.

At 32 bits, the selected binary tower produced 214.23 candidates/query against
211 exact, while 4.08% of pattern incidence remained bit-invisible. This last
disagreement is precisely why average retrieval cannot replace the support
ledger.

Full measurements and least obstructions are in
`experiments/e9_rich_projection/RESULT.md`.

## Projection selection trajectory

1. Freeze an experimental atom stream and its bounded code width.
2. Construct the exact complete-pattern oracle.
3. Build the protected one-letter/hard-negative graph.
4. Compile a nested prefix route minimizing protected collisions and private
   invisibility at each declared support point.
5. Reserve the finest full-rank completion as a conformance endpoint.
6. Tune only on a development split; freeze the row order and widths.
7. Measure candidate tails, invisible mass, posting cost, and exact-verified
   quality on a disjoint split.
8. Reject any scale at which a protected pattern or mutation loses the required
   private/SNR margin.

## Low-confidence items that remain

The projection algebra no longer depends on a small literal alphabet, but K1
is still blocked by observation policy. The next research profile must choose,
without claiming production architecture, one canonical File Manager stream.
Candidates should remain independent: raw filename scalars, token atoms, and
path-segment atoms are different observation maps and must not be fused before
their individual deformation behavior is measured.

The other immediate unknown is per-symbol influence. One source-symbol change
can remove and add several patterns across offset states. The next theorem must
express that Jacobian through certificate incidence, private old/new cells, and
collision cancellation; per-pattern visibility is necessary but not yet a
tight symbol-level guarantee.
