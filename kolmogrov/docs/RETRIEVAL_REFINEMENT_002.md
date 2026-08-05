# Retrieval refinement 002: quotient first, then specialize

Status: **CANDIDATE active retrieval direction with exact deletion semantics and
finite binary measurements; content projection and release layout unresolved**.

## What changed

The prior history-phase direction treated compact character selection as the
main unresolved variable. E8 found a harder boundary: the full
identical-descendant quotient is not representable exactly by cancellative
per-position product phases from radius two. More phase channels cannot fix the
`aaba` obstruction.

The construction now separates two algebras:

```text
additive deformation jet
  -> graded energy, radius flow, multiplicity, fuzzy rescue

idempotent observable certificate
  -> history deduplication, candidate postings, exact membership control
```

This is not two copies of the same hash. They answer different questions and
remain independently addressable.

## Semantic support floor

T-DELETION-CLOSURE-1 proves that radius `t` deletion containment closes at
interaction degree `t+1`. The degree is both sufficient and necessary:

```text
radius 1 -> degree 2
radius 2 -> degree 3
radius 3 -> degree 4
```

The alternating obstruction `(ab)^t a / b^(t+1)` defeats every lower degree.
This replaces the former low-confidence guess `K approximately T+1` with a
theorem for exact positional deletion evidence.

## Active retrieval shorthand

### Offline schedule compilation

```text
ITERATE bounded affine directions over target positions       [offline]
  SELECT coverage-balanced degree-(t+1) subsets               [frozen identity]
PRECOMPUTE C(2t+1,t+1) monotone offset states                 [tiny for frozen t]
```

For `t=1,2,3`, the per-certificate offset counts are `3,10,35`.

### Source certificate construction

```text
ITERATE Q frozen certificates                                 [width budget]
  ITERATE C(t+m,m) monotone offset states                     [fixed]
    GATHER m symbols                                          [unrolled]
    SET one pattern bit/address                               [idempotent quotient]
```

There is **no iteration over `C(n,t)` deletion histories**. Histories with the
same observable pattern collapse at the bit set.

### Query and index

```text
ITERATE Q certificates                                        [bounded]
  ENCODE one m-symbol query key                               [unrolled]
  LOOK UP posting                                             [index operation]
INTERSECT smallest postings first                             [short-circuit]
EXACTLY VERIFY remaining records                              [authoritative]
```

The current experiment reports `Q` lookups and resulting candidate load. A
production index should order postings by measured cardinality, but that is not
yet measured here.

### Numeric jet construction

```text
ITERATE n source positions                                    [unavoidable scan]
  ITERATE degree 0..K                                         [K=t+1 floor]
    UPDATE fixed radius cells 0..T together                  [unrolled]
      UPDATE Q numeric probes together                       [SIMD target]
```

The radius-two specialization deliberately computes a few zero boundary cells
to remove reachability and radius-loop branches. It was 1.55–1.77 times faster
than the generic Python recurrence from length 20 through 1,000.

## Safe linear history quotient

Within a constant-symbol run, only the deletion count changes the descendant.
The run-block recurrence replaces position subsets with one branch per count:

```text
ITERATE source runs R                                         [R<=n]
  ITERATE prior radius u                                      [0..T]
    ITERATE deletions d from this run                         [0..T-u]
      APPLY precomputed retain/select block M^(run_length-d) [small K matrix]
```

Singleton runs use the ordinary streaming cell update. The quotient therefore
adapts to repetition without penalizing the alternating worst case with a
combinatorial history loop.

At length ten, run histories averaged `5.5`, `16.0`, and `31.625` classes for
radii one, two, and three, versus `10`, `45`, and `120` position histories.
Alternating content remains the least-support obstruction and receives no
quotient benefit.

## Retrieval measurements

On the exhaustive 1,024-source binary length-ten corpus, all affine certificate
configurations retained every exact descendant.

At radius two, degree-three certificates produced:

```text
64 bits   /  8 lookups -> precision 0.6376, mean 87.83 candidates
128 bits  / 16 lookups -> precision 0.8588, mean 65.21 candidates
192 bits  / 24 lookups -> precision 0.9271, mean 60.41 candidates
exact truth mean        -> 56 candidates
```

At radius three, degree-four certificates produced:

```text
64 bits   /  4 lookups -> precision 0.7648, mean 230.13 candidates
128 bits  /  8 lookups -> precision 0.9277, mean 189.72 candidates
256 bits  / 16 lookups -> precision 0.9911, mean 177.58 candidates
exact truth mean        -> 176 candidates
```

The complete degree-`t+1` schedules were exact but cost 144, 448, and 560
idealized bits for radii one, two, and three. The bounded affine schedules are
candidate front indexes: their omitted directions leave a residual for exact
verification.

## Granular support law

A degree-`m` schedule with `Q` certificates contains `Qm` target-position
incidences. If `Qm<L`, at least one position of a length-`L` target is invisible,
so one symbol can change without changing the certificate hash. With `c` rescue
witnesses per position,

```text
Qm >= cL.
```

For an exact alphabet-`A` mask, `B=Q A^m` payload bits. Therefore

```text
B >= ceil(cL/m) A^m,
m >= t+1 for deletion closure.
```

This is the requested content-size breakdown statement in a concrete support
basis. A fixed-width certificate must either declare a maximum sensitive
length, reduce coverage, hash the pattern axis and accept collisions, or move
detail into additional independently retrievable addresses.

## Specialization trajectory

1. Freeze `T` at the retrieval policy boundary and set the first semantic
   degree floor to `K=T+1`.
2. Compile affine schedules once per supported `(length band,T,K)` identity.
3. Precompute offset-to-source gather indices; unroll `m` symbol encoding.
4. Store binary/small-alphabet masks directly; for rich content, compare
   independently rescued hashed pattern satellites at equal total bits.
5. Order query postings by cardinality and stop intersecting when empty.
6. Use run-block quotienting only where run compression is present; take the
   singleton path otherwise.
7. Unroll fixed radius numeric cells and batch probe channels contiguously.
8. Measure posting bytes, cache misses, update amplification, verification
   latency, and mixed-edit recall before choosing a release representation.

## Confidence

- **High:** degree `t+1` is the exact deletion interaction floor.
- **High:** full outcome quotienting cannot be repaired inside product phases
  from radius two.
- **High:** monotone-offset certificate construction removes the history loop.
- **Medium:** affine certificate schedules are a useful bounded front index;
  the binary measurements are strong but narrow.
- **Medium:** run-block quotienting will help repetitive structured features;
  its real distribution is unknown.
- **Low:** exact pattern masks will transfer economically to rich file features
  without a carefully rescued content projection.
- **Unmeasured:** native throughput, posting representation, mixed edits,
  quantization, and end-to-end search latency.
