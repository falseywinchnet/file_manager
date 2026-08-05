# Deformation algorithm refinement 001

Status: **CANDIDATE active algorithm with exact reference equivalence and
MEASURED Python scaling; native optimization not selected**.

Continuation: `RETRIEVAL_REFINEMENT_002.md` fixes the deletion interaction floor
at `K=T+1`, adds safe run-count and idempotent observable quotients, measures an
affine certificate front index, and specializes the radius-two recurrence.

## Shorthand: what the hash builder does

For each frozen probe channel, scan the object once. At source position `r`,
each partial state has three possible semantic continuations:

```text
retain:  same degree, same radius, multiply current-gap weight
delete:  same degree, radius+1, multiply history phase
select:  degree+1, same radius, multiply content-slot weight
```

The direct recurrence is

```text
next[j,u]
  = gap[j]      * previous[j,u]
  + history[r]  * previous[j,u-1]
  + content[j]  * previous[j-1,u].
```

At end of input, row `j` contains every deformation-jet radius for occurrence
degree `j`. Quantization and indexing follow; they are not inside the recurrence.

## Iteratives, highlighted

### Exact descendant oracle

```text
ITERATE deletion radius t
  ITERATE C(n,t) deletion histories R
    ITERATE C(n-t,k) descendant occurrences I
      ITERATE selected symbols and gap coordinates
```

This is the correctness oracle. It is combinatorial and cannot be active.

### Occurrence-product collapse

```text
ITERATE C(n,k) source occurrences I
  ITERATE n-k unselected positions
    ITERATE radius coefficients 0..T
```

The history loop is gone, but occurrence enumeration remains combinatorial.

### Active streaming ladder

```text
ITERATE n source positions                         [unavoidable input scan]
  ITERATE degree states 0..K                      [small frozen support]
    ITERATE radius states 0..T                    [small frozen support]
      UPDATE Q probe channels together            [SIMD/batched dimension]
```

There is **no iteration over histories**, **no iteration over occurrences**, and
**no separate iteration per degree**. For fixed `K,T,Q`, construction is `O(n)`.

## Measured reference scaling

E7 used degree `3`, radius `2`, one modular probe, and three exactly equivalent
Python implementations on arm64 Python 3.14.0.

| Length | Oracle iterations | Product iterations | Streaming cells | Oracle time range | Product time range | Streaming time range |
|---:|---:|---:|---:|---:|---:|---:|
| 8 | 933 | 840 | 77 | 1.491–2.692 ms | 0.144–0.173 ms | 15.816–16.944 us |
| 12 | 10,199 | 5,940 | 125 | 17.414–18.943 ms | 0.955–1.050 ms | 26.023–26.776 us |
| 16 | 51,657 | 21,840 | 173 | 91.971–144.690 ms | 3.549–3.561 ms | 36.968–37.631 us |
| 20 | 175,771 | 58,140 | 221 | 319.279–320.040 ms | 9.654–9.930 ms | 48.685–50.007 us |

Across two consecutive timing runs, length-20 streaming was 6,385–6,574 times
faster than descendant enumeration and 198.3–198.6 times faster than occurrence
products. The stable iteration shape, not Python timing variance, is the
transferable result.

## Active-path refinement trajectory

1. **Freeze small `K,T`.** Current credible first target is bounded combination
   degree and edit radius, not arbitrary exact containment in the compact path.
2. **Batch probes inside the state cell.** Store channel values contiguously;
   vectorize multiply-add/reduction across `Q`.
3. **Generate phases incrementally.** Moment exponents are polynomial in source
   position, so finite differences replace repeated powers/exponentiation.
4. **Use two reusable slabs.** No per-object or per-position allocation.
5. **Specialize configurations.** Compile-time `K,T,Q`, unrolled small-radius
   updates, scalar fallback for portability.
6. **Fuse degrees, not evidence roles.** One DP emits all degree rows, but base,
   radial, and history channels remain separately serialized and scored.
7. **Keep query coherence low-order.** Exact all-feature character correlation
   has an exponential frequency-tuple loop. Bind several query positions inside
   degree-`k` occurrence atoms, then use pair/low-order coherence over `Q`
   channels instead.
8. **Index before exact verification.** Quantized address buckets generate
   candidates; exact catalogue records resolve history and identity.

## Current performance risk

The build algorithm is no longer the main mathematical risk. The recurrence has
the desired linear form. The risk is address count and scoring support:

```text
serialized bytes/object ~ Q * selected degree rows * selected radius rows
query work             ~ candidate count * retained coherence products * Q.
```

E7 shows that eight of eleven radius-one characters nearly separate all
protected degree-one pairs at length eight, but 32 of 121 radius-two characters
remain far from separation. The next refinement must reduce competing-history
support through higher combination degree, history/outcome equivalence, or a
better selected phase family—not hide it with more iterations.

T-PAIRWISE-HISTORY-1 fixes the minimum semantic order: radius-one exact support
sets are intervals, so pair coherence is globally sufficient; radius two first
needs triple binding, with least obstruction `ababa`/`bbb`. The active path can
therefore avoid all-feature iterative products while increasing combination
order only when deformation radius requires it.
