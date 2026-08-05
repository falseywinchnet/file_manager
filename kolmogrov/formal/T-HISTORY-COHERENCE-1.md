# T-HISTORY-COHERENCE-1: character coherence exactly counts common histories

Status: **HYPOTHESIS with complete finite paper proof; fixed-width character
sampling and quantization absent**.

## Setup

Fix deletion radius `t`, prime `p>max(n-1,t)`, and the injective history moment
code `c_p(R)` from T-HISTORY-MOMENTS-1. Let

```text
H = F_p^t,
chi_a(c) = omega^(a dot c),    a,c in H,
```

where `omega` is a primitive `p`th root of unity.

For an exact target evidence atom `f`, let `S_x(f)` be the source deletion
histories whose descendants contain it. Define its history-character response

```text
M_a(f) = sum_(R in S_x(f)) chi_a(c_p(R)).
```

Each `M_a` is obtainable from the history-bound orbit in
T-HISTORY-COUPLING-1 by coefficient/content/gap selection.

## Pair coherence

For two atoms,

```text
(1/|H|) sum_(a in H) M_a(f) conjugate(M_a(g))
  = |S_x(f) intersect S_x(g)|.
```

### Proof

Expand the left side. Every pair `(R,R')` contributes

```text
(1/|H|) sum_a chi_a(c_p(R)-c_p(R')).
```

Character orthogonality makes this one when the codes agree and zero otherwise.
The history code is injective, so agreement means `R=R'`. The surviving pairs
are exactly the common histories. QED.

## Multi-atom coherence

For atoms `f_1,...,f_m`, define

```text
C(f_1,...,f_m)
  = (1/|H|^(m-1))
    sum_(a_1,...,a_(m-1) in H)
      product_(i=1)^(m-1) M_(a_i)(f_i)
      * M_(-sum_i a_i)(f_m).
```

Then

```text
C(f_1,...,f_m)
  = |intersection_(i=1)^m S_x(f_i)|.
```

### Proof

Expand over histories `R_i in S_x(f_i)`. For each `i<m`, summation over `a_i`
contains the character of `c_p(R_i)-c_p(R_m)` and yields one exactly when those
codes agree. All independent sums therefore survive precisely when every
history code equals the last. Injectivity makes all histories identical. Each
common history contributes one. QED.

## Consequence for exact deletion containment

Use the degree-one exact positional atoms `f_i(y)` of query `y` from
T-HISTORY-COUPLING-1. Then

```text
y is a t-deletion descendant of x
  iff C(f_0(y),...,f_(|y|-1)(y)) > 0.
```

The `bab`/`aa` obstruction has positive unlabelled response for each query atom
but coherence zero. A true descendant has coherence at least one. This is an
exact oracle statistic for common deformation history.

## Compact-hash boundary

The full formula is not the intended release representation: it uses the
complete history-character family and an order equal to the number of query
atoms. The fixed-width candidate retains a small, versioned selection of
characters and low-order coherence products, combined with higher-degree
occurrence atoms that bind several query positions before projection.

A true common history contributes a phase-consistent diagonal term. Tuples of
incompatible histories form the interference term. The next theorem must bound
or directly obstruct that interference for the selected deterministic phase
family, support budget, and query degree. No generic independence assumption is
available.
