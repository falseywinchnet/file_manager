# T-EDIT-1: substitution and adjacent-transposition factorization

Status: **HYPOTHESIS with complete paper proof; independent review and formal
encoding absent**.

## One genuine substitution

Let `x` have length `n`, and replace the symbol at index `r` with a different
symbol. At degree `k`:

- exactly `C(n-1,k-1)` witnesses change pattern;
- exactly `C(n-1,k)` retain pattern;
- every changed witness has pattern Hamming distance one;
- every gap vector is unchanged;
- the changed fraction is `k/n`.

### Proof

A witness changes exactly when it selects `r`; choose its other `k-1` indices
from `n-1`. Every such witness contains the replaced position exactly once, so
one pattern coordinate changes. Witnesses avoiding `r` are unchanged. Object
positions do not move, so all gap vectors are identical. The binomial ratio is
`C(n-1,k-1)/C(n,k)=k/n`. QED.

## One genuine adjacent transposition

Let distinct symbols at adjacent indices `r,r+1` be swapped. At degree `k`:

- `C(n-2,k)` witnesses select neither and remain unchanged;
- `2*C(n-2,k-1)` select exactly one and have pattern Hamming distance one;
- `C(n-2,k-2)` select both and have pattern Hamming distance two;
- every gap vector is unchanged.

Out-of-range binomial terms are defined as zero.

### Proof

Partition occurrences by selecting neither, exactly one, or both transposed
indices. The counts follow by choosing remaining indices from the other `n-2`
positions. Distinctness makes the selected symbol change at each selected
transposed position. The index tuple itself is fixed, so its gap vector is
fixed. QED.

## Consequence

Deletion, substitution, and transposition occupy visibly different channel
signatures:

- deletion destroys witnesses and transports surviving position gaps;
- substitution changes content on an exact incidence fraction without moving
  position;
- adjacent transposition changes one- or two-symbol ordered patterns without
  moving position.

This factorization is a candidate basis for deformation-aware distance reports;
it is not yet a selected distance or perceptual weighting.
