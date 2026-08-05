# E4 gap-orthant containment result

Status: **MEASURED** on the declared finite domain; orthant matching remains a
**CANDIDATE** retrieval channel.

Date: 2026-08-05.

## Replay and artifacts

```sh
cd /Users/quentinkuttenkuler/file_manager/kolmogrov
python3 experiments/e4_orthant_containment/run.py
```

Raw SHA-256:
`e3c0d784ee3d4259f2c658262f88df9426b106e5fdc715300666c35fcad50897`.
Pair-stream SHA-256:
`8fd9d42d69f00d89f8ca13d7e4ce90ae79c780420a64395ca4f354b6e8375f75`.

## Observations

The corpus contains every nonempty binary query/record pair with record length
at most six and query length no greater than record length, evaluated at every
degree through query length.

- Every exact ordered-subsequence query scored one at every degree.
- Full-query degree agreed exactly with literal ordered-subsequence control.
- Orthant matching never exceeded pattern-only occurrence containment.
- At degree one, gap compatibility reduced false positives from 2,520 to 154.
- At degree two, it reduced false positives from 550 to four.
- No degree-three orthant false positive appeared within this finite bound,
  although pattern-only containment still had 66. This is bounded evidence, not
  an injectivity theorem.

The first retained low-degree failures are:

```text
degree 1: query ba,  record aabb
degree 2: query baa, record aabbab
```

Each query witness can be matched to some locally compatible record witness,
but the matches do not assemble into one global query embedding.

## Boundary

This is exact maximum matching over uncompressed occurrences. It does not
measure perceptual relevance, approximate matching, cross-channel weights,
serialized size, latency, or index behavior. Full-degree exactness is a control,
not a proposal to enumerate full-query subsequences in production.
