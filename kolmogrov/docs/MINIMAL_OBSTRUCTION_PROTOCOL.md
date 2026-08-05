# Minimal-obstruction protocol

Status: **GIVEN research method from the architect, 2026-08-05**.

## Rule

Tests and searches may locate a failure, but their volume is not evidence. An
advancement record retains the least-support obstruction available in the
declared model and proves why no smaller support can obstruct the claim.

## Support order

“Least” is always relative to a named model. Report, in order:

1. number of fine states that must be distinguished;
2. active atoms in the signed difference or edit direction;
3. alphabet cardinality and object length;
4. number of axes, cells, and action labels;
5. scalar certificates required to repair the failure.

If two witnesses trade these quantities, they are incomparable; do not hide the
tradeoff in one scalar score.

## Required obstruction record

Every retained obstruction states:

- the universal or sufficiency claim attacked;
- the exact smallest witness;
- the observable values that coincide;
- the required response that differs;
- a lower-bound argument excluding smaller support;
- the least known repair and what contract it changes;
- the restricted theorem that survives.

An enumeration order does not prove minimality. Exhaustion may establish that
no smaller finite object exists, but the record should replace that output with
a direct argument whenever possible.

## Consequence for artifacts

Do not create raw-output, checksum, environment, or pass-count artifacts for a
paper obstruction. A benchmark artifact remains appropriate only when the claim
is intrinsically empirical—for example retrieval quality, latency, or learned
generalization. Even then, retain the smallest explanatory hard negative in
addition to aggregate measurements.
