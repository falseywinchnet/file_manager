# E1 gap-deletion law result

Status: **MEASURED** within the declared finite corpus; companion paper proofs
remain **HYPOTHESIS** pending review.

Date: 2026-08-05.

## Replay and artifacts

```sh
cd /Users/quentinkuttenkuler/file_manager/kolmogrov
python3 experiments/e1_gap_deletion_laws/run.py
```

Raw SHA-256:
`aff5cc3e882b0cdd0865c6ff8e73ccc65ac6bf31079274cd78c8c9504bec4087`.
Profile-stream SHA-256:
`f93f79be723a99aa0f3c34b9e031fcfaa8391e5766aa3af973a4e210bdce8b06`.

## Observations

The exhaustive run covered all 254 nonempty binary sequences through length
seven, every one of their 1,538 single-deletion pairs, and every degree from one
through source length: 9,722 profiles total.

- 71,280 survivor correspondences preserved content and order.
- Every survivor moved by exactly one standard-basis step in gap coordinates.
- 72,818 destroyed occurrences exactly matched the binomial and pattern-mass
  excess laws.
- Source damage fraction was exactly `k/n` in every profile.
- Every nonempty target degree was directionally contained in its source with
  score one.

## Boundary

This checks the implementation against T-GAP-1 and T-DELETE-1 over the finite
corpus. It is not their universal proof, a perceptual-quality result, a compact
representation, or a resource benchmark. Latency and memory are
**UNMEASURED** because the exponential oracle is not a performance candidate.
