# T-RESIDUE-1: modular gap satellites and coupling limit

Status: **HYPOTHESIS with partial paper proof and retained finite
counterexamples; independent review and formal encoding absent**.

## Candidate construction

For modulus `m >= 2`, project a gap witness

```text
(pattern, g_0, ..., g_k)
```

to the independently retrievable address

```text
(pattern, g_0 mod m, ..., g_k mod m).
```

The object address is the multiplicity map of all projected occurrences at a
declared degree. Modulus, degree, object scale policy, and feature version are
part of representation identity.

## Local deletion statement

Under the survivor correspondence of T-DELETE-1, every modulus-`m` projected
witness preserves its pattern and changes exactly one residue coordinate by one
step on the cycle `Z/mZ`. Thus its coordinate Hamming distance is one and its
sum of cyclic coordinate distances is one.

### Proof

T-DELETE-1 makes the source gap vector equal to the target vector plus one
standard basis vector. Coordinatewise reduction modulo `m` preserves equality
in every other coordinate. In the changed coordinate, residues differ by `1`
modulo `m`, whose cyclic distance is one for `m >= 2`. QED.

## Bounded joint-address injectivity

If `m > n-k`, no gap coordinate can reach `m`, because the nonnegative gap mass
sums to `n-k`. Reduction modulo `m` is therefore the identity on every degree-`k`
gap coordinate.

For `n >= 2` and `k=2`, the exact multiset of `(pattern,gaps)` reconstructs the
sequence: each gap vector reconstructs its exact pair `(i,j)` by T-GAP-1, and
the associated pattern supplies `(x_i,x_j)`. Every position participates in at
least one pair, so every symbol is recovered. Therefore a **joint** modulus-`m`
degree-2 address is injective on length `n` whenever `m > n-2`.

This is a bounded theorem, not a reason to choose large moduli: address cost,
collision fanout, stability, and usefulness across unequal lengths remain
unmeasured.

## Why separate coprime marginals are not a CRT proof

Suppose mod-2 and mod-3 projected occurrence multisets are stored as independent
addresses. The Chinese remainder theorem could reconstruct a gap coordinate
only if the mod-2 and mod-3 residues belonging to the same occurrence remained
coupled. Aggregation retains two marginals and discards that coupling. No
per-occurrence CRT conclusion follows.

E2 finds the minimal binary collision through its declared search:

```text
aaabbabbaaa
bbaaaaaaabb
```

at length 11. Their mod-2 and mod-3 degree-2 address multisets are each equal,
but their joint mod-6 address multisets differ. This rejects the candidate claim
that coprime independent residue marginals are information-equivalent to the
same-size joint residue address.

## Research consequence

Independence and interaction are separate axes. Primary content, position, and
combination evidence should remain addressable, but a bounded set of explicit
interaction certificates may be necessary to preserve which measurements
co-occurred. The open problem is to find the minimum coupling sufficient for
collision rescue without collapsing the whole representation back into one
fused location.
