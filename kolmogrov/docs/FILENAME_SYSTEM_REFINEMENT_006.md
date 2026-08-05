# Filename system refinement 006: address roles, schedule support, typed evidence, and liveness

Status: **CANDIDATE research system with several impossibility boundaries
closed; production projection and relevance remain unselected**.

## Issues closed this round

| Issue | Result |
|---|---|
| Guard/coverage composition | Separate addresses retain recall but can mix witnesses; only a coupled cell proves a common projected witness. |
| Equal-support accounting | Equal dense mask cells do not imply equal postings, dictionaries, or query predicates. |
| Projection versus certificate support | A 64-cell coupled projection barely improved E12's p95 tail; certificate/history support remains limiting. |
| Certificate count | Two and four certificates were much too loose; eight first passed tuning and then failed the evaluation tail threshold. |
| Posting-union iteration | Universal one-lookup precombination needs `2^B-1` predicates; keep explicit cell ORs or a declared restricted mask language. |
| Hard negatives | Exact typed evidence identifies genuine digit/extension neighbors, but no universal veto survives intended boundary edits. |
| Deletion/update semantics | Immutable postings plus a segment liveness bitmap are exact; dead membership reclamation belongs to compaction. |

## Corrected address architecture

The current honest experimental organization is:

```text
coverage address
  owns candidate generation and recall

protected guard
  audits named deformation response
  may supply post-candidate evidence
  does not silently become a second required witness

coupled jet/certificate evidence
  admitted only when common history/provenance is retained

exact filename record
  owns edit verification and typed policy evidence
```

E12 rejects storing a separate guard merely because it is mathematically
available. At equal dense cells, split guard/coverage doubled posting payload
and admitted provenance-mixed candidates. The guard earns stored support only
if a later rank or failure-detection gate demonstrates value beyond the
coverage control.

## Where the remaining false candidates live

Projection width is no longer the only plausible bottleneck. On E12 evaluation:

```text
coverage4                    16 cells -> p95 16 candidates
split guard3/coverage3       16 cells -> p95 12 candidates
coupled guard3/coverage3     64 cells -> p95 12 candidates
```

The coupled control removes split-witness provenance but does not materially
move the displayed tail. The residual is consistent with:

- incompatible histories across different certificates;
- omitted affine target-position directions;
- coarse structural view collisions;
- genuine radius-one ambiguity and typed relevance distinctions.

The next support should attack those objects directly. Adding cell bits without
naming which residual they close is no longer justified.

## Certificate specialization

For the tuning family, increasing `Q` from two to eight changed p95 candidates
from 40 to 12 while posting payload rose from 17.28 to 61.40 bytes/record. Four
certificates were nearly as expensive as half the final layout but retained p95
32 candidates.

This suggests the useful schedule is not a smooth scalar width. Certificates
are discrete positional directions, and some late directions carry most of the
selectivity. The next compiler should select certificate subsets against
incompatible-history obstructions and report marginal candidate reduction per
posting byte. Coverage-first affine order is only a control.

No certificate count is frozen: the selected eight regressed from p95 12 on
tuning to 16 on evaluation.

## Fastest honest query form

T-OCCUPANCY-QUERY-1 prevents a universal constant-lookup exact solution. The
active fast form is therefore:

```text
PER SEGMENT
  READ cardinality metadata                              [small directory]
  ORDER certificate predicates by estimated union size  [no posting payload]
  FOR each required certificate/view
    READ selected sparse lists or one contiguous bit-sliced block
    OR occupied cells wordwise                           [explicit |q| loop]
    AND running candidate bitmap                         [early zero exit]
  AND segment liveness bitmap                            [one exact filter]

ACROSS SEGMENTS
  UNION live candidates
  VERIFY one edit in one filename scan
  ATTACH typed filename evidence
```

Possible batching is now specific rather than aspirational:

- batch cell bitmap ORs across machine words;
- batch certificates only when their list blocks are contiguous;
- stop after an empty candidate bitmap;
- do not materialize descendants, edit matrices, or all query-mask unions;
- apply liveness once per segment after candidate construction unless dead-list
  reads measure as the bottleneck.

## Typed evidence boundary

`classify_filename_edit` now returns exact edit kind plus fold equality,
structural equality, digit involvement, extension location, and boundary
involvement. E12's genuine verified digit/extension decoys were all exposed by
this evidence.

But intended separator deletions were exposed by the same broad
`policy_sensitive` flag. Kolmogrov can deliver the evidence; File Manager must
declare which transformations are positive, negative, or context-dependent.
Until that policy exists, the evidence may explain candidates but may not veto
them in the production contract.

## Immutable liveness

For segment-local ordinal set `[N]`, candidate expression `C` and live set `V`,
return `C intersection V`. This commutes with every posting union/intersection
and exactly removes deleted ordinals.

E12's 288-record control used 36 bitmap bytes and filtered 29 deleted records
without a posting rebuild. It left 2,409 dead memberships. A practical segment
ledger must therefore record:

```text
live bitmap bytes,
dead membership fraction,
dead posting bytes read/query,
new-segment update bytes,
compaction rewrite bytes and stall,
generation/continuation invalidation.
```

The first item is formal and measured synthetically. The rest remain native
engine measurements.

## Confidence

- **High:** guard/coverage provenance law, precombination lower bound, typed
  evidence sufficiency boundary, liveness exactness, and iteration shape.
- **Medium on generated data:** balanced coverage is a stable control; eight
  certificates and four projection bits provide a workable experimental point.
- **Low:** either value transfers to real filenames, or the affine certificate
  order is near optimal.
- **Very low / rejected:** union protection or equal-dense-support split guards
  automatically improve the system.

## Next support target

The next mathematically justified target is a **certificate-direction
compiler** coupled to common-history coherence:

1. identify the smallest incompatible-history witnesses surviving the current
   eight directions;
2. price each candidate certificate by marginal witness separation, posting
   memberships, and query cells;
3. keep the numeric deformation jet as a separate graded address;
4. select on development+tuning and attack once on a fourth generator;
5. stop adding directions when marginal verified false-candidate energy falls
   below its posting/update charge.

That attacks the residual E12 located. Another row-only round would not.
