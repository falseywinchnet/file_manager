# Support, scale, Jacobian, and contribution-energy program

Status: **CANDIDATE mathematical program; no hash width, norm, noise floor, or
production representation selected**.

## 1. Notation and the question being closed

The architect's scale phrase is provisionally read as

```text
K_m = b^m,              K_(m+1) = b^(m+1),  b >= 2,
```

where `K_m` is coordinate support at scale `m`. The literal alternative
`K_m+1` is recorded but does not create a uniform block contraction; it requires
a different tower and remains unresolved.

Use `a` for alphabet cardinality, `B_m` for serialized bits, and `K_m` for
pre-quantized coordinate dimension. These are not interchangeable.

For object `x`, let

```text
Phi_m(x) in X_m              pre-quantized support state,
h_m(x) = Q_m(Phi_m(x))       quantized hash,
A_m : X_(m+1) -> X_m         scale contraction.
```

The desired scale law is a projective consistency condition

```text
Phi_m(x) = A_m Phi_(m+1)(x),
```

or a bounded residual when exact commutation is impossible. Quantized hashes do
not inherit this law automatically.

The block contraction below is a mathematical control, not the claimed
innovation. Its role is to make scale loss exact. The research contribution, if
earned, must lie in the structured `Phi_m`, its channel interactions, its
support bounds, and its collision-rescue law—not in renaming ordinary averaging.

## 2. Seven meanings of support

“Minimum support” is underdetermined until the support object is named.

| Support object | Definition | Question it answers |
|---|---|---|
| ambient support | `K_m = dim(X_m)` | How many coordinates exist? |
| active support | `#{j : Phi_m(x)_j != 0}` | How many coordinates are occupied by this object? |
| effective energy support | `(sum e_j)^2 / sum e_j^2`, `e_j=|Phi_j|^2` | Across how many coordinates is energy materially spread? |
| affine-direction support | `rank(J_m)` or stable rank | How many locally independent perturbation directions survive? |
| code support | `B_m` serialized bits and at most `2^B_m` codewords | How many exact/fuzzy classes can be named? |
| influence support | changed coordinates/bits per required atomic edit | Is one edit detectably represented? |
| scale support | smallest `m` passing energy-tail and SNR gates | At what granularity is the object adequately represented? |

No theorem may substitute one row for another without an explicit bridge.

## 3. Structured affine directions

The object domain is discrete, so begin with a declared edit family
`E(x)={e_1,...,e_p}`. The finite-edit secant operator is the matrix whose columns
are

```text
Delta_i Phi_m(x) = Phi_m(e_i x) - Phi_m(x).
```

If a relaxed measure/object space is later defined and `Phi_m` is
differentiable, its Fréchet derivative may replace these columns locally. The
binary map `h_m` itself has no useful classical Jacobian: threshold maps are
locally constant away from boundaries and discontinuous on them.

Tag every output coordinate by exactly one primary channel, interaction family,
and scale. Content-position crosses belong to an interaction subspace rather
than being counted once as content and again as position. With disjoint Hilbert
coordinates,

```text
E_i = ||Delta_i Phi_m||_2^2
    = sum_(channel c, scale r) ||P_(c,r) Delta_i Phi_m||_2^2.
```

For a direction subspace `V`, local preservation asks for frame bounds

```text
alpha ||v|| <= ||J_m v|| <= beta ||v||,  v in V.
```

The lower singular value `alpha` is separation; `beta` is sensitivity; their
ratio is anisotropy. Rank alone says only that directions are nonzero, not that
they survive noise.

Cross-channel coherence is separately visible through off-diagonal Gram blocks
`J_c^* J_d`. Large coherence means the channels respond to the same direction;
it is neither independent rescue nor automatically harmful.

For a projectively consistent tower, finite-edit Jacobians commute with scale:

```text
J_m(x) = A_m J_(m+1)(x).
```

In the Hilbert specialization every direction `v` has exact flow

```text
||J_(m+1)v||^2 = ||J_m v||^2 + ||D_m J_(m+1)v||^2.
```

Given a detectability threshold `tau`, define directional support
`d_m(tau)` as the number of singular values of `J_m` at least `tau`. This is a
thresholded, theorem-bounded support measure. A nonzero but sub-threshold
singular direction does not count as supported merely to improve rank. In the
exact Hilbert tower, every ordered singular value and therefore `d_m(tau)` is
nondecreasing with refinement; an approximate tower must charge any violation
to its projective-residual response.

## 4. Banach contraction tower

Partition `X_(m+1)` into `K_m` blocks of `b` coordinates. For `1 <= p < infinity`
define

```text
(A_m z)_j = b^(1/p-1) sum_(r=0)^(b-1) z_(bj+r).
```

For `p=infinity`, use the ordinary block average. Hölder's inequality gives
`||A_m z||_p <= ||z||_p`. Repeating each coarse coordinate with factor
`b^(-1/p)` defines an isometric refinement `R_m` with `A_m R_m=I`.

At `p=2`, `R_m=A_m^*`. The projection `P_m=R_m A_m` is the orthogonal projection
onto block-constant fine states and

```text
D_m z = (I-P_m)z,
||z||_2^2 = ||A_m z||_2^2 + ||D_m z||_2^2.
```

For an exact projective tower this telescopes:

```text
||Phi_M(x)||^2 = ||Phi_m(x)||^2
               + sum_(r=m)^(M-1) ||D_r Phi_(r+1)(x)||^2.
```

The same identity applies to every finite-edit response. This yields exact
coarse contribution energy and exact discarded detail energy rather than an
informal notion of granularity.

Define scale breakdown

```text
beta_r(x) = ||D_r Phi_(r+1)(x)||^2 / ||Phi_(r+1)(x)||^2
```

when the denominator is nonzero. A representation may stop refining only under
a declared tail-energy gate and a declared atomic-influence gate. Neither gate
implies the other.

## 5. Minimum-support lower bounds

### Exact cardinality

A `B`-bit code has at most `2^B` values. Exact injectivity on all length-`n`
strings over an alphabet of size `a` requires

```text
B >= n log_2(a).
```

Supporting every length through `n` requires

```text
2^B >= sum_(r=0)^n a^r = (a^(n+1)-1)/(a-1).
```

This is an identity bound, not a perceptual-hash objective.

### Fuzzy reconstruction

If a `B`-bit code and decoder must reconstruct every length-`n` string within
q-ary Hamming error `t`, each decoded representative covers at most

```text
V_a(n,t) = sum_(i=0)^t C(n,i)(a-1)^i
```

inputs. Therefore

```text
B >= n log_2(a) - log_2 V_a(n,t).
```

This lower bound requires a decoder and a worst-case distortion promise. It
does not apply merely because a retrieval hash puts similar objects nearby.

### Local affine rank

If `J_m` has a positive lower frame bound on a `d`-dimensional direction
subspace, then `rank(J_m) >= d`, hence `K_m >= d`. Effective dimension cannot be
hidden by a clever basis.

### Contribution-energy/SNR support

For `p` required atomic edits define

```text
I_i = ||Delta_i Phi_m(x)||^2,
E_total = sum_i I_i.
```

Then `min_i I_i <= E_total/p`. If reliable detection requires
`I_i >= Gamma N_m`, where `N_m` is the declared noise energy and `Gamma` the
required SNR, a necessary condition is

```text
p <= E_total/(Gamma N_m).
```

For one required replacement at each of `n` positions, `p=n`. If every one of
`a-1` alternatives must be detected, `p=n(a-1)`. This is the rigorous form of
“the content has become too large when one-letter influence falls below the
noise floor.” It depends on a real total-response bound and noise model; neither
may be invented after seeing the result.

### Combined necessary envelope

If scale `m` has `K_m` coordinates, `q_m` serialized bits per coordinate, and
`B_m=q_m K_m` total bits, then a system demanding exact length-`n` identity,
detectability of all `a-1` replacements at each position, and preservation of a
`d`-dimensional affine direction space must simultaneously satisfy

```text
B_m >= n log_2(a),
K_m >= d,
n(a-1) <= E_total/(Gamma N_m).
```

For fuzzy decoding, replace the first inequality by the Hamming-covering bound.
For perceptual candidate retrieval without a decoder, the first inequality is
not available; a task-specific separation/recall condition must replace it.

## 6. Binary influence and the smoothness floor

Normalize a `B`-bit sign hash as

```text
h(x) in {-1/sqrt(B), +1/sqrt(B)}^B.
```

If an edit flips `r` bits,

```text
||h(e x)-h(x)||_2^2 = 4r/B.
```

The smallest nonzero squared response is `4/B`. Thus:

- if an upper smoothness requirement is strictly below `4/B`, every allowed
  atomic edit must leave the binary hash unchanged;
- if reliable detection requires energy `Gamma N`, at least
  `ceil(B Gamma N/4)` bits must flip;
- if total flip incidence over `p` required edits is bounded by `R`, at least
  one edit flips at most `R/p` bits.

Unnormalized Hamming distance has a different noise scale; “one bit is too
small” has no invariant meaning until normalization and noise are stated.

## 7. Quantization does not automatically commute with scale

Even when `Phi_m=A_m Phi_(m+1)` exactly, generally

```text
Q_m A_m Phi_(m+1) != C_m Q_(m+1) Phi_(m+1)
```

for any contraction `C_m` that sees only fine sign bits. Two fine vectors can
have identical signs but opposite coarse block-sum signs, for example

```text
(100,-99)  -> positive sum,
(1,-100)   -> negative sum.
```

Both expose the same fine sign pair `(+,-)`. Therefore a coarse sign cannot be
reconstructed from fine signs alone over an unrestricted amplitude domain.

A fuzzy contraction theorem requires margin information. If a fine state
approximation has `l_infinity` error at most `epsilon`, the block-average
contraction also has error at most `epsilon`. A coarse sign is certified when
its approximate value has magnitude greater than `epsilon`. Otherwise the
coarse bit is epistemically unresolved and must be recomputed or accompanied by
additional amplitude/margin support.

## 8. Scale selection and capacity stop rule

For a required edit family `E_req(x)`, define

```text
SNR_min(m,x) = min_(e in E_req(x))
               ||Phi_m(e x)-Phi_m(x)||^2 / N_m.
```

Define the supported-content boundary at scale `m` as the largest declared
object size `n` for which the worst-case required edit remains above `Gamma`
and the discarded detail tail remains below `epsilon`.

This is deliberately a boundary definition, not a claim that support is
monotone in `m`. Refinement can increase signal, noise, cancellation, and
quantization fragility differently. Monotonicity itself is a theorem target.

## 9. Epistemic lower boundaries

- `B` alone cannot determine perceptual support length without a separation or
  reconstruction requirement.
- A Jacobian rank cannot establish detectable influence without a lower
  singular value and noise model.
- Total output norm does not automatically bound total edit-Jacobian energy.
- Binary bit influence is metric- and normalization-dependent.
- Fine-to-coarse coefficient contraction does not imply fine-to-coarse binary
  hash contraction.
- Zero collisions in a finite corpus does not prove injectivity.
- Intended invariances must be removed from the required edit set before using
  a minimum-influence gate.
- Combinatorial log cardinality, Shannon entropy, individual description
  complexity, and geometric contribution energy are different ledgers.
- Summed marginal address entropy double-counts redundancy; incremental relevant
  information is conditional and depends on the declared target variable.

These are not rhetorical caveats. Each prevents a false theorem.

## 10. Entropic cybernetic flow

Let `Y` name the fact to preserve: exact anchor, allowed-deformation class,
retrieval target, or another declared variable. A `B_m`-bit scale hash obeys

```text
I(Y;H_m) <= H(H_m) <= B_m.
```

If coarse hash is a deterministic contraction of fine hash, then

```text
I(Y;H_(m+1))
= I(Y;H_m) + I(Y;H_(m+1)|H_m).
```

The conditional term is exact relevant-information breakdown across scale. For
multiple addresses, use conditional mutual information in an explicit order;
never sum marginal entropies as though redundant evidence were independent.

This entropic decomposition and the Hilbert detail-energy decomposition answer
different questions. A future observation/noise theorem may relate them, but no
such bridge is currently claimed.

## 11. Next theorem targets

1. Prove exact Banach contraction and Hilbert energy decomposition for the
   scale tower (`T-SCALE-1`).
2. Separate exact-cardinality, fuzzy-decoding, affine-rank, and SNR bounds
   (`T-SUPPORT-1`, `T-INFLUENCE-1`).
3. Prove sign-only scale commutation impossible and state a margin-qualified
   fuzzy replacement (`T-QUANT-SCALE-1`).
4. Define the actual Kolmogrov pre-quantized `Phi_m` over gap-simplex witnesses
   so projective residual and Jacobian spectra can be measured.
5. Seek a scale law for content, position, combination, and interaction blocks
   separately before any fused norm is admitted.
6. Measure `d_m(tau)`, projective residual response, and channel Gram coherence
   across the scale tower before claiming that fine directions survive coarse
   compression.
7. Instantiate a relevant variable `Y` and corpus distribution, then measure
   conditional information contributed by each address and each refinement
   scale without confusing it with geometric energy.
8. Build the semantic cylinder route in `ALGORITHMIC_SUPPORT_BASIS.md` and test
   whether coarse-plus-detail support retains the declared gap-simplex
   directions before quantization.
