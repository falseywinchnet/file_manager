# T-MULTIDEL-1: exact deletion-set witness law

Status: **HYPOTHESIS with complete paper proof; independent review and formal
encoding absent**.

## Statement

Let `x` have length `n`, let `R` be a set of `t` deleted indices with
`1 <= t <= n`, and let `y` be the order-preserving sequence on the remaining
indices. For degree `k` in `[1,n]`:

- `C(n-t,k)` occurrence witnesses survive;
- `C(n,k)-C(n-t,k)` are destroyed;
- every survivor preserves pattern and order;
- every source-minus-target survivor gap vector is nonnegative with coordinate
  sum `t`, so gap `L1` distance is exactly `t`;
- target pattern multiplicity is coordinatewise contained in source
  multiplicity;
- destroyed source-mass fraction is
  `1 - C(n-t,k)/C(n,k)`.

When `k <= n-t`, directional occurrence-mass containment of `y` in `x` is one.

## Proof

A degree-`k` source occurrence survives exactly when every selected index lies
in the `n-t` retained positions, yielding `C(n-t,k)` choices. All others are
destroyed. Order preservation maps each target occurrence bijectively to one
surviving source occurrence with the same symbols.

Relative to a fixed survivor, every deleted index lies in exactly one of its
`k+1` unselected gap regions. If `d_j` deleted indices lie in region `j`, the
source gap is the target gap plus `d_j`. Thus every coordinate difference is
nonnegative and the sum, hence `L1`, is `sum(d_j)=t`.

The survivor injection proves coordinatewise pattern containment. Summing the
coordinate differences gives the total-count difference
`C(n,k)-C(n-t,k)`, and division by `C(n,k)` gives the damage fraction. QED.

## Interpretation

Degree is an exact stability/discrimination dial. Low degrees lose a smaller
fraction under deletion; high degrees discard more witnesses but carry richer
combination evidence. No weight between degrees is selected by this theorem.
