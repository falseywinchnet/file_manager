# T-SEGMENT-LIVENESS-1: exact deletion over immutable candidate postings

Status: **HYPOTHESIS with complete set proof and payload bound; durability and
compaction policy absent**.

## Segment object

For one immutable segment with local record universe `[N]`, let every
certificate address have posting set `L_a subseteq [N]`. Let

```text
V subseteq [N]
```

be the live-record set after deletions or superseding updates. Candidate
production first computes any exact Boolean expression `C` over posting sets
and returns

```text
C intersection V.
```

## Exactness and commutation

Set intersection distributes over posting intersections and unions:

```text
(A intersection B) intersection V
  = (A intersection V) intersection (B intersection V),

(A union B) intersection V
  = (A intersection V) union (B intersection V).
```

Thus liveness may be applied once after a segment candidate is built or pushed
into every posting without changing the live result. A deleted ordinal cannot
survive; a live ordinal accepted by the original candidate expression cannot be
removed by liveness.

## Support and update law

An exact dense liveness bitmap costs `N` bits plus framing. Deleting a record
changes one live bit. An update can append the new exact record to a later
segment and clear the old ordinal's live bit; no historical similarity posting
must be rewritten for semantic correctness.

Queries union live candidates across segments and exactly verify their external
record anchors. Compaction may later rebuild postings from live records, but it
is a resource policy rather than a correctness requirement.

## Breakdown

The no-rewrite property does not bound space amplification: obsolete postings
remain until compaction. With `D` deleted/superseded ordinals, at most the sum
of their old logical memberships is dead posting mass, while the liveness
bitmap remains `N` bits. Candidate work before the final live AND may still read
dead postings.

## Boundary

This theorem supplies exact deletion semantics for a future immutable segment
experiment. It does not select segment size, generation authority, crash
recovery, bitmap codec, compaction threshold, or filesystem persistence.
