# T-HISTORY-QUOTIENT-1: safe history quotienting separates linear and idempotent support

Status: **HYPOTHESIS with complete paper proofs for the run quotient and the
multiplicative obstruction; finite binary quotient atlas through length ten**.

## Three distinct quotients

For a source `x` and radius `t`, let `R` be a `t`-position deletion history and
let `D_x(R)=x\R`. There are three different equivalence questions:

1. **position history** — retain every set `R`;
2. **run-count history** — retain only how many deletions occurred in each
   constant-symbol run;
3. **outcome history** — identify every `R,R'` with `D_x(R)=D_x(R')`.

They cannot be substituted silently. The run-count quotient is locally
factorable and safe. The full outcome quotient is smaller but is generally
incompatible with an exact cancellative multiplicative phase law.

## Safe run-count quotient

Partition `x` into `s` maximal constant-symbol runs with lengths
`ell_0,...,ell_(s-1)`. Define

```text
q_run(R)_i = |R intersect run_i|.
```

Equal run-count vectors produce equal descendants: deleting any `d_i`
positions from a constant run leaves exactly `ell_i-d_i` copies of its symbol.
Concatenating those residual runs gives the same sequence, including any merges
caused by a fully deleted intervening run.

The number of radius-`t` run classes is

```text
[z^t] product_i (1+z+...+z^(ell_i)),
```

instead of `C(n,t)` position histories. This quotient may give no reduction on
an alternating source, where every run has length one.

### Factorable exact run code

Regard a run-count vector as the multiset containing run index `i` exactly
`d_i` times. For prime `p>max(s-1,t)`, its first `t` power sums identify that
multiset by the Newton-recursion proof in T-HISTORY-MOMENTS-1. A character has
the position-factorized form

```text
chi_a(R)
  = product_(r in R)
      omega^(sum_j a_j run(r)^j).
```

All positions in one run share a phase. The exact run-history identity cost is
therefore controlled by run count rather than source length on repetitive
objects.

Phase equality alone does not remove multiplicity: ordinary position expansion
still contributes `C(ell_i,d_i)` copies of a run-count class. Exact quotient
amplitude uses one branch per deletion count:

```text
product_i sum_(d=0)^min(ell_i,T)
  lambda_i^d s^d M_(symbol_i)^(ell_i-d),
```

where `M_c` is the retain/select transfer matrix for one surviving symbol `c`.
This is the block recurrence implemented by
`run_quotient_deformation_ladder`. It counts each run vector once, retains
degree/radius flow, and never enumerates position histories.

## Full outcome quotient obstruction

Suppose an invertible multiplicative phase law assigns position factors
`lambda_r`. In additive exponent notation, history code is

```text
h(R) = sum_(r in R) h_r.
```

Requiring equal code for every pair of histories with the same descendant can
force two different descendants to collide.

The least radius-two witness is

```text
x = aaba.
```

The histories `{0,2}`, `{1,2}`, and `{2,3}` all produce `aa`. Outcome constancy
therefore gives

```text
h_0+h_2 = h_1+h_2 = h_2+h_3,
```

so cancellation forces `h_0=h_1=h_3`. But `{0,1}` produces `ba` and `{0,3}`
produces `ab`, while

```text
h_0+h_1 = h_0+h_3.
```

The distinct outcomes are necessarily aliased.

For every `t>=2`, the same obstruction is realized by `x=a^t b a`. Histories
that delete `b` and retain any two of the `t+1` copies of `a` all produce `aa`;
outcome constancy forces every `a` position factor equal. Deleting the first
`t` copies of `a` produces `ba`, while deleting the last `a` and all but one
leading `a` produces `ab`; both histories then have the same product phase.

Thus no exact full-outcome quotient exists universally inside a cancellative
per-position product phase family from radius two onward. Adding more
characters cannot repair this algebraic collision.

## Idempotent observable quotient

Retrieval membership does not need additive multiplicity. For a scheduled set
`J` of `m` target positions, store the set of patterns produced on `J` by any
radius-`t` history. Histories producing the same pattern set one bit once:

```text
mask_(x,J)[pattern] = OR_R [D_x(R)|_J = pattern].
```

T-DELETION-CLOSURE-1 reduces construction to `C(t+m,m)` monotone offset states.
This quotient is exact for the declared observable and naturally indexable:
the query encodes one pattern key per scheduled `J`, retrieves its posting, and
intersects postings before exact verification.

This Boolean/idempotent projection is not the additive deformation jet. The
two have complementary roles:

- additive run/history addresses retain graded energy, multiplicity, and fuzzy
  contribution;
- idempotent certificates eliminate history multiplicity for candidate
  membership.

## Boundary

Run-count classes can still duplicate a complete outcome when fully deleted
runs cause alternate survivor explanations. The block recurrence deliberately
retains those classes because the full quotient cannot be made universally safe
inside the factorized phase law. Certificate masks are exact only for their
scheduled positions and explicit alphabet; hashing or quantizing their pattern
axis reintroduces collisions that need an equal-support retrieval measurement.
