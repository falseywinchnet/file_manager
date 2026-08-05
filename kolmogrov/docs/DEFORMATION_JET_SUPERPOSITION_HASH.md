# Deformation-jet superposition hash

Status: **CANDIDATE original construction direction; exact deletion calculus
proved on paper, compact probe quality and novelty unverified**.

## 1. Product goal

Build a better fixed-width perceptual hash for a high-quality local search
engine and, through exact anchored retrieval, future AI memory systems.

“Better” means that at equal serialized support the hash should retrieve more
true structural/deformation neighbors, expose why they matched, and survive a
collision or missing channel through independent evidence. It is not enough to
rename an existing embedding or attach new bounds to a conventional hash.

The demanded invention is a compact observation of a state in which perceptual
deformations have a closed law.

## 2. Concealed state

A terminal hash value is not closed under edits. Its missing state is the
deformation profile of the object's evidence:

```text
evidence atom
  x semantic channel
  x gap/structural position
  x combination degree
  x deformation radius
  x deformation history.
```

For symbolic sequences, the exact atom is a gap-simplex occurrence. Package all
occurrences of pattern `p` and degree `k` into the polynomial

```text
G_(x,k,p)(z) = sum_(I : x_I=p) z^g(I).
```

Content is the pattern label, position is the gap monomial, combination is the
selected occurrence, and scale is the polynomial degree `n-k`.

This is the pre-hash support field. The release object will contain only fixed
probes of it.

## 3. The differential orbit

T-DELETION-ORBIT-1 proves

```text
sum_(|R|=t) G_(x\R,k,p) = D^t G_(x,k,p) / t!,
D = sum_j partial_(z_j).
```

All unlabelled deletion descendants are therefore coefficients of one
translation:

```text
G_(x,k,p)(z+s*1).
```

This is the first construction-specific scale law. Deleting source material is
not modeled as generic noise in an embedding. It is differentiation of the
support object: selected evidence exits through a typed destruction channel;
surviving evidence moves one gap step.

Normalize by the number of source deletion sets:

```text
J_t G = D^t G / n^(underline t).
```

`J_t G` is the exact average degree-`k` support of all radius-`t` deletion
descendants. Under one uniformly averaged deletion, the orbit state shifts:

```text
(J_0,J_1,...,J_T) -> (J_1,J_2,...,J_(T+1)).
```

A scalar base address is nonclosed. A deformation jet is closed through its
declared radius, with `J_(T+1)` as the exact boundary residual.

## 4. Why the unlabelled jet is not enough

Orbit averaging forgets which features came from the same descendant. The
least failure is:

```text
source bab
query  aa.
```

One deletion produces an `a` at query position zero; another produces an `a`
at query position one. No deletion produces `aa`. An unbound marginal can fuse
the two facts into a false candidate explanation.

The better hash must preserve some deformation-history coupling without
storing every descendant.

## 5. History-bound superposition

Give source position `r` a probe-specific phase `lambda_(q,r)`. A deletion set
receives the multiplicative history phase

```text
chi_q(R) = product_(r in R) lambda_(q,r).
```

Every content, position, and combination feature produced by the same history
is multiplied by that same phase. T-HISTORY-COUPLING-1 proves that the projected
all-radius orbit is computable directly from source occurrences:

```text
O_(x,k,p)^lambda(z,s)
  = sum_(I : x_I=p)
      product_j product_(r in gap_j(I)) (z_j+s lambda_r).
```

The coefficient of `s^t` is the phase-bound superposition of every radius-`t`
descendant. Setting all phases to one recovers the unlabelled differential
orbit.

The central candidate mechanism is coherence:

- true query features produced by one edit history carry a common phase term;
- features borrowed from incompatible histories carry different phase terms;
- multiple independently addressable phase probes expose that disagreement;
- exact record verification resolves every surviving candidate.

This is classical superposition over deformation histories, not fusion into one
semantic location.

### A concrete history phase family

For deletion radius `t`, T-HISTORY-MOMENTS-1 encodes a deletion set by its first
`t` positional power sums. Over a prime modulus larger than the source range and
`t`, those moments determine the exact deletion set. Their Fourier characters
factor over positions, so they plug directly into the history product above.

This supplies a structured candidate rather than arbitrary phases:

```text
lambda_(a,r) = omega^(a_1 r + a_2 r^2 + ... + a_t r^t).
```

The exact bounded-range history identity costs approximately `t log_2 n` bits.
Smaller moduli and selected characters deliberately collide, creating multiple
fixed-width satellites whose rescue behavior must be proved or measured. The
number of semantic history axes is controlled by edit radius `t`, while the
modulus controls positional range.

### Exact coherence before compression

For an exact query atom `f`, let `M_a(f)` be the sum of its supporting history
characters. T-HISTORY-COHERENCE-1 proves that full character correlation counts
common histories exactly. For two atoms,

```text
(1/|H|) sum_a M_a(f) conjugate(M_a(g))
  = |S_x(f) intersect S_x(g)|.
```

A zero-sum higher-order character product gives the exact intersection count
for every query atom. Thus the exhaustive construction has a precise retrieval
semantic: positive coherence means that one deletion history supports the
whole query.

The fixed-width hash samples this coherence field. Its signal is the diagonal
common-history contribution; its noise is the sum of incompatible-history
terms that fail to cancel under the retained characters. This—not a generic
embedding distortion—is the central SNR object.

T-SAMPLED-HISTORY-1 makes that SNR exact for pair coherence. If `mu_A` is the
largest off-origin sidelobe of the retained history-character kernel and each
protected atom is supported by at most `L` histories, zero versus positive
common-history support is certified when

```text
1-mu_A(2L^2-1) > 2 epsilon,
```

where `epsilon` is the complete stored-score uncertainty. Breakdown can now be
charged to history multiplicity, phase sidelobes, or quantization separately.

## 6. Fixed-width address candidate

For address channel `q`, freeze:

- a content sign/phase `sigma_q(p)`;
- a gap-position probe `zeta_q`;
- a source-history phase law `lambda_(q,r)`;
- selected occurrence degrees `k`;
- selected radii or radius bands `t`;
- amplitude normalization and quantizer.

The pre-quantized scalar is

```text
a_(q,k,t)(x)
  = sum_p sigma_q(p)
      [s^t] O_(x,k,p)^lambda_q(zeta_q,s).
```

Quantized scalars remain separated by channel, degree, radius, and probe. The
serialized hash is their bounded collection, not a concatenation followed by a
single opaque distance.

Three address roles are immediately distinct:

1. **base addresses** (`t=0`) locate the observed object;
2. **radial jet addresses** (`lambda=1`) expose exact averaged deformation flow
   and scale contraction;
3. **history addresses** (`lambda` nonconstant) retain coupling needed to reject
   incompatible descendant mixtures.

The query computes the same base/channel probes. A record is a candidate when
query evidence appears coherently in its appropriate orbit-radius channels.
The engine then verifies the exact record and reports the contributing channels.

T-STREAMING-JET-1 computes every selected degree/radius coefficient without
enumerating either occurrences or deletion histories. Under factorized probe
weights, one source scan updates retain/delete/select states and emits the full
degree ladder. `DEFORMATION_ALGORITHM_REFINEMENT_001.md` records the active form
and its iteration budget.

## 7. Granular fuzzy scale

Weight jet radius `t` by `rho^t`, with `0<=rho<1`. `rho=0` is the literal base
support; increasing `rho` admits progressively deeper deformation evidence.
T-DELETION-ORBIT-1 bounds the discarded coefficient mass above stored radius
`T` by

```text
||G||_1
  * (rho*(n-k)/n)^(T+1)
  / (1-rho*(n-k)/n).
```

This gives a granular scale with an exact tail charge. Different `rho`, `k`,
and history probes can be independently indexed. A coarse hash is therefore a
declared truncation of the same deformation object, not a separately invented
hash at another width.

## 8. Construction-specific breakdown

The hash fails through named mechanisms:

### Radius truncation

Required history lies beyond `T`. The omitted jet component and fuzzy tail bound
measure this directly.

### History dilution

In a uniform radius-`t` superposition, a feature unique to one deletion history
has raw coefficient weight `1/C(n,t)`. Once normalization and quantization make
that contribution smaller than the address uncertainty, that history cannot be
guaranteed visible. Phase channels are intended to concentrate coherent
multi-feature evidence before this point.

### History aliasing

Different deletion sets receive indistinguishable phase tuples across every
retained history channel. This is a configuration-relative collision, not an
identity merger.

### Semantic cancellation

Content signs or gap probes cancel true evidence. Independent channels and
positive control addresses must measure this failure.

### Missing combination order

Low-degree atoms remain jointly compatible under separate histories but not one
history. The `bab`/`aa` and `aabb`/`ba` obstructions are mandatory controls.

### Quantized jet failure

Pre-quantized deformation response exists but falls inside an uncertified bin
or changes by less than the stored margin. This is the point where an atomic
change has become too small for the configured hash.

No aggregate score may hide which mechanism fired.

## 9. Why this may be qualitatively better

Conventional similarity hashes normally place the observed object. This
candidate also places compact observations of its structured deformation
neighborhood, computed from one canonical support object rather than by
enumerating or learning every neighbor.

Its hoped-for advantages are:

- exact algebraic continuity under deletion before quantization;
- direct support for short-query/long-record containment;
- several independently searchable deformation radii;
- shared history phases binding content and position evidence;
- a construction-derived scale contraction and truncation residual;
- explicit reasons for collision, rescue, and loss;
- exact external verification rather than perceptual identity claims.

These are **HYPOTHESIS** advantages. Only equal-support retrieval against strong
controls can establish “better.”

## 10. Immediate invention gates

E8 closes and redirects the first gate. T-DELETION-CLOSURE-1 proves the exact
semantic degree floor `t+1`. T-HISTORY-QUOTIENT-1 proves that full
identical-descendant quotienting cannot remain a universally separating product
phase from radius two; phase selection alone cannot solve that obstruction.

The next gates are:

1. Compare the two-algebra construction—idempotent degree-`t+1` affine
   certificates plus additive numeric jets—against equal total-bit terminal,
   marginal, and numeric-only controls on rich content.
2. Replace exact binary certificate patterns with independently rescued content
   projections while retaining measured deletion recall and candidate load.
3. Measure run-count quotient prevalence and stability on file-derived feature
   sequences; keep the singleton path as the no-benefit obstruction control.
4. Extend monotone-offset closure and the streaming product law to substitution
   and transposition using typed mutation variables.
5. Choose gap probes whose addresses inherit a projective rule across length;
   charge phase or certificate reindexing failure as residual.
6. Instantiate the certificate coverage bound and numeric atomic-influence bound
   under one declared quantizer/noise policy.

Radius-two unrolling and run-block/singleton specialization are now reference
candidates. Native probe batching and posting layout still wait for the content
projection and supported length bands to stabilize.
