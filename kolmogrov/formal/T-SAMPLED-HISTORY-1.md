# T-SAMPLED-HISTORY-1: coherent signal and incompatible-history interference

Status: **HYPOTHESIS with complete conditional paper proof; phase set,
support-count bound, and quantizer uncertainty uninstantiated**.

## Selected character kernel

Use the history code group `H=F_p^t` from T-HISTORY-COHERENCE-1, but retain only
a nonempty character-frequency set `A subseteq H`. Define

```text
K_A(d) = (1/|A|) sum_(a in A) chi_a(d),
mu_A = max_(d != 0) |K_A(d)|.
```

`K_A(0)=1`. The sidelobe `mu_A` measures the worst phase coherence between two
different deletion histories under the selected address family.

For exact evidence atoms `f,g`, retain their selected responses `M_a(f)` and
define the pair score

```text
C_A(f,g)
  = (1/|A|) sum_(a in A) M_a(f) conjugate(M_a(g)).
```

## Exact signal/interference decomposition

Let `c=|S_x(f) intersect S_x(g)|`. Then

```text
C_A(f,g)
  = c
    + sum_(R in S_x(f), R' in S_x(g), R != R')
        K_A(c_p(R)-c_p(R')).
```

Consequently,

```text
|C_A(f,g)-c|
  <= mu_A (|S_x(f)| |S_x(g)|-c).
```

### Proof

Expand the character responses as in T-HISTORY-COHERENCE-1. Equal histories
have code difference zero and each contribute `K_A(0)=1`; there are exactly
`c`. Every unequal pair contributes the displayed sidelobe. Apply the triangle
inequality. QED.

The diagonal `c` is the coherent true-history signal. The remaining terms are
the incompatible-history interference. This decomposition is exact for the
chosen phase family; it makes no independence or random-cancellation claim.

## A finite-support separation certificate

Assume every protected atom has at most `L` supporting histories. A pair with no
common history satisfies

```text
|C_A(f,g)| <= mu_A L^2.
```

A pair with at least one common history satisfies

```text
Re C_A(f,g) >= 1-mu_A(L^2-1).
```

Therefore one threshold separates all such protected zero-versus-positive
pair intersections whenever

```text
mu_A < 1/(2L^2-1).
```

If the stored/quantized score has worst-case additive uncertainty `epsilon`, a
sufficient certified condition is

```text
1-mu_A(2L^2-1) > 2 epsilon.
```

This is a theorem-bounded support/breakdown law for the candidate hash. More
competing histories demand a lower-sidelobe phase family or more uncertainty
budget; once the inequality fails, the selected addresses cannot certify the
protected pair contract through this statistic.

## Boundary

Pairwise common history does not imply one history common to an arbitrary
feature family. Higher-degree occurrence atoms bind several query positions
before hashing, while higher-order zero-sum character products provide the
exact oracle control. Any release claim must state which combination order is
protected.

The theorem does not assert that a fixed `|A|` attains the required `mu_A`, that
the support-count bound `L` is small, or that the score wins retrieval. Those
are the next construction-specific uncertainty gates.
