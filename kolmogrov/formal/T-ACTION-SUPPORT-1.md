# T-ACTION-SUPPORT-1: position-resolved edits force literal binary support

Status: **HYPOTHESIS with complete paper proof and finite E6 support; independent
review absent**.

## Controlled substitution statement

Let `S_n={0,1}^n`, observe only Hamming weight `w(x)`, and declare one
deterministic action `F_i` that flips position `i` for every `i`. Any
observation-preserving congruence for all `F_i` is literal equality. Exact closed
support therefore requires `2^n` states.

## Proof

If `x` and `y` are equivalent, observation preservation gives `w(x)=w(y)`.
Congruence gives `F_i x` equivalent to `F_i y`, hence
`w(F_i x)=w(F_i y)`. But

```text
w(F_i x)-w(x)=1-2x_i.
```

The corresponding differences for `x` and `y` are equal, so `x_i=y_i`. This
holds for every position, hence `x=y`. QED.

## Averaged substitution statement

Replace the position-labelled controls by one kernel that chooses a uniformly
random position and flips it. The weight partition with `n+1` blocks is strongly
lumpable and is the coarsest observation-preserving support. From weight `c`:

```text
c -> c+1 with probability (n-c)/n,
c -> c-1 with probability c/n.
```

### Proof

The transition probabilities depend only on `c`, so equal-weight states have
equal mass into every weight block. Because every valid support must refine the
weight observation, this lumpable partition is coarsest. QED.

## Controlled deletion statement

On all binary strings of lengths zero through `N`, observe `(length,weight)` and
declare a position-labelled deletion `D_i` for `0<=i<N`, acting as identity
when `i` is invalid for the current string. Any observation-preserving
deterministic congruence is literal equality and needs

```text
sum_(n=0)^N 2^n = 2^(N+1)-1
```

states.

### Proof

Equivalent strings have equal length and weight. Their `D_i` images must have
equal weight, while

```text
w(D_i x)=w(x)-x_i.
```

Thus every bit agrees. QED.

## Averaged deletion statement

One kernel choosing a uniformly random valid position is lumpable by
`(length,weight)`. At length `n>0`, weight `c`:

```text
(n,c) -> (n-1,c)   with probability (n-c)/n,
(n,c) -> (n-1,c-1) with probability c/n.
```

The minimum observation-preserving support through length `N` has

```text
sum_(n=0)^N(n+1)=(N+1)(N+2)/2
```

states.

## Consequence

Compression capacity depends on the control resolution. A perceptual retrieval
state should not be required to predict position-labelled exact mutation
dynamics unless that power is truly part of its contract; doing so recreates
identity support. Exact anchors may handle identity while perceptual support
models averaged or equivalence-class deformation flow.
