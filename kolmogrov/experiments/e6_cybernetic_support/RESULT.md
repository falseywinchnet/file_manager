# E6 cybernetic-support census result

Status: **MEASURED by exact finite enumeration; the general count formulas
also have paper proofs**.

Date: 2026-08-05.

## Question

For a fixed coarse observation, how much state support is forced by the action
contract? The census compares position-resolved control with an averaged
deformation channel.

## Replay and artifacts

```sh
cd /Users/quentinkuttenkuler/file_manager/kolmogrov
python3 experiments/e6_cybernetic_support/run.py
```

Raw SHA-256:
`82a5750cd0336facad3378b4325cd2f52fb6988168c642362a6046782587030c`.

## Declared finite systems

- Fixed-length binary strings observed only by Hamming weight. One system has a
  separately controlled flip at every position; the other has one uniformly
  averaged random-flip action.
- Binary strings of every length through `N`, observed by `(length, weight)`.
  One system has separately controlled position deletions; the other has one
  uniformly averaged random-deletion action.

The implementation applies exact rational partition refinement until every
action has block-constant transition probabilities.

## Result

- For fixed lengths one through eight, position-controlled flips refined the
  `n+1` Hamming-weight cells to all `2^n` literal states. For every `n>=2`, one
  split round was sufficient. The averaged-flip system remained exactly at
  `n+1` cells with zero refinement.
- For strings through maximum lengths one through eight, position-controlled
  deletions refined the triangular `(length,weight)` support to all
  `2^(N+1)-1` literal states. For every `N>=2`, one split round was sufficient.
  The averaged-deletion system remained at `(N+1)(N+2)/2` cells with zero
  refinement.
- At `N=8`, this is the contrast between 511 literal deletion-flow states and
  45 averaged deletion-flow states. For fixed length eight, it is 256 literal
  flip-flow states versus nine averaged flip-flow states.

No numerical tolerance enters the result: all transition masses are exact
rational numbers, and all counts are exhaustive over the declared systems.

## Interpretation boundary

The census measures minimum exact Markov support only for these observations
and actions. Averaging the action is a change in the control contract, not a
free compression theorem. No inference is licensed about approximate closure,
semantics, continuous inputs, learned embeddings, or acceptable task loss.
