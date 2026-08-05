# E3 edit-factorization result

Status: **MEASURED** within the declared finite corpus; companion theorem
objects remain **HYPOTHESIS** pending review.

Date: 2026-08-05.

## Replay and artifacts

```sh
cd /Users/quentinkuttenkuler/file_manager/kolmogrov
python3 experiments/e3_edit_factorization/run.py
```

Raw SHA-256:
`c710f601339d0746bbcc2e3347fea1fe6b66fba803ba594d1250aa4306cb70bb`.
Profile-stream SHA-256:
`ac5ffc5eafabd3c8642be072b03df64ab193573636e6389c5eda10e713a82b07`.

## Observations

Across all 126 nonempty binary sequences through length six, the run checked:

- 30,306 deletion-set/degree profiles over every nonempty deletion set;
- 3,450 genuine substitution/degree profiles;
- 1,404 distinct-symbol adjacent-transposition/degree profiles.

Every deletion-set survivor moved gap `L1` distance equal to the number of
deletions, with nonnegative gap deltas summing to that count. Every substitution
changed exactly `k/n` of degree-`k` occurrences and moved no gap. Every genuine
adjacent transposition matched the exact neither/one/both incidence partition
and moved no gap.

## Boundary

These are finite implementation checks of T-MULTIDEL-1 and T-EDIT-1. Mixed
edits, perceptual weights, unequal-length distance, compact projection, and
retrieval behavior remain unresolved. Performance distributions are
**UNMEASURED** for the exponential reference path.
