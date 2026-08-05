# T-UNIVERSAL-SUPPORT-1: minimum support shared by several task observations

Status: **HYPOTHESIS with complete finite paper proof; concrete modalities
absent**.

## Statement

Let `S` be a finite exhaustive state set. Each task or modality `j` requires an
observation

```text
o_j : S -> O_j.
```

A shared representation `h:S->H` is sufficient when every observation factors
through it: `o_j=d_j composed with h` for some decoder `d_j`.

Define joint observational equivalence by

```text
s ~* t  iff  o_j(s)=o_j(t) for every j.
```

Then the `~*` partition is the unique coarsest shared sufficient support. Its
block count is `|image(o_1,...,o_r)|`, and a binary exact code needs at least

```text
ceil(log2 |image(o_1,...,o_r)|)
```

bits.

## Proof

If `h(s)=h(t)`, every decoder returns the same `o_j` value, so `s~*t`. Thus each
`h` cell lies inside one `~*` cell and `h` has at least as many states. The joint
observation `(o_1,...,o_r)` is constant exactly on `~*` cells and attains the
bound. The bit bound is cardinality. QED.

## Least incompatibility obstruction

Three fine states are necessary and sufficient for two individually binary
observations to require strictly more than two shared states. Let

```text
S={a,b,c},
o_A cells: {a,b} | {c},
o_B cells: {a,c} | {b}.
```

Each observation alone needs two support states. Their joint observation has
three distinct values and needs three. With at most two fine states, any binary
observation using two values is already literal, so no strict shared-support
increase is possible. The obstruction is least.

On the full product `S=A x B` with coordinate observations, the bound is sharp
at `|A||B|`: independently varying axes multiply universal exact support.

## Consequence

A common contraction law or coordinate grammar can be shared across modalities.
A common partition is economical only when their required distinctions overlap.
Otherwise the minimum universal support is their joint refinement, not the
largest individual support.

This does not reject modality adapters. It rejects treating one semantic route
as support-neutral before the modality observations are declared.

## Falsification gate

For every proposed universal route, exhibit each task observation as a decoder
of every claimed sufficient prefix. If the joint observation has more cells
than the route prefix, the sufficiency claim is impossible independent of
training or indexing.
