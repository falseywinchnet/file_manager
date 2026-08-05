# T-CYBERNETIC-SUPPORT-1: coarsest observation-preserving closed support

Status: **HYPOTHESIS with complete finite-state paper proof; approximate/SNR
relaxation and independent review absent**.

## Finite flow contract

Let `S` be a finite state set, `o:S->O` a required observation, and for each
declared control action `a`, let `P_a(s,t)` be a transition kernel on `S`.

A partition `Pi` is observation-preserving when each block has one observation.
It is strongly lumpable for the actions when, for any `s,s'` in one block, any
action `a`, and any block `C`:

```text
sum_(t in C) P_a(s,t) = sum_(t in C) P_a(s',t).
```

Then the compressed block state has exact closed dynamics.

## Refinement construction

Start with the observation partition `Pi_0`. Given `Pi_r`, split states by the
signature consisting of:

1. their current block;
2. for every action, their transition mass into every `Pi_r` block.

Call the result `Pi_(r+1)`.

## Statement

The sequence stabilizes after finitely many rounds. Its fixed point `Pi_*` is
the unique coarsest observation-preserving partition strongly lumpable for all
declared actions. Therefore `|Pi_*|` is the exact minimum state support for the
declared observation/control flow.

## Proof

Each round only splits blocks. A strict split increases block count, which is at
most `|S|`, so the sequence stabilizes. At a fixed point, equal-block states
have equal transition mass to every block by construction, hence lumpability.

Let `Lambda` be any observation-preserving lumpable partition. It refines
`Pi_0`. Inductively assume it refines `Pi_r`. Every `Pi_r` block is a union of
`Lambda` blocks. Lumpability gives equal transition mass from two
`Lambda`-equivalent states into each `Lambda` block, hence into every such union.
They therefore share the `Pi_(r+1)` refinement signature. Thus `Lambda` refines
every `Pi_r` and the fixed point. All valid partitions are finer than `Pi_*`,
which is therefore uniquely coarsest and has minimum block count. QED.

## Deterministic specialization

For deterministic actions `T_a`, the condition becomes

```text
s equivalent s'  implies  T_a(s) equivalent T_a(s').
```

The support equivalence is a congruence of the controlled transition system.

## Linear closure and breakdown

Let `C` aggregate fine state distributions into partition blocks. Lumpability is
equivalent to existence of coarse transition operators `Pbar_a` satisfying

```text
C P_a = Pbar_a C.
```

For a candidate non-lumpable support, the commutation residual
`C P_a-Pbar_a C` is its dynamic breakdown. An approximate theory must choose a
norm, state distribution, task loss, and SNR gate before bounding that residual.

## Boundary

Minimum support is relative to the observation and control alphabet. Adding
action labels can strictly increase it; averaging or hiding controls can reduce
it while changing the question the compressed state answers.
