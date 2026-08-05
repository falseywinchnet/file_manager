# E9 result: rich projection needs coupled cells and obstruction-selected scale

Status: **MEASURED on an exhaustive four-symbol, length-seven, radius-two
certificate domain; File Manager feature quality and engine cost unmeasured**.

## Domain and control

E9 used every one of the `4^7=16,384` sources and `4^5=1,024` queries. The
complete ten-certificate degree-three schedule and exact 64-cell pattern mask
gave exactly 211 source candidates for every query. It is the deletion-semantic
control, not a proposed payload.

Every projected layout retained all exact-control candidates. That follows from
membership projection and is not an empirical quality claim.

## Equal 16-bit layouts

All rows below use 16 bits per certificate and 160 dense payload bits per
source.

| Projection | Precision | Mean candidates | Maximum | Invisible pattern mass | Certificates with invisible mass |
|---|---:|---:|---:|---:|---:|
| forward mixed-radix prefix | 0.3273 | 644.69 | 1,048 | 67.92% | 93.75% |
| one balanced affine cell | 0.8182 | 257.90 | 704 | 17.58% | 43.52% |
| private-witness-selected binary quotient | 0.8097 | 260.58 | 421 | 11.99% | 27.64% |
| two independent 8-cell views | 0.5503 | 383.46 | 1,922 | 24.16% | 59.56% |
| four independent 4-cell views | 0.0676 | 3,122.70 | 3,720 | 47.42% | 73.85% |

The selected binary quotient used the four-dimensional row space
`[7,9,18,35]`, whose kernel is `{0,27,45,54}`. It was selected by minimizing
private-witness loss over all 651 four-dimensional binary quotients on this
same finite domain. This is a tuned construction, not held-out evidence.

The support objective and retrieval objective were related but not identical.
Relative to the balanced affine cell, the selected quotient reduced invisible
pattern mass and the worst candidate load while giving up 0.0084 precision.
They must remain separate selection terms.

## Refinement curves

The obstruction-selected binary tower refined monotonically:

```text
cells/certificate   precision   mean candidates   maximum   invisible mass
16                  0.8097      260.58            421       11.99%
32                  0.9849      214.23            229        4.08%
64                  1.0000      211.00            211        0.00%
```

The full six-row map is invertible, so 64 cells close the finite pattern
universe exactly. The 32-cell prefix was only 3.23 candidates/query above the
exact control, but 10.38% of source certificates still contained at least one
pattern with no private bit. Near-exact average retrieval therefore does not
certify atomic contribution support.

By contrast, the affine code `[1,3,5]` plateaued:

```text
cells/certificate   precision   invisible mass
32                  0.9748      10.45%
64                  0.9748      10.45%
```

Patterns `(0,2,0)` and `(1,0,1)` both have integer code six. More depth cannot
separate a collision in the underlying affine map. The breakdown is structural,
not quantization pressure.

## Least obstructions retained

1. **Slot provenance:** source patterns `{(0,0),(1,1)}` make independent slot
   marginals accept absent query `(0,1)`. Two slots and two source patterns are
   minimal.
2. **Cross-view provenance:** even when four 4-cell views jointly identify each
   individual degree-three pattern, independent membership accepted absent
   patterns assembled from different source witnesses. This explains the
   0.0676 precision row.
3. **Fine injectivity:** the first untuned six-row binary map was exact at 64
   cells but only 0.2955 precise at its 16-cell prefix. Fine exactness does not
   choose a useful progressive order.
4. **Permanent affine kernel:** `(0,2,0)` and `(1,0,1)` are the least retained
   collision for the `[1,3,5]` integer code once modular wrap is gone.

## Inference

The active rich-content candidate should use one **coupled whole-pattern cell
per certificate view**, not independent slot marginals. Multiple narrow views
may still be useful as candidate evidence, but their membership intersections
do not preserve witness provenance and cannot be described as exact rescue.

Projection depth should be selected by a joint objective containing at least:

```text
candidate load and tail
+ invisible pattern mass
+ protected-edge separation
+ serialized support
```

A full-rank fine map is only a migration endpoint. Its prefix row order must be
compiled against protected obstructions and then evaluated on a disjoint
workload. The E9-selected rows are not eligible for production because the
selection and measurement domain are the same.

## Remaining low-confidence boundary

- actual filename/path/content atom streams and their Unicode versions;
- held-out stability of obstruction-selected rows;
- lifting pattern-private influence to a tight per-source-symbol Jacobian;
- mixed insertion, substitution, and transposition closure;
- posting bytes and query/update latency for dense versus sparse cells;
- whether 32–64 cells per certificate remain economical after length-band and
  positional coverage costs are included.
