# T-ENTROPY-SCALE-1: relevant-information flow across addresses and scale

Status: **HYPOTHESIS with complete finite/discrete information-theoretic proof;
task variable and distributions uninstantiated; formal encoding absent**.

## Complexity objects kept separate

- combinatorial complexity: log cardinality of a declared object or quotient
  class;
- distributional complexity: Shannon entropy under a declared distribution;
- individual description complexity: a declared coding/program length for one
  object;
- geometric contribution energy: squared norm of a support response.

No equality among these objects is assumed.

## Relevant variable

Let `Y` be the exact anchor, deformation class, retrieval target, or other
declared fact the hash must retain. Let `H_m` be a discrete scale-`m` hash with
at most `B_m` bits. Then

```text
I(Y;H_m) <= H(H_m) <= B_m.
```

If `H(Y|H_m)<=epsilon`, necessarily

```text
B_m >= H(Y)-epsilon.
```

### Proof

Mutual information is no larger than the entropy of either variable. A
`B_m`-bit value has entropy at most `B_m`. The identity
`I(Y;H_m)=H(Y)-H(Y|H_m)` gives the second bound. QED.

## Exact scale decomposition

Suppose coarse hash `H_m` is a deterministic function of fine hash `H_(m+1)`.
Then

```text
I(Y;H_(m+1))
= I(Y;H_m) + I(Y;H_(m+1)|H_m).
```

The conditional term is the relevant information present at the fine scale and
absent from the coarse scale.

### Proof

The chain rule gives

```text
I(Y;H_m,H_(m+1))
=I(Y;H_m)+I(Y;H_(m+1)|H_m).
```

Because `H_m` is a function of `H_(m+1)`, the pair contains exactly the same
information as `H_(m+1)`, yielding the left side. QED.

Iteration telescopes conditional relevant information across a projective hash
tower. Data processing gives `I(Y;H_m)<=I(Y;H_(m+1))`.

## Independent address decomposition

For addresses `A_1,...,A_q`, a declared ordering gives the exact chain rule

```text
I(Y;A_1,...,A_q)
=sum_j I(Y;A_j | A_1,...,A_(j-1)).
```

Each conditional term is the address's incremental relevant information under
that order. The decomposition is order-dependent; it is not an intrinsic claim
that one address deserves the shared information.

Summing `H(A_j)` or `I(Y;A_j)` generally overcounts redundancy. If `A_2=A_1`,
the joint address contains no more information than `A_1` although the marginal
sum doubles. A coupling certificate is justified by relevant conditional
information it adds beyond the primary marginals, not by raw bit count alone.

## Effective entropic support

`2^H(H_m)` is the perplexity/effective occupied-codeword count under the declared
distribution. It is at most `2^B_m` and can be much smaller when codewords are
unused or highly imbalanced. It does not replace worst-case code cardinality.

## Scale obstruction

T-QUANT-SCALE-1 shows that a coarse sign may not be a function of fine sign bits.
In that case the deterministic Markov chain required above is absent. Both
hashes may still be computed from a common pre-quantized state, but their
information relationship must be evaluated jointly rather than called a
contraction.

## Boundary

Differential entropy of continuous pre-quantized coordinates is not invariant
under reparameterization and is not used as support merely because the support
space is Banach. Geometric response energy and relevant Shannon information
remain separate until a theorem connects a declared distribution, noise model,
and observation channel.
