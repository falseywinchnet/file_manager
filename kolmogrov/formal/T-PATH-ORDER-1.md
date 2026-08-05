# T-PATH-ORDER-1: least equal-support path-order obstruction

Status: **HYPOTHESIS with complete finite-Hilbert paper proof; semantic axes
absent**.

## Setup

Let `H=R^2_C tensor R^2_P` with content basis `c0,c1` and position basis
`p0,p1`. Let `P_C,P_P` be orthogonal projections onto the constant vector of
their axis. Two progressive paths have the same two-dimensional intermediate
support and the same four-dimensional final support:

```text
content first:   I_C tensor P_P  -> I_C tensor I_P
position first:  P_C tensor I_P  -> I_C tensor I_P.
```

## Least obstruction

The mass-conserving position direction

```text
v_P = c0 tensor p0 - c0 tensor p1
```

has two active atoms, squared norm two, and

```text
(I_C tensor P_P)v_P = 0,
||(P_C tensor I_P)v_P||^2 = 1.
```

At equal intermediate dimension, content-first loses all of this response while
position-first retains half its energy. For the content direction

```text
v_C = c0 tensor p0 - c1 tensor p0,
```

the preference reverses.

### Proof

`P_P(p0-p1)=0`, while `P_C c0=(c0+c1)/2`, whose squared norm is `1/2`.
Tensor norm multiplicativity gives the first energy statement. Exchange the
axes for the second. QED.

## Minimality

A nonzero mass-conserving edit direction cannot have one active atom, because
its coefficient sum would be nonzero. Two atoms are therefore least support,
and both displayed directions attain it.

## Consequence

Axis contractions commute and the final state is path-independent, but
progressive truncation is not path-independent. No first-axis schedule dominates
both least-support affine directions. A schedule must therefore be evaluated
against the declared direction family and budget, or expose independently
addressable axis details rather than one compulsory prefix.

## Falsification gate

At every serialized support budget, compare schedules using the same final axis
partitions and coefficient count. A universal schedule claim fails when another
path retains strictly more required-direction energy or task margin without
increasing support.
