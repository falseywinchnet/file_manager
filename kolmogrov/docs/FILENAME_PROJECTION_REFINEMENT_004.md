# Filename projection refinement 004: atom lanes, mutation flow, and index charge

Status: **CANDIDATE filename research spine; same-length symbolic mechanisms
defined, cross-length and held-out production gates open**.

## What is now concrete

Filename Observation Profile 001 provides three independent source-anchored
streams:

```text
literal scalar        21 exact bits/atom
anchored fold         270 exact bits/atom under Unicode 16.0.0
structural role         4 exact bits/atom
```

No digest collision precedes projection. The large exact atom is an input space,
not stored payload. A projection route selects inspectable affine directions
over its bits.

T-SYMBOL-JACOBIAN-1 then factors one scalar substitution:

```text
source position
  -> affected certificate offset states
  -> exact removed/added pattern sets
  -> projected cells crossing zero
  -> bit energy by certificate and view
```

Zero response is labelled as intended observation invariance, exact history-set
redundancy, or projection collision. These are not interchangeable.

## Active route

Do not project the low bits of the exact atom integer. E10 showed that the
length lane then dominates folded scalars and erases literal changes whose code
difference is divisible by 16.

The active development route is:

1. flatten all `m` exact atom lanes;
2. compile a small ordered binary Jacobian whose protected input differences
   have nonzero early columns;
3. build one coupled whole-pattern occupancy cell;
4. measure exact pattern events, bit energy, private mass, and posting entries;
5. add rows only where named obstructions or candidate tails require them;
6. freeze the route before a disjoint evaluation.

The `compile_balanced_binary_rows` control merely prevents zero individual input
columns. It does not optimize multi-bit cancellation or File Manager relevance.
The next compiler must operate on protected atom-difference vectors and source-
symbol Jacobian columns, not just individual bits.

## Active loops

```text
OBSERVE
ITERATE n source scalars                                  [unavoidable]
  EMIT 3 source-anchored atoms                            [literal/fold/class]

BUILD PER VIEW
ITERATE Q certificates                                    [bounded schedule]
  ITERATE C(t+m,m) offset states                          [fixed]
    GATHER m atom integers                                [unroll]
    ITERATE d selected binary rows                        [small/frozen]
      PARITY flattened atom lanes                         [word-batched]
    SET one coupled cell                                  [idempotent]

QUERY PER VIEW
ITERATE Q certificates                                    [bounded]
  COMPUTE one coupled cell                                [d row parities]
  LOOK UP one posting                                     [Q lookups/view]
INTERSECT smallest postings first                         [short-circuit]
VERIFY exact records                                      [authoritative]
```

Fold observation can expand one scalar to at most 33 UTF-8 bytes internally,
but it still emits one fixed 270-bit atom lane and does not add a string-length
loop to certificate construction.

## Support accounting

For each object and view:

```text
dense mask bits  = Q k^d,
posting entries  = sum_c |pi(S_c)|,
query lookups    = Q,
bit energy       = sum_c Hamming(M_c(x),M_c(x')).
```

Low posting count is not automatically good: the failed folded low-bit layout
used roughly one occupied cell per certificate because it erased content. A
resource comparison must condition posting work on private/SNR and candidate
quality gates.

## Confidence

- **High:** the three filename views are canonical and source-anchored under
  their exact research descriptor.
- **High:** the mutation-flow decomposition and three zero-response causes are
  exact.
- **High:** low-bit prefixing of composite atom codes is rejected.
- **Medium:** protected Jacobian-row compilation can economically preserve the
  required filename mutations.
- **Low:** the current views and 16-cell scale will win retrieval on disjoint
  real filename distributions.

## Immediate unresolved objects

T-TYPED-EDIT-SEAM-1 and T-SYMMETRIC-CERTIFICATE-1 now close the semantic shape
of the first two items: same-length rewrites are joint secants and use occupied-
cell posting unions; insertion/deletion are cross-length spans. They also retain
least exact-history obstructions, so neither path is promoted to exact hashing.

The remaining objects are:

1. Protected filename mutation graph and disjoint generator families.
2. Obstruction-selected row compiler over the exact filename atom lanes.
3. Concrete posting codec bytes, physical reads, updates, and verification work.
4. Bounded exact-length query plan and length-band rebuild behavior.
5. Held-out ranking/candidate evidence against engine controls.
