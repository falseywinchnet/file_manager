# T-HISTORY-SCALE-1: exact projective contraction of history cells

Status: **PROVED for the declared uniform interval cell map**.

For mixed fingerprint residue `0<=y<P`, define

```text
c_B(y) = floor(y B / P).
```

Let `B_f=r B_c` for positive integer `r`. Then

```text
c_Bc(y) = floor(c_Bf(y) / r).
```

The identity follows from integer interval nesting: the `r` consecutive fine
intervals with indices `rj,...,rj+r-1` partition coarse interval `j`.

For occupancy set `A_B(x)`, therefore,

```text
A_Bc(x) = { floor(j/r) : j in A_Bf(x) }.
```

For a coupled tuple, apply the quotient componentwise. Consequently:

- a fine history hash answers any declared coarser scale without rereading the
  content or recomputing polynomial fingerprints;
- exact declared-relation recall is invariant at every scale;
- the coarse candidate set contains the fine candidate set;
- contraction commutes with occupancy union and duplicate elimination.

This is an exact projective tower of finite quotient addresses. It is not a
Banach fixed-point contraction: the quotient is non-invertible and no complete
metric with a strict global contraction constant has been declared. The exact
useful claim is nested quotient compatibility.

The tower permits granular operation: start with a cheap coarse key, refine a
broad partition by reading more bits of the same coordinates, and preserve
deterministic compatibility across index generations. Fine keys derive coarse
keys; coarse keys cannot reconstruct fine keys.
