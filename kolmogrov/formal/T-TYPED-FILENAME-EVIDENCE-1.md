# T-TYPED-FILENAME-EVIDENCE-1: exact post-candidate policy evidence

Status: **HYPOTHESIS with complete sufficiency boundary; relevance ordering is
not selected**.

## Evidence object

After T-ONE-EDIT-VERIFIER-1 accepts a filename pair, retain:

```text
edit kind and exact changed source positions,
fold-view equality,
structural-view equality,
whether an affected scalar is a digit,
whether it lies after the last dot,
whether it is a dot/separator boundary.
```

`classify_filename_edit` derives this object from the exact candidate record
and query. It adds no stored identity and no projection collision.

## Sufficiency and impossibility boundary

Any hard-negative policy that is a deterministic function of this evidence can
be applied exactly after candidate generation. For example, a declared policy
may distinguish a basename letter transposition from a digit change or an
extension change even though all are radius-one edit neighbors.

Conversely, if two query/record pairs have identical evidence but the desired
labels differ, no classifier using only this evidence can separate them. One
additional observation that differs between the pairs is necessary. This is
the least epistemic boundary: relevance cannot be recovered from a state that
identifies the two cases.

## Cost and role

The evidence scan is fused with exact one-edit verification and inspects at most
the filename once, plus last-dot location. It may reorder or reject verified
candidates under an explicit product policy, but it cannot reduce postings
already read to produce them.

Hash depth therefore cannot solve every hard decoy. E11's digit/version and
extension siblings survived six-bit refinement because they are legitimate
one-edit neighbors. Their distinction belongs to typed exact evidence and rank
policy, not extra collision support alone.

## Boundary

Kolmogrov records the evidence and its provenance. It does not decide that
digits, extensions, or boundaries are universally dissimilar; that is a File
Manager relevance decision requiring owner-approved labels and evaluation.
