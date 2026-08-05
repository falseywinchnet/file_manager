# T-SUPPORT-1: exact, fuzzy, and affine minimum-support bounds

Status: **HYPOTHESIS with complete elementary proofs; independent review and
formal encoding absent**.

## Exact code cardinality

An injective `B`-bit encoding of all length-`n` strings over an alphabet of size
`a >= 2` requires

```text
B >= n log_2(a).
```

For all lengths zero through `n`, it requires

```text
2^B >= (a^(n+1)-1)/(a-1).
```

### Proof

There are `a^n` objects of exact length `n` and at most `2^B` codewords. For the
cumulative domain, sum the geometric series. Injectivity requires at least as
many codewords as objects. QED.

## Worst-case fuzzy decoding

Suppose a `B`-bit encoder and decoder reconstruct every length-`n` input within
q-ary Hamming distance `t`. Let

```text
V_a(n,t)=sum_(i=0)^t C(n,i)(a-1)^i.
```

Then

```text
B >= n log_2(a)-log_2 V_a(n,t).
```

### Proof

For each codeword, the decoder emits one representative. At most `V_a(n,t)`
inputs lie within distance `t` of that representative. The union of all
`2^B` decoded balls must cover `a^n` inputs, so
`2^B V_a(n,t) >= a^n`. Take logarithms. QED.

This is a covering lower bound; overlaps only make coverage less efficient.

## Affine direction support

Let a linear or derivative operator `J:V->R^K` obey

```text
||Jv|| >= alpha ||v||
```

for every `v` in a `d`-dimensional subspace `V`, with `alpha>0`. Then `K>=d`.

### Proof

The lower bound makes `J|V` injective, hence rank at least `d`. A map into
`R^K` has rank at most `K`. QED.

## Boundary

The three results answer different questions. Exact cardinality does not bound
perceptual retrieval, fuzzy decoding requires an actual decoder and distortion
promise, and affine rank requires a relaxed or finite-secant direction model.
