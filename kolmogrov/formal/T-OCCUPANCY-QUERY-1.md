# T-OCCUPANCY-QUERY-1: support lower bound for arbitrary mask-overlap queries

Status: **HYPOTHESIS with complete finite proof; physical block layout remains
engine-private**.

## Predicate family

Let a certificate have `B` projected cells. A stored record has nonempty mask
`s subseteq [B]` and a same-length query has nonempty mask `q`. The exact front
predicate is

```text
F_q(s) = 1[s intersection q != empty].
```

The ordinary inverted representation stores one posting bitmap per cell and
answers `F_q` by OR-ing the postings for cells in `q`.

## Exponential precombination obstruction

Every two distinct nonempty query masks define different Boolean predicates on
stored masks. If `q != q'`, choose a singleton stored mask from their symmetric
difference. It intersects exactly one of `q,q'`.

Therefore an exact table with one precombined posting for every possible query
predicate needs

```text
2^B - 1
```

distinct precombined predicates in the worst case. No universal constant-size
precombination removes occupied-cell iteration while retaining every possible
query mask.

This is a semantic count, independent of compression.

## Bit-sliced block survivor

Store the `B` cell postings as one contiguous bit-sliced block of `BN` bits for
`N` segment records. A query reads the block or selected slices and ORs the
`|q|` bitmaps wordwise. This uses linear rather than exponential stored support:

```text
storage       BN bits,
word OR work  |q| ceil(N/word_bits),
```

before later certificate intersections. SIMD can batch word slices, and an
empty intermediate intersection can stop later certificates, but the exact
predicate family has not disappeared.

Sparse lists remain preferable when their encoded selected slices cost less
than the dense block. The codec decision is conditional on document frequency,
query occupancy, and cache state.

## Boundary

The theorem applies to arbitrary projected occupancy masks. A restricted mask
language could admit fewer precombinations, but its closure and recall must be
proved. E11/E12 do not establish that restriction, so the active algorithm
keeps the occupied-cell loop explicit.
