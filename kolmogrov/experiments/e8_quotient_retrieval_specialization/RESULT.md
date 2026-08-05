# E8 result — semantic order closes; safe quotienting changes the active index

Status: **MEASURED on exhaustive binary length-ten retrieval and Python
reference kernels; broader quality and native performance unmeasured**.

Environment: Darwin arm64, Python 3.14.0.

## Interaction order and exact candidate load

Every source of length ten and every binary query at radii one through three
was compared with exact deletion truth. Local order-`m` acceptance asks whether
every `m` query positions share some deletion history; different subsets may
use different histories.

| Radius | Order | False candidates | Precision | Mean candidates/query | Exact mean |
|---:|---:|---:|---:|---:|---:|
| 1 | 1 | 33,734 | 0.1431 | 76.89 | 11 |
| 1 | 2 | 0 | 1.0000 | 11.00 | 11 |
| 2 | 1 | 87,674 | 0.1405 | 398.48 | 56 |
| 2 | 2 | 6,020 | 0.7043 | 79.52 | 56 |
| 2 | 3 | 0 | 1.0000 | 56.00 | 56 |
| 3 | 1 | 68,194 | 0.2483 | 708.77 | 176 |
| 3 | 2 | 12,822 | 0.6373 | 276.17 | 176 |
| 3 | 3 | 704 | 0.9697 | 181.50 | 176 |
| 3 | 4 | 0 | 1.0000 | 176.00 | 176 |

Recall was one in every row because every local constraint is necessary. The
least failures are the exact family

```text
(ab)^t a  /  b^(t+1):
aba/bb, ababa/bbb, abababa/bbbb.
```

T-DELETION-CLOSURE-1 now proves that order `t+1` is sufficient and necessary
for every finite sequence, not only this census.

## History quotient atlas

At source length ten:

| Radius | Position histories | Mean run classes | Mean outcome classes | Unsafe multiplicative sources |
|---:|---:|---:|---:|---:|
| 1 | 10 | 5.5 | 5.5 | 0 / 1,024 |
| 2 | 45 | 16.0 | 14.0 | 846 / 1,024 |
| 3 | 120 | 31.625 | 22.0 | 942 / 1,024 |

The run-count quotient is safe and factorable. The full identical-descendant
quotient is smaller, but making a product phase constant on every outcome class
is unsafe from radius two. The least failures are `aaba` at radius two and
`aaaba` at radius three: the imposed phase relations alias outcomes `ab` and
`ba`. T-HISTORY-QUOTIENT-1 generalizes the obstruction to `a^t b a` for all
`t>=2`.

Thus the active construction cannot obtain the full outcome quotient merely by
adding or selecting phase characters. Run quotienting belongs in a run-block
linear recurrence; complete observable deduplication belongs in an idempotent
membership layer.

## Indexable affine certificates

For each scheduled degree-`m` target-position subset, the source stores a bitset
of binary patterns produced there by some deletion history. Construction uses
only `C(t+m,m)` monotone offset states per certificate. Querying performs `Q`
posting lookups and intersects them before exact verification.

Selected degree-`t+1` affine results:

| Radius | Bits/source | Certificates | Offset states/cert | Precision | False candidates | Mean candidates/query |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 128 | 32 | 3 | 0.8462 | 1,024 | 13.00 |
| 1 | 144 | 36 | 3 | 1.0000 | 0 | 11.00 |
| 2 | 64 | 8 | 10 | 0.6376 | 8,148 | 87.83 |
| 2 | 128 | 16 | 10 | 0.8588 | 2,358 | 65.21 |
| 2 | 192 | 24 | 10 | 0.9271 | 1,128 | 60.41 |
| 3 | 64 | 4 | 35 | 0.7648 | 6,928 | 230.13 |
| 3 | 128 | 8 | 35 | 0.9277 | 1,756 | 189.72 |
| 3 | 256 | 16 | 35 | 0.9911 | 202 | 177.58 |
| 3 | 336 | 21 | 35 | 0.9959 | 92 | 176.72 |

All rows retained every exact descendant. The complete schedules close exactly
but cost 144, 448, and 560 idealized bits at radii one, two, and three. The
radius-two and radius-three affine families omit some required subsets and
therefore retain 1,128 and 92 false candidates even when exhausted. They are
strong bounded front indexes, not exact replacements for verification.

The meaningful improvement is query work and candidate load, not a synthetic
hash-distance score: radius-three degree-four support at 256 bits reduced the
mean candidate set from 708.77 marginal candidates to 177.58, only 1.58 above
the exact mean, with 16 posting lookups.

## Specialization

The radius-two rectangular recurrence computes a few extra zero boundary cells
but removes the radius loop and its reachability branches.

| Length | Generic cells | Rectangular cells | Median Python speedup |
|---:|---:|---:|---:|
| 20 | 221 | 240 | 1.55x |
| 100 | 1,181 | 1,200 | 1.68x |
| 1,000 | 11,981 | 12,000 | 1.77x |

This is reference-language evidence, not a native latency claim. It does show
that regular fixed support can beat fewer irregular assignments.

The run-block quotient used 60 transfer cells for a constant length-twenty
source, 300 for a five-run source, and the singleton fast path used 240 for an
alternating source. The corresponding run-vector state counts through radius
two were 3, 21, and 211. Alternating content is the least-support obstruction:
there is no run quotient to harvest.

## Inference boundary

E8 uses an exact binary alphabet, fixed length, deletion-only truth, idealized
bit masks, and an exhaustive 1,024-source corpus. It does not measure content
hash collisions, quantization, mixed edits, file-derived features, posting
encoding bytes, cache behavior, update cost, or exact-verification latency.

The surviving trajectory is nevertheless narrower:

1. use degree `t+1` as the semantic support floor;
2. use safe run blocks when repetition exists;
3. use idempotent affine certificates as the front index;
4. retain numeric deformation jets for graded fuzzy energy and independent
   rescue, not for exact history deduplication;
5. specialize frozen radii and batch channels only after certificate width and
   candidate load are selected.
